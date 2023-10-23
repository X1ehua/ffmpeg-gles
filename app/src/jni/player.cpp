#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "player.h"

// #define MP4_URI "/mnt/extSdCard/clear.ts"
//#define MP4_URI "/sdcard/DCIM/CCLive/1019-075349.mp4"
//#define MP4_URI "/sdcard/Movies/10s.mp4"
//#define MP4_URI "http://mozicode.com/garfield.mp4"
//#define MP4_URI "http://mozicode.com/10s.mp4"

#include <android/log.h>
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, "FFGL", __VA_ARGS__)

GlobalContext global_context;

static void sigterm_handler(int sig) {
	av_log(NULL, AV_LOG_ERROR, "sigterm_handler : sig is %d \n", sig);
	exit(123);
}

static void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
	if (level > AV_LOG_DEBUG) return;

    int droid_log_level = 0;
	if (level > AV_LOG_DEBUG) {
		return;
	}
	else if (level <= AV_LOG_ERROR) {
		droid_log_level = ANDROID_LOG_ERROR;
	}
	else if (level == AV_LOG_WARNING) {
		droid_log_level = ANDROID_LOG_WARN;
	}
	else if (level == AV_LOG_INFO) {
		droid_log_level = ANDROID_LOG_INFO;
	}
	else if (level == AV_LOG_DEBUG) {
		droid_log_level = ANDROID_LOG_DEBUG;
	}
	else {
		__android_log_print(AV_LOG_WARNING, "FFGL", "Unknown av_log_level: %d", level);
		droid_log_level = ANDROID_LOG_WARN;
	}

	__android_log_vprint(droid_log_level, "FFGL", fmt, vl);
}

void* open_media(void *argv) {
	unsigned int i;
	int err = 0;
//	int framecnt;
	AVFormatContext *fmt_ctx = NULL;
//	AVDictionaryEntry *dict = NULL;
	AVPacket pkt;
	int video_stream_index = -1;
	pthread_t thread;

	global_context.quit = 0;
	global_context.pause = 0;

	// register INT/TERM signal
	signal(SIGINT, sigterm_handler); /* Interrupt (ANSI).    */
	signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */

	av_log_set_callback(ffmpeg_log_callback);

	// set log level
	av_log_set_level(AV_LOG_WARNING);

	/* register all codecs, demux and protocols */
	avfilter_register_all();
	av_register_all();
	avformat_network_init();

	fmt_ctx = avformat_alloc_context();

	const char* videoUri = (const char*)argv;
	//av_log(NULL, AV_LOG_WARNING, ">> videoUri: %s", videoUri);
	err = avformat_open_input(&fmt_ctx, videoUri, NULL, NULL);
	if (err < 0) {
		char err_buf[256];
		av_strerror(err, err_buf, 256);
		av_log(NULL, AV_LOG_ERROR, "avformat_open_input() failed: %s [err %d]", err_buf, err);
		err = -1;
		goto failure;
	}

	if ((err = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info : err %d", err);
		err = -1;
		goto failure;
	}

	// search video stream in all streams.
	for (i = 0; i < fmt_ctx->nb_streams; i++) {
		// because video stream only one, so found and stop.
		if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			LOGW(">> video_stream_index %d", video_stream_index);
			break;
		}
	}

	// if no video and audio, exit
	if (-1 == video_stream_index) {
		goto failure;
	}

	// open video
	if (-1 != video_stream_index) {
		global_context.vcodec_ctx = fmt_ctx->streams[video_stream_index]->codec;
		global_context.vstream = fmt_ctx->streams[video_stream_index];
		global_context.vcodec = avcodec_find_decoder(
				global_context.vcodec_ctx->codec_id);
		if (NULL == global_context.vcodec) {
			av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder video failure.");
			goto failure;
		}

		if (avcodec_open2(global_context.vcodec_ctx, global_context.vcodec, NULL) < 0) {
			av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failure.");
			goto failure;
		}

		if ((global_context.vcodec_ctx->width > 0) && (global_context.vcodec_ctx->height > 0)) {
			setBuffersGeometry(global_context.vcodec_ctx->width, global_context.vcodec_ctx->height);
		}

		av_log(NULL, AV_LOG_ERROR, "video : width is %d, height is %d .",
				global_context.vcodec_ctx->width,
				global_context.vcodec_ctx->height);

	}

	if (-1 != video_stream_index) {
		pthread_create(&thread, NULL, video_decode_render_thread, NULL);
	}
	packet_queue_init(&global_context.video_queue);

    static int sum[2] = {0};
	// read url media data circle
	while ((av_read_frame(fmt_ctx, &pkt) >= 0) && (!global_context.quit)) {
        sum[pkt.stream_index] += pkt.size;
		LOGW(">> pkt size %d, sum %d+%d=%d, st_idx %d [video_index %d]",
             pkt.size, sum[0], sum[1], sum[0]+sum[1], pkt.stream_index, video_stream_index);
		if (pkt.stream_index == video_stream_index) {
			packet_queue_put(&global_context.video_queue, &pkt);
		} else {
			av_free_packet(&pkt);
		}
		usleep(1000 * 10);
	}

	// wait exit
	while (!global_context.quit) {
		usleep(1000);
	}

	failure:

	if (fmt_ctx) {
		avformat_close_input(&fmt_ctx);
		avformat_free_context(fmt_ctx);
	}

	avformat_network_deinit();

	return 0;
}

// moved from util.cpp
void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	pthread_mutex_init(&q->mutex, NULL);
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
	AVPacketList *pkt1;

	if ((NULL == pkt) || (NULL == q)) {
		av_log(NULL, AV_LOG_ERROR,
				"packet_queue_put failure, q or pkt is NULL. \n");
		return -1;
	}

	if (av_dup_packet(pkt) < 0) {
		av_log(NULL, AV_LOG_ERROR, "packet_queue_put av_dup_packet failure.\n");
		return -1;
	}

	pkt1 = (AVPacketList*) av_malloc(sizeof(AVPacketList));
	if (!pkt1) {
		av_log(NULL, AV_LOG_ERROR, "packet_queue_put av_malloc failure.\n");
		return -1;
	}

	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	pthread_mutex_lock(&q->mutex);

	if (!q->last_pkt) {
		q->first_pkt = pkt1;
	} else {
		q->last_pkt->next = pkt1;
	}

	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;

	pthread_mutex_unlock(&q->mutex);

	return 0;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt) {
	AVPacketList *pkt1;
	int ret;

	if (global_context.quit) {
		return -1;
	}

	pthread_mutex_lock(&q->mutex);

	pkt1 = q->first_pkt;

	if (pkt1) {
		q->first_pkt = pkt1->next;

		if (!q->first_pkt) {
			q->last_pkt = NULL;
		}

		q->nb_packets--;
		q->size -= pkt1->pkt.size;
		*pkt = pkt1->pkt;
		av_free(pkt1);
		ret = 1;
	} else {
		ret = 0;
	}

	pthread_mutex_unlock(&q->mutex);

	return ret;
}

int packet_queue_size(PacketQueue *q) {
	return q->size;
}
