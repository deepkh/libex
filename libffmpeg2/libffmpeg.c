/*
* Copyright 2017 NetSync.tv Grey Huang (deepkh@gmail.com) All rights reserved. 
* license: GNU Lesser General Public License v2.1 
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
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/audio_fifo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff_buffer.h"

#ifndef min
#define min(X,Y) (X < Y ? X : Y)
#endif

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
static int libffmpeg_avfilter_init = 0;

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
	int64_t first_vid_dts;
	int no_vid_pts;
	double seek_secs;

	ff_codec aud;
	int audio_failed_decode_count;
	int32_t sample_rate;					//out sample_rate
	SwrContext *aud_cvrt;

	// aud tmp buf
	int is_aud_first_frame;
	int64_t first_aud_pts;
	struct ff_buffer *aud_buf;
	int64_t pts;
	int32_t num_frame;
	int32_t frame_size;
	
	ff_codec sub;

	//for sync video/audio if start PTS are not ZERO
	int64_t init_vid_dts;
	double init_vid_dts_secs;
	double init_vid_dts_secs2;
	int64_t init_aud_pts;
	double init_aud_pts_secs;
	int sync_audio_2;

	firefly_buffer *vid_outbuf;
	firefly_buffer *aud_outbuf;

	libffmpeg_log log;
	char log_str[DBG_MSG_LEN];
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

static int ff_codec_open(libffmpeg_data *p, enum AVMediaType type, AVFormatContext *fmt_ctx, ff_codec *c, int decode_threads, int stream_idx)
{
	int ret;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;
	AVRational r_frame_rate ;

	if (stream_idx == -1) {
		if ((ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0)) <0) {
			libffmpeg_setmsg("Could not find %s stream", av_get_media_type_string(type));
			goto error;
		}
	} else {
		ret = stream_idx;
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

		fflog(p->log, p->log_str, "wh:%dx%d, coded_wh:%dx%d"
			, c->width, c->height
			, c->dec_ctx->coded_width, c->dec_ctx->coded_height);

		if (c->dec_ctx->coded_width > 0 && c->dec_ctx->coded_height > 0) {
#if 0
			c->width = c->dec_ctx->coded_width;
			c->height = c->dec_ctx->coded_height;
#else
			//only for width not align to 16 but not height not align to 16
			if (c->dec_ctx->coded_width != c->width) {
				c->width = MSDK_ALIGN16(c->dec_ctx->coded_width);
				c->height = MSDK_ALIGN16(c->dec_ctx->coded_height);
				fflog(p->log, p->log_str, "aligned wh:%dx%d, ", c->width, c->height);
			}
#endif
		}

		fflog(p->log, p->log_str, "fps1 %d %d = %f", st->avg_frame_rate.num, st->avg_frame_rate.den, st->avg_frame_rate.num/(double)st->avg_frame_rate.den);
		fflog(p->log, p->log_str, "fps2 %d %d = %f", r_frame_rate.num, r_frame_rate.den, r_frame_rate.num/(double)r_frame_rate.den);

		if (r_frame_rate.den && r_frame_rate.num 
			&& !(r_frame_rate.num == 90000 && r_frame_rate.den == 1)) { //workaround for some ambiguous framerate, using avg_frame_rate instead Elecard_about_Tomsk_part3_HEVC_UHD.mp4
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
	} else if (c->dec_ctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
		int stream_len = 0;
		int i;
		stream_len = fmt_ctx->nb_streams;
		fflog(p->log, p->log_str, "*********************** stream_len:%d", stream_len);

		for (i=0; i<stream_len; i++) {
			fflog(p->log, p->log_str, 
				"*********************** codec_type: %d, id:%08X"
				, fmt_ctx->streams[i]->codec->codec_type
				, fmt_ctx->streams[i]->codec->codec_id);
		}
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

//read first video DTS, and first audio PTS
static int ff_codec_probe(libffmpeg_data *p, double origin_secs)
{
	int stream_idx = -1;
	int found_vid_frame = 0;
	int found_aud_frame = 0;
	double pts;

	int ret;
	int got_frame = 0;
	int64_t org_pts, org_dts;

	if (p->vid.dec_ctx == NULL) {
		found_vid_frame = 1;
	}

	if (p->aud.dec_ctx == NULL) {
		found_aud_frame = 1;
	}

	while(found_vid_frame == 0 || found_aud_frame == 0) {
		if (av_read_frame(p->fmt_ctx, &p->pkt) < 0) {
			libffmpeg_setmsg("END of file");
			goto error;
		}
		
		stream_idx = p->pkt.stream_index;

		if (found_vid_frame == 0 && p->vid.dec_ctx && (stream_idx == p->vid.idx)) {
			//rescale
			av_packet_rescale_ts(&p->pkt
				, p->fmt_ctx->streams[stream_idx]->time_base
				, p->fmt_ctx->streams[stream_idx]->codec->time_base);

			if (p->pkt.pts == AV_NOPTS_VALUE) {
				p->init_vid_dts = 0;
				p->init_vid_dts_secs = 0.0f;
				p->init_vid_dts_secs2 = 0.0f;
			} else {
				pts = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
				p->init_vid_dts = ff_video_ts_mapping(pts, p->vid.fps_f);
				p->init_vid_dts_secs = pts;
				p->init_vid_dts_secs2 = pts;
			}

			found_vid_frame = 1;
		} else if (found_aud_frame == 0 && p->aud.dec_ctx &&  stream_idx == p->aud.idx) {

			/** ugly hack */
			if (p->pkt.size > 8820000) {
				libffmpeg_setmsg("audio buffer %d overflow", p->pkt.size);
				goto error;
			}

			while (p->pkt.size > 0) {

				org_pts = p->pkt.pts;
				org_dts = p->pkt.dts;

				av_packet_rescale_ts(&p->pkt
					, p->fmt_ctx->streams[stream_idx]->time_base
					, p->fmt_ctx->streams[stream_idx]->codec->time_base);
			
				if ((ret = avcodec_decode_audio4(p->fmt_ctx->streams[stream_idx]->codec, p->frame, &got_frame, &p->pkt)) < 0) {
					if (p->audio_failed_decode_count++ >= FAILED_AUDIO_DECODE_COUNT) {
						libffmpeg_setmsg("failed to decode audio over %d times", p->audio_failed_decode_count);
						goto error;
					}
					fflog(p->log, p->log_str, "WARN: failed to decode audio ret:%d %d/%d", ret, p->audio_failed_decode_count, FAILED_AUDIO_DECODE_COUNT);
					break; //skip this packet (this may be causing AV not sync);
				}

				if (got_frame) {
					pts = av_frame_get_best_effort_timestamp(p->frame) * av_q2d(p->fmt_ctx->streams[stream_idx]->codec->time_base);
					p->init_aud_pts = ff_audio_ts_mapping(pts, p->sample_rate);
					p->init_aud_pts_secs = pts;
					found_aud_frame = 1;
					break;
				}
				
				p->audio_failed_decode_count = 0;
				p->pkt.data += ret;
				p->pkt.size -= ret;
				p->pkt.pts = org_pts;
				p->pkt.dts = org_dts;
			}
		}

		if (p->pkt.data /*&& p->pkt.size > 0*/) {
			av_free_packet(&p->pkt);
		}
		p->pkt.data = NULL;
		p->pkt.size = 0;
	}

	if ((ret=av_seek_frame(p->fmt_ctx, p->vid.idx, origin_secs, AVSEEK_FLAG_BACKWARD)) < 0){
		fflog(p->log, p->log_str, "WARN: failed to seek begin of file ret:%d sec:%f", ret, origin_secs);
		
		//The AVSEEK_FLAG_ANY enables seeking to every frame and not just keyframes
		//(may be first frame are not I frame, so seek to any frame
		if ((ret = av_seek_frame(p->fmt_ctx, -1, origin_secs, AVSEEK_FLAG_ANY)) < 0){
			libffmpeg_setmsg("failed to seek begin of file ret:%d sec:%f", ret, origin_secs);
			goto error;
		}
	}

	fflog(p->log, p->log_str, "init video dts: %f %"PRId64, p->init_vid_dts_secs, p->init_vid_dts);
	fflog(p->log, p->log_str, "init audio pts: %f %"PRId64, p->init_aud_pts_secs, p->init_aud_pts);

	return 0;
