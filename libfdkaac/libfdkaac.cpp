/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#include "libfdkaac.h"
#include <stdint.h>
extern "C" {
}
#include <fdk-aac/aacenc_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char g_szMsg[8192];

typedef struct {
	AACENC_InfoStruct info;
	HANDLE_AACENCODER handle;
	AACENC_BufDesc in_buf;
	AACENC_InArgs in_args;
	int in_identifier;
	int in_elem_size;
	AACENC_BufDesc out_buf;
	AACENC_OutArgs out_args;
	int out_identifier;
	int out_elem_size;
	
	int frame_size;

	uint8_t *in_frame;
	int in_frame_num;
	int in_frame_num_current;
	int in_frame_size;

	uint8_t *out_frame;
	int out_frame_size;

	int64_t frame_counter;
	//firefly_buffer out_buf2;
} libfdkaac_enc_data;

int EXPORTS MINGWAPI libfdkaac_enc_open(libfdkaac_enc_t *h
	, int in_channels, int in_sample_rate, int in_bit_rate
	, int out_vbr, int out_bit_rate, int *enc_num_frame, int out_bitstream_fmt)
{
	libfdkaac_enc_data *p = NULL;
	int aot;
	CHANNEL_MODE mode; 
	int channel_order;
	int vbr;
	int bit_stream_format;
	int afterburner;

	if ((p = (libfdkaac_enc_data*)calloc(1, sizeof(libfdkaac_enc_data))) == NULL) {
		libfdkaac_enc_setmsg("failed calloc libfdkaac_enc_data");
		return -1;
	}

	aot = 2;									// 2: MPEG-4 AAC Low Complexity. 129: MPEG-2 AAC Low Complexity.
	mode = in_channels == 2 ? MODE_2 : MODE_1;
	channel_order = 1;							//0: MPEG channel ordering (e. g. 5.1: C, L, R, SL, SR, LFE). (default)
												//1: WAVE file format channel ordering (e. g. 5.1: L, R, C, LFE, SL, SR).
	vbr = out_vbr ? 8 : 0;						//0: constant , 8: vbr
	bit_stream_format = out_bitstream_fmt;		//0:RAW 1: ADIF (for mp4) 2: ADTS (for MPEG-2 AAC LC), 6,7:LATM
	afterburner = 1;							//0: disable, 1: enable. increase audio quality

	//0x01 meain only AAC LC
	//0x00 default is AAC+SBR+PS (HE-AAC v2)
	if (aacEncOpen(&p->handle, 0x00, in_channels) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to open encoder");
		goto error;
	}

	if (aacEncoder_SetParam(p->handle, AACENC_AOT, aot) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to set the AOT");
		goto error;
	}

	if (aacEncoder_SetParam(p->handle, AACENC_SAMPLERATE, in_sample_rate) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to set the sample_rate");
		goto error;
	}

	if (aacEncoder_SetParam(p->handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to set the channel mode");
		goto error;
	}

	if (aacEncoder_SetParam(p->handle, AACENC_CHANNELORDER, channel_order) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to set the wav channel order");
		goto error;
	}

	if (vbr) {
		if (aacEncoder_SetParam(p->handle, AACENC_BITRATEMODE, 1) != AACENC_OK) { //VBR range 1 ~ 5
			libfdkaac_enc_setmsg("failed to set the VBR bitrate mode");
			goto error;
		}
	} else {
		if (aacEncoder_SetParam(p->handle, AACENC_BITRATE, out_bit_rate) != AACENC_OK) {
			libfdkaac_enc_setmsg("failed to set the bitrate");
			goto error;
		}
	}

	if (aacEncoder_SetParam(p->handle, AACENC_TRANSMUX, bit_stream_format) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to set the ADTS transmux");
		goto error;
	}

	if (aacEncoder_SetParam(p->handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to set the afterburner mode");
		goto error;
	}

	if (aacEncEncode(p->handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to initialize the encoder");
		goto error;
	}

	if (aacEncInfo(p->handle, &p->info) != AACENC_OK) {
		libfdkaac_enc_setmsg("failed to get the encoder info");
		goto error;
	}

	p->frame_size = in_channels * (in_bit_rate/8);

	p->in_frame_num = p->info.frameLength;
	p->in_frame_size = p->frame_size * p->in_frame_num;
	p->out_frame_size = 20480;

	if ((p->in_frame = (uint8_t *) calloc(1, p->in_frame_size)) == NULL) {
		libfdkaac_enc_setmsg("failed to calloc in_frame buffer");
		goto error;
	}

	if ((p->out_frame = (uint8_t *)calloc(1, p->out_frame_size)) == NULL) {
		libfdkaac_enc_setmsg("failed to calloc out_frame buffer");
		goto error;
	}
	
	p->in_args.numInSamples = p->in_frame_size/2;

	p->in_identifier = IN_AUDIO_DATA;
	p->in_elem_size = 2;
	p->in_buf.numBufs = 1;
	p->in_buf.bufs = (void**) &p->in_frame;
	p->in_buf.bufferIdentifiers = &p->in_identifier;
	p->in_buf.bufSizes = &p->in_frame_size;
	p->in_buf.bufElSizes = &p->in_elem_size;

	p->out_identifier = OUT_BITSTREAM_DATA;
	p->out_elem_size = 1;
	p->out_buf.numBufs = 1;
	p->out_buf.bufs = (void **) &p->out_frame;
	p->out_buf.bufferIdentifiers = &p->out_identifier;
	p->out_buf.bufSizes = &p->out_frame_size;
	p->out_buf.bufElSizes = &p->out_elem_size;

	/*p->out_buf2.header.track_id = 1;
	p->out_buf2.header.type = FIREFLY_TYPE_AAC;
	p->out_buf2.header.channels = c->in_channels;
	p->out_buf2.header.sample_rate = c->in_sample_rate;
	p->out_buf2.header.bit_rate = c->in_bit_rate;
	p->out_buf2.buf = p->out_frame;*/
	*enc_num_frame = p->in_frame_num;

	*h = p;
	return 0;
error:
	libfdkaac_enc_close(p);
	return -1;
}

int EXPORTS MINGWAPI libfdkaac_enc_encode(libfdkaac_enc_t h, libfdkaac_enc *enc)
{
	libfdkaac_enc_data *p = (libfdkaac_enc_data *)h;
	AACENC_ERROR err;
	int copy_frame;
	int remain_frame;

	if (enc->in_num_frame > p->in_frame_num) {
		libfdkaac_enc_setmsg("%d over %d failed", enc->in_num_frame, p->in_frame_num);
		return -1;
	}

	// less
	if ((p->in_frame_num_current + enc->in_num_frame) < p->in_frame_num) {
		memcpy(p->in_frame+p->in_frame_num_current*p->frame_size, enc->in_buf, enc->in_buf_size);
		p->in_frame_num_current += enc->in_num_frame;
		*enc->out_buf_size = 0;
		*enc->out_buf = NULL;
		*enc->out_num_frame = 0;
		//p->out_buf2.buf_size = 0;
		//p->out_buf2.header.num_frame = 0;
		//*out = &p->out_buf2;
		goto finally;
	}

	// greater & equals
	copy_frame = p->in_frame_num - p->in_frame_num_current;
	memcpy(p->in_frame+p->in_frame_num_current*p->frame_size, enc->in_buf, copy_frame*p->frame_size);

	if ((err = aacEncEncode(p->handle, &p->in_buf, &p->out_buf, &p->in_args, &p->out_args)) != AACENC_OK) {
		if (err == AACENC_ENCODE_EOF) {
			libfdkaac_enc_setmsg("EOF");
			goto finally;
		}
		libfdkaac_enc_setmsg("AAC Encoding failed. %d", err);
		goto error;
	}

	*enc->out_buf_size = p->out_args.numOutBytes;
	*enc->out_pts = p->frame_counter;
	*enc->out_buf = p->out_frame;
	*enc->out_num_frame = p->in_frame_num;
	//p->out_buf2.buf_size = p->out_args.numOutBytes;
	//p->out_buf2.header.pts = p->frame_counter;
	//p->out_buf2.header.num_frame = p->in_frame_num;
	p->frame_counter += p->in_frame_num;
	//*out = &p->out_buf2;

	//copy remain bytes
	remain_frame = enc->in_num_frame - copy_frame;
	memcpy(p->in_frame, enc->in_buf+(copy_frame*p->frame_size), remain_frame*p->frame_size);
	p->in_frame_num_current = remain_frame;

finally:
	return 0;
error:
	return -1;
}

int EXPORTS MINGWAPI libfdkaac_enc_done(libfdkaac_enc_t h)
{
	return -1;
}

int EXPORTS MINGWAPI libfdkaac_enc_reset(libfdkaac_enc_t h)
{
	libfdkaac_enc_data *p = (libfdkaac_enc_data *)h;

	if (!p) {
		return -1;
	}

	p->in_frame_num_current = 0;
	p->frame_counter = 0;
	return 0;
}

int EXPORTS MINGWAPI libfdkaac_enc_close(libfdkaac_enc_t h)
{
	libfdkaac_enc_data *p = (libfdkaac_enc_data *)h;

	if (!p) {
		return -1;
	}

	if (p->in_frame) {
		free(p->in_frame);
	}

	if (p->out_frame) {
		free(p->out_frame);
	}

	if (p->handle) {
		aacEncClose(&p->handle);
	}
	
	free(p);
	return 0;
}

void EXPORTS MINGWAPI libfdkaac_enc_get_msg(char *msg)
{
	sprintf(msg, "%s", g_szMsg);
}

void libfdkaac_enc_setmsg2(const char *file, int line, const char *fmt, ...)
{
	va_list args;
	sprintf(g_szMsg, "%s(%d): ", file, line);
	va_start(args, fmt);
	vsprintf(strlen(g_szMsg) + g_szMsg, fmt, args);
	va_end(args);
}

