#include <unistd.h>

#include "player.h"

static int img_convert(AVPicture *dst, int dst_pix_fmt, const AVPicture *src,
		int src_pix_fmt, int src_width, int src_height) {
	int w;
	int h;
	struct SwsContext *pSwsCtx;

	w = src_width;
	h = src_height;

	pSwsCtx = sws_getContext(w, h, (AVPixelFormat) src_pix_fmt, w, h,
			(AVPixelFormat) dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
	sws_scale(pSwsCtx, (const uint8_t* const *) src->data, src->linesize, 0, h,
			dst->data, dst->linesize);

	return 0;
}

void* video_decode_render_thread(void *argv) {
	AVPacket pkt1;
	AVPacket *packet = &pkt1;
	int got_frame;
	AVFrame *frame;
	// double pts;

	EGLBoolean success = eglMakeCurrent(global_context.eglDisplay,
			global_context.eglSurface, global_context.eglSurface,
			global_context.eglContext);
	if (!success) {
		av_log(NULL, AV_LOG_ERROR, "eglMakeCurrent() failed");
		return 0;
	}

	CreateProgram();

	frame = av_frame_alloc();
	bool started = false;

	for (;;) {
		static int counter = 0;
		//av_log(NULL, AV_LOG_ERROR, "video_decode_render_thread #%d", counter++);
		counter++;
		if (global_context.quit) {
			av_log(NULL, AV_LOG_ERROR, "video_decode_render_thread need exit. \n");
			break;
		}

		if (global_context.pause) {
			usleep(8000 / 2);
			continue;
		}

		if (packet_queue_get(&global_context.video_queue, packet) <= 0) {
			// means we quit getting packets
			if (started) {
				break;
			}
			usleep(8000 / 2);
			continue;
		}

		avcodec_decode_video2(global_context.vcodec_ctx, frame, &got_frame, packet);

		//av_log(NULL, AV_LOG_ERROR, "packet_queue_get size i%d, format %d\n", packet->size, frame->format);
		av_log(NULL, AV_LOG_ERROR, "frameDecoded/Finished: %d , for-counter %d", got_frame, counter);

		// Did we get a video frame?
		if (got_frame) {
			AVPicture pict;
			// uint8_t *dst_data[4];
			// int dst_linesize[4];

			started = true;
            //static int cc = 0;
            static clock_t t0 = clock();
            //if (cc++ > 24*1.5)
                av_log(NULL, AV_LOG_INFO, ">>> video show time: %.3f", ((double)(clock() - t0)) / CLOCKS_PER_SEC);
			av_image_alloc(pict.data, pict.linesize,
					global_context.vcodec_ctx->width,
					global_context.vcodec_ctx->height, AV_PIX_FMT_RGB565LE, 16);

			// Convert the image into YUV format that SDL uses
			img_convert(&pict, AV_PIX_FMT_RGB565LE, (AVPicture *) frame,
					global_context.vcodec_ctx->pix_fmt,
					global_context.vcodec_ctx->width,
					global_context.vcodec_ctx->height);

			renderSurface(pict.data[0]);

			av_freep(&pict.data[0]);
		}

		av_packet_unref(packet);
		av_init_packet(packet);

		// about framerate
		usleep(8000);
	}

	av_free(frame);

	return 0;
}
