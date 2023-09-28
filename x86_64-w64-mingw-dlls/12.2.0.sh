#!/bin/bash

install() {
	if [ ! -f "${RUNTIME}/bin/libgcc_s_seh-1.dll" ];then 
		echo ====copy x86_64-w64-mingw dlls ====
    cp -arpf /usr/lib/gcc/x86_64-w64-mingw32/12-posix/libgcc_s_seh-1.dll ${RUNTIME}/bin
    cp -arpf /usr/lib/gcc/x86_64-w64-mingw32/12-posix/libstdc++-6.dll ${RUNTIME}/bin
    cp -arpf /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll ${RUNTIME}/bin
	fi
}

$@
