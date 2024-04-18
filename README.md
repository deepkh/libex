[![workflow](https://github.com/deepkh/libex/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/deepkh/libex/actions)

## Build collection

This repo provide some scripts and Makefile to build thoses libraries to the following target and arch.
The final build to target `x86_64` and `x86_64-w64-mingw32` is successful on host `Debian 12 bookworm amd64`. 
The final build to target `aarch64` is successful on host `Debian 11 Bullseye aarch64`.

## The reasons for gRPC build materials were removed

I love gRPC functionally, but the tooling is an absolute mess. I mean, the code bases are really a mess to support building on various arch, like mingw-w64 and aarch64, by using the same code base version. The ZeroMq should be an alternative cross-platform, clean option or use debian official built gRpc instead.

## Clone

```bash
git clone --recursive https://github.com/deepkh/libex
```

## Build target linux_x86-64 on `Debian amd64`

```bash
PF=linux source source.sh
make
```

## Build target x86_64-windows on `Debian amd64`

```bash
PF=mingw.linux source source.sh
make
```

## Build target Linux.aarch64 on `Debian aarch64`

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
