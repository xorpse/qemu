# Overview

Qemu tracer - a tracer based on [qemu](https://github.com/qemu/qemu)
project. It executes a binary executable and saves trace data using
[Protocol Buffer](https://developers.google.com/protocol-buffers/)
format. The contents of the trace data is defined in
[bap-traces](https://github.com/BinaryAnalysisPlatform/bap-traces)
project.

# Installing released binaries

If you don't want to mess with the source and building, then you can just 
dowload a tarball with prebuilt binaries. Look at the latest release and 
it might happen, that we have built binaries for your linux distribution,
if it is not the case, then create an issue, and we will build it for you.

Let's pretend, that you're using Ubuntu Trusty, and install it. First 
download it with your favorite downloader:

```
wget https://github.com/BinaryAnalysisPlatform/qemu/releases/download/v2.0.0-tracewrap-alpha/qemu-tracewrap-ubuntu-14.04.4-LTS.tgz
```

Install it in the specified prefix with a command like `tar -C <prefix> -xf qemu-tracewrap-ubuntu-14.04.4-LTS.tgz`, e.g., 
to install in your home directory:
```
tar -C $HOME -xf qemu-tracewrap-ubuntu-14.04.4-LTS.tgz
```



# Build

## Preparation

Note: the instructions assume that you're using Ubuntu, but it
may work on other systems, that uses apt-get.

Before building the qemu-tracewrap, you need to install the following packages:
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

## Building

Download [bap-traces](https://github.com/BinaryAnalysisPlatform/bap-traces) with
following command

```bash
$ git clone https://github.com/BinaryAnalysisPlatform/bap-traces.git
```

Download qemu tracer with following command

```bash
$ git clone git@github.com:BinaryAnalysisPlatform/qemu.git -b tracewrap
```

Change folder to qemu and build tracer with command
```bash
$ ./configure --prefix=$HOME --with-tracewrap=`realpath ../bap-frames` \
--extra-ldflags=-Lprotobuf --target-list="arm-linux-user i386-linux-user \
x86_64-linux-user mips-linux-user"
$ make -C protobuf
$ make
$ make install
```

# Usage

To run executable `exec` and to save the trace data to `exec.trace`, use

```bash
$ qemu-arm -tracefile exec.trace exec # trace ARM target executable
$ qemu-i386 -tracefile exec.trace exec # trace X86 target executable
$ qemu-x86_64 -tracefile exec.trace exec # trace X86-64 target executable
$ qemu-mips -tracefile exec.trace exec # trace MIPS target executable
```

Hints: use option -L to set the elf interpreter prefix to 'path'. Use
[fetchlibs.sh](https://raw.githubusercontent.com/BinaryAnalysisPlatform/bap-traces/master/test/fetchlibs.sh)
to download arm and x86 libraries.

# Notes
  Only ARM, X86, X86-64, MIPS targets are supported in this branch.