error:
	if (p->pkt.data /*&& p->pkt.size > 0*/) {
		av_free_packet(&p->pkt);
	}
	p->pkt.data = NULL;
	p->pkt.size = 0;
	return -1;
}

#if 0
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

		fflog(p->log, p->log_str, "len:%d data:%p size:%d newpkt.data:%p newpkt.size:%d"
			, len, data, size, newpkt.data, newpkt.size);
		if (with_annex_b) {
			av_free(newpkt.data);
		}

		if (size == 0 && len > 0) {
			fflog(p->log, p->log_str, "need more bytes");
			continue;
		}

		fflog(p->log, p->log_str, "coded_width:%d coded_height:%d width:%d height:%d"
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
#endif

int EXPORTS MINGWAPI libffmpeg_open(libffmpeg_t *h, libffmpeg_config *cfg, libffmpeg_log log)
{
	libffmpeg_data *p = NULL;
	char tmp[96];
	char *silence = NULL;
	int silence_frame_num = 0;
	int silence_size = 0;

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

	fflog(p->log, p->log_str, "%s %s %s %s %s"
		, LIBAVFORMAT_IDENT
		, LIBAVCODEC_IDENT
		, LIBSWSCALE_IDENT
		, LIBSWRESAMPLE_IDENT
		, LIBAVFILTER_IDENT);

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

	//subtitle
#if 0
	if (ff_codec_open(p, AVMEDIA_TYPE_SUBTITLE, p->fmt_ctx, &p->sub, cfg->decode_threads, -1) == 0) {

	}
#endif

	if (ff_codec_open(p, AVMEDIA_TYPE_VIDEO, p->fmt_ctx, &p->vid, cfg->decode_threads, -1) == 0) {
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
			&& (p->vid.pix_fmt == AV_PIX_FMT_YUV420P || p->vid.pix_fmt == AV_PIX_FMT_YUV420P10LE)) {
			cfg->video_type = FIREFLY_TYPE_X265;
			
			//HEVC Main 10, but yuv format still YUV420P
			if (p->vid.pix_fmt == AV_PIX_FMT_YUV420P10LE) {
				cfg->bit_depth_minus8 = 2;
			}

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

			fflog(p->log, p->log_str, "p->vid.h264_bsfc: %p", p->vid.h264_bsfc);
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
	if (ff_codec_open(p, AVMEDIA_TYPE_AUDIO, p->fmt_ctx, &p->aud, cfg->decode_threads, -1) == 0) {
		p->is_aud_first_frame = 1;
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
	fflog(p->log, p->log_str, "duration: %f", cfg->duration);
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
		fflog(p->log, p->log_str, "%s %dx%d,%s,%f"
			, p->vid.dec_ctx->codec->name, p->vid.width, p->vid.height, av_get_pix_fmt_name(p->vid.pix_fmt)
			, (double)p->vid.fps_num / (double)p->vid.fps_den);
		sprintf(cfg->vcodec, "%s", p->vid.dec_ctx->codec->name);
	} else if (cfg->video_type == 0) {
		cfg->audio_only = 1;
	}

	if (cfg->audio_type) {
		av_get_channel_layout_string(tmp, sizeof(tmp), p->aud.channels, p->aud.channel_layout);
		fflog(p->log, p->log_str, "%s,%s,%d,%d"
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

	//probe: to get initial video/audio DTS/PTS
	if (ff_codec_probe(p, 0.0f)) {
		goto error;
	}

	//sync_audio_1: init_aud_pts > init_vid_pts insert_audio_frame. some file are not begining at 0 sec
	if (p->aud.dec_ctx) {
		if (p->init_aud_pts > 0 && (p->init_aud_pts_secs > p->init_vid_dts_secs)) {
			silence_frame_num = (p->init_aud_pts_secs-p->init_vid_dts_secs) * p->sample_rate;
			silence_size =  silence_frame_num * p->frame_size;
			
			p->num_frame = silence_frame_num;
			p->pts = p->init_vid_dts_secs * p->sample_rate;
			
			if ((silence = ff_calloc(silence_size)) == NULL) {
				libffmpeg_setmsg( "failed to ff_calloc %d", silence_size);
				goto error;
			}

			//fflog(p->log, p->log_str, "sync_audio_1: insert audio frame %d. begin pts:%f %f %"PRId64, silence_frame_num, p->init_vid_dts_secs, p->pts/(double)p->sample_rate, p->pts);
			ff_buffer_push(p->aud_buf, silence, silence_size);
			ff_free(silence);
			silence = NULL;
		}
	}
	
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
		outbuf->header.pts = p->first_vid_dts;
		pts = firefly_video_pts(outbuf);
	} else {
		pts = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
		outbuf->header.pts = ff_video_ts_mapping(pts, p->vid.fps_f)/* - outbuf->header.offset_pts*/;
	}

	outbuf->header.dts = p->first_vid_dts;

	if (double_equals(firefly_video_pts(outbuf), pts, 0.001) == 0) {
#if 1
		fflog(p->log, p->log_str, "[VID] org_pts:%f != new_pts:%f", pts, firefly_video_pts(outbuf));
		fflog(p->log, p->log_str, "\t%f x %f = %d", pts, p->vid.fps_f, (int)(pts*p->vid.fps_f));
		fflog(p->log, p->log_str, "\t%d / %f = %f"
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
//	static int already_prt = 0;

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
	outbuf->header.dts = ff_video_ts_mapping(p->first_vid_dts, p->vid.fps_f);

	if (double_equals(firefly_video_pts(outbuf), pts, 0.001) == 0) {
#if 1
		fflog(p->log, p->log_str, "[VID] org_pts:%f != new_pts:%f", pts, firefly_video_pts(outbuf));
		fflog(p->log, p->log_str, "\t%f x %f = %d", pts, p->vid.fps_f, (int)(pts*p->vid.fps_f));
		fflog(p->log, p->log_str, "\t%d / %f = %f"
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
#if 0
		if (already_prt++ == 0) {
		fflog(p->log, p->log_str, "sws in_stide:%d %d %d out_stride:%d %d %d wh:%d"
			, in_vid_stride[0], in_vid_stride[1], in_vid_stride[2]
			, outbuf->stride[0], outbuf->stride[1], outbuf->stride[2]
			, p->vid.width, p->vid.height);
		}
#endif
	} else {
		av_image_copy(
			outbuf->plane, outbuf->stride,
			(const uint8_t **)(in_vid_plane), in_vid_stride,
			p->vid.pix_fmt, p->vid.width, p->vid.height);
#if 0
		fflog(p->log, p->log_str, "in_stide:%d %d %d out_stride:%d %d %d wh:%d"
			, in_vid_stride[0], in_vid_stride[1], in_vid_stride[2]
			, outbuf->stride[0], outbuf->stride[1], outbuf->stride[2]
			, p->vid.width, p->vid.height);
#endif
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
			
		//fflog(p->log, p->log_str, "flush_pcm remain:%d pts:%f %"PRId64, p->num_frame, (p->pts)/(double)p->sample_rate, p->pts);

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

	if (p->is_aud_first_frame) {
		fflog(p->log, p->log_str, "aud first PTS %.3f %.3f"
			, firefly_audio_pts(outbuf)
			, pts);
		p->is_aud_first_frame = 0;
	}

	//printf("[%d] aud pts: %f, num_frame:%d\n", *cc, pts, *num_frame);

	if (double_equals(firefly_audio_pts(outbuf), pts, 0.001) == 0) {
		fflog(p->log, p->log_str, "[AUD] org_pts:%f != new_pts:%f", pts, firefly_audio_pts(outbuf));
		fflog(p->log, p->log_str, "\t%f x %d = %d", pts, p->aud.sample_rate, (int)(pts*p->aud.sample_rate));
		fflog(p->log, p->log_str, "\t%"PRId64 "/ %d = %f"
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

static int ff_decode_subtitle(libffmpeg_data *p, int stream_idx, firefly_buffer *outbuf)
{
	int ret;
	int i;
	char line[256];
	int got_frame = 0;
	double pkt_pts = 0, pkt_dts = 0, pts = 0;
	AVSubtitle sub;

	{
		if (p->pkt.pts != AV_NOPTS_VALUE) {
			pkt_pts = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
			sprintf(line, "pkt_pts: %.3f ", pkt_pts);
		} else {
			sprintf(line, "pkt_pts: N/A ");
		}

		if (p->pkt.dts != AV_NOPTS_VALUE) {
			pkt_dts = (p->pkt.dts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
			sprintf(line, "pkt_dts: %.3f ", pkt_dts);
		} else {
			sprintf(line, "pkt_dts: N/A ");
		}
	}

	if ((ret = avcodec_decode_subtitle2(p->fmt_ctx->streams[stream_idx]->codec, &sub, &got_frame, &p->pkt)) < 0) {
		libffmpeg_setmsg("failed to decode subtitle @ %.3f %.3f ", pkt_pts, pkt_dts);
		goto error;
	}

	if (!got_frame) {
		goto finally;
	}

	if (sub.pts != AV_NOPTS_VALUE) {
		pts = sub.pts / (double)AV_TIME_BASE;
		sprintf(line+strlen(line), "pts: %.3f ", pts);
	} else {
		sprintf(line+strlen(line), "pts: N/A ");
	}

	//sub.format == 0, Graph
	//sub.format == 1, text ?!
	sprintf(line+strlen(line), "fmt:%d start:%.3f end:%.3f num_rects:%d"
		, sub.format, sub.start_display_time / 1000.0f, sub.end_display_time / 1000.0f
		, sub.num_rects);
	fflog(p->log, p->log_str, line);

	for (i=0; i<sub.num_rects; i++) {
		line[0] = 0;

		if (sub.rects[i]->text) { //0 terminated plain UTF-8 text
			fflog(p->log, p->log_str, "[%d] text:'%s' ", i, sub.rects[i]->text);
		} else  {
			fflog(p->log, p->log_str, "[%d] text: N/A ", i);
		}

		if (sub.rects[i]->ass) { //0 terminated ASS/SSA compatible event line
			fflog(p->log, p->log_str, "[%d] ass:'%s' ", i, sub.rects[i]->ass);
		} else  {
			fflog(p->log, p->log_str, "[%d] ass: N/A ", i);
		}
	}

	avsubtitle_free(&sub);

#if 0
	in_vid_plane = p->frame->data;
	in_vid_stride = p->frame->linesize;
	
	p->video_failed_decode_count = 0;
	pts = av_frame_get_best_effort_timestamp(p->frame) * av_q2d(p->fmt_ctx->streams[stream_idx]->codec->time_base);
	//pts2 = (p->frame->pkt_pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
	//dts2 = (p->frame->pkt_pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
	//printf("avframe pts:%f pkt_pts:%"PRId64" %.4f pkt_dts:%"PRId64" %.4f\n", pts, p->frame->pkt_pts, pts2, p->frame->pkt_dts, dts2);

	outbuf->header.pts = ff_video_ts_mapping(pts, p->vid.fps_f)/* - outbuf->header.offset_pts*/;
	outbuf->header.dts = ff_video_ts_mapping(p->first_vid_dts, p->vid.fps_f);

	if (double_equals(firefly_video_pts(outbuf), pts, 0.001) == 0) {
#if 1
		fflog(p->log, p->log_str, "[VID] org_pts:%f != new_pts:%f", pts, firefly_video_pts(outbuf));
		fflog(p->log, p->log_str, "\t%f x %f = %d", pts, p->vid.fps_f, (int)(pts*p->vid.fps_f));
		fflog(p->log, p->log_str, "\t%d / %f = %f"
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
#if 0
		if (already_prt++ == 0) {
		fflog(p->log, p->log_str, "sws in_stide:%d %d %d out_stride:%d %d %d wh:%d"
			, in_vid_stride[0], in_vid_stride[1], in_vid_stride[2]
			, outbuf->stride[0], outbuf->stride[1], outbuf->stride[2]
			, p->vid.width, p->vid.height);
		}
#endif
	} else {
		av_image_copy(
			outbuf->plane, outbuf->stride,
			(const uint8_t **)(in_vid_plane), in_vid_stride,
			p->vid.pix_fmt, p->vid.width, p->vid.height);
#if 0
		fflog(p->log, p->log_str, "in_stide:%d %d %d out_stride:%d %d %d wh:%d"
			, in_vid_stride[0], in_vid_stride[1], in_vid_stride[2]
			, outbuf->stride[0], outbuf->stride[1], outbuf->stride[2]
			, p->vid.width, p->vid.height);
#endif
	}
#endif

	return 1;
finally:
	return 0;
error:
	return -1;
}

int EXPORTS MINGWAPI libffmpeg_decode(libffmpeg_t h, firefly_buffer **in)/*libffmpeg_dec *dec)*/
{
	libffmpeg_data *p = (libffmpeg_data *)h;
	int stream_idx = -1;
	int ret2 = -1;
	int ret = -1;
	double dts;
	
	*in = NULL;

	if (p->aud.dec_ctx) {
		if (ff_decode_audio_flush_pcm(p, p->aud_outbuf) == 1) {
			/** got aud frame */
			/** sync_audio_2: init_aud_pts < init_vid_pts skip_audio_frame. some file are not begining at 0 sec */
			if (firefly_audio_pts(p->aud_outbuf) < p->init_vid_dts_secs2) {
				//fflog(p->log, p->log_str, "sync_audio_2: skip audio frame %f < %f"
				//	, firefly_audio_pts(p->aud_outbuf)
				//	, p->init_vid_dts_secs2);
				p->sync_audio_2 = 1;
				goto finally;
			} else {
				if (p->sync_audio_2) {
					p->num_frame = 0;
					ff_buffer_reset(p->aud_buf);
					p->sync_audio_2 = 0;
				}
			}
			p->aud_outbuf->header.pts -= p->init_vid_dts_secs*(double)p->sample_rate;			//sync_pts_to_zero
			*in = p->aud_outbuf;
			goto finally;
		}
	}

	if ((ret = av_read_frame(p->fmt_ctx, &p->pkt)) < 0) {
		if (ret == AVERROR_EOF) {
			ret2 = FFMPEG_EOF;
			libffmpeg_setmsg("END of file");
		} else {
			libffmpeg_setmsg("failed to av_read_frame %d", ret);
		}
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
				p->first_vid_dts = ff_video_ts_mapping(p->seek_secs, p->vid.fps_f);
				if (p->no_vid_pts == 0) {
					fflog(p->log, p->log_str, "NOPTS FAKE DTS/PTS %"PRId64, p->first_vid_dts);
				}
				p->no_vid_pts = 1;
			} else {
				dts = (p->pkt.pts*p->fmt_ctx->streams[stream_idx]->codec->time_base.num)/(double)p->fmt_ctx->streams[stream_idx]->codec->time_base.den;
				p->first_vid_dts = ff_video_ts_mapping(dts, p->vid.fps_f);
			}
			p->is_vid_first_frame = 0;
			fflog(p->log, p->log_str, "video first DTS: %.3f %"PRId64, p->first_vid_dts/(double)p->vid.fps_f, p->first_vid_dts);
		}

		if (p->vid.h264_bsfc) {
			ret =ff_read_h264_video(p, stream_idx, p->vid_outbuf);
		} else {
			ret = ff_decode_video(p, stream_idx, p->vid_outbuf);
		}

		p->first_vid_dts++;

		if (ret < 0) {
			goto error;
		} else if (ret == 1) {
			p->vid_outbuf->header.dts -= p->init_vid_dts;				//sync_pts_to_zero
			p->vid_outbuf->header.pts -= p->init_vid_dts;				//sync_pts_to_zero
			*in = p->vid_outbuf;
		}
	}else if (p->aud.dec_ctx &&  stream_idx == p->aud.idx && p->cfg->disable_decode_audio == 0) {
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
				fflog(p->log, p->log_str, "WARN: failed to decode audio ret:%d %d/%d", ret, p->audio_failed_decode_count, FAILED_AUDIO_DECODE_COUNT);
				goto finally;
			}
			p->audio_failed_decode_count = 0;
			p->pkt.data += ret;
			p->pkt.size -= ret;
		}

		//push audio frame to stack that over 1024
		ff_decode_audio_split(p, p->aud_outbuf);
	
		if (p->aud_outbuf->header.num_frame > 0) {
			/** sync_audio_2: init_aud_pts < init_vid_pts skip_audio_frame. some file are not begining at 0 sec */
			if (firefly_audio_pts(p->aud_outbuf) < p->init_vid_dts_secs2) {
				//fflog(p->log, p->log_str, "sync_audio_2: skip audio frame %f < %f"
					//,firefly_audio_pts(p->aud_outbuf)
					//, p->init_vid_dts_secs2);
					p->sync_audio_2 = 1;
				goto finally;
			} else {
				if (p->sync_audio_2) {
					p->num_frame = 0;
					ff_buffer_reset(p->aud_buf);
					p->sync_audio_2 = 0;
				}
			}
			p->aud_outbuf->header.pts -= p->init_vid_dts_secs*(double)p->sample_rate;			//sync_pts_to_zero
			*in = p->aud_outbuf;
		}
	} else if (p->sub.dec_ctx &&  stream_idx == p->sub.idx) {
		if ((ret = ff_decode_subtitle(p, stream_idx, NULL)) < 0) {
			goto error;
		}

		if (ret) {
			//got frame
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
	return ret2;
}

int EXPORTS MINGWAPI libffmpeg_seek(libffmpeg_t h, void *p1, void *p2)
{
	libffmpeg_data *p = (libffmpeg_data *)h;
	/** we use audio to A/V sync, 
	 * 		if aud_pts < vid_dts then skip audio frame
	 * 		if aud_pts > vid_dts then insert audio frame
	 * 		so the video dts is the based timestamp */
	double secs = *((double *)p1); 
	int64_t seek_base = 0;
	AVRational avr;
	
	avr.num = 1;
	avr.den = AV_TIME_BASE;

	//fflog(p->log, p->log_str, "seek to: %f + %f = %f", p->init_vid_dts_secs, secs,  secs + p->init_vid_dts_secs);
	//if initial PTS are not ZERO, when seeking if aud_pts < vid_pts, then we need to skip audio frame before 
	//audio frame PTS >= video frame DTS
	secs += p->init_vid_dts_secs;
	p->init_vid_dts_secs2 = secs;

	seek_base = av_rescale_q(
		(int64_t)(secs * AV_TIME_BASE)
		, avr, p->fmt_ctx->streams[p->vid.idx]->time_base);

	if (av_seek_frame(p->fmt_ctx, p->vid.idx, seek_base, AVSEEK_FLAG_BACKWARD) < 0){
		libffmpeg_setmsg("failed to seek pos %.3f", secs);
		goto error;
	}

	p->is_vid_first_frame = 1;
	p->is_aud_first_frame = 1;
	p->first_vid_dts = 0;
	p->first_aud_pts = 0;
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
	//libffmpeg_data *p = (libffmpeg_data *)h;	
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
	//libffmpeg_data *p = (libffmpeg_data *)h;
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

	//subtitle
	ff_codec_close(&p->sub);

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

void fflog2(const char *file, int line, libffmpeg_log log, char *log_str, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (file && line) {
		sprintf(log_str, "%s(%d): ", file, line);
		vsprintf(strlen(log_str)+log_str, fmt, args);
	} else {
		vsprintf(log_str, fmt, args);
	}
	va_end(args);
	log(log_str);
}


typedef struct {
	libffmpeg_img_scale_config *cfg;
	struct SwsContext *sws;
	firefly_buffer *outbuf;
	libffmpeg_log log;
	char log_str[DBG_MSG_LEN];
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

/***************************************************
 * image scaling
 **************************************************/

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


	fflog(p->log, p->log_str, "swscale method: %d %s", swscale_method, cfg->swscale_method);

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


/***************************************************
 * ADTS Header
 **************************************************/

typedef struct
{
  unsigned char *data;      /* data bits */
  long numBit;          /* number of bits in buffer */
  long size;            /* buffer size in bytes */
  long currentBit;      /* current bit position in bit stream */
  long numByte;         /* number of bytes read/written (only file) */
} BitStream;

static int GetSRIndex(unsigned int sampleRate)
{
	if (92017 <= sampleRate) return 0;
	if (75132 <= sampleRate) return 1;
	if (55426 <= sampleRate) return 2;
	if (46009 <= sampleRate) return 3;
	if (37566 <= sampleRate) return 4;
	if (27713 <= sampleRate) return 5;
	if (23004 <= sampleRate) return 6;
	if (18783 <= sampleRate) return 7;
	if (13856 <= sampleRate) return 8;
	if (11502 <= sampleRate) return 9;
	if (9391 <= sampleRate) return 10;
	return 11;
}

/* size in bytes! */
static int bs_init(BitStream *bitStream, unsigned char *buffer, int size)
{
	memset(bitStream, 0, sizeof(BitStream));
    memset(buffer, 0, size);
    bitStream->data = buffer;
    bitStream->size = size;
	return 0;
}

#define BYTE_NUMBIT 8       /* bits in byte (char) */
#define bit2byte(a) (((a)+BYTE_NUMBIT-1)/BYTE_NUMBIT)

static int bs_close(BitStream *bitStream)
{
    return bit2byte(bitStream->numBit);
}

static int bs_write_byte(BitStream *bitStream,
                     unsigned long data,
                     int numBit)
{
    long numUsed,idx;

    idx = (bitStream->currentBit / BYTE_NUMBIT) % bitStream->size;
    numUsed = bitStream->currentBit % BYTE_NUMBIT;
    bitStream->data[idx] |= (data & ((1<<numBit)-1)) <<
        (BYTE_NUMBIT-numUsed-numBit);
    bitStream->currentBit += numBit;
    bitStream->numBit = bitStream->currentBit;

    return 0;
}

static int bs_put_bit(BitStream *bitStream,
           unsigned long data,
           int numBit)
{
    int num,maxNum,curNum;
    unsigned long bits;

    if (numBit == 0)
        return 0;

    /* write bits in packets according to buffer byte boundaries */
    num = 0;
    maxNum = BYTE_NUMBIT - bitStream->currentBit % BYTE_NUMBIT;
    while (num < numBit) {
        curNum = min(numBit-num,maxNum);
        bits = data>>(numBit-num-curNum);
        if (bs_write_byte(bitStream, bits, curNum)) {
            return 1;
        }
        num += curNum;
        maxNum = BYTE_NUMBIT;
    }

    return 0;
}

static int bs_byte_align(BitStream *bitStream, int writeFlag, int bitsSoFar)
{
    int len, i,j;

    if (writeFlag)
    {
        len = bitStream->numBit;
    } else {
        len = bitsSoFar;
    }

    j = (8 - (len%8))%8;

    if ((len % 8) == 0) j = 0;
    if (writeFlag) {
        for( i=0; i<j; i++ ) {
            bs_put_bit(bitStream, 0, 1);
        }
    }
    return j;
}

static void libffaac_enc_set_adts(uint8_t *buf, int len, int id, int object_type, int sample_rate, int channel)
{
	BitStream bitStream;

	bs_init(&bitStream, buf, 7);

	bs_put_bit(&bitStream, 0xFFFF, 12); /* 12 bit Syncword */
	bs_put_bit(&bitStream, id, 1); /* ID == 0 for MPEG4 AAC, 1 for MPEG2 AAC */
	bs_put_bit(&bitStream, 0, 2); /* layer == 0 */
	bs_put_bit(&bitStream, 1, 1); /* protection absent */

	bs_put_bit(&bitStream, object_type - 1, 2); /* AAC LC = 2, AAC SSR = 3 profile */
	bs_put_bit(&bitStream, GetSRIndex(sample_rate), 4); /* sampling rate */
	bs_put_bit(&bitStream, 0, 1); /* private bit */

	bs_put_bit(&bitStream, channel, 3); /* ch. config (must be > 0) */
												 /* simply using numChannels only works for
													6 channels or less, else a channel
													configuration should be written */
	bs_put_bit(&bitStream, 0, 1); /* original/copy */
	bs_put_bit(&bitStream, 0, 1); /* home */

#if 0 // Removed in corrigendum 14496-3:2002
	if (hEncoder->config.mpegVersion == 0)
		bs_put_bit(&bitStream, 0, 2); /* emphasis */
#endif

	/* Variable ADTS header */
	bs_put_bit(&bitStream, 0, 1); /* copyr. id. bit */
	bs_put_bit(&bitStream, 0, 1); /* copyr. id. start */

	bs_put_bit(&bitStream, len/*hEncoder->usedBytes*/, 13);
	bs_put_bit(&bitStream, 0x7FF, 11); /* buffer fullness (0x7FF for VBR) */
	bs_put_bit(&bitStream, 0, 2); /* raw data blocks (0+1=1) */

	bs_byte_align(&bitStream, 1, 0);
	bs_close(&bitStream);
}

static void libffaac_enc_set_adts_len(uint8_t *buf, int len)
{
	buf[3] = 0x80/*(2 << 6)*/ | (len >> 11);
	buf[4] = (len >> 3) & 0xFF;
	buf[5] = ((len & 7) << 5) | 0x1F; //((0x7FF >> 6) & 0x1F);
}


/***************************************************
 * FFMPEG AAC Encoder
 **************************************************/

char g_szMsg3[8192];

typedef struct {
	AVCodecContext *aud_ctx;

	/** audio filter graph */
	AVFilterGraph *graph;
    AVFilterContext *src;
	AVFilterContext *sink;
	
	int frame_size;
	uint8_t *in_frame;
	int in_frame_num;
	int in_frame_num_current;
	int in_frame_size;
	int in_sample_rate;
	int in_audio_gain;

	AVAudioFifo *aud_filter_fifo;

	uint8_t *out_frame;					//AAC-LC
	int out_frame_size;

	int64_t frame_counter;
	
	libffaac_log log;
	char log_str[DBG_MSG_LEN];
} libffaac_enc_data;

static const char *get_error_text(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}

static int init_filter_graph(AVFilterGraph **graph, AVFilterContext **src, AVFilterContext **sink
	,int in_sample_fmt, int in_sample_rate, int64_t in_channel_layout
	,int out_sample_fmt, int out_sample_rate, int64_t out_channel_layout
	,int in_audio_gain)
{
	AVFilterGraph *filter_graph;
	AVFilterContext *abuffer_ctx;
	AVFilter        *abuffer;
	AVFilterContext *volume_ctx;
	AVFilter        *volume;
	AVFilterContext *aformat_ctx;
	AVFilter        *aformat;
	AVFilterContext *abuffersink_ctx;
	AVFilter        *abuffersink;

//	AVDictionary *options_dict = NULL;
	char options_str[1024];
	char ch_layout[64];
	int err;

	/* Create a new filtergraph, which will contain all the filters. */
	if ((filter_graph = avfilter_graph_alloc()) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_graph_alloc)");
		goto error;
	}

	/* Create the abuffer filter;
	 * it will be used for feeding the data into the graph. */
	if ((abuffer = avfilter_get_by_name("abuffer")) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_get_by_name)");
		goto error;
	}

	if ((abuffer_ctx = avfilter_graph_alloc_filter(filter_graph, abuffer, "src")) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_graph_alloc_filter)");
		goto error;
	}

	/* Set the filter options through the AVOptions API. */
	av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, in_channel_layout);
	av_opt_set    (abuffer_ctx, "channel_layout", ch_layout,                            AV_OPT_SEARCH_CHILDREN);
	av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(in_sample_fmt), AV_OPT_SEARCH_CHILDREN);
	av_opt_set_q  (abuffer_ctx, "time_base",      (AVRational){ 1, in_sample_rate },  AV_OPT_SEARCH_CHILDREN);
	av_opt_set_int(abuffer_ctx, "sample_rate",    in_sample_rate,                     AV_OPT_SEARCH_CHILDREN);

	/* Now initialize the filter; we pass NULL options, since we have already
	 * set all the options above. */
	if ((err = avfilter_init_str(abuffer_ctx, NULL)) < 0) {
		libffaac_enc_setmsg("failed to avfilter_init_str)");
		goto error;
	}

	/* Create volume filter. */
	if (in_audio_gain == 1) {
		//by pass
		if ((volume = avfilter_get_by_name("anull")) == NULL) {
			libffaac_enc_setmsg("failed to avfilter_get_by_name anull)");
			goto error;
		}

		if ((volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "anull")) == NULL) {
			libffaac_enc_setmsg("failed to avfilter_graph_alloc_filter anull");
			goto error;
		}
	} else {
		//Dynamic Audio Normalizer
		//https://ffmpeg.org/ffmpeg-filters.html#dynaudnorm
		if ((volume = avfilter_get_by_name("dynaudnorm")) == NULL) {
			libffaac_enc_setmsg("failed to avfilter_get_by_name dynaudnorm");
			goto error;
		}

		if ((volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "dynaudnorm")) == NULL) {
			libffaac_enc_setmsg("failed to avfilter_graph_alloc_filter dynaudnorm");
			goto error;
		}

		snprintf(options_str, sizeof(options_str),
				 "f=50:p=0.95:m=%d.0", in_audio_gain);
		err = avfilter_init_str(volume_ctx, options_str);
		if (err < 0) {
			libffaac_enc_setmsg("failed to avfilter_init_str");
			goto error;
		}
	}

	/* Create the aformat filter;
	 * it ensures that the output is of the format we want. */
	if ((aformat = avfilter_get_by_name("aformat")) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_get_by_name)");
		goto error;
	}

	if ((aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat")) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_graph_alloc_filter)");
		goto error;
	}

	/* A third way of passing the options is in a string of the form
	 * key1=value1:key2=value2.... */
	snprintf(options_str, sizeof(options_str),
			 "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%"PRIx64,
			 av_get_sample_fmt_name(out_sample_fmt), out_sample_rate,
			 (uint64_t)out_channel_layout);
	err = avfilter_init_str(aformat_ctx, options_str);
	if (err < 0) {
		libffaac_enc_setmsg("failed to avfilter_init_str)");
		goto error;
	}

	/* Finally create the abuffersink filter;
	 * it will be used to get the filtered data out of the graph. */
	if ((abuffersink = avfilter_get_by_name("abuffersink")) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_get_by_name)");
		goto error;
	}

	if ((abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink")) == NULL) {
		libffaac_enc_setmsg("failed to avfilter_graph_alloc_filter)");
		goto error;
	}

	/* This filter takes no options. */
	if ((err = avfilter_init_str(abuffersink_ctx, NULL)) < 0) {
		libffaac_enc_setmsg("failed to avfilter_init_str)");
		goto error;
	}

	/* Connect the filters;
	 * in this simple case the filters just form a linear chain. */
	err = avfilter_link(abuffer_ctx, 0, volume_ctx, 0);
	if (err >= 0)
		err = avfilter_link(volume_ctx, 0, aformat_ctx, 0);
	if (err >= 0)
		err = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
	if (err < 0) {
		libffaac_enc_setmsg("failed to avfilter_get_by_name)");
		goto error;
	}

	/* Configure the graph. */
	err = avfilter_graph_config(filter_graph, NULL);
	if (err < 0) {
		libffaac_enc_setmsg("failed to avfilter_graph_config");
		goto error;
	}

	*graph = filter_graph;
	*src   = abuffer_ctx;
	*sink  = abuffersink_ctx;
	return 0;
error:
	return -1;
}


int EXPORTS MINGWAPI libffaac_enc_open(libffaac_enc_t *h
	, int in_channels, int in_sample_rate, int in_bit_rate, int in_audio_gain
	, int out_vbr, int out_bit_rate, int *enc_num_frame, int out_bitstream_fmt, libffaac_log fflog)
{
	AVCodec *aud_dec = NULL;
	libffaac_enc_data *p = NULL;
	int ret;

	if (!libffmpeg_init) {
		av_register_all();
		libffmpeg_init = 1;
	}

	if (!libffmpeg_avfilter_init) {
		avfilter_register_all();
		libffmpeg_avfilter_init = 1;
	}

	if (in_channels != 2) {
		libffaac_enc_setmsg("failed to set in_channels: %d, only support 2 channel", in_channels);
		goto error;
	}
	
	if (in_sample_rate != 44100) {
		libffaac_enc_setmsg("failed to set in_sample_rate: %d, only support 44100 ", in_sample_rate);
		goto error;
	}

	if (in_bit_rate != 16) {
		libffaac_enc_setmsg("failed to set in_bit_rate: %d, only support 16 bit", in_bit_rate);
		goto error;
	}
	
	if (out_vbr) {
		libffaac_enc_setmsg("failed to set out_vbr: %d, only support constant mode", out_vbr);
		goto error;
	}
	
	//0:RAW 1: ADIF (for mp4) 2: ADTS (for MPEG-2 AAC LC), 6,7:LATM (refernce from fdkaac)
	if (out_bitstream_fmt != 2) {
		libffaac_enc_setmsg("failed to set out_bitstream_fmt: %d, only support ADTS fmt", out_bitstream_fmt);
		goto error;
	}

	if ((p = (libffaac_enc_data*)ff_calloc(sizeof(libffaac_enc_data))) == NULL) {
		libffaac_enc_setmsg("failed ff_calloc libffaac_enc_data");
		goto error;
	}

	if ((aud_dec = avcodec_find_encoder(AV_CODEC_ID_AAC)) == NULL) {
		libffaac_enc_setmsg("failed to found AAC Encoder");
		goto error;
	}

	if ((p->aud_ctx = avcodec_alloc_context3(aud_dec)) == NULL) {
		libffaac_enc_setmsg("failed to avcodec_alloc_context3");
		goto error;
	}

	p->log = fflog;
	p->in_audio_gain = in_audio_gain;
	p->aud_ctx->codec_id = AV_CODEC_ID_AAC;
	p->aud_ctx->profile = FF_PROFILE_AAC_LOW;
	p->aud_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP; //only FDKAAC support AV_SAMPLE_FMT_S16
	p->aud_ctx->sample_rate = in_sample_rate;  
	p->aud_ctx->channels = in_channels;  
	p->aud_ctx->channel_layout = av_get_default_channel_layout(in_channels);  
	p->aud_ctx->bit_rate = out_bit_rate;
	p->aud_ctx->time_base.den = in_sample_rate;
    p->aud_ctx->time_base.num = 1;

	/** Allow the use of the experimental AAC encoder (build-in AAC Encoder) */
    p->aud_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

	/** Open the encoder for the audio stream to use it later. */
	if ((ret = avcodec_open2(p->aud_ctx, aud_dec, NULL)) < 0) {
		libffaac_enc_setmsg("failed to open output codec (%s')",get_error_text(ret));
		goto error;
	}

	//fflog(p->log, p->log_str, "frame_size: %d\nin_sample_rate:%d", p->aud_ctx->frame_size, in_sample_rate);
	p->frame_size = in_channels * (in_bit_rate/8);						//16bit per sample
	p->in_frame_num = p->aud_ctx->frame_size;
	p->in_frame_size = p->frame_size * p->in_frame_num;
	p->in_sample_rate = in_sample_rate;
	p->out_frame_size = 20480;

	//IN S16
	if ((p->in_frame = (uint8_t *) ff_calloc(p->in_frame_size)) == NULL) {
		libffaac_enc_setmsg("failed to calloc in_frame buffer");
		goto error;
	}

	//store filtered audio buffer
	if ((p->aud_filter_fifo =  av_audio_fifo_alloc(p->aud_ctx->sample_fmt, p->aud_ctx->channels, 1)) == NULL) {
		libffmpeg_setmsg("failed to aud_filter_fifo ");
		goto error;
	}

	//OUT Volume Adjusting + FMT convertor
	if (init_filter_graph(&p->graph, &p->src, &p->sink
	,AV_SAMPLE_FMT_S16, p->aud_ctx->sample_rate, av_get_default_channel_layout(p->aud_ctx->channels)
	,AV_SAMPLE_FMT_FLTP, p->aud_ctx->sample_rate, av_get_default_channel_layout(p->aud_ctx->channels)
	,p->in_audio_gain)) {
		goto error;
	}

	//OUT AAC-LC
	if ((p->out_frame = (uint8_t *)	ff_calloc(p->out_frame_size)) == NULL) {
		libffaac_enc_setmsg("failed to calloc out_frame buffer");
		goto error;
	}

	/** set ADTS header only once, due to not change */
	libffaac_enc_set_adts(p->out_frame, 0, 0, 2/** AAC LC*/, in_sample_rate, in_channels);

	*enc_num_frame = p->in_frame_num;
	*h = p;
	return 0;
error:
	libffaac_enc_close(p);
	return -1;
}

int EXPORTS MINGWAPI libffaac_enc_encode(libffaac_enc_t h, libffaac_enc *enc)
{
	libffaac_enc_data *p = (libffaac_enc_data *)h;
	int copy_frame;
	int remain_frame;
	AVPacket pkt;
	int got_output;
	int ret;
	AVFrame *in_frame = NULL;
	AVFrame *out_frame = NULL;
	AVFrame *out_frame2 = NULL;

	if (enc->in_num_frame > p->in_frame_num) {
		libffaac_enc_setmsg("%d over %d failed", enc->in_num_frame, p->in_frame_num);
		return -1;
	}

	// less
	if ((p->in_frame_num_current + enc->in_num_frame) < p->in_frame_num) {
		memcpy(p->in_frame+p->in_frame_num_current*p->frame_size, enc->in_buf, enc->in_buf_size);
		p->in_frame_num_current += enc->in_num_frame;
		*enc->out_buf_size = 0;
		*enc->out_buf = NULL;
		*enc->out_num_frame = 0;
		return 0;
	}

	//greater & equals
	copy_frame = p->in_frame_num - p->in_frame_num_current;
	memcpy(p->in_frame+p->in_frame_num_current*p->frame_size, enc->in_buf, copy_frame*p->frame_size);

	//copy to AVFrame, because filter will unref AVFrame, so can't reused
	if ((in_frame  = av_frame_alloc()) == NULL) {
		libffaac_enc_setmsg("failed to av_frame_alloc ");
		goto error;
	}

    in_frame->nb_samples     = p->aud_ctx->frame_size;
    in_frame->channel_layout = AV_CH_LAYOUT_STEREO;
    in_frame->format         = AV_SAMPLE_FMT_S16;
    in_frame->sample_rate    = p->in_sample_rate;

	if ((ret = av_frame_get_buffer(in_frame, 0)) < 0) {
		libffaac_enc_setmsg("failed to allocate output frame samples (error '%s')",
				get_error_text(ret));
		goto error;
	}

	memcpy(in_frame->extended_data[0], p->in_frame, p->in_frame_num * p->frame_size);

	//convert S16 to FLTP and Volume Up
	if ((out_frame  = av_frame_alloc()) == NULL) {
		libffaac_enc_setmsg("failed to av_frame_alloc ");
		goto error;
	}

	/* Send the frame to the input of the filtergraph. */
	if ((ret = av_buffersrc_add_frame(p->src, in_frame)) < 0) {
		libffaac_enc_setmsg("failed to av_buffersrc_add_frame (error '%s')", get_error_text(ret));
		goto error;
	}

#if 0
	/* Get all the filtered output that is available. */
	if (!(ret = av_buffersink_get_frame(p->sink, out_frame) >= 0)) {
		libffaac_enc_setmsg("failed to av_buffersink_get_frame (error '%s') %d %d", get_error_text(ret), ret, out_frame->nb_samples);
		goto error;
	}

	//fflog(p->log, p->log_str, "%d %d planar:%d ch:%d s:%d"
		, ret, AVERROR(EAGAIN), av_sample_fmt_is_planar(out_frame->format)
		, av_get_channel_layout_nb_channels(out_frame->channel_layout)
		, av_get_bytes_per_sample(out_frame->format));

	if (out_frame->nb_samples != p->in_frame_num) {
		libffaac_enc_setmsg("failed to cvrt S16 to FLTP, out_frame->nb_samples %d not eq p->in_frame_num %d"
			, out_frame->nb_samples, p->in_frame_num);
		goto error;
	}
#endif

#if 1
	while((ret = av_buffersink_get_frame(p->sink, out_frame)) >= 0) {
		if (out_frame->nb_samples == 0) {
			continue;
		}

		if ((ret =  av_audio_fifo_realloc(p->aud_filter_fifo, av_audio_fifo_size(p->aud_filter_fifo) + out_frame->nb_samples)) < 0) {
			libffaac_enc_setmsg("failed to av_audio_fifo_realloc (error '%s') %d %d"
				, get_error_text(ret), ret, out_frame->nb_samples);
			goto error;
		}

		if (av_audio_fifo_write(p->aud_filter_fifo, (void **) out_frame->data,
								out_frame->nb_samples) < out_frame->nb_samples) {
			libffaac_enc_setmsg("failed to av_audio_fifo_write %d", out_frame->nb_samples);
			goto error;
		}
		
		//fflog(p->log, p->log_str, "push: %d/%d", out_frame->nb_samples, av_audio_fifo_size(p->aud_filter_fifo));
	}
#endif

	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		//do nothing, normal situation
	} else if (ret < 0) {
		libffaac_enc_setmsg("failed to av_buffersink_get_frame (error '%s') %d %d", get_error_text(ret), ret, out_frame->nb_samples);
		goto error;
	}

	if (av_audio_fifo_size(p->aud_filter_fifo) <  p->in_frame_num) {
		//fflog(p->log, p->log_str, "aud_filter_buf_num %d not full", av_audio_fifo_size(p->aud_filter_fifo));
		*enc->out_buf_size = 0;
		*enc->out_buf = NULL;
		*enc->out_num_frame = 0;
		goto copy_remain;
	}

	//copy queued buffer to out_frame2
	if ((out_frame2 = av_frame_alloc()) == NULL) {
		libffaac_enc_setmsg("failed to av_frame_alloc ");
		goto error;
	}

    out_frame2->sample_rate    = p->in_sample_rate;
    out_frame2->format         = AV_SAMPLE_FMT_FLTP;
    out_frame2->channel_layout = av_get_default_channel_layout(2);
    out_frame2->nb_samples     = p->in_frame_num;

	if ((ret = av_frame_get_buffer(out_frame2, 0)) < 0) {
		libffaac_enc_setmsg("failed to allocate output frame samples (error '%s')",
				get_error_text(ret));
		goto error;
	}

	if ((ret=av_audio_fifo_read(p->aud_filter_fifo, (void **)out_frame2->data, out_frame2->nb_samples)) < out_frame2->nb_samples) {
		libffaac_enc_setmsg("failed to av_audio_fifo_read ret:%d %d"
			, ret, out_frame2->nb_samples);
		goto error;
	}

	//fflog(p->log, p->log_str, "pop: %d/%d", out_frame2->nb_samples, av_audio_fifo_size(p->aud_filter_fifo));

	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	//encode
	if ((ret = avcodec_encode_audio2(p->aud_ctx, &pkt, out_frame2, &got_output)) < 0) {  
		libffaac_enc_setmsg("failed to avcodec_encode_audio2 AAC (error '%s')",
				get_error_text(ret));
		goto error;
	}

	if (!(got_output > 0)) {
		//fflog(p->log, p->log_str, "no get frame");
		*enc->out_buf_size = 0;
		*enc->out_buf = NULL;
		*enc->out_num_frame = 0;
		goto copy_remain;
	}

	//fflog(p->log, p->log_str, "got_aac_frame: %d %f", pkt.size, p->frame_counter/44100.0f);
	libffaac_enc_set_adts_len(p->out_frame, pkt.size+7);
	memcpy(p->out_frame+7, pkt.data, pkt.size);

	*enc->out_buf_size = pkt.size + 7;
	*enc->out_pts = p->frame_counter;
	*enc->out_buf = p->out_frame;
	*enc->out_num_frame = p->in_frame_num;
	p->frame_counter += p->in_frame_num;

	av_free_packet(&pkt);
copy_remain:

	if (in_frame) {
		av_frame_free(&in_frame);
	}
	if (out_frame) {
		av_frame_free(&out_frame);
	}
	if (out_frame2) {
		av_frame_free(&out_frame2);
	}

	//copy remain bytes
	remain_frame = enc->in_num_frame - copy_frame;
	memcpy(p->in_frame, enc->in_buf+(copy_frame*p->frame_size), remain_frame*p->frame_size);
	p->in_frame_num_current = remain_frame;
	return 0;
error:
	if (in_frame) {
		av_frame_free(&in_frame);
	}
	if (out_frame) {
		av_frame_free(&out_frame);
	}
	if (out_frame2) {
		av_frame_free(&out_frame2);
	}
	return -1;
}

int EXPORTS MINGWAPI libffaac_enc_done(libffaac_enc_t h)
{
	return -1;
}

int EXPORTS MINGWAPI libffaac_enc_reset(libffaac_enc_t h)
{
	libffaac_enc_data *p = (libffaac_enc_data *)h;

	if (!p) {
		return -1;
	}

	p->in_frame_num_current = 0;
	p->frame_counter = 0;
	av_audio_fifo_reset(p->aud_filter_fifo);
	p->src = NULL;
	p->sink = NULL;
	
	if (p->graph) {
		avfilter_graph_free(&p->graph);
		p->graph = NULL;
	}

	//OUT Volume Adjusting + FMT convertor
	if (init_filter_graph(&p->graph, &p->src, &p->sink
	,AV_SAMPLE_FMT_S16, p->aud_ctx->sample_rate, av_get_default_channel_layout(p->aud_ctx->channels)
	,AV_SAMPLE_FMT_FLTP, p->aud_ctx->sample_rate, av_get_default_channel_layout(p->aud_ctx->channels)
	,p->in_audio_gain)) {
		return -1;
	}

	return 0;
}

int EXPORTS MINGWAPI libffaac_enc_close(libffaac_enc_t h)
{
	libffaac_enc_data *p = (libffaac_enc_data *) h;

	if (!p) {
		return -1;
	}

	if (p->aud_filter_fifo) {
		av_audio_fifo_free (p->aud_filter_fifo);
		p->aud_filter_fifo = NULL;
	}

	if (p->aud_ctx) {
		avcodec_close(p->aud_ctx);
		p->aud_ctx = NULL;
	}

	if (p->in_frame) {
		ff_free(p->in_frame);
		p->in_frame = NULL;
	}

	if (p->out_frame) {
		ff_free(p->out_frame);
		p->out_frame = NULL;
	}

	if (p->graph) {
		avfilter_graph_free(&p->graph);
	}

	ff_free(p);
	return 0;
}

void EXPORTS MINGWAPI libffaac_enc_get_msg(char *msg)
{
	sprintf(msg, "%s", g_szMsg3);
}

void libffaac_enc_setmsg2(const char *file, int line, const char *fmt, ...)
{
	va_list args;
	sprintf(g_szMsg3, "%s(%d): ", file, line);
	va_start(args, fmt);
	vsprintf(strlen(g_szMsg3) + g_szMsg3, fmt, args);
	va_end(args);
}

