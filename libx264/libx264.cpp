/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#include "libx264.h"
#include <stdint.h>
extern "C" {
#include <x264.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char g_szMsg[8192];

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
	FIREFLY_TYPE_X265,
	FIREFLY_TYPE_TS_STREAM = 20,
	FIREFLY_TYPE_SRT = 30,
	FIREFLY_TYPE_VTT = 31
};
typedef enum {
	FIREFLY_X264_MODE_ABR = 0,			//Average Bit Rate (Most for streaming)
	FIREFLY_X264_MODE_CRF = 1,			//Constant Rate Factory (Most for save as file). Ie: constant quality
	FIREFLY_X264_MODE_CQP = 2,			//Constant Qantization Parameter for I, P, B frame

} FIREFLY_X264_MODE;

enum FIREFLY_FRAME_TYPE {
	FIREFLY_FRAME_TYPE_I = 0,
	FIREFLY_FRAME_TYPE_P = 1,
	FIREFLY_FRAME_TYPE_B = 2
};

typedef struct {
	x264_t *x264;
	x264_param_t param;
	int csp;
} libx264_data;

static int inline x264_cs_mapping(enum FIREFLY_TYPE type)
{
	switch (type) {
	case FIREFLY_TYPE_YUV420P:
		return X264_CSP_I420;
	case FIREFLY_TYPE_YUV422P:
		return X264_CSP_I422;
	case FIREFLY_TYPE_YUV444P:
		return X264_CSP_I444;
	default:
		libx264_setmsg("x264 unsupport format %d", type);
		return -1;
	}
	return 0;
}

int EXPORTS MINGWAPI libx264_open(libx264_t *h
	, char *profile, char *preset, char *tune
	, int width, int height, int fps_num, int fps_den, int x264_mode
	, int kbits, int crf, int qp
	, int type)
{
	int rc;
	int b_turbo = 0;
	libx264_data *p = NULL;
	x264_param_t param;
	int fps = (int)((double)fps_num / (double)fps_den +0.5f);

	if ((p = (libx264_data*)calloc(1, sizeof(libx264_data))) == NULL) {
		libx264_setmsg("failed calloc libx2264_data");
		return -1;
	}

	if ((p->csp = x264_cs_mapping((FIREFLY_TYPE)type)) == -1) {
		goto error;
	}

	memset(&param, 0, sizeof(x264_param_t));
	if (preset || tune) {
		if ((rc = x264_param_default_preset(&param, preset, tune))) {
			libx264_setmsg("failed to x264_param_default_preset: %d", rc);
			goto error;
		}
	}

	if (preset && !strcmp(preset, "placebo")) {
		b_turbo = 0;
	}

#if 0
	//profile and level. http://blog.mediacoderhq.com/h264-profiles-and-levels/
	if (width <= 720 && height <= 480) {
		param.i_level_idc = 30;					//
		if (fps >= 30) {
			param.i_level_idc = 31;
		}
	} else if (width <= 1280 && height <= 1024) {
		param.i_level_idc = 31;
		if (fps > 30 || height >= 1024) {
			param.i_level_idc = 32;
		}
	} else if (width <= 2048 && height <= 1088) {
		param.i_level_idc = 41;
		if (fps > 30) {
			param.i_level_idc = 42;
		}
	} else if (width <= 4096 && height <= 2304) {
		param.i_level_idc = 50;
		if (fps >= 120 || width >= 4096) {
			param.i_level_idc = 51;
		}
	}
#else
	param.i_level_idc = 31;
	if (strcmp(profile, "baseline") == 0) {
		param.i_level_idc = 30;
	} else if (strcmp(profile, "main") == 0) {
		param.i_level_idc = 31;
	} else if (strcmp(profile, "high") == 0) {
		param.i_level_idc = 41;
	}
#endif

#if defined(DEBUG)
	//param.i_log_level = X264_LOG_DEBUG;
#endif

	param.b_deblocking_filter = 1;
	param.i_threads = X264_THREADS_AUTO;
	if (param.i_threads != 1) {
		param.b_deterministic = 1;				//better quality for multithread
	}
	param.i_width = width;
	param.i_height = height;
	param.i_csp = p->csp;
	if (fps > 0) {
		param.b_vfr_input = 0;
		param.i_fps_num = fps_num;
		param.i_fps_den = fps_den;
	}
	param.b_aud = 1;							//access unit delimiter
	param.b_annexb = 1;							//insert start code
	param.b_repeat_headers = 1;					//insert SPS,PPS in every I Frame
	param.i_keyint_min = fps;					//GOP
	param.i_keyint_max = fps;					//GOP
	param.b_intra_refresh = 0;					//periodic intra instead of IDR
	param.i_scenecut_threshold = 0;				/* how aggressively to insert extra I frames */

	/** rate cont0ol */
	if (x264_mode == FIREFLY_X264_MODE_ABR) {
		param.rc.i_rc_method = X264_RC_ABR;
		param.rc.i_bitrate = (int)((kbits/(double)1024*0.8)*1000);				//Kilobits
		param.rc.i_vbv_max_bitrate = param.rc.i_bitrate;		//default is zero
		param.rc.i_vbv_buffer_size = param.rc.i_bitrate;		//default is zero
	}
	else if (x264_mode == FIREFLY_X264_MODE_CRF) {
		param.rc.i_rc_method = X264_RC_CRF;
		param.rc.f_rf_constant = (float) crf;
		param.rc.f_rf_constant_max = (float) crf;
	}
	else if (x264_mode == FIREFLY_X264_MODE_CQP) {
		param.rc.i_rc_method = X264_RC_CQP;
		param.rc.i_qp_constant = qp;
	}

	if (b_turbo) {
		x264_param_apply_fastfirstpass(&param);
	}

	if ((rc = x264_param_apply_profile(&param, profile))) {
		libx264_setmsg("failed to x264_param_apply_profile: %d", rc);
		goto error;
	}

	if ((p->x264 = x264_encoder_open(&param)) == NULL) {
		libx264_setmsg("failed to x264_encoder_open");
		goto error;
	}

	x264_encoder_parameters(p->x264, &p->param);
	*h = p;
	return 0;
error:
	if (p) {
		if (p->x264) {
			x264_encoder_close(p->x264);
		}
	}
	libx264_close(p);
	return -1;
}

