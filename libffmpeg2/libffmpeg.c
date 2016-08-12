/*
* Copyright 2014-2015 Guan-Da Huang (doublescn@gmail.com)
* All rights reserved.
* license on GPL v2. please see license.txt for more details
*/

#include "libffmpeg.h"
#include <stdint.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff_buffer.h"

char g_szMsg[8192];

enum {
    NAL_SLICE           = 1,
    NAL_DPA             = 2,
    NAL_DPB             = 3,
    NAL_DPC             = 4,
    NAL_IDR_SLICE       = 5,
    NAL_SEI             = 6,
    NAL_SPS             = 7,
    NAL_PPS             = 8,
    NAL_AUD             = 9,
    NAL_END_SEQUENCE    = 10,
    NAL_END_STREAM      = 11,
    NAL_FILLER_DATA     = 12,
    NAL_SPS_EXT         = 13,
    NAL_AUXILIARY_SLICE = 19,
    NAL_FF_IGNORE       = 0xff0f001,
};


#define MAX_AAC_FRAME_SIZE 1024
#define FAILED_VIDEO_DECODE_COUNT 30
#define FAILED_AUDIO_DECODE_COUNT 44
#define MSDK_ALIGN16(value)                      (((value + 15) >> 4) << 4) // round up to a multiple of 16
#define MSDK_ALIGN32(value)                      (((value + 31) >> 5) << 5) // round up to a multiple of 32
#define MSDK_ALIGN64(value)                      (((value + 63) >> 6) << 6) // round up to a multiple of 64

static int libffmpeg_init = 0;

typedef struct {
	int idx;
	AVCodecContext *dec_ctx;
	AVRational st_time_base;
	AVRational dec_time_base;
	AVBitStreamFilterContext *h264_bsfc;
	AVCodecParserContext *parser;			//for h264/h265

	//video
	enum AVPixelFormat pix_fmt;				//AV_PIX_FMT_YUV420P...
	int width;
	int height;
	int fps_num;
	int fps_den;
	double fps_f;

	//audio
	enum AVSampleFormat sample_fmt;			//AV_SAMPLE_FMT_S16 ...		like waveout bitrate.
	uint64_t channel_layout;				//AV_CH_LAYOUT_STEREO...	like waveout channel
	int channels;
	int sample_rate;						//48000...
} ff_codec;

typedef struct {
	libffmpeg_config *cfg;
	AVFormatContext *fmt_ctx;
	double duration;

	AVFrame *frame;
	AVPacket pkt;

	ff_codec vid;
	int video_failed_decode_count;
	struct SwsContext *vid_cvrt;
	int is_vid_first_frame;
	int64_t vid_dts;
	int no_vid_pts;
	double seek_secs;

	ff_codec aud;
	int audio_failed_decode_count;
	int32_t sample_rate;					//out sample_rate
	SwrContext *aud_cvrt;

	// aud tmp buf
	struct ff_buffer *aud_buf;
	int64_t pts;
	int32_t num_frame;
	int32_t frame_size;

	firefly_buffer *vid_outbuf;
	firefly_buffer *aud_outbuf;

	libffmpeg_log log;
} libffmpeg_data;

static int64_t ff_video_ts_mapping(double secs, double fps_f)
{
	return (int64_t)(secs*fps_f + 0.5f);
}

static int64_t ff_audio_ts_mapping(double secs, int sample_rate)
{
	return (int64_t)(secs*sample_rate + 0.5f);
}

static int double_equals(double a, double b, double epsilon)
{
	return fabs(a - b) < epsilon;
}

static int ff_codec_close(ff_codec *c)
{
	if (c->dec_ctx) {
		if (c->h264_bsfc) {
			av_bitstream_filter_close(c->h264_bsfc);
			c->h264_bsfc = NULL;
		}
		if(c->parser) {
			av_parser_close(c->parser);
			c->parser = NULL;
		}
		avcodec_close(c->dec_ctx);
		c->dec_ctx = NULL;
	}
	return 0;
}

static int ff_codec_open(libffmpeg_data *p, enum AVMediaType type, AVFormatContext *fmt_ctx, ff_codec *c, int decode_threads)
{
	int ret;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;
	AVRational r_frame_rate ;

	if ((ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0)) <0) {
		libffmpeg_setmsg("Could not find %s stream", av_get_media_type_string(type));
		goto error;
	}

	c->idx = ret;
	st = fmt_ctx->streams[c->idx];

	c->dec_ctx = st->codec;
	if (!(dec = avcodec_find_decoder(c->dec_ctx->codec_id))) {
		libffmpeg_setmsg("failed to find %s codec", av_get_media_type_string(type));
		goto error;
	}

	if (decode_threads > 0) {
		c->dec_ctx->thread_count = decode_threads;
	}
	//c->dec_ctx->thread_type = FF_THREAD_SLICE;
	if (avcodec_open2(c->dec_ctx, dec, &opts) < 0) {
		libffmpeg_setmsg("failed to open %s codec", av_get_media_type_string(type));
		goto error;
	}

	c->st_time_base = st->time_base;
	c->dec_time_base = st->codec->time_base;

	if (c->dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		memset(&r_frame_rate, 0, sizeof(r_frame_rate));
		r_frame_rate = av_stream_get_r_frame_rate(st);

		c->pix_fmt = c->dec_ctx->pix_fmt;

		c->width =  c->dec_ctx->width;
		c->height =  c->dec_ctx->height;

		fflog(p->log, "wh:%dx%d, coded_wh:%dx%d"
			, c->width, c->height
			, c->dec_ctx->coded_width, c->dec_ctx->coded_height);

		if (c->dec_ctx->coded_width > 0 && c->dec_ctx->coded_height > 0) {
			c->width = c->dec_ctx->coded_width;
			c->height = c->dec_ctx->coded_height;
		}

		fflog(p->log, "fps1 %d %d = %f", st->avg_frame_rate.num, st->avg_frame_rate.den, st->avg_frame_rate.num/(double)st->avg_frame_rate.den);
		fflog(p->log, "fps2 %d %d = %f", r_frame_rate.num, r_frame_rate.den, r_frame_rate.num/(double)r_frame_rate.den);

		if (r_frame_rate.den && r_frame_rate.num) {
			c->fps_num = st->r_frame_rate.num;
			c->fps_den = st->r_frame_rate.den;
		} else if (st->avg_frame_rate.den && st->avg_frame_rate.num) {
			//c->fps = (int)(av_q2d(st->avg_frame_rate) + 0.5f);
			c->fps_num = st->avg_frame_rate.num;
			c->fps_den = st->avg_frame_rate.den;
		} else {
			libffmpeg_setmsg("failed to get frame rate");
			goto error;
		}

		c->fps_f = c->fps_num / (double)c->fps_den;
	} else if (c->dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		c->sample_fmt = c->dec_ctx->sample_fmt;
		c->channel_layout = c->dec_ctx->channel_layout;
		c->channels = c->dec_ctx->channels;
		c->sample_rate = c->dec_ctx->sample_rate;
	}

	return 0;
