require "pp"

MRuby::Build.new do |conf|
  toolchain :clang

  conf.build_dir = "host"
  conf.gem_clone_dir = "host/gems"

  conf.cc.flags = "-fPIC -O3 -I/usr/local/include"
  conf.linker.flags = "-L/usr/local/lib"

  #conf.gembox "default"

  #File.binwrite "host.conf", conf.pretty_inspect
end

MRuby::CrossBuild.new("libcoruby") do |conf|
  toolchain :clang

  conf.build_dir = "libcoruby"
  conf.gem_clone_dir = "libcoruby/gems"

  conf.bins = []

  conf.cc.command = "cc"
  conf.cc.defines.concat %w(
    MRB_INT64=1
    MRB_DISABLE_STDIO=1)
  conf.cc.flags <<
    "-DUSE_FPU_KERN" <<
    "-O2 -pipe  -fno-strict-aliasing -Werror -D_KERNEL -DKLD_MODULE -nostdinc   -I. -I/usr/src/sys -fno-common  -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer   -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -msoft-float  -fno-asynchronous-unwind-tables -ffreestanding -fwrapv -fstack-protector -Wall -Wredundant-decls -Wnested-externs -Wstrict-prototypes  -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual  -Wundef -Wno-pointer-sign -D__printf__=__freebsd_kprintf__  -Wmissing-include-dirs -fdiagnostics-show-option  -Wno-unknown-pragmas  -Wno-error-tautological-compare -Wno-error-empty-body  -Wno-error-parentheses-equality -Wno-error-unused-function  -Wno-error-pointer-sign -Wno-error-shift-negative-value  -mno-aes -mno-avx  -std=iso9899:1999 -I./../../contrib/mruby/include -I./colibc -I./colibm -msse"
  conf.cc.flags.concat %w(
    -fvisibility=hidden
    -I/usr/src/lib/msun/src
    -std=c11
    -msse
    -Wno-error-missing-prototypes -Wno-missing-prototypes
    -Wno-error-cast-qual -Wno-cast-qual
    -Wno-error-unused-variable -Wunused-variable
    -Wno-error-return-type -Wreturn-type
    -Wno-error-sometimes-uninitialized -Wsometimes-uninitialized)
  conf.cc.include_paths.concat %W(
    ./../colibc
    ./../colibm)
  conf.linker.flags << " -d -warn-common"

  #File.binwrite "libcoruby.conf", conf.pretty_inspect
end
