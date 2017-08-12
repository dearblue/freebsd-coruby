#ifndef CORUBY_H
#define CORUBY_H 1

#include <sys/types.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/ioccom.h>

MALLOC_DECLARE(M_CORUBY);
MALLOC_DECLARE(M_CORUBY_MRB);

enum {
    CORUBY_MAX_SHORT_CODESIZE = IOCPARM_MASK - sizeof(uint32_t) - sizeof(char[256]),
};

struct coruby_ioc_install_begin
{
    union {
        struct {
            uint32_t flags;
            char name[256];
            int32_t size;
        };
        struct {
            uint32_t status;
            char text[1024];
        };
    };
};

struct coruby_ioc_install_end
{
    union {
        struct {
            uint32_t flags;
            char name[256];
        };
        struct {
            uint32_t status;
            uint32_t context_id;
            char text[1024];
        };
    };
};

struct coruby_ioc_install_short
{
    union {
        struct {
            uint32_t flags; /* error recovery */
            char name[256];
            char code[CORUBY_MAX_SHORT_CODESIZE];
        };
        struct {
            uint32_t status;
            uint32_t context_id;
            char text[1024];
        };
    };
};

struct coruby_ioc_uninstall
{
    union {
        struct {
            uint32_t flags; /* force uninstall, ... */
            uint32_t context_id;
        };
        struct {
            uint32_t status;
            uint32_t context_id__;
            char text[1024];
        };
    };
};

enum {
    CORUBY_IOC_GROUP         = 'r',
    CORUBY_IOC_INSTALL_BEGIN = _IOWR(CORUBY_IOC_GROUP, 1, sizeof(struct coruby_ioc_install_begin)),
    CORUBY_IOC_INSTALL_END   = _IOWR(CORUBY_IOC_GROUP, 2, sizeof(struct coruby_ioc_install_end)),
    CORUBY_IOC_INSTALL_SHORT = _IOWR(CORUBY_IOC_GROUP, 3, IOCPARM_MASK),
    CORUBY_IOC_UNINSTALL     = _IOWR(CORUBY_IOC_GROUP, 4, sizeof(struct coruby_ioc_uninstall)),
    /*
    CORUBY_IOC_LIST         = _IOWR(CORUBY_IOC_GROUP, 3, sizeof(coruby_ioc_list_t)),
    CORUBY_IOC_STATUS       = _IOWR(CORUBY_LOG_GROUP, 4, sizeof(coruby_ioc_status_t)),
    CORUBY_IOC_LOG          = _IOWR(CORUBY_LOG_GROUP, 5, sizeof(coruby_ioc_log_t)),
    */
};

#endif /* CORUBY_H */
