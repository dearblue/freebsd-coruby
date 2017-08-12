# coruby - mruby in kernel for FreeBSD

書き掛けです。

## HOW TO USAGE

``` shell:shell
$ git clone --recurse-submodules https://github.com/dearblue/coruby.git
$ cd coruby
$ make
$ su -c kldload `pwd`/mod/coruby.ko
kldload: unexpected relocation type 23
kldload: unexpected relocation type 23
kldload: unexpected relocation type 23
kldload: unexpected relocation type 23
Hello, world by mruby in FreeBSD kernel!
rand: 48, 0.062462672372922
$
```
