//
// Created by developer on 2018/9/15.
//

#include "FFmpegDecode.h"
#include "../global_header.h"


FFmpegDecode::FFmpegDecode() {
    LOGE("initFFmpegDecode");
}

FFmpegDecode::~FFmpegDecode() {

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
        codec = avcodec_find_decoder_by_name("h264_mediacodec");
//        codec = avcodec_find_decoder(parameters.codecParameters->codec_id);
    } else {
        codec = avcodec_find_decoder(parameters.codecParameters->codec_id);
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

    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        isAudio = true;
        LOGD("avcodec audio open2 success!");
    } else {
        isAudio = false;
        LOGD("avcodec video open2 success!");
    }
    return true;
}

bool FFmpegDecode::sendPacket(AVData pkt) {
    int re = avcodec_send_packet(codecContext, (const AVPacket *) pkt.data);
    if (re != 0) {
//        LOGE("ffmpeg sendPacket failed :%s",av_err2str(re));
        return false;
    }
//    LOGE("ffmpeg sendPacket success size:%d pts:%d", pkt.size,pkt.pts);
    return true;
}

AVData FFmpegDecode::receiveCacheFrame() {
    while (true) {
        int re = avcodec_send_packet(codecContext, NULL);
        while (true) {
            if (!avFrame) {
                avFrame = av_frame_alloc();
            }
            int ret = avcodec_receive_frame(codecContext, avFrame);
            if (ret == AVERROR_EOF) {
//                LOGE("------->avcodec receive cached end");
                isRunning = true;
                return AVData();
            }
            AVData avData;
            avData.data = (unsigned char *) avFrame;
//            LOGE("------->avcodec receive cached frame success  pts:%lld",
//                 ((AVFrame *) avData.data)->pts);
            if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
                //样本字节数 * 单通道样本数 * 通道数
                avData.size =
                        av_get_bytes_per_sample((AVSampleFormat) avFrame->format) *
                        avFrame->nb_samples * 2;

            } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
                avData.size = (avFrame->linesize[0] + avFrame->linesize[1] + avFrame->linesize[2]) *
                              avFrame->height;
                avData.width = avFrame->width;
                avData.height = avFrame->height;
            }

            avData.format = avFrame->format;
            memcpy(avData.datas, avFrame->data, sizeof(avData.datas));
            avData.pts = avFrame->pts;
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
    if (!codecContext) {
        return AVData();
    }
    if (!avFrame) {
        avFrame = av_frame_alloc();
    }
    int ret = avcodec_receive_frame(codecContext, avFrame);
    if (ret != 0) {
        return AVData();
    }
    AVData avData;
    avData.data = (unsigned char *) avFrame;
    LOGE("------->avcodec receive frame success  pts:%lld", ((AVFrame *) avData.data)->pts);
    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        //样本字节数 * 单通道样本数 * 通道数
        avData.size =
                av_get_bytes_per_sample((AVSampleFormat) avFrame->format) * avFrame->nb_samples * 2;
        avData.isAudio=true;

    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        avData.size = (avFrame->linesize[0] + avFrame->linesize[1] + avFrame->linesize[2]) *
                      avFrame->height;
        avData.width = avFrame->width;
        avData.height = avFrame->height;
        avData.isAudio=false;
    }

    avData.format = avFrame->format;
    memcpy(avData.datas, avFrame->data, sizeof(avData.datas));
    avData.pts = avFrame->pts;

    return avData;
}

