/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are NOT permitted without specific written permission
* from above copyright holder.
*/

#include "firefly_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <windows.h>
#include <tchar.h>
#include <wchar.h>

#define ff_assert(cond) do {                                           \
    if (!(cond)) {                                                      \
        printf("Assertion failed at %s:%d\n",    \
               __FILE__, __LINE__);                 \
        abort();                                                        \
	    }                                                                   \
} while (0)

int ff_alignment_size(int size)
{
	return ((size / FF_ALIGNMENT_SIZE) + 1)*FF_ALIGNMENT_SIZE;
}

void *ff_calloc(int size)
{
	void *ptr = NULL;
	long diff;
	ptr = calloc(1, size + FF_ALIGNMENT_SIZE);
	if (!ptr) {
		return ptr;
	}
	diff = ((~(long)ptr)&(FF_ALIGNMENT_SIZE - 1)) + 1;
	ptr = (char *)ptr + diff;
	((char *)ptr)[-1] = diff;
	return ptr;
}

void *ff_realloc(void *ptr, int size)
{
	int diff;
	if (!ptr) {
		return ff_calloc(size);
	}
	diff = ((char *)ptr)[-1];
	ff_assert(diff>0 && diff <= FF_ALIGNMENT_SIZE);
	ptr = realloc((char *)ptr - diff, size + diff);
	if (ptr) {
		ptr = (char *)ptr + diff;
	}
	return ptr;
}

void ff_free(void *ptr)
{
	if (ptr) {
		int v = ((char *)ptr)[-1];
		ff_assert(v>0 && v <= FF_ALIGNMENT_SIZE);
		free((char *)ptr - v);
	}
}




double firefly_audio_pts(firefly_buffer *p)
{
	return (p->header.pts+p->header.offset_pts)/(double)p->header.sample_rate;
}

int64_t firefly_audio_pts_by(firefly_buffer *p, int time_base)
{
	return (int64_t) firefly_audio_pts(p)*time_base;
}

double firefly_video_fps(firefly_buffer *p)
{
	if (((int)p->header.fps_f) != 0) {
		p->header.fps_f = (double)p->header.fps_num/(double)p->header.fps_den;
	}
	return p->header.fps_f;
}

double firefly_video_pts(firefly_buffer *p)
{
	if (p->header.pts_in_real_time) {
		return (p->header.pts+p->header.offset_pts)/(double)FF_TIME_BASE;
	} else {
		return (p->header.pts+p->header.offset_pts)/firefly_video_fps(p);
	}
}

int64_t firefly_video_pts_by(firefly_buffer *p, int time_base)
{
	return (int64_t)firefly_video_pts(p)*time_base;
}

double firefly_video_dts(firefly_buffer *p)
{
	if (p->header.pts_in_real_time) {
		return (p->header.dts+p->header.offset_pts)/(double)FF_TIME_BASE;
	} else {
		return (p->header.dts+p->header.offset_pts)/firefly_video_fps(p);
	}
}

int64_t firefly_video_dts_by(firefly_buffer *p, int time_base)
{
	return (int64_t)firefly_video_dts(p)*time_base;
}

double firefly_srt_pts(firefly_buffer *p)
{
	return (p->header.pts+p->header.offset_pts)/1000.0f;
}

int firefly_frame_set_video_plane(firefly_buffer *p, uint8_t *buf)
{
	int width = p->header.width;
	int height = p->header.height;
	int wxh = width*height;

	if ((p->header.type == FIREFLY_TYPE_RGB24) || (p->header.type == FIREFLY_TYPE_RGB32) || (p->header.type == FIREFLY_TYPE_X264)) {
		p->plane[0] = buf;
	} else if (p->header.type == FIREFLY_TYPE_YUV420P) {
		p->plane[0] = buf;
		p->plane[1] = buf + wxh;
		p->plane[2] = p->plane[1] + wxh/4;
	} else if (p->header.type == FIREFLY_TYPE_YUV422P) {
		p->plane[0] = buf;
		p->plane[1] = buf + wxh;
		p->plane[2] = p->plane[1] + wxh/2;
	} else if (p->header.type == FIREFLY_TYPE_YUV444P) {
		p->plane[0] = buf;
		p->plane[1] = buf + wxh;
		p->plane[2] = p->plane[1] + wxh;
	}
	
	return 0;
}

int firefly_frame_set_video_header(firefly_buffer *p
		, enum FIREFLY_TYPE type, int track_id, int width, int height, int fps_num, int fps_den, int buf_alloc_inside)
{
	p->header.track_id = track_id;
	p->header.type = type;
	p->header.width = width;
	p->header.height = height;
	p->header.fps_num = fps_num;
	p->header.fps_den = fps_den;
	p->header.fps_f = (double)fps_num/(double)fps_den;
	p->buf_alloc_inside = buf_alloc_inside;

	if (type == FIREFLY_TYPE_RGB32 || type == FIREFLY_TYPE_X264) {
		p->header.bit_per_sample = 32;
		p->buf_size = width*height*4;
		p->buf_max_size = p->buf_size;
		p->stride[0] = width * 4;
		p->plane_num = 1;
	} else if (type == FIREFLY_TYPE_RGB24) {
		p->header.bit_per_sample = 24;
		p->buf_size = width*height*3;
		p->buf_max_size = p->buf_size;
		p->stride[0] = width * 3;
		p->plane_num = 1;
	} else if (type == FIREFLY_TYPE_YUV420P) {
		p->header.bit_per_sample = 12;				//8*1.5
		p->buf_size = width*height*3/2;
		p->buf_max_size = p->buf_size;
		p->stride[0] = width;
		p->stride[1] = width/2;
		p->stride[2] = width/2;
		p->plane_num = 3;
	} else if (type == FIREFLY_TYPE_YUV422P) {
		p->header.bit_per_sample = 16;				//8*2
		p->buf_size = width*height*2;
		p->buf_max_size = p->buf_size;
		p->stride[0] = width;
		p->stride[1] = width/2;
		p->stride[2] = width/2;
		p->plane_num = 3;
	} else if (type == FIREFLY_TYPE_YUV444P) {
		p->header.bit_per_sample = 24;
		p->buf_size = width*height*3;
		p->buf_max_size = p->buf_size;
		p->stride[0] = width;
		p->stride[1] = width;
		p->stride[2] = width;
		p->plane_num = 3;
	} else {
		printf( "unsupport format %d\n", type);
		goto error;
	}

	return 0;
error:
	return -1;
}

