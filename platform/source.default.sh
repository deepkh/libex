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

