/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#ifndef _LIBFFMPEG_H_
#define _LIBFFMPEG_H_
#ifdef __MINGW32__
#include <windows.h>
#define EXPORTS __declspec(dllexport)
#define MINGWAPI __cdecl
#else
#define EXPORTS
#define MINGWAPI
#endif
#include "firefly_common.h"
#include <stdint.h>

#define DBG_MSG_LEN 1024

typedef struct {
	//in
	const char *file_name;
	int decode_threads;				//0 -> AUTO
	int disable_decode_h264;

	//out
	double duration;				//seconds
	int audio_only;
	
	//video out
	int video_type;					//FIREFLY_TYPE_X264 or FIREFLY_TYPE_YUV420P
	int width;
	int height;
	int fps_num;
	int fps_den;

	//out audio
	int audio_type;					//FIREFLY_TYPE_PCM 
	int sample_rate;				//8000Hz, 44100hz, 48000hz
	int bit_rate;					//8bits, 16bits
	int channels;					//1,2
} __attribute__((aligned (FF_ALIGNMENT_SIZE))) libffmpeg2_config;

typedef void *libffmpeg2_t;
typedef int (*libffmpeg2_log)(const char *fmt, ...);
int EXPORTS MINGWAPI libffmpeg2_open(libffmpeg2_t *h, libffmpeg2_config *cfg, libffmpeg2_log log);
int EXPORTS MINGWAPI libffmpeg2_decode(libffmpeg2_t h, firefly_buffer **in);
int EXPORTS MINGWAPI libffmpeg2_seek(libffmpeg2_t h, void *p1, void *p2);
int EXPORTS MINGWAPI libffmpeg2_set_video_offset(libffmpeg2_t h, int64_t video_offset);
int EXPORTS MINGWAPI libffmpeg2_set_audio_offset(libffmpeg2_t h, int64_t audio_offset);
int EXPORTS MINGWAPI libffmpeg2_reset(libffmpeg2_t h);
int EXPORTS MINGWAPI libffmpeg2_close(libffmpeg2_t h);
void EXPORTS MINGWAPI libffmpeg2_get_msg(char *msg);
void libffmpeg2_setmsg2(const char *file, int line, const char *fmt, ...);
#define libffmpeg2_setmsg(fmt, ...) libffmpeg2_setmsg2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void fflog2(const char *file, int line, libffmpeg2_log log, const char *fmt, ...);
#define fflog(log, fmt, ...) fflog2(__FILE__, __LINE__, log, fmt, ##__VA_ARGS__)

typedef struct {
	char swscale_method[32];

	int in_type_is_ffmpeg;		//in_video_type is ffmpeg or libfirefly
	int in_type;					//ffmpeg pix_fmt type or firefly
	int in_width;
	int in_height;

	int out_type_is_ffmpeg;		//out_video_type is ffmpeg or libfirefly
	int out_type;					//ffmpeg pix_fmt type or firefly
	int out_width;
	int out_height;
} __attribute__((aligned (FF_ALIGNMENT_SIZE))) libffmpeg2_img_scale_config;

int EXPORTS MINGWAPI libffmpeg2_img_scale_open(libffmpeg2_t *h, libffmpeg2_img_scale_config *cfg, libffmpeg2_log log);
int EXPORTS MINGWAPI libffmpeg2_img_scale_scaling(libffmpeg2_t h, firefly_buffer *in, firefly_buffer **out);
int EXPORTS MINGWAPI libffmpeg2_img_scale_close(libffmpeg2_t h);

//scaling
int EXPORTS MINGWAPI libffmpeg2_video_scaling(
	int in_type, int in_width, int in_height, uint8_t **in_plane, int32_t *in_stride
	, int out_type, int out_width, int out_height, uint8_t **out_plane, int32_t *out_stride
);

#endif
