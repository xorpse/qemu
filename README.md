# Overview

Qemu tracer - a tracer based on [qemu](https://github.com/qemu/qemu)
project. It executes a binary executable and saves trace data using
[Protocol Buffer](https://developers.google.com/protocol-buffers/)
format. The contents of the trace data is defined in
[bap-traces](https://github.com/BinaryAnalysisPlatform/bap-traces)
project.

# Preparing to build

Note: building instructions assume that you're using Ubuntu, but it
may work on other systems, that uses apt-get.

Before build qemu tracer, you need to install the following packages:
   * qemu build dependencies
   * autoconf, libtool, protobuf-c-compiler
   * [piqi library](http://piqi.org/doc/ocaml)

To install qemu build dependencies, use the following command

```bash
$ sudo apt-get --no-install-recommends -y build-dep qemu
```

To install autoconf, libtool, protobuf-c-compiler, use the
following command

```bash
$ sudo apt-get install autoconf libtool protobuf-c-compiler
```

To install [piqi library](http://piqi.org/doc/ocaml) with
[opam](https://opam.ocaml.org/doc/Install.html), use the following command
```bash
$ opam install piqi
```

# Build process

Download [bap-frames](https://github.com/BinaryAnalysisPlatform/bap-frames) with
following command

```bash
$ git clone https://github.com/BinaryAnalysisPlatform/bap-frames.git
```

Download qemu tracer with following command

```bash
$ git clone git@github.com:BinaryAnalysisPlatform/qemu.git
```

Change folder to qemu and build tracer:
```bash
$ cd qemu
$ ./configure --prefix=$HOME --with-tracewrap=../bap-frames --target-list="`echo {arm,i386,x86_64,mips}-linux-user`"
$ make
$ make install
```

# Usage

To run executable `exec` compiled for `arch`, use `qemu-arch exec` command, e.g.,
`qemu-x86_64 /bin/ls`. It will dump the trace into `ls.frames` file. You can configure
the filename with `-tracefile` option, e.g., `qemu-arm -tracefile arm.ls.frames ls`


Hints: use option -L to set the elf interpreter prefix to 'path'. Use
[fetchlibs.sh](https://raw.githubusercontent.com/BinaryAnalysisPlatform/bap-frames/master/test/fetchlibs.sh)
to download arm and x86 libraries.

# Notes
  Only ARM, X86, X86-64, MIPS targets are supported in this branch.
