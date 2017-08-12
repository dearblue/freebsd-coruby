/*
 * coruby.c
 *
 * author: dearblue <dearblue@users.noreply.github.com>
 * license: 2-clause BSD License
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/priv.h>
#include <sys/disk.h>
//#include <sys/bus.h>
#include <sys/filio.h>
#include <sys/endian.h>
#include <sys/kthread.h>
#include <sys/proc.h>
#include <sys/sched.h>

#include <machine/bus.h>
#include <machine/vmparam.h>
#include <machine/fpu.h>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/irep.h>

#include "coruby.h"

MALLOC_DEFINE(M_CORUBY, "coruby", "coruby memory storage");
MALLOC_DEFINE(M_CORUBY_MRB, "coruby_mrb", "mruby memory storage in coruby");

typedef struct coruby coruby_t;

struct coruby
{
    mrb_state *mrb;
    const void *code;
    size_t codesize;
    int laststatus;
    struct thread *thread;
    struct fpu_kern_ctx *fpu;
    uint32_t id;
    coruby_t *next;
};

static coruby_t *coruby_list, *coruby_list_end;


static struct cdev *coruby_dev;

static d_ioctl_t coruby_ioctl;

static void *
coruby_start_entry(void *pp)
{
    coruby_t *p = pp;
    mrb_load_irep(p->mrb, p->code);
    return NULL;
}

static void
coruby_start_entry_pre(void *pp)
{
    coruby_t *p = pp;
    void *status;
    fpu_kern_enter(p->thread, p->fpu, FPU_KERN_NORMAL);
    colibc_exittrap(&status, coruby_start_entry, pp);
    mrb_close(p->mrb);
    p->mrb = NULL;
    fpu_kern_leave(p->thread, p->fpu);
    fpu_kern_free_ctx(p->fpu);
    p->fpu = NULL;
    // TODO: その他の終了処理を行う

    // NOTE: free はまだ行わない。
    // NOTE: ログメッセージがあるし、再実行させるかもしれないため、
    // NOTE: ユーティリティコマンドが指示を出すまでおあずけ。
}

#define FAILED_TO(LABEL, ERR, COND) \
    do {                            \
        if (!(COND)) {              \
            err = (ERR);            \
            goto LABEL;             \
        }                           \
    } while (0)                     \

static void *
coruby_allocator(struct mrb_state *mrb, void *addr, size_t size, void *ud)
{
    (void)ud;

    if (addr) {
        if (size > 0) {
            return realloc(addr, size, M_CORUBY_MRB, M_NOWAIT | M_NODUMP);
        } else {
            free(addr, M_CORUBY_MRB);
            return NULL;
        }
    } else {
        if (size > 0) {
            return malloc(size, M_CORUBY_MRB, M_NOWAIT | M_NODUMP);
        } else {
            return NULL;
        }
    }
}

static int
coruby_start(const char name[], const char code[], size_t codesize, int flags)
{
    int err = 0;
    coruby_t *p = malloc(sizeof(coruby_t), M_CORUBY, M_NOWAIT | M_ZERO | M_NODUMP);
    FAILED_TO(failed, ENOMEM, p);

    p->mrb = mrb_open();
    FAILED_TO(failed, ENOMEM, p->mrb);
    p->mrb->allocf = coruby_allocator;

    p->fpu = fpu_kern_alloc_ctx(FPU_KERN_NORMAL);
    FAILED_TO(failed, ENOMEM, p->fpu);

    p->code = code;
    int pages = 0;
    err = kthread_add(coruby_start_entry_pre, p, NULL, &p->thread, 0, pages, "coruby:%s", name);
    FAILED_TO(failed, err, !err);

    // TODO: ロックを掛けるべし
    if (coruby_list) {
        coruby_list_end = coruby_list_end->next = p;
    } else {
        coruby_list_end = coruby_list = p;
    }

    sched_add(p->thread, SRQ_BORING);

    return 0;

failed:
    if (p->fpu) {
        fpu_kern_free_ctx(p->fpu);
        p->fpu = NULL;
    }

    if (p->mrb) {
        mrb_close(p->mrb);
        p->mrb = NULL;
    }

    if (p) {
        free(p, M_CORUBY);
        p = NULL;
    }

    return err;
}

ssize_t
mrbx_check_code(const void *code, size_t maxsize)
{
    if (maxsize < 14) { return -1; }

    /* NOTE: シグネチャやバージョンの確認は mruby に丸投げ */

    size_t codesize = be32dec((const char *)code + 10);
    if (codesize > maxsize) {
        return -1;
    }

    return codesize;
}

static int
coruby_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flags, struct thread *td)
{
    int error = 0;

    switch (cmd) {
    case CORUBY_IOC_INSTALL_BEGIN:
        return ENOIOCTL;
    case CORUBY_IOC_INSTALL_END:
        return ENOIOCTL;
    case CORUBY_IOC_INSTALL_SHORT:
        {
            struct coruby_ioc_install_short *p = (struct coruby_ioc_install_short *)data;
            ssize_t codesize = mrbx_check_code(p->code, sizeof(p->code));
            if (codesize < 0) {
                return EINVAL;
            }
            // TODO: code は malloc してコピーすべし
            coruby_start(p->name, p->code, codesize, p->flags);
            return 0;
        }
    case CORUBY_IOC_UNINSTALL:
        // TODO: 全ての mruby context を停止させて破棄する

        error = ENOIOCTL;
        break;
    case FIONBIO:
        break;
    case FIOASYNC:
        if (*(int *)data != 0) {
            error = EINVAL;
        }
        break;
    default:
        error = ENOIOCTL;
        break;
    }

    return error;
}