int EXPORTS MINGWAPI libx264_encode(libx264_t h, libx264_enc *enc)
{
	libx264_data *p = (libx264_data *)h;
	x264_picture_t pic_in;
	x264_picture_t pic_out;
	x264_nal_t *nal;
	int nal_num;
	int frame_size;

	x264_picture_init(&pic_in);

	if (enc->in_plane) {
		pic_in.img.i_csp = p->csp;
		pic_in.i_pts = *enc->in_pts;
		pic_in.i_dts = *enc->in_dts;
		pic_in.i_type = X264_TYPE_AUTO;
		memcpy(pic_in.img.plane, enc->in_plane, sizeof(pic_in.img.plane));
		memcpy(pic_in.img.i_stride, enc->in_stride, sizeof(pic_in.img.i_stride));
	}

	if ((frame_size = x264_encoder_encode(p->x264, &nal, &nal_num, enc->in_plane ? &pic_in : NULL, &pic_out)) < 0) {
		libx264_setmsg("failed to encoder");
		return -1;
	}

	if (frame_size == 0) {
		*enc->out_size = 0;
		goto finally;
	}

	*enc->out_buf = nal[0].p_payload;
	*enc->out_size = frame_size;
	*enc->out_dts = pic_out.i_dts;
	*enc->out_pts = pic_out.i_pts;

	switch (pic_out.i_type) {
	case X264_TYPE_IDR:
	case X264_TYPE_I:
		enc->out_frame_type = FIREFLY_FRAME_TYPE_I;
		break;
	case X264_TYPE_P:
		enc->out_frame_type = FIREFLY_FRAME_TYPE_P;
		break;
	case X264_TYPE_B:
	case X264_TYPE_BREF:
		enc->out_frame_type = FIREFLY_FRAME_TYPE_B;
		break;
	}

finally:
	return 0;
}

int EXPORTS MINGWAPI libx264_done(libx264_t h)
{
	libx264_data *p = (libx264_data *)h;
	return x264_encoder_delayed_frames(p->x264) ? 0 : 1;
}

int EXPORTS MINGWAPI libx264_reset(libx264_t h)
{
	libx264_data *p = (libx264_data *)h;

	x264_picture_t pic_out;
	x264_nal_t *nal;
	int nal_num;
	int frame_size;
	int c = 0;

#if 0
	//this function should not work
	while(x264_encoder_delayed_frames(p->x264) > 0) {
		if ((frame_size = x264_encoder_encode(p->x264, &nal, &nal_num, NULL, &pic_out)) < 0) {
			libx264_setmsg("failed to encoder");
			return -1;
		}
		printf("release %d\n", c++);
	}

	printf("remain: %d\n", x264_encoder_delayed_frames(p->x264));
	//x264_encoder_reconfig(p->x264, &p->param);
#else
	x264_encoder_close(p->x264);
	if ((p->x264 = x264_encoder_open(&p->param)) == NULL) {
		libx264_setmsg("failed to x264_encoder_open");
		return -1;
	}
#endif
	return 0;
}

int EXPORTS MINGWAPI libx264_close(libx264_t h)
{
	libx264_data *p = (libx264_data *)h;

	if (!p) {
		return -1;
	}

	if (p->x264) {
		x264_encoder_close(p->x264);
	}

	free(p);
	return 0;
}

void EXPORTS MINGWAPI libx264_get_msg(char *msg)
{
	sprintf(msg, "%s", g_szMsg);
}

void libx264_setmsg2(const char *file, int line, const char *fmt, ...)
{
	va_list args;
	sprintf(g_szMsg, "%s(%d): ", file, line);
	va_start(args, fmt);
	vsprintf(strlen(g_szMsg) + g_szMsg, fmt, args);
	va_end(args);
}