error:
	ff_codec_close(c);
	return -1;
}

static int get_swscale_method(char *str)
{
	unsigned int i;
	static const char *strary[] = {"sws_fast_bilinear", "sws_bilinear", "sws_bicubic"
		, "sws_x", "sws_point", "sws_area", "sws_bicublin", "sws_gauss"
		, "sws_sinc", "sws_lanczos"};
	static int intval[] = {
		SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_BICUBIC
		, SWS_X, SWS_POINT, SWS_AREA, SWS_BICUBLIN, SWS_GAUSS
		, SWS_SINC, SWS_LANCZOS
	};

	for (i=0; i<sizeof(strary)/sizeof(strary[0]); i++) {
		if (strcmp(strary[i], str) == 0) {
			//printf("OK %s, %08X\n", str, intval[i]);
			return intval[i];
		}
	}
	return SWS_FAST_BILINEAR;
}

static int ff_codec_init_parser(libffmpeg_data *p)
{
	AVPacket newpkt;
	int with_annex_b = 0;
	int ret;
	double pts;

	int found_sps = 0;
	int found_pps = 0;
	int found_aud = 0;
	int nalu_type;

	uint8_t* data;
	int size;
	int len;

	while(1) {
		if (av_read_frame(p->fmt_ctx, &p->pkt) < 0) {
			libffmpeg_setmsg("END of file");
			goto error;
		}

		if (p->pkt.stream_index != p->vid.idx) {
			continue;
		}

		memset(&newpkt, 0, sizeof(newpkt));

		if ((ret = av_bitstream_filter_filter(p->vid.h264_bsfc,
			p->vid.dec_ctx, "dont care",
			&(newpkt.data), &(newpkt.size),
			p->pkt.data, p->pkt.size, 0)) > 0) {
			with_annex_b = 1;
		} else {
			newpkt.data = p->pkt.data;
			newpkt.size = p->pkt.size;
		}

		ff_prt_hex(newpkt.data, newpkt.size, newpkt.size);

		len = av_parser_parse2(p->vid.parser, p->vid.dec_ctx, &data, &size
			,newpkt.data, newpkt.size, newpkt.pts, newpkt.dts, newpkt.pos);

		fflog(p->log, "len:%d data:%p size:%d newpkt.data:%p newpkt.size:%d"
			, len, data, size, newpkt.data, newpkt.size);
		if (with_annex_b) {
			av_free(newpkt.data);
		}

		if (size == 0 && len > 0) {
			fflog(p->log, "need more bytes");
			continue;
		}

		fflog(p->log, "coded_width:%d coded_height:%d width:%d height:%d"
			, p->vid.parser->coded_width, p->vid.parser->coded_height
			, p->vid.parser->width , p->vid.parser->height);

		break;
	}

	if (av_seek_frame(p->fmt_ctx, p->vid.idx, 0, AVSEEK_FLAG_BACKWARD) < 0){
		libffmpeg_setmsg("failed to seek begin of file");
		goto error;
	}

	return 0;
error:
	return -1;
}

