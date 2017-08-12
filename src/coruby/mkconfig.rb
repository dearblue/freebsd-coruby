#!ruby

require "optparse"

opt = OptionParser.new
cc = ENV["CC"].dup || "cc"
cflags = ENV["CFLAGS"].dup || ""
cppflags = ENV["CPPFLAGS"].dup || ""
ldflags = ENV["LDFLAGS"].dup || ""
opt.instance_eval do
  on("--cc=cc") { |x| cc = x }
  on("--cflags=cflags") { |x| cflags = x }
  on("--cppflags=cppflags") { |x| cppflags = x }
  on("--ldflags=ldflags") { |x| ldflags = x }

  parse!
end

#cflags.sub!(/(?:(?<=^)|(?<=\s))-mno-sse(?=$|\s)/, "")

DIR = File.dirname __FILE__

File.binwrite("build_config.rb", <<-"BUILD_CONFIG")
require "pp"

MRuby::Build.new do |conf|
  toolchain :clang

  conf.build_dir = "host"
  conf.gem_clone_dir = "host/gems"

  conf.cc.defines << "MRB_INT64=1"
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

  conf.cc.command = #{cc.inspect}
  conf.cc.defines.concat %w(
    MRB_INT64=1
    MRB_DISABLE_STDIO=1)
  conf.cc.flags <<
    #{cppflags.inspect} <<
    #{cflags.inspect}
  conf.cc.flags.concat %w(
    -fvisibility=protected
    -I/usr/src/lib/msun/src
    -std=c11
    -msse
    -Wno-error-undef -Wundef
    -Wno-error-missing-prototypes -Wno-missing-prototypes
    -Wno-error-cast-qual -Wno-cast-qual
    -Wno-error-unused-variable -Wunused-variable
    -Wno-error-return-type -Wreturn-type
    -Wno-error-sometimes-uninitialized -Wsometimes-uninitialized)
  conf.cc.include_paths.concat %W(
    #{DIR}/../colibc
    #{DIR}/../colibm)
  conf.linker.flags << #{ldflags.inspect}

  gem core: "mruby-array-ext"
  gem core: "mruby-class-ext"
  gem core: "mruby-enum-ext"
  gem core: "mruby-enum-lazy"
  gem core: "mruby-enumerator"
  gem core: "mruby-error"
  #gem core: "mruby-eval"
  gem core: "mruby-exit"
  gem core: "mruby-fiber"
  gem core: "mruby-hash-ext"
  gem core: "mruby-inline-struct"
  gem core: "mruby-kernel-ext"
  gem core: "mruby-math"
  gem core: "mruby-numeric-ext"
  gem core: "mruby-object-ext"
  gem core: "mruby-objectspace"
  gem core: "mruby-proc-ext"
  gem core: "mruby-random"
  gem core: "mruby-range-ext"
  gem core: "mruby-sprintf"
  gem core: "mruby-string-ext"
  gem core: "mruby-struct"
  gem core: "mruby-symbol-ext"
  #gem core: "mruby-time"
  gem core: "mruby-toplevel-ext"

  #File.binwrite "libcoruby.conf", conf.pretty_inspect
end
BUILD_CONFIG
