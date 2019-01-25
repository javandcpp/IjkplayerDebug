//
// Created by developer on 2018/9/15.
//

#include "FFmpegDecode.h"
#include "../global_header.h"


FFmpegDecode::FFmpegDecode() {
    LOGE("initFFmpegDecode");
    pFILE = fopen("/mnt/sdcard/test.yuv", "wb+");
}

FFmpegDecode::~FFmpegDecode() {
    if (pFILE) {
        fclose(pFILE);
        pFILE = NULL;
    }
    if (video_out_buffer) {
        av_free(video_out_buffer);
    }
    if (inAvFrame) {
        av_frame_free(&inAvFrame);
    }

    if (outAvFrame) {
        av_frame_free(&outAvFrame);
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
    }

    LOG_E("ffmpegDecode release");
}

/**
 * 打开解码器
 * @param parameters 
 * @return 
 */
bool FFmpegDecode::openCodec(AVParameters parameters) {

    if (!parameters.codecParameters) {
        LOGD("openCodec failed!");
        return false;
    }
    AVCodec *codec = NULL;
    if (parameters.codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
//        codec = avcodec_find_decoder_by_name("h264_mediacodec");
        codec = avcodec_find_decoder(parameters.codecParameters->codec_id);
    } else {
        codec = avcodec_find_decoder_by_name("libfdk_aac");
//        codec = avcodec_find_decoder(parameters.codecParameters->codec_id);
    }

    if (!codec) {
        LOGD("avcodec find decoder failed!");
        return false;
    }


    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, parameters.codecParameters);
    codecContext->thread_count = 8;
    int ret = avcodec_open2(codecContext, NULL, NULL);
    if (ret < 0) {
        LOGE("avcodec open error :%s", av_err2str(ret));
        return false;
    }
    outAvFrame = av_frame_alloc();
    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        isAudio = true;
        LOGD("avcodec audio open2 success!");
    } else {
        isAudio = false;
        LOGD("avcodec video open2 success!");
        long outputWidth = mScaleWidth;
        long outputHeight = mScaleHeight;
        sws_ctx =
                sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                               outputWidth, outputHeight, AV_PIX_FMT_YUV420P,
                               SWS_BICUBIC, NULL, NULL, NULL);


        int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, outputWidth,
                                                   outputHeight, 1);
        video_out_buffer = (uint8_t *) av_malloc(buffer_size);
        av_image_fill_arrays(outAvFrame->data, outAvFrame->linesize, video_out_buffer,
                             AV_PIX_FMT_YUV420P, outputWidth, outputHeight, 1);

    }
    return true;
}


bool FFmpegDecode::sendPacket(AVData pkt) {
    AVPacket *packet = (AVPacket *) pkt.data;
    int re = avcodec_send_packet(codecContext, packet);
//    av_packet_free(&packet);
    if (re != 0) {
        LOGE("ffmpeg sendPacket failed :%s", av_err2str(re));
        return false;
    }
//    LOGE("ffmpeg sendPacket success size:%d pts:%d", pkt.size,pkt.pts);
    return true;
}

AVData FFmpegDecode::receiveCacheFrame() {
    while (true) {
        int re = avcodec_send_packet(codecContext, NULL);
        while (true) {
            if (!inAvFrame) {
                inAvFrame = av_frame_alloc();
            }
            int ret = avcodec_receive_frame(codecContext, inAvFrame);
            if (ret == AVERROR_EOF) {

//                LOGE("------->avcodec receive cached end");
                isRunning = true;
                return AVData();
            }
            AVData avData;
            avData.data = (unsigned char *) inAvFrame;
//            LOGE("------->avcodec receive cached frame success  pts:%lld",
//                 ((AVFrame *) avData.data)->pts);
            if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
                //样本字节数 * 单通道样本数 * 通道数
                avData.size =
                        av_get_bytes_per_sample((AVSampleFormat) inAvFrame->format) *
                        inAvFrame->nb_samples * 2;

            } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
                avData.size =
                        (inAvFrame->linesize[0] + inAvFrame->linesize[1] + inAvFrame->linesize[2]) *
                        inAvFrame->height;
                avData.width = inAvFrame->width;
                avData.height = inAvFrame->height;

//                sws_scale();
            }

            avData.format = inAvFrame->format;
            memcpy(avData.datas, inAvFrame->data, sizeof(avData.datas));
            avData.pts = inAvFrame->pts;
            return avData;
        }
    }

}