int EXPORTS MINGWAPI libffmpeg_open(libffmpeg_t *h, libffmpeg_config *cfg, libffmpeg_log log)
{
	libffmpeg_data *p = NULL;
	char tmp[96];

	if (!libffmpeg_init) {
		av_register_all();
		libffmpeg_init = 1;
	}

	if ((p = (libffmpeg_data*)calloc(1, sizeof(libffmpeg_data))) == NULL) {
		libffmpeg_setmsg("failed to calloc libffmpeg_data");
		goto error;
	}

	p->log = log;
	p->cfg = cfg;

	/* open input file, and allocate format context */
	if (avformat_open_input(&p->fmt_ctx, cfg->file_name, NULL, NULL) < 0) {
		libffmpeg_setmsg("failed to open '%s'", cfg->file_name);
		goto error;
	}

	/* retrieve stream information */
	if (avformat_find_stream_info(p->fmt_ctx, NULL) < 0) {
		libffmpeg_setmsg("failed to avformat_find_stream_info '%s'", cfg->file_name);
		goto error;
	}

	/* hardcode hack flac, ape, mp3, aac
	 * due to some file may include part of mjpeg video. this will cause
	 * transcoder wrong */
	if (strcmp(cfg->file_name + strlen(cfg->file_name) - 4, "flac") == 0
		|| strcmp(cfg->file_name + strlen(cfg->file_name) - 3, "ape") == 0
		|| strcmp(cfg->file_name + strlen(cfg->file_name) - 3, "mp3") == 0
		|| strcmp(cfg->file_name + strlen(cfg->file_name) - 3, "aac") == 0
		|| strcmp(cfg->file_name + strlen(cfg->file_name) - 3, "m4a") == 0
		|| strcmp(cfg->file_name + strlen(cfg->file_name) - 3, "wav") == 0
		|| strcmp(cfg->file_name + strlen(cfg->file_name) - 3, "wma") == 0
		) {
		cfg->video_type = 0;
		goto openaudio;
	}

	if (ff_codec_open(p, AVMEDIA_TYPE_VIDEO, p->fmt_ctx, &p->vid, cfg->decode_threads) == 0) {
		cfg->width = p->vid.width;
		cfg->height = p->vid.height;
		cfg->fps_num = p->vid.fps_num;
		cfg->fps_den = p->vid.fps_den;
		p->is_vid_first_frame = 1;

		//without decode h264 frame
		if (cfg->disable_decode_h264 
			&& p->vid.dec_ctx->codec_id == AV_CODEC_ID_H264
			&& p->vid.pix_fmt == AV_PIX_FMT_YUV420P) {
			cfg->video_type = FIREFLY_TYPE_X264;

			if ((p->vid_outbuf = firefly_frame_video_calloc(
				(enum FIREFLY_TYPE) cfg->video_type, DEFAULT_VIDEO_TRACK_ID
				, cfg->width, cfg->height, cfg->fps_num, cfg->fps_den, 1)) == NULL) {
				libffmpeg_setmsg("failed to calloc video frame %dx%d", cfg->width, cfg->height);
				goto error;
			}

			// H264 NALU Annex-B*/
			if ((p->vid.h264_bsfc = av_bitstream_filter_init("h264_mp4toannexb")) == NULL) {
				libffmpeg_setmsg("failed to init filter of h264_mp4toannexb");
				goto error;
			}
#if 1
		} else if (cfg->disable_decode_h264 
			&& p->vid.dec_ctx->codec_id == AV_CODEC_ID_HEVC
			&& (p->vid.pix_fmt == AV_PIX_FMT_YUV420P )) {
			cfg->video_type = FIREFLY_TYPE_X265;

			if ((p->vid_outbuf = firefly_frame_video_calloc(
				(enum FIREFLY_TYPE) cfg->video_type, DEFAULT_VIDEO_TRACK_ID
				, cfg->width, cfg->height, cfg->fps_num, cfg->fps_den, 1)) == NULL) {
				libffmpeg_setmsg("failed to calloc video frame %dx%d", cfg->width, cfg->height);
				goto error;
			}

			// HEVC NALU Annex-B*/
			if ((p->vid.h264_bsfc = av_bitstream_filter_init("hevc_mp4toannexb")) == NULL) {
				libffmpeg_setmsg("failed to init filter of hevc_mp4toannexb");
				goto error;
			}

			fflog(p->log, "p->vid.h264_bsfc: %p", p->vid.h264_bsfc);
#endif
		} else {
			cfg->disable_decode_h264 = 0;
			cfg->video_type = FIREFLY_TYPE_YUV420P;

			//only cvrt when FMT != YUV420
			if (p->vid.pix_fmt != AV_PIX_FMT_YUV420P) {
				if ((p->vid_cvrt = sws_getContext(
					cfg->width, cfg->height
					, p->vid.pix_fmt
					, cfg->width, cfg->height
					, AV_PIX_FMT_YUV420P
					, SWS_POINT, NULL, NULL, NULL)) == NULL) {
					libffmpeg_setmsg("failed to sws_getContext");
					goto error;
				}
			}

			//force output to YUV420P
			if ((p->vid_outbuf = firefly_frame_video_calloc(
				FIREFLY_TYPE_YUV420P, DEFAULT_VIDEO_TRACK_ID
				, cfg->width, cfg->height, cfg->fps_num, cfg->fps_den, 1)) == NULL) {
				libffmpeg_setmsg("failed to calloc video frame %dx%d", cfg->width, cfg->height);
				goto error;
			}
		}
#if 0
		//only for HW decoding
		if (p->vid.h264_bsfc) {
			if ((p->vid.parser = av_parser_init(p->vid.dec_ctx->codec_id)) == NULL) {
				libffmpeg_setmsg("failed to av_parser_init %d ", p->vid.dec_ctx->codec_id);
				goto error;
			}
		}
#endif
	}

openaudio:
	if (ff_codec_open(p, AVMEDIA_TYPE_AUDIO, p->fmt_ctx, &p->aud, cfg->decode_threads) == 0) {
		cfg->audio_type = FIREFLY_TYPE_PCM;
		cfg->sample_rate = p->sample_rate = 44100;
		cfg->bit_rate = 16;
		cfg->channels = 2;
		
		//init aud tmp buf
		p->frame_size = cfg->bit_rate / 8 * cfg->channels;

		if ((p->aud_buf = ff_buffer_new(1)) == NULL) {
			libffmpeg_setmsg("failed to swr_alloc '%s'", cfg->file_name);
			goto error;
		}

		if ((p->aud_cvrt = swr_alloc()) == NULL) {
			libffmpeg_setmsg("failed to swr_alloc '%s'", cfg->file_name);
			goto error;
		}

		if (p->aud.channel_layout == 0) {
			p->aud.channel_layout = AV_CH_LAYOUT_STEREO;
		}

		av_opt_set_channel_layout(p->aud_cvrt, "in_channel_layout", p->aud.channel_layout, 0);
		av_opt_set_channel_layout(p->aud_cvrt, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
		av_opt_set_int(p->aud_cvrt, "in_sample_rate", p->aud.sample_rate, 0);
		av_opt_set_int(p->aud_cvrt, "out_sample_rate", cfg->sample_rate, 0);
		av_opt_set_sample_fmt(p->aud_cvrt, "in_sample_fmt", p->aud.sample_fmt, 0);
		av_opt_set_sample_fmt(p->aud_cvrt, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

		if (swr_init(p->aud_cvrt)) {
			libffmpeg_setmsg("failed to swr_init '%s'", cfg->file_name);
			goto noaudio;
		}

		/** alloc output audio frame */
		if ((p->aud_outbuf = firefly_frame_audio_calloc(
			FIREFLY_TYPE_PCM, DEFAULT_AUDIO_TRACK_ID, cfg->channels
			, cfg->sample_rate, cfg->bit_rate, cfg->sample_rate*50)) == NULL) {
			libffmpeg_setmsg( "failed to firefly_frame_audio_calloc '%s'", cfg->file_name);
			goto noaudio;
		}

		goto finally;
noaudio:
		cfg->audio_type = 0;
	}

finally:
	cfg->duration = p->duration = p->fmt_ctx->duration / (double) AV_TIME_BASE;
	//av_dump_format(p->fmt_ctx, 0, cfg->file_name, 0);

	if (!p->vid.dec_ctx && !p->aud.dec_ctx) {
		libffmpeg_setmsg("failed to find video or audio stream '%s'", cfg->file_name);
		goto error;
	}

	if ((p->frame = av_frame_alloc()) == NULL) {
		libffmpeg_setmsg("failed to av_frame_alloc '%s'", cfg->file_name);
		goto error;
	}

	if (cfg->video_type) {
		fflog(p->log, "%s %dx%d,%s,%f"
			, p->vid.dec_ctx->codec->name, p->vid.width, p->vid.height, av_get_pix_fmt_name(p->vid.pix_fmt)
			, (double)p->vid.fps_num / (double)p->vid.fps_den);
	} else if (cfg->video_type == 0) {
		cfg->audio_only = 1;
	}

	if (cfg->audio_type) {
		av_get_channel_layout_string(tmp, sizeof(tmp), p->aud.channels, p->aud.channel_layout);
		fflog(p->log, "%s,%s,%d,%d"
			, p->aud.dec_ctx->codec->name, av_get_sample_fmt_name(p->aud.sample_fmt), tmp, p->aud.channels, p->aud.sample_rate);
	}

	av_init_packet(&p->pkt);
	p->pkt.data = NULL;
	p->pkt.size = 0;

#if 0
	//init parser, read first frame to parse coded_width, codec_height from NALU
	if (ff_codec_init_parser(p)) {
		goto error;
	}
#endif

	*h = p;
	return 0;
error:
	libffmpeg_close(p);
	return -1;
}

int ff_get_nalu_type2(uint8_t *buf, int size, int *found_sps, int *found_pps, int *found_aud)
{
	static const uint8_t start_code_32[4] = {0, 0, 0, 1};
	static const uint8_t start_code_24[3] = {0, 0, 1};
	uint8_t *p, *p_end;
	int start_code_len = 0;
//	char mmsg[4096] = {0};
	int nalu_type = 0;
	int n;
	  
	p = buf;
	p_end = p + size;

	while((p < (p_end - 5))) {
	
		if (memcmp(p, start_code_24, 3) == 0) {
			start_code_len = 3;
		} else if (memcmp(p, start_code_32, 4) == 0) {
			start_code_len = 4;
		} else {
			p++;
			continue;
		}

		p += start_code_len;
	//	sprintf(mmsg+strlen(mmsg), "%02X/%d ", p[0], p[0] & 0x1F);

		n = p[0] & 0x1F;
		switch(n) {
			case NAL_SLICE:
			case NAL_IDR_SLICE:
				nalu_type = n;
				goto finally;
			case NAL_SPS:
				*found_sps = 1;
				break;
			case NAL_PPS:
				*found_pps = 1;
				break;
			case NAL_AUD:
				*found_aud = 1;
				break;
		}
	}

finally:
	//fflog("size:%d nalu:%s", size, mmsg);	
	return nalu_type;
}


static int ff_read_h264_video(libffmpeg_data *p, int stream_idx, firefly_buffer *outbuf)
{
	AVPacket newpkt;
	int with_annex_b = 0;
	int ret;
	double pts;

	int found_sps = 0;
	int found_pps = 0;
	int found_aud = 0;
	int nalu_type;
#if 0
	{
		double dts2 = 0.0f, pts2 = 0.0f;
		if (p->pkt.pts != AV_NOPTS_VALUE) {
			pts2 = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
			fprintf(stderr, "pts:%"PRId64" %.4f \n", p->pkt.pts, pts2);
		} else {
			fprintf(stderr, "pts:NOPTS \n");
		}

		if (p->pkt.dts != AV_NOPTS_VALUE) {
			dts2 = (p->pkt.dts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
			fprintf(stderr,"dts:%"PRId64" %.4f ",  p->pkt.dts, dts2);
		} else {
			fprintf(stderr,"dts:NOPTS ");
		}
			
		fprintf(stderr,"size1:%d ", p->pkt.size);
	}
#endif

	memset(&newpkt, 0, sizeof(newpkt));
	if ((ret = av_bitstream_filter_filter(p->vid.h264_bsfc,
		p->vid.dec_ctx, "dont care",
		&(newpkt.data), &(newpkt.size),
		p->pkt.data, p->pkt.size, 0)) > 0) {
		with_annex_b = 1;
	} else {
		newpkt.data = p->pkt.data;
		newpkt.size = p->pkt.size;
	}

//	printf("newpkt.size :%d\n", newpkt.size);
//	fflush(stdout);

	if (p->cfg->enable_aud) {
		nalu_type = ff_get_nalu_type2(newpkt.data, newpkt.size, &found_sps, &found_pps, &found_aud);
		outbuf->buf_size = 0;

		if (!found_aud) {
			static uint8_t aud_nal[] = { 0x00, 0x00, 0x00, 0x01, 0x09, 0xf0 };
			memcpy(outbuf->buf, aud_nal, sizeof(aud_nal));
			outbuf->buf_size = sizeof(aud_nal);
		}

		if (nalu_type == NAL_IDR_SLICE) {
			outbuf->header.frame_type = FIREFLY_FRAME_TYPE_I;
		} else {
			outbuf->header.frame_type = FIREFLY_FRAME_TYPE_P;
		}

		memcpy(outbuf->buf+outbuf->buf_size, newpkt.data, newpkt.size);
		outbuf->buf_size += newpkt.size;
	} else {
		memcpy(outbuf->buf, newpkt.data, newpkt.size);
		outbuf->buf_size = newpkt.size;
	}

	if (with_annex_b) {
		av_free(newpkt.data);
	}

	if (p->no_vid_pts) {
		outbuf->header.pts = p->vid_dts;
		pts = firefly_video_pts(outbuf);
	} else {
		pts = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
		outbuf->header.pts = ff_video_ts_mapping(pts, p->vid.fps_f)/* - outbuf->header.offset_pts*/;
	}

	outbuf->header.dts = p->vid_dts;

	if (double_equals(firefly_video_pts(outbuf), pts, 0.001) == 0) {
#if 1
		fflog(p->log,"[VID] org_pts:%f != new_pts:%f", pts, firefly_video_pts(outbuf));
		fflog(p->log,"\t%f x %f = %d", pts, p->vid.fps_f, (int)(pts*p->vid.fps_f));
		fflog(p->log,"\t%d / %f = %f"
			, (int)(pts*p->vid.fps_f)
			, p->vid.fps_f
			, firefly_video_pts(outbuf));
#endif
	}
	return 1;
}

static int ff_decode_video(libffmpeg_data *p, int stream_idx, firefly_buffer *outbuf)
{
	int ret;
	int got_frame = 0;
	double pts;

	uint8_t **in_vid_plane;
	int32_t *in_vid_stride;

#if 0
	{
		double dts2 = 0.0f, pts2 = 0.0f;
		if (p->pkt.pts != AV_NOPTS_VALUE) {
			pts2 = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
			fprintf(stderr, "pts:%"PRId64" %.4f \n", p->pkt.pts, pts2);
		} else {
			fprintf(stderr, "pts:NOPTS \n");
		}

		if (p->pkt.dts != AV_NOPTS_VALUE) {
			dts2 = (p->pkt.dts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
			printf("dts:%"PRId64" %.4f ",  p->pkt.dts, dts2);
		} else {
			printf("dts:NOPTS ");
		}
	}
#endif

	if ((ret = avcodec_decode_video2(p->fmt_ctx->streams[stream_idx]->codec, p->frame, &got_frame, &p->pkt)) < 0) {
		if (p->video_failed_decode_count++ >= FAILED_VIDEO_DECODE_COUNT) {
			libffmpeg_setmsg("failed to decode video over %d times", p->video_failed_decode_count);
			goto error;
		}
		//dont care every thing
		goto finally;
	}

	if (!got_frame) {
		printf("\n");
		goto finally;
	}

	in_vid_plane = p->frame->data;
	in_vid_stride = p->frame->linesize;
	
	p->video_failed_decode_count = 0;
	pts = av_frame_get_best_effort_timestamp(p->frame) * av_q2d(p->fmt_ctx->streams[stream_idx]->codec->time_base);
	//pts2 = (p->frame->pkt_pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
	//dts2 = (p->frame->pkt_pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
	//printf("avframe pts:%f pkt_pts:%"PRId64" %.4f pkt_dts:%"PRId64" %.4f\n", pts, p->frame->pkt_pts, pts2, p->frame->pkt_dts, dts2);

	outbuf->header.pts = ff_video_ts_mapping(pts, p->vid.fps_f)/* - outbuf->header.offset_pts*/;
	outbuf->header.dts = ff_video_ts_mapping(p->vid_dts, p->vid.fps_f);

	if (double_equals(firefly_video_pts(outbuf), pts, 0.001) == 0) {
#if 1
		fflog(p->log,"[VID] org_pts:%f != new_pts:%f", pts, firefly_video_pts(outbuf));
		fflog(p->log,"\t%f x %f = %d", pts, p->vid.fps_f, (int)(pts*p->vid.fps_f));
		fflog(p->log,"\t%d / %f = %f"
			, (int)(pts*p->vid.fps_f)
			, p->vid.fps_f
			, firefly_video_pts(outbuf));
#endif
	}

	if (p->vid_cvrt) {
		sws_scale(p->vid_cvrt
			, (const uint8_t **)in_vid_plane, in_vid_stride
			, 0, p->vid.height
			, outbuf->plane, outbuf->stride);
	} else {
		av_image_copy(
			outbuf->plane, outbuf->stride,
			(const uint8_t **)(in_vid_plane), in_vid_stride,
			p->vid.pix_fmt, p->vid.width, p->vid.height);
	}

	return 1;
finally:
	return 0;
error:
	return -1;
}

static int ff_decode_audio_flush_pcm(libffmpeg_data *p
	, firefly_buffer *outbuf)
{
	int out_samples = 0;

	//flush over 1024 part of audio PCM buffer
	if (p->num_frame > 0) {
		out_samples = p->num_frame;

		ff_buffer_pop(p->aud_buf, (char *)outbuf->buf, ff_buffer_data_size(p->aud_buf));
		outbuf->header.pts = p->pts;

		if (out_samples <= MAX_AAC_FRAME_SIZE) {
			outbuf->header.num_frame = out_samples;
			outbuf->buf_size = outbuf->header.num_frame * p->frame_size;
			p->num_frame = 0;
		} else {
			outbuf->header.num_frame = MAX_AAC_FRAME_SIZE;
			outbuf->buf_size = outbuf->header.num_frame * p->frame_size;

			ff_buffer_push(p->aud_buf, (char*)outbuf->buf + outbuf->buf_size, (out_samples - MAX_AAC_FRAME_SIZE)*p->frame_size);

			p->num_frame = out_samples - MAX_AAC_FRAME_SIZE;
			p->pts = outbuf->header.pts + MAX_AAC_FRAME_SIZE;
		}

		//printf("flush %d remain %d\n", *num_frame, p->num_frame);
		return 1;
	}

	return 0;
}

static int ff_decode_audio(libffmpeg_data *p, int stream_idx, firefly_buffer *outbuf)
{
	int ret;
	int got_frame = 0;
	double pts;
	int in_samples = 0;
	int out_samples = 0;
	int64_t org_pts, org_dts;
	uint8_t *aud_buf = outbuf->buf + outbuf->buf_size;

	org_pts = p->pkt.pts;
	org_dts = p->pkt.dts;

	av_packet_rescale_ts(&p->pkt
		, p->fmt_ctx->streams[stream_idx]->time_base
		, p->fmt_ctx->streams[stream_idx]->codec->time_base);
	
	if ((ret = avcodec_decode_audio4(p->fmt_ctx->streams[stream_idx]->codec, p->frame, &got_frame, &p->pkt)) < 0) {
		goto finally;
	}

	if (!got_frame) {
		goto finally;
	}

	pts = av_frame_get_best_effort_timestamp(p->frame) * av_q2d(p->fmt_ctx->streams[stream_idx]->codec->time_base);
	p->audio_failed_decode_count = 0;
	in_samples = p->frame->nb_samples;
	out_samples = (int)av_rescale_rnd(
					swr_get_delay(p->aud_cvrt, p->aud.sample_rate) + in_samples
					, p->sample_rate
					, p->aud.sample_rate, AV_ROUND_UP);

	/*printf("in samples: %d, out sample:%d, ret:%d, pkt.size:%d pts:%f\n"
	, in_samples, out_samples
	, ret, p->pkt.size, pts);*/

	out_samples = swr_convert(
		p->aud_cvrt, &aud_buf
		, out_samples, (const uint8_t **)p->frame->extended_data, in_samples);

	outbuf->header.pts = ff_audio_ts_mapping(pts, p->sample_rate)/* - outbuf->header.offset_pts*/;
	outbuf->header.num_frame += out_samples;
	outbuf->buf_size = outbuf->header.num_frame * p->frame_size;

	//printf("[%d] aud pts: %f, num_frame:%d\n", *cc, pts, *num_frame);

	if (double_equals(firefly_audio_pts(outbuf), pts, 0.001) == 0) {
		fflog(p->log, "[AUD] org_pts:%f != new_pts:%f", pts, firefly_audio_pts(outbuf));
		fflog(p->log, "\t%f x %d = %d", pts, p->aud.sample_rate, (int)(pts*p->aud.sample_rate));
		fflog(p->log, "\t%"PRId64 "/ %d = %f"
			, outbuf->header.pts
			, p->sample_rate
			, outbuf->header.pts / (double)p->sample_rate);
	}

finally:
	p->pkt.pts = org_pts;
	p->pkt.dts = org_dts;
	return ret;
}

static int ff_decode_audio_split(libffmpeg_data *p, firefly_buffer *outbuf)
{
	int max_frame_size = MAX_AAC_FRAME_SIZE * p->frame_size;
	int remain_frame = outbuf->header.num_frame - MAX_AAC_FRAME_SIZE;
	int remain_frame_size = remain_frame * p->frame_size;

	if (outbuf->header.num_frame <= MAX_AAC_FRAME_SIZE) {
		return 0;
	}

	outbuf->header.num_frame = MAX_AAC_FRAME_SIZE;
	outbuf->buf_size = max_frame_size;

	ff_buffer_push(p->aud_buf, (char*)outbuf->buf + max_frame_size, remain_frame_size);
	p->num_frame = remain_frame;
	p->pts = outbuf->header.pts + MAX_AAC_FRAME_SIZE;
	return 0;
}

int EXPORTS MINGWAPI libffmpeg_decode(libffmpeg_t h, firefly_buffer **in)/*libffmpeg_dec *dec)*/
{
	libffmpeg_data *p = (libffmpeg_data *)h;
	int stream_idx = -1;
	int ret;
	double dts;
	
	*in = NULL;

	if (p->aud.dec_ctx) {
		if (ff_decode_audio_flush_pcm(p, p->aud_outbuf) == 1) {
			/** got aud frame */
			*in = p->aud_outbuf;
			goto finally;
		}
	}

	if (av_read_frame(p->fmt_ctx, &p->pkt) < 0) {
		libffmpeg_setmsg("END of file");
		goto error;
	}

	//let av_frame_get_best_effort_timestamp to guess pts and dts
	/*if (p->pkt.dts == AV_NOPTS_VALUE && p->pkt.pts == AV_NOPTS_VALUE) {
		libffmpeg_setmsg("no pts and dts at all");
		goto error;
	}*/

	stream_idx = p->pkt.stream_index;

	if (p->vid.dec_ctx && (stream_idx == p->vid.idx)) {

		//rescale
		av_packet_rescale_ts(&p->pkt
			, p->fmt_ctx->streams[stream_idx]->time_base
			, p->fmt_ctx->streams[stream_idx]->codec->time_base);

		//record first frame as DTS start, because first Frame is I frame
		if (p->is_vid_first_frame) {
			if (p->no_vid_pts || p->pkt.pts == AV_NOPTS_VALUE) {
				p->vid_dts = ff_video_ts_mapping(p->seek_secs, p->vid.fps_f);
				if (p->no_vid_pts == 0) {
					fflog(p->log, "NOPTS FAKE DTS/PTS %"PRId64, p->vid_dts);
				}
				p->no_vid_pts = 1;
			} else {
				dts = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
				p->vid_dts = ff_video_ts_mapping(dts, p->vid.fps_f);
			}
			p->is_vid_first_frame = 0;
			fflog(p->log, "FIRST DTS: %"PRId64, p->vid_dts);
		}

		if (p->vid.h264_bsfc) {
			ret =ff_read_h264_video(p, stream_idx, p->vid_outbuf);
		} else {
			ret = ff_decode_video(p, stream_idx, p->vid_outbuf);
		}

		p->vid_dts++;

		if (ret < 0) {
			goto error;
		} else if (ret == 1) {
			*in = p->vid_outbuf;
		}
	}else if (p->aud.dec_ctx &&  stream_idx == p->aud.idx) {
		p->aud_outbuf->header.num_frame = 0;
		p->aud_outbuf->buf_size = 0;
		
		/** ugly hack */
		if (p->pkt.size > 8820000) {
			libffmpeg_setmsg("audio buffer %d overflow", p->pkt.size);
			goto error;
		}

		while (p->pkt.size > 0) {
			if ((ret = ff_decode_audio(p, stream_idx, p->aud_outbuf)) < 0) {
				if (p->audio_failed_decode_count++ >= FAILED_AUDIO_DECODE_COUNT) {
					libffmpeg_setmsg("failed to decode audio over %d times", p->audio_failed_decode_count);
					goto error;
				}
				goto finally;
			}
			p->audio_failed_decode_count = 0;
			p->pkt.data += ret;
			p->pkt.size -= ret;
		}

		//push audio frame to stack that over 1024
		ff_decode_audio_split(p, p->aud_outbuf);
	
		if (p->aud_outbuf->header.num_frame > 0) {
			*in = p->aud_outbuf;
		}
	}

finally:
	if (p->pkt.data /*&& p->pkt.size > 0*/) {
		av_free_packet(&p->pkt);
	}
	p->pkt.data = NULL;
	p->pkt.size = 0;
	return 0;
error:
	if (p->pkt.data /*&& p->pkt.size > 0*/) {
		av_free_packet(&p->pkt);
	}
	p->pkt.data = NULL;
	p->pkt.size = 0;
	return -1;
}

int EXPORTS MINGWAPI libffmpeg_seek(libffmpeg_t h, void *p1, void *p2)
{
	libffmpeg_data *p = (libffmpeg_data *)h;
	double secs = *((double *)p1);
	int64_t seek_base = 0;
	AVRational avr;
	
	avr.num = 1;
	avr.den = AV_TIME_BASE;

	seek_base = av_rescale_q(
		(int64_t)(secs * AV_TIME_BASE)
		, avr, p->fmt_ctx->streams[p->vid.idx]->time_base);

	if (av_seek_frame(p->fmt_ctx, p->vid.idx, seek_base, AVSEEK_FLAG_BACKWARD) < 0){
		libffmpeg_setmsg("failed to seek pos %.3f", secs);
		goto error;
	}

	p->is_vid_first_frame = 1;
	p->vid_dts = 0;
	p->num_frame = 0;
	p->seek_secs = secs;
	p->no_vid_pts = 0;
	ff_buffer_reset(p->aud_buf);
	return 0;
error:
	return -1;
}

int EXPORTS MINGWAPI libffmpeg_set_video_offset(libffmpeg_t h, int64_t video_offset)
{
	libffmpeg_data *p = (libffmpeg_data *)h;	
	//20160413: I have no idead why offset_pts exists, may be MFT H264 Encoder needed?
	//20160422: offset_pts now exists in vod_play_transcode.c, vid/aud_skip_comp_dts
	//if (p->vid_outbuf) {
	//	p->vid_outbuf->header.pts = 0;
	//	p->vid_outbuf->header.offset_pts = video_offset;
	//}
	return 0;
}

int EXPORTS MINGWAPI libffmpeg_set_audio_offset(libffmpeg_t h, int64_t audio_offset)
{
	libffmpeg_data *p = (libffmpeg_data *)h;
	//20160413: I have no idead why offset_pts exists, may be MFT H264 Encoder needed?
	//20160422: offset_pts now exists in vod_play_transcode.c, vid/aud_skip_comp_dts
	//if (p->aud_outbuf) {
	//	p->aud_outbuf->header.pts = 0;
	//	p->aud_outbuf->header.offset_pts = audio_offset;
	//}
	return 0;
}

int EXPORTS MINGWAPI libffmpeg_reset(libffmpeg_t h)
{
	libffmpeg_data *p = (libffmpeg_data *)h;

	if (!p->vid.h264_bsfc && p->vid.dec_ctx) {
		avcodec_flush_buffers(p->vid.dec_ctx);
	}

	if (p->aud.dec_ctx) {
		avcodec_flush_buffers(p->aud.dec_ctx);
	}

	p->num_frame = 0;
	ff_buffer_reset(p->aud_buf);
	return 0;
}

int EXPORTS MINGWAPI libffmpeg_close(libffmpeg_t h)
{
	libffmpeg_data *p = (libffmpeg_data *)h;

	if (!p) {
		return -1;
	}

	//video
	ff_codec_close(&p->vid);

	if (p->vid_cvrt) {
		sws_freeContext(p->vid_cvrt);
	}

	//audio
	ff_codec_close(&p->aud);

	if (p->aud_buf) {
		ff_buffer_free(p->aud_buf);
	}

	if (p->aud_cvrt) {
		swr_free(&p->aud_cvrt);
	}

	//free frame
	if (p->frame) {
		av_frame_free(&p->frame);
	}

	//fmt
	if (p->fmt_ctx) {
		avformat_close_input(&p->fmt_ctx);
	}

	if (p->vid_outbuf) {
		firefly_frame_free(p->vid_outbuf);
	}

	if (p->aud_outbuf) {
		firefly_frame_free(p->aud_outbuf);
	}

	free(p);
	return 0;
}

void EXPORTS MINGWAPI libffmpeg_get_msg(char *msg)
{
	sprintf(msg, "%s", g_szMsg);
}

void libffmpeg_setmsg2(const char *file, int line, const char *fmt, ...)
{
	va_list args;
	sprintf(g_szMsg, "%s(%d): ", file, line);
	va_start(args, fmt);
	vsprintf(strlen(g_szMsg) + g_szMsg, fmt, args);
	va_end(args);
}

int EXPORTS MINGWAPI libffmpeg_video_scaling(
	int in_type, int in_width, int in_height, uint8_t **in_plane, int32_t *in_stride
	, int out_type, int out_width, int out_height, uint8_t **out_plane, int32_t *out_stride) {

	struct SwsContext *sws;
	
	if ((sws = sws_getContext(
		in_width, in_height
		, AV_PIX_FMT_YUV420P
		, out_width, out_height
		, AV_PIX_FMT_YUV420P
		, SWS_POINT/*SWS_BILINEAR*/, NULL, NULL, NULL)) == NULL) {
		libffmpeg_setmsg("failed to sws_getContext");
		goto error;
	}

	sws_scale(sws
		, (const uint8_t * const*) in_plane
		, in_stride
		, 0, in_height
		, out_plane
		, out_stride);

	sws_freeContext(sws);
	return 0;
error:
	return -1;
}

char _ffmsg[DBG_MSG_LEN];

void fflog2(const char *file, int line, libffmpeg_log log, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (file && line) {
		sprintf(_ffmsg, "%s(%d): ", file, line);
		vsprintf(strlen(_ffmsg)+_ffmsg, fmt, args);
	} else {
		vsprintf(_ffmsg, fmt, args);
	}
	va_end(args);
	log(_ffmsg);
}


typedef struct {
	libffmpeg_img_scale_config *cfg;
	struct SwsContext *sws;
	firefly_buffer *outbuf;
	libffmpeg_log log;
} libffmpeg_img_scale_data;

static int firefly_type_to_ffmpeg(int type)
{
	switch(type) {
		case FIREFLY_TYPE_RGB32:
			return AV_PIX_FMT_BGR32;
		case FIREFLY_TYPE_YUV420P:
			return AV_PIX_FMT_YUV420P;
		default:
			libffmpeg_setmsg("failed to firefly_type_to_ffmpeg: %d", type);
	}
	return -1;
}

static int ffmpeg_type_to_firefly(int type) {
	switch(type) {
		case AV_PIX_FMT_BGR32:
			return FIREFLY_TYPE_RGB32;
		case AV_PIX_FMT_YUV420P:
			return FIREFLY_TYPE_YUV420P;
		default:
			libffmpeg_setmsg("failed to firefly_type_to_ffmpeg: %d", type);
	}
	return -1;
}

int EXPORTS MINGWAPI libffmpeg_img_scale_open(libffmpeg_t *h, libffmpeg_img_scale_config *cfg, libffmpeg_log log)
{
	libffmpeg_img_scale_data *p = NULL;
	int in_type = -1;
	int out_type = -1;
	int out_type_firefly = -1;
	int swscale_method = -1;

	if (cfg->in_type_is_ffmpeg) {
		in_type = cfg->in_type;
	} else {
		if ((in_type = firefly_type_to_ffmpeg(cfg->in_type)) == -1) {
			goto error;
		}
	}

	if (cfg->out_type_is_ffmpeg) {
		out_type = cfg->out_type;
		if ((out_type_firefly = ffmpeg_type_to_firefly(cfg->out_type)) == -1) {
			goto error;
		}
	} else {
		out_type_firefly = cfg->out_type;
		if ((out_type = firefly_type_to_ffmpeg(cfg->out_type)) == -1) {
			goto error;
		}
	}

	swscale_method = get_swscale_method(cfg->swscale_method);

	if (!libffmpeg_init) {
		av_register_all();
		libffmpeg_init = 1;
	}

	if ((p = (libffmpeg_img_scale_data*)calloc(1, sizeof(libffmpeg_img_scale_data))) == NULL) {
		libffmpeg_setmsg("failed to calloc libffmpeg_img_scale_data");
		goto error;
	}

	p->log = log;
	p->cfg = cfg;

	/** alloc output video frame */
	if ((p->outbuf = firefly_frame_video_calloc(
		(enum FIREFLY_TYPE) out_type_firefly, DEFAULT_VIDEO_TRACK_ID
		, cfg->out_width, cfg->out_height, 30, 1, 1)) == NULL) {
		libffmpeg_setmsg("failed to calloc video frame %dx%d", cfg->out_width, cfg->out_height);
		goto error;
	}


	fflog(p->log, "swscale method: %d %s", swscale_method, cfg->swscale_method);

	if ((p->sws = sws_getContext(
		cfg->in_width, cfg->in_height
		, in_type
		, cfg->out_width, cfg->out_height
		, out_type
		, swscale_method, NULL, NULL, NULL)) == NULL) {
		libffmpeg_setmsg("failed to sws_getContext");
		goto error;
	}

	*h = p;
	return 0;
error:
	libffmpeg_img_scale_close(p);
	return -1;
}

int EXPORTS MINGWAPI libffmpeg_img_scale_scaling(libffmpeg_t h, firefly_buffer *in, firefly_buffer **out)
{
	libffmpeg_img_scale_data *p = (libffmpeg_img_scale_data *) h;
	//int type = p->outbuf->header.type; 

	*out = NULL;

	sws_scale(p->sws
		, (const uint8_t * const*) in->plane
		, in->stride
		, 0, in->header.height
		, p->outbuf->plane
		,  p->outbuf->stride);

	p->outbuf->header.pts_in_real_time = in->header.pts_in_real_time;
	p->outbuf->header.dts = in->header.dts;
	p->outbuf->header.pts = in->header.pts;
	p->outbuf->header.frame_counter = in->header.frame_counter;
	p->outbuf->header.offset_pts = in->header.offset_pts;
	p->outbuf->header.fps_f = in->header.fps_f;
	p->outbuf->header.fps_num = in->header.fps_num;
	p->outbuf->header.fps_den = in->header.fps_den;
	p->outbuf->header.bit_per_sample = in->header.bit_per_sample;
	*out = p->outbuf;
	return 0;
}

int EXPORTS MINGWAPI libffmpeg_img_scale_close(libffmpeg_t h)
{
	libffmpeg_img_scale_data *p = (libffmpeg_img_scale_data *) h;

	if (!p) {
		return -1;
	}

	if (p->sws) {
		sws_freeContext(p->sws);
	}

	if (p->outbuf) {
		firefly_frame_free(p->outbuf);
	}

	free(p);
	return 0;
}

