## A C/C++ third party library build collection

This repo can help who would like to build the following library which target for linux_x86-64 and windows_x86-64 on single ubuntu/debian build server. Also is the third party library build scrips for [Netsync Media Server](https://netsync.tv). 

Currently only tested on host of Debian-Stretch with toolchains of `GNU/GCC 6.3.0 20170516` for target platform linux-x86_64 and `GNU/GCC x86_64-w64-mingw32-* 6.3.0 20170516` for target platform windows. But it may working correctly on other ubuntu/debian and toolchain version. 
  
### Cons 
  
  Currently only suppport `Mingw-W64 toolchain` for target platform windows. 

## Build library

* [golang 1.9.1](https://golang.org/)
* [libevent 2.1.8-stable 2017-01-22](https://libevent.org/) (build with SSL support)
* [fdkaac 0.1.3](https://sourceforge.net/projects/opencore-amr/)
* [ffmpeg 2.8.7](https://ffmpeg.org/)
* [libiconv 1.14](https://www.gnu.org/software/libiconv/)
* [libjansoon](https://digip.org/jansson/) (json serialize/unserialize for C)
* [jsoncpp 1.8.4](https://github.com/open-source-parsers/jsoncpp) (json serialize/unserialize for C++)
* [khash.h](https://attractivechaos.github.io/klib/#About)
* [LodePNG 20141130](https://lodev.org/lodepng/)
* [mosquitto 1.5.7](https://mosquitto.org/) (build with mosquitto (broker), mosquitto_pub, mosquitto_sub, libmosquitto.so)
* [openssl 1.1.0f](https://www.openssl.org/)
* [protobuf 3.6.1](https://github.com/protocolbuffers/protobuf) 
* queue.h (OpenBSD: queue.h,v 1.16 2000/09/07 19:47:59, TAILQ_* list header based library) 
* [uchardet](https://github.com/BYVoid/uchardet)

## Clone

```bash
git clone --recursive https://github.com/deepkh/libex
```

## Build target platform linux_x86-64 on Debian/Ubuntu server

```bash
PF=linux source source.sh
make
```

## Build target platform x86_64-windows on Debian/Ubuntu server

```bash
PF=mingw.linux source source.sh
make
```

## Runtime structures

* runtime.[linux64,win64]/
  * bin/
  * include/
  * lib/
  * objs/
    * libevent
    * ...
  * share/
