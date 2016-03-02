/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#ifndef _LIBFDKAAC_H_
#define _LIBFDKAAC_H_
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
	uint8_t *in_buf;
	int in_buf_size;
	int in_num_frame;
	uint8_t **out_buf;
	int *out_buf_size;
	int *out_num_frame;
	int64_t *out_pts;
} __attribute__((aligned (16))) libfdkaac_enc;

typedef void *libfdkaac_enc_t;
int EXPORTS MINGWAPI libfdkaac_enc_open(libfdkaac_enc_t *h
	, int in_channels, int in_sample_rate, int in_bit_rate
	, int out_vbr, int out_bit_rate, int *enc_num_frame, int out_bitstream_fmt);
int EXPORTS MINGWAPI libfdkaac_enc_encode(libfdkaac_enc_t h, libfdkaac_enc *enc);
int EXPORTS MINGWAPI libfdkaac_enc_done(libfdkaac_enc_t h);
int EXPORTS MINGWAPI libfdkaac_enc_reset(libfdkaac_enc_t h);
int EXPORTS MINGWAPI libfdkaac_enc_close(libfdkaac_enc_t h);
void EXPORTS MINGWAPI libfdkaac_enc_get_msg(char *msg);

void libfdkaac_enc_setmsg2(const char *file, int line, const char *fmt, ...);
#define libfdkaac_enc_setmsg(fmt, ...) libfdkaac_enc_setmsg2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)


#ifdef  __cplusplus
}
#endif 



#endif
