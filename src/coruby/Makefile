
.PATH: ${.CURDIR}

BASEDIR != dirname $(MAKEFILE)
KMOD = coruby
KONAME = $(KMOD).ko
LIBMRUBY = ./libcoruby/lib/libmruby.a
LIBMATH = $(BASEDIR)/colibm/libmath.a
LIBS = $(LIBMRUBY) $(LIBMATH)
OBJS = #coruby.o
#CFLAGS = -g -DINVARIANT_SUPPORT -DINVARIANTS
MRUBYDIR := $(BASEDIR)/../../contrib/mruby

COLIBM_SRCS != ls $(BASEDIR)/colibm/*.c
SRCS = $(BASEDIR)/coruby.c $(BASEDIR)/colibc/colibc.c

.include <bsd.kmod.mk>

CFLAGS += -I$(MRUBYDIR)/include -I$(BASEDIR)/colibc -I$(BASEDIR)/colibm
OBJS += $(LIBMRUBY) $(LIBMATH)
LDFLAGS += -L$(BASEDIR)/colibm

#CFLAGS += -fvisibility=hidden
#CFLAGS += -fvisibility=protected
CFLAGS += -Wno-error-dangling-else -Wno-error-unused-variable -Wno-error-missing-braces -Wno-error-missing-prototypes -Wno-error-cast-qual -Wno-error-undef

.if ${MACHINE_CPUARCH} == "amd64"
CPPFLAGS += -DUSE_FPU_KERN
CFLAGS += -msse
.endif

.export CC
.export CPPFLAGS
.export CFLAGS
.export LDFLAGS


clean: clean_colibc clean_colibm clean_mruby

fullclean: clean fullclean_colibc fullclean_colibm

clean_colibc:
	-@make -C ${COLIBCDIR} clean

fullclean_colibc:
	-@make -C ${COLIBCDIR} fullclean

clean_colibm:
	-@rm -f $(COLIBM_SRCS:.c$=.o)

fullclean_colibm:
	-@rm -f $(COLIBM_SRCS:.c$=.o) $(LIBMATH)

clean_mruby:
	-@ruby $(MRUBYDIR)/minirake -f $(MRUBYDIR)/Rakefile MRUBY_CONFIG=build_config.rb INSTALL_DIR=mruby clean

$(KONAME): $(LIBS)

$(MRUBYDIR):
	git submodule init $(MRUBYDIR)
	git submodule update $(MRUBYDIR)

$(COLIBCPATH) $(COLIBMPATH):
	make -C `dirname $@`

$(LIBMRUBY): build_config.rb beforedepend
	ruby $(MRUBYDIR)/minirake -f $(MRUBYDIR)/Rakefile MRUBY_CONFIG=build_config.rb INSTALL_DIR=mruby

build_config.rb: $(BASEDIR)/mkconfig.rb
	ruby $(BASEDIR)/mkconfig.rb

coruby.o: $(MRUBYDIR)

$(LIBMATH): $(COLIBM_SRCS:.c$=.o)
	$(AR) rc $(LIBMATH) $(COLIBM_SRCS:.c$=.o)
