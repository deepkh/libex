/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#ifndef _FIREFLY_DEF_H_
#define _FIREFLY_DEF_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <time.h>
#include <errno.h>
#include <wchar.h>
#ifdef __MINGW32__
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h> /* mach_absolute_time */
#include <mach/mach.h>      /* host_get_clock_service, mach_... */
#include <mach/clock.h>     /* clock_get_time */
#include <sys/time.h>
#endif


#ifdef __MINGW32__
#define EXPORTS __declspec(dllexport)
#define MINGWAPI __cdecl
#else
#define EXPORTS
#define MINGWAPI
#endif

#include <stdint.h>

#define FF_ALIGNMENT_SIZE 16
#define FF_TIME_BASE 1000000

#define DEFAULT_VIDEO_TRACK_ID 0
#define DEFAULT_AUDIO_TRACK_ID 1
#define DEFAULT_SRT_TRACK_ID 2

int ff_alignment_size(int size);
void *ff_memcpy(void * to, const void * from, size_t len);
void *ff_calloc(int size);
void *ff_realloc(void *ptr, int size);
void ff_free(void *ptr);

enum FIREFLY_TYPE {
	FIREFLY_TYPE_NONE = 0,
	FIREFLY_TYPE_PCM = 1,
	FIREFLY_TYPE_AAC,
	FIREFLY_TYPE_MP3,
	FIREFLY_TYPE_RGB24 = 10,
	FIREFLY_TYPE_RGB32,
	FIREFLY_TYPE_YUV420P,
	FIREFLY_TYPE_YUV422P,
	FIREFLY_TYPE_YUV444P,
	FIREFLY_TYPE_X264,
	FIREFLY_TYPE_TS_STREAM = 20,
	FIREFLY_TYPE_SRT = 30,
	FIREFLY_TYPE_VTT = 31
};

enum FIREFLY_FRAME_TYPE {
	FIREFLY_FRAME_TYPE_I = 0,
	FIREFLY_FRAME_TYPE_P = 1,
	FIREFLY_FRAME_TYPE_B = 2
};

enum FIREFLY_YUV_COLOR {
	FIREFLY_YUV_COLOR_BT709 = 0,
	FIREFLY_YUV_COLOR_BT656 = 1,
};

typedef struct  {
	int track_id;
	enum FIREFLY_TYPE type;
	enum FIREFLY_FRAME_TYPE frame_type;

	int pts_in_real_time;
	int64_t dts;
	int64_t pts;
	uint32_t frame_counter;
	int64_t offset_pts;			//for seek movie file

	//video config
	int width;
	int height;
	double fps_f;
	int fps_num;
	int fps_den;
	int bit_per_sample;			//8,16,24,32

	//audio
	int channels;				//1,2
	int sample_rate;			//8000Hz, 44100hz, 48000hz
	int bit_rate;				//8bits, 16bits
	int num_frame;				//number of frame
	int silence;
	int frame_size;
}__attribute__((aligned (FF_ALIGNMENT_SIZE))) firefly_header;

typedef struct {
	firefly_header header;

	int buf_alloc_inside;
	uint8_t *buf;
	int buf_size;
	int buf_max_size;

	//video
	uint8_t *plane[4];
	int32_t stride[4];
	int plane_num;
}__attribute__((aligned (FF_ALIGNMENT_SIZE))) firefly_buffer;

typedef struct {
	int fps[10];
	int fps_prev_idx;
	int fps_tmp;
} firefly_fps;

double firefly_audio_pts(firefly_buffer *p);
int64_t firefly_audio_pts_by(firefly_buffer *p, int time_base);
double firefly_video_fps(firefly_buffer *p);
double firefly_video_pts(firefly_buffer *p);
int64_t firefly_video_pts_by(firefly_buffer *p, int time_base);
double firefly_video_dts(firefly_buffer *p);
int64_t firefly_video_dts_by(firefly_buffer *p, int time_base);

double firefly_srt_pts(firefly_buffer *p);

int firefly_frame_set_video_plane(firefly_buffer *p, uint8_t *buf);
int firefly_frame_set_video_header(firefly_buffer *p
	, enum FIREFLY_TYPE type, int track_id, int width, int height, int fps_num, int fps_den, int buf_alloc_inside);
firefly_buffer *firefly_frame_video_calloc(
enum FIREFLY_TYPE type, int track_id, int width, int height, int fps_num, int fps_den, int buf_alloc_inside);

int firefly_frame_set_audio_header(firefly_buffer *p
	, enum FIREFLY_TYPE type, int track_id, int channels, int sample_rate, int bit_rate, int alloc_frame);
firefly_buffer *firefly_frame_audio_calloc(
enum FIREFLY_TYPE type, int track_id, int channels, int sample_rate, int bit_rate, int alloc_frame);

int firefly_frame_free(firefly_buffer *p);
int firefly_fps_calc(firefly_fps *fps, int64_t t1);

void ff_prt_hex(uint8_t *pb, int cb, int len);

#endif
