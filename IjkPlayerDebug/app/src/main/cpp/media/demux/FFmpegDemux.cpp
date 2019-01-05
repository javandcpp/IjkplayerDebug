//
// Created by developer on 2018/8/27.
//


#include "FFmpegDemux.h"
#include "../global_header.h"


bool FFmpegDemux::isFirst = false;


static double r2d(AVRational avRational) {
    return avRational.num == 0 || avRational.den == 0 ? 0. : (double) avRational.num /
                                                             (double) avRational.den;
}

FFmpegDemux::FFmpegDemux() {
    if (!isFirst) {
        initAVCodec();

        isFirst = false;
    }
}

FFmpegDemux::~FFmpegDemux() {

}

static int custom_interrupt_callback(void *arg) {
    FFmpegDemux *fFmpegDemux = (FFmpegDemux *) arg;
    return 0;//0继续阻塞
}


bool FFmpegDemux::open(const char *url) {
    int ret = 0;
    avFormatContext = avformat_alloc_context();
    if (!avFormatContext) {
        LOGE("Could not allocate context.\n");
    }
    avFormatContext->interrupt_callback.callback = custom_interrupt_callback;
    avFormatContext->interrupt_callback.opaque = this;
    if ((ret = avformat_open_input(&avFormatContext, url, 0, 0)) < 0) {
        LOGE("avformat open input failed  :%s!", av_err2str(ret));
        goto fail;
    }
    LOGD("avformat open input successful!");
    if ((ret = avformat_find_stream_info(avFormatContext, 0)) < 0) {
        LOGE("avformat find stream info failed:  %s", av_err2str(ret));
        goto fail;
    }

    setVideoStreamIndex(av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0));
    setAudioStreamIndex(av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, 0, 0));

    LOGD("find stream index  videoStream:%d   audioStream:%d", getVideoStreamIndex(),
         getAudioStreamIndex());

    return true;

    fail:
    return false;
}

AVParameters *FFmpegDemux::getAudioParameters() {
    if (!avFormatContext || getAudioStreamIndex() < 0) NULL;
    audioAvParameters = new AVParameters();
    audioAvParameters->codecParameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(audioAvParameters->codecParameters,
                            avFormatContext->streams[getAudioStreamIndex()]->codecpar);
    return audioAvParameters;
}

AVParameters *FFmpegDemux::getVideoParamters() {
    if (!avFormatContext || getVideoStreamIndex() < 0)NULL;
    videoAvParameters = new AVParameters();
    videoAvParameters->codecParameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(videoAvParameters->codecParameters,
                            avFormatContext->streams[getVideoStreamIndex()]->codecpar);
    return videoAvParameters;
}

/**
 * 解码
 * @return
 */
AVData FFmpegDemux::readMediaData() {
    if (!avFormatContext)return AVData();

    AVData avData;
    AVPacket *pkt = av_packet_alloc();
    int re = av_read_frame(avFormatContext, pkt);
    if (re != 0) {
        if(re==AVERROR_EOF){
            isExit=true;
            //读到结尾
            LOGE("read frame eof");
            return AVData();
        }
        char buf[100] = {0};
        av_strerror(re, buf, sizeof(buf));
        LOGD("av_read_frame:%s", buf);
        return AVData();
    }
    avData.data = (unsigned char *) pkt;
    avData.size = pkt->size;

    if (pkt->stream_index == audioStreamIndex) {
        avData.isAudio = true;
    } else if (pkt->stream_index == videoStreamIndex) {
        avData.isAudio = false;
    } else {
        av_packet_free(&pkt);
        return AVData();
    }


    pkt->pts = pkt->pts* (1000*r2d(avFormatContext->streams[pkt->stream_index]->time_base));
    pkt->dts = pkt->dts* (1000*r2d(avFormatContext->streams[pkt->stream_index]->time_base));
    avData.pts = (int) pkt->pts;
    if (avData.isAudio) {
        LOGD("read audio media data size:%d pts:%lld ", avData.size, pkt->pts);
    } else {
        LOGD("read video media data size:%d pts:%lld ", avData.size, pkt->pts);
    }

    if (avData.isAudio) {
        audioPts = avData.pts;
    } else {
        videoPts = avData.pts;
    }
//    if (pkt->pts/1000 > 100) {
//        if (streamer) {
//            ((FileStreamer *) streamer)->ClosePushStream();
//            isExit = true;
////            streamer= nullptr;
//            LOGD("stop");
//        }
//    }
    return avData;
}

void FFmpegDemux::setStreamer(FileStreamer *pStreamer) {
    this->streamer = pStreamer;
}

void FFmpegDemux::initAVCodec() {
    av_register_all();
    avformat_network_init();
    LOGE("initAVCodec");
}

int FFmpegDemux::getVideoStreamIndex() const {
    return videoStreamIndex;
}

void FFmpegDemux::setVideoStreamIndex(int videoStreamIndex) {
    FFmpegDemux::videoStreamIndex = videoStreamIndex;
}

int FFmpegDemux::getAudioStreamIndex() const {
    return audioStreamIndex;
}

void FFmpegDemux::setAudioStreamIndex(int audioStreamIndex) {
    FFmpegDemux::audioStreamIndex = audioStreamIndex;
}
