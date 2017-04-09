# libns3p

libns3p is a third-party lib build scrips for Netsync Media Server (https://netsync.tv). If you like it, you can download it and modify it as you want, but please leave the original copyright holder.

# get libns3p build scripts
```sh
git clone --recursive https://github.com/deepkh/libns3p.git
```

# get Mingw-w64 (32bit) Cross Compiler toolchain build script from Zeranoe

We build libns3p in the Linux-x64 cross-compile environment. So please get the Mingw-w64 (32bit) Cross-Compiler toolchain build script first. Here I was used "MingGW-w64 Build Script 3.6.7" and the build OS is debian-8.5-x64.

Once you got the mingw-w64-3.6.7-i686_x86_64 toolchain in your Linux-x64 environemnt, you must be modify the mk/source.mingw.centos7.sh of "export PATH="/root/toolchain/mingw-w64-3.6.7-i686_x86_64/bin:$PATH" as your own toolchain path.

# build libns3p
```sh
source source.mingw.centos7.sh
make
```

Once the above scripts completed, you got all of necessary lib binary in the runtime/ folder.