firefly_buffer *firefly_frame_video_calloc(
		enum FIREFLY_TYPE type, int track_id, int width, int height, int fps_num, int fps_den, int buf_alloc_inside)
{
	firefly_buffer *p;

	if ((p = (firefly_buffer *) ff_calloc(sizeof(firefly_buffer))) == NULL) {
		printf( "failed to calloc\n");
		return NULL;
	}

	if (firefly_frame_set_video_header(p, type, track_id, width, height, fps_num, fps_den, buf_alloc_inside)) {
		goto error;
	}
		
	if (buf_alloc_inside) {
		if ((p->buf = (uint8_t*) ff_calloc(p->buf_max_size)) == NULL) {
			printf("failed to calloc frame. %s\n", strerror(errno));
			goto error;
		}

		if (firefly_frame_set_video_plane(p, p->buf)) {
			printf("failed to calloc firefly_frame_set_plane. %s\n", strerror(errno));
			goto error;
		}
	}

	return p;

error:
	if (p) {
		firefly_frame_free(p);
	}
	return NULL;
}

int firefly_frame_set_audio_header(firefly_buffer *p
		, enum FIREFLY_TYPE type, int track_id, int channels, int sample_rate, int bit_rate, int alloc_frame)
{
	p->header.track_id = track_id;
	p->header.type = type;
	p->header.channels = channels;
	p->header.sample_rate = sample_rate;
	p->header.bit_rate = bit_rate;
	
	p->header.frame_size = p->header.bit_rate/8 * p->header.channels;
	p->buf_alloc_inside =  alloc_frame > 0 ? 1: 0;
	p->buf_max_size = p->header.frame_size * alloc_frame;

	if (type != FIREFLY_TYPE_PCM) {
		printf( "unsupport format %d, only support PCM\n", type);
		goto error;
	}

	return 0;
error:
	return -1;
}

firefly_buffer *firefly_frame_audio_calloc(
	enum FIREFLY_TYPE type, int track_id, int channels, int sample_rate, int bit_rate, int alloc_frame)
{
	firefly_buffer *p;

	if ((p = (firefly_buffer *) ff_calloc(sizeof(firefly_buffer))) == NULL) {
		printf("failed to calloc\n");
		return NULL;
	}

	if (firefly_frame_set_audio_header(p, type, track_id, channels, sample_rate, bit_rate, alloc_frame)) {
		printf("failed to set header\n");
		goto error;
	}

	if (alloc_frame) {
		if ((p->buf = (uint8_t*) ff_calloc(p->buf_max_size)) == NULL) {
			printf("failed to calloc frame. %s\n", strerror(errno));
			goto error;
		}
	}

	return p;
error:
	if (p) {
		firefly_frame_free(p);
	}
	return NULL;
}

int firefly_frame_free(firefly_buffer *p)
{
	if (p == NULL) {
		printf("buffer non initial\n");
		return -1;
	}

	if (p->buf_alloc_inside && p->buf) {
		ff_free(p->buf);
	}

	ff_free(p);
	return 0;
}

int firefly_fps_calc(firefly_fps *fps, int64_t t1)
{
	int i;
	int fps_idx;
	int fps_1secs = 0;

	fps_idx = (int)((t1 / 100000)) % 10;			//0 ~ 9
	if (fps_idx != fps->fps_prev_idx) {
		fps->fps_tmp = fps->fps[fps_idx];
		fps->fps[fps_idx] = 0;
	}
	fps->fps_prev_idx = fps_idx;
	fps->fps[fps_idx]++;

	for (i = 0; i<10; i++) {
		if (i == fps_idx) {
			fps_1secs += fps->fps_tmp;
		}
		else {
			fps_1secs += fps->fps[i];
		}
	}

	return fps_1secs;
}

void ff_prt_hex(uint8_t *pb, int cb, int len)
{
	int i;
	//char line[1024] = {0};
	char *line = NULL;

	if ((line = (char *)calloc(1, cb * 5 + 1 + (20 * (len / 16 + 1)))) == NULL) {
		printf("failed to calloc %d\n", cb * 5 + 1);
		return;
	}

	line[0] = 0;
	sprintf(line, "len:%d cb:%d\n", len, cb);
	for (i = 0; i<len && i < cb; i++) {
		if (i % 16 == 0) {
			if (i > 0) {
				sprintf(line + strlen(line), "\n %04X: ", i);
			}
			else {
				sprintf(line + strlen(line), " %04X: ", i);
			}
		}
		sprintf(line + strlen(line), "%02X ", pb[i]);
	}
	printf("%s\n", line);

	free(line);
}

