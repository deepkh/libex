/*
* Copyright 2017 NetSync.tv Grey Huang (deepkh@gmail.com) All rights reserved. 
* license: GNU Lesser General Public License v2.1 
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

enum FFMPEG {
	FFMPEG_EOF = -10, 
	FFMPEG_HW_DECODER_ERROR = -2, 
	FFMPEG_HW_DECODER_DEVICE_ERROR = -100,	//MFX H.264 Decoder sometimes failed on 4790k HD4600, but not happend on 4720hq HD4600
};

typedef struct {
	//in
	const char *file_name;
	int decode_threads;				//0 -> AUTO
	int disable_decode_h264;
	int enable_aud;					//HLS need this, dash dont, when do RAW HLS stream (without transcode)

	//out
	double duration;				//seconds
	int audio_only;
	
	//video out
	int video_type;					//FIREFLY_TYPE_X264 or FIREFLY_TYPE_YUV420P
	int bit_depth_minus8;			//if yuv10bit then fill 2, if yuv8bit then fill 0, most for HEVC Main 10bit(AV_PIX_FMT_YUV420P10LE)
	int width;
	int height;
	int fps_num;
	int fps_den;

	//out audio
	int audio_type;					//FIREFLY_TYPE_PCM 
	int sample_rate;				//8000Hz, 44100hz, 48000hz
	int bit_rate;					//8bits, 16bits
	int channels;					//1,2
} __attribute__((aligned (FF_ALIGNMENT_SIZE))) libffmpeg_config;

typedef void *libffmpeg_t;
typedef int (*libffmpeg_log)(const char *fmt, ...);
int EXPORTS MINGWAPI libffmpeg_open(libffmpeg_t *h, libffmpeg_config *cfg, libffmpeg_log log);
int EXPORTS MINGWAPI libffmpeg_decode(libffmpeg_t h, firefly_buffer **in);
int EXPORTS MINGWAPI libffmpeg_seek(libffmpeg_t h, void *p1, void *p2);
int EXPORTS MINGWAPI libffmpeg_set_video_offset(libffmpeg_t h, int64_t video_offset);
int EXPORTS MINGWAPI libffmpeg_set_audio_offset(libffmpeg_t h, int64_t audio_offset);
int EXPORTS MINGWAPI libffmpeg_reset(libffmpeg_t h);
int EXPORTS MINGWAPI libffmpeg_close(libffmpeg_t h);
void EXPORTS MINGWAPI libffmpeg_get_msg(char *msg);
void libffmpeg_setmsg2(const char *file, int line, const char *fmt, ...);
#define libffmpeg_setmsg(fmt, ...) libffmpeg_setmsg2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void fflog2(const char *file, int line, libffmpeg_log log, char *log_str, const char *fmt, ...);
#define fflog(log, log_str, fmt, ...) fflog2(__FILE__, __LINE__, log, log_str, fmt, ##__VA_ARGS__)


/***************************************************
 * sw_scale scaling
 **************************************************/
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
} __attribute__((aligned (FF_ALIGNMENT_SIZE))) libffmpeg_img_scale_config;

int EXPORTS MINGWAPI libffmpeg_img_scale_open(libffmpeg_t *h, libffmpeg_img_scale_config *cfg, libffmpeg_log log);
int EXPORTS MINGWAPI libffmpeg_img_scale_scaling(libffmpeg_t h, firefly_buffer *in, firefly_buffer **out);
int EXPORTS MINGWAPI libffmpeg_img_scale_close(libffmpeg_t h);

//scaling
int EXPORTS MINGWAPI libffmpeg_video_scaling(
	int in_type, int in_width, int in_height, uint8_t **in_plane, int32_t *in_stride
	, int out_type, int out_width, int out_height, uint8_t **out_plane, int32_t *out_stride
);

/***************************************************
 * FFMPEG AAC Encoder
 **************************************************/
typedef struct {
	uint8_t *in_buf;
	int in_buf_size;
	int in_num_frame;
	uint8_t **out_buf;
	int *out_buf_size;
	int *out_num_frame;
	int64_t *out_pts;
} __attribute__((aligned (FF_ALIGNMENT_SIZE))) libffaac_enc;

typedef void *libffaac_enc_t;
typedef int (*libffaac_log)(const char *fmt, ...);
int EXPORTS MINGWAPI libffaac_enc_open(libffaac_enc_t *h
	, int in_channels, int in_sample_rate, int in_bit_rate, int in_audio_gain
	, int out_vbr, int out_bit_rate, int *enc_num_frame, int out_bitstream_fmt, libffaac_log fflog);
int EXPORTS MINGWAPI libffaac_enc_encode(libffaac_enc_t h, libffaac_enc *enc);
int EXPORTS MINGWAPI libffaac_enc_done(libffaac_enc_t h);
int EXPORTS MINGWAPI libffaac_enc_reset(libffaac_enc_t h);
int EXPORTS MINGWAPI libffaac_enc_close(libffaac_enc_t h);
void EXPORTS MINGWAPI libffaac_enc_get_msg(char *msg);
void libffaac_enc_setmsg2(const char *file, int line, const char *fmt, ...);
#define libffaac_enc_setmsg(fmt, ...) libffaac_enc_setmsg2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
