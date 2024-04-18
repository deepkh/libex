## Build collection

This repo provide some scripts and Makefile to build thoses libraries on different platform and arch.
The final build host for x86_64 and x86_64-w64-mingw32 is on Debian 12 bookworm. 
The final build host for aarch64 is on Debian 11 Bullseye.

## The reasons for gRPC build materials were removed

I love gRPC functionally, but the tooling is an absolute mess. I mean, the code bases are really a mess to support building on various arch, like mingw-w64 and aarch64, by using the same code base version. The ZeroMq should be an alternative cross-platform, clean and beautiful option.

## Clone

```bash
git clone --recursive https://github.com/deepkh/libex
```

## Build target linux_x86-64 on Debian host

```bash
PF=linux source source.sh
make
```

## Build target x86_64-windows on Debian host

```bash
PF=mingw.linux source source.sh
make
```

## Build target Linux.aarch64 on Debian host

```bash
PF=linux.aarch64 source source.sh
make
```

## Runtime folder structures

* runtime.[linux64,win64,aarch64]/
  * bin/
  * include/
  * lib/
  * objs/
    * libevent
    * ...
  * share/
