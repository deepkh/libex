/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#ifndef _LIBX264_H_
#define _LIBX264_H_
#ifdef __MINGW32__
#include <windows.h>
#define EXPORTS __declspec(dllexport)
#define MINGWAPI __cdecl
#else
#define EXPORTS
#define MINGWAPI
#endif
#include <stdint.h>

#ifdef  __cplusplus
extern  "C" {
#endif 

typedef struct {
	int64_t *in_pts;
	int64_t *in_dts;
	uint8_t **in_plane;
	int32_t *in_stride;
	uint8_t **out_buf;
	int *out_size;
	int64_t *out_pts;
	int64_t *out_dts;
	int out_frame_type;
} __attribute__((aligned (16))) libx264_enc;

typedef void *libx264_t;
int EXPORTS MINGWAPI libx264_open(libx264_t *h
	, char *profile, char *preset, char *tune
	, int width, int height, int fps_num, int fps_den, int x264_mode
	, int kbits, int crf, int qp
	, int type);
int EXPORTS MINGWAPI libx264_encode(libx264_t h, libx264_enc *enc);
int EXPORTS MINGWAPI libx264_done(libx264_t h);
int EXPORTS MINGWAPI libx264_reset(libx264_t h);
int EXPORTS MINGWAPI libx264_close(libx264_t h);
void EXPORTS MINGWAPI libx264_get_msg(char *msg);

void libx264_setmsg2(const char *file, int line, const char *fmt, ...);
#define libx264_setmsg(fmt, ...) libx264_setmsg2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)


#ifdef  __cplusplus
}
#endif 



#endif