static struct cdevsw coruby_cdevsw = {
    .d_version  = D_VERSION,
    .d_name     = "coruby",
    .d_ioctl    = coruby_ioctl,
};

static mrb_value
corb_puts(mrb_state *mrb, mrb_value self)
{
    const char *text;
    mrb_int len;
    mrb_get_args(mrb, "z!", &text, &len);
    printf("%s%s", text, (len > 0 && text[len - 1] == '\n' ? "" : "\n"));
    return mrb_nil_value();
}

static mrb_value
corb_uputs(mrb_state *mrb, mrb_value self)
{
    const char *text;
    mrb_int len;
    mrb_get_args(mrb, "z!", &text, &len);
    uprintf("%s%s", text, (len > 0 && text[len - 1] == '\n' ? "" : "\n"));
    return mrb_nil_value();
}

static void
corb_init(mrb_state *mrb)
{
    mrb_define_method(mrb, mrb->kernel_module, "puts", corb_puts, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, mrb->kernel_module, "uputs", corb_uputs, MRB_ARGS_REQ(1));
    mrb_include_module(mrb, mrb->object_class, mrb->kernel_module);

#if 0
    mFreeBSD = mrb_define_module(mrb, "FreeBSD");
    mrb_define_const(mFreeBSD, "VERSION", ...);
    mrb_define_const(mFreeBSD, "ARCHTECTURE", ...);
    mrb_define_const(mFreeBSD, "HOSTNAME", ...);
    CPU_PACKAGE
    CPU_THREAD
    REAL_MEMORY
    FREE_MEMORY
    AVAIL_MEMORY
#endif
}

static int
coruby_modevent(module_t mod, int type, void *data)
{
    switch(type) {
    case MOD_LOAD:
        if (bootverbose) {
            //printf("coruby: <coruby device>\n");
        }

        {
            const uint8_t __attribute__((aligned(4))) hello[] = {
                0x45,0x54,0x49,0x52,0x30,0x30,0x30,0x34,0x18,0xaa,0x00,0x00,0x00,0xd2,0x4d,0x41,
                0x54,0x5a,0x30,0x30,0x30,0x30,0x49,0x52,0x45,0x50,0x00,0x00,0x00,0xb4,0x30,0x30,
                0x30,0x30,0x00,0x00,0x00,0xac,0x00,0x01,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x10,
                0x06,0x00,0x80,0x00,0x3d,0x00,0x00,0x01,0xa0,0x00,0x80,0x00,0x06,0x00,0x80,0x00,
                0xbd,0x00,0x00,0x01,0x06,0x00,0x80,0x01,0x83,0x31,0x40,0x02,0xa0,0x80,0x80,0x01,
                0x3e,0xc0,0x00,0x01,0x3d,0x01,0x80,0x01,0x3e,0xc0,0x00,0x01,0x06,0x00,0x80,0x01,
                0x20,0x80,0x80,0x01,0x3e,0xc0,0x00,0x01,0xa0,0x40,0x80,0x00,0x4a,0x00,0x00,0x00,
                0x00,0x00,0x00,0x04,0x00,0x00,0x28,0x48,0x65,0x6c,0x6c,0x6f,0x2c,0x20,0x77,0x6f,
                0x72,0x6c,0x64,0x20,0x62,0x79,0x20,0x6d,0x72,0x75,0x62,0x79,0x20,0x69,0x6e,0x20,
                0x46,0x72,0x65,0x65,0x42,0x53,0x44,0x20,0x6b,0x65,0x72,0x6e,0x65,0x6c,0x21,0x00,
                0x00,0x06,0x72,0x61,0x6e,0x64,0x3a,0x20,0x00,0x00,0x02,0x2c,0x20,0x00,0x00,0x00,
                0x00,0x00,0x00,0x03,0x00,0x04,0x70,0x75,0x74,0x73,0x00,0x00,0x05,0x75,0x70,0x75,
                0x74,0x73,0x00,0x00,0x04,0x72,0x61,0x6e,0x64,0x00,0x45,0x4e,0x44,0x00,0x00,0x00,
                0x00,0x08,
            };

            mrb_state *mrb;
            mrb = mrb_open();
            corb_init(mrb);
            mrb_load_irep(mrb, hello);
            mrb_close(mrb);
        }

        coruby_dev = make_dev_credf(MAKEDEV_ETERNAL_KLD, &coruby_cdevsw, 0,
                                    NULL, UID_ROOT, GID_WHEEL, 0660,
                                    "coruby/.master");

        break;
    case MOD_UNLOAD:
        if (coruby_dev) {
            destroy_dev(coruby_dev);
        }
        break;
    case MOD_SHUTDOWN:
        break;
    default:
        return (EOPNOTSUPP);
    }

    return (0);
}

DEV_MODULE(coruby, coruby_modevent, NULL);
MODULE_VERSION(coruby, 1);

#if 0

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/mount.h>

static vfs_mount_t 	corubyfs_mount;
static vfs_root_t	corubyfs_root;
static vfs_statfs_t	corubyfs_statfs;
static vfs_unmount_t	corubyfs_unmount;

static int
nullfs_mount(struct mount *mp)
{


static struct vfsops corubyfs_vfsops = {
    .vfs_mount =    corubyfs_mount,
    .vfs_root =     corubyfs_root,
    .vfs_statfs =   corubyfs_statfs,
    .vfs_unmount =  corubyfs_unmount,
};

VFS_SET(corubyfs_vfsops, corubyfs, VFCF_SYNTHETIC);
#endif