AVData FFmpegDecode::receiveFrame() {
//    0:                 success, a frame was returned
//    *      AVERROR(EAGAIN):   output is not available right now - user must try
//    *                         to send new input
//    *      AVERROR_EOF:       the decoder has been fully flushed, and there will be
//    *                         no more output frames
//    *      AVERROR(EINVAL):   codec not opened, or it is an encoder
//    *      other negative values: legitimate decoding errors
    if (!codecContext || !outAvFrame) {
        return AVData();
    }
//    if (!inAvFrame) {
    inAvFrame = av_frame_alloc();
//    }
    int ret = avcodec_receive_frame(codecContext, inAvFrame);
    if (ret != 0) {
        char buf[100] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("avcodec receive error :%s", buf);
        return AVData();
    }
    AVData avData;
    avData.data = (unsigned char *) inAvFrame;
    LOGE("------->avcodec receive frame success  pts:%lld", inAvFrame->pts);
    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        //样本字节数 * 单通道样本数 * 通道数
        avData.size =
                av_get_bytes_per_sample((AVSampleFormat) inAvFrame->format) *
                inAvFrame->nb_samples * 2;
        avData.isAudio = true;

        avData.format = inAvFrame->format;
        memcpy(avData.datas, inAvFrame->data, sizeof(inAvFrame->data));
        avData.pts = inAvFrame->pts;
        avData.pkt_pts = inAvFrame->pkt_pts;


    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        //过滤B帧
        if (inAvFrame->pict_type==AV_PICTURE_TYPE_B) {
//            av_frame_free(&inAvFrame);
//            LOG_E("is b frame");
//            return AVData();
        }

        sws_scale(sws_ctx, (const uint8_t *const *) inAvFrame->data,
                  inAvFrame->linesize, 0, codecContext->height,
                  outAvFrame->data, outAvFrame->linesize);

        avData.size =
                (outAvFrame->linesize[0] + outAvFrame->linesize[1] + outAvFrame->linesize[2]) *
                codecContext->height;
        avData.width = codecContext->width;
        avData.height = codecContext->height;
        avData.isAudio = false;
        avData.linesize[0] = outAvFrame->linesize[0];
        avData.linesize[1] = outAvFrame->linesize[1];
        avData.linesize[2] = outAvFrame->linesize[2];


        int i = mScaleWidth * mScaleHeight;
//        fwrite(outAvFrame->data[0], 1, i, pFILE);
//        fwrite(outAvFrame->data[1], 1, i / 4, pFILE);
//        fwrite(outAvFrame->data[2], 1, i / 4, pFILE);
//        fflush(pFILE);

        avData.format = inAvFrame->format;
        memcpy(avData.datas, outAvFrame->data, sizeof(outAvFrame->data));
        avData.pts = inAvFrame->pts;
        avData.pkt_pts = inAvFrame->pkt_pts;
    }


    av_frame_free(&inAvFrame);

    return avData;
}

void FFmpegDecode::setVideoScaleHeight(long i) {
    this->mScaleHeight = i;
}

void FFmpegDecode::setVideoScaleWidth(long i) {
    this->mScaleWidth = i;
}

void FFmpegDecode::addAudioEncode(AudioEncoder *pEncoder) {
    this->mAudioEncoder = pEncoder;
}


void FFmpegDecode::addVideoEncode(VideoEncoder *pEncoder) {
    this->mVideoEncoder = pEncoder;
}


