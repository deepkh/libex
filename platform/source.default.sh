# Copyright (c) 2018, Gary Huang, deepkh@gmail.com, https://github.com/deepkh
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
if [ -z "${HAVE_LIB_SYS}" ];then
	export HAVE_LIB_SYS=1
fi

if [ -z "${HAVE_LIB_KLIB}" ];then
	export HAVE_LIB_KLIB=1
fi

if [ -z "${HAVE_LIB_LODEPNG}" ];then
	export HAVE_LIB_LODEPNG=1
fi

if [ -z "${HAVE_LIB_UCHARDET}" ];then
	export HAVE_LIB_UCHARDET=1
fi

if [ -z "${HAVE_BIN_UCHARDET}" ];then
	export HAVE_BIN_UCHARDET=1
fi

if [ -z "${HAVE_LIB_ICONV}" ];then
	export HAVE_LIB_ICONV=1
fi

if [ -z "${HAVE_LIB_OPENSSL}" ];then
	export HAVE_LIB_OPENSSL=1
fi

if [ -z "${HAVE_LIB_EVENT}" ];then
	export HAVE_LIB_EVENT=1
fi

if [ -z "${HAVE_LIB_JANSSON}" ];then
	export HAVE_LIB_JANSSON=1
fi

if [ -z "${HAVE_LIB_X264}" ];then
	export HAVE_LIB_X264=1
fi

if [ -z "${HAVE_LIB_FDKAAC}" ];then
	export HAVE_LIB_FDKAAC=1
fi

if [ -z "${HAVE_LIB_FFMPEG}" ];then
	export HAVE_LIB_FFMPEG=1
fi

if [ -z "${HAVE_GOCOMPILER}" ];then
	export HAVE_GOCOMPILER=1
fi

if [ -z "${HAVE_PROTOBUF}" ];then
	if [ "${CROSS_COMPILE_MODE}" = "1" ];then
		export HAVE_PROTOBUF=1
	fi
fi

if [ -z "${HAVE_LIB_PROTOBUF}" ];then
	export HAVE_LIB_PROTOBUF=1
fi

# out of date
#if [ -z "${HAVE_I686W64MINGW32DLLS}" ];then
#	if [[ "${PLATFORM}" = "mingw" || "${PLATFORM}" = "mingw.linux" ]];then	
#		export HAVE_I686W64MINGW32DLLS=1
#	fi
#fi

# since x86_64-mingw-w64 12.2
if [ -z "${HAVE_X8664W64MINGWDLLS}" ];then
	if [ "${PLATFORM}" = "mingw.linux" ];then	
		export HAVE_X8664W64MINGWDLLS=1
	fi
fi

if [ -z "${HAVE_LIB_MOSQUITTO_ALL}" ];then
	export HAVE_LIB_MOSQUITTO_ALL=1
fi

if [ -z "${HAVE_LIB_MOSQUITTO}" ];then
	export HAVE_LIB_MOSQUITTO=1
fi

if [ -z "${HAVE_BIN_MOSQUITTO_PUB}" ];then
	export HAVE_BIN_MOSQUITTO_PUB=1
fi

if [ -z "${HAVE_BIN_MOSQUITTO_SUB}" ];then
	export HAVE_BIN_MOSQUITTO_SUB=1
fi

if [ -z "${HAVE_BIN_MOSQUITTO}" ];then
	export HAVE_BIN_MOSQUITTO=1
fi

if [ -z "${HAVE_BIN_MOSQUITTO_PASSWD}" ];then
	export HAVE_BIN_MOSQUITTO_PASSWD=1
fi

if [ -z "${HAVE_LIB_JSONCPP}" ];then
	export HAVE_LIB_JSONCPP=1
fi

if [ -z "${HAVE_NSIS}" ];then
	export HAVE_NSIS=1
fi
