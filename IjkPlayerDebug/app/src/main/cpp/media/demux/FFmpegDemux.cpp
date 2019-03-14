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
    LOG_E("ffmpegDemux release");

    if (audioAvParameters) {
        delete audioAvParameters;
        audioAvParameters = NULL;
    }
    if (videoAvParameters) {
        delete videoAvParameters;
        videoAvParameters = NULL;
    }
}

MetaData FFmpegDemux::getMetaData() {
    MetaData metaData;
    metaData.duration = mVideoDuration;
    metaData.video_rotate = mVideoRotate;
    metaData.video_width = video_src_width;
    metaData.video_height = video_src_height;

    return metaData;
}


void FFmpegDemux::setAvFormatContext(AVFormatContext *avformatContext) {
    this->avFormatContext = avformatContext;
}


/**
 * 获取时长
 */
void FFmpegDemux::getDuration() {
    if (avFormatContext) {
        mVideoDuration = (avFormatContext->streams[videoStreamIndex]->duration);
    }
}


/**
 * 获取旋转角度
 */
void FFmpegDemux::getRotate() {
    if (!hasRotateValue) {
        AVDictionaryEntry *m = NULL;
        m = av_dict_get(avFormatContext->streams[videoStreamIndex]->metadata, "rotate", m,
                        AV_DICT_IGNORE_SUFFIX);
        if (m) {
            LOG_E("====Key:%s ===value:%s\n", m->key, m->value);
            hasRotateValue = true;
            mVideoRotate = m->value;
        }
    }
}

static int custom_interrupt_callback(void *arg) {
    FFmpegDemux *fFmpegDemux = (FFmpegDemux *) arg;
    return 0;//0继续阻塞
}


double FFmpegDemux::get_rotation(AVStream *st) {
    AVDictionaryEntry *rotate_tag = av_dict_get(st->metadata, "rotate", NULL, 0);
    uint8_t *displaymatrix = av_stream_get_side_data(st,
                                                     AV_PKT_DATA_DISPLAYMATRIX, NULL);
    double theta = 0;

    if (rotate_tag && *rotate_tag->value && strcmp(rotate_tag->value, "0")) {
        char *tail;
        theta = av_strtod(rotate_tag->value, &tail);
        if (*tail)
            theta = 0;
    }
    if (displaymatrix && !theta)
        theta = -av_display_rotation_get((int32_t *) displaymatrix);

    theta -= 360 * floor(theta / 360 + 0.9 / 360);

    if (fabs(theta - 90 * round(theta / 90)) > 2)
        av_log(NULL, AV_LOG_WARNING, "Odd rotation angle.\n"
                "If you want to help, upload a sample "
                "of this file to ftp://upload.ffmpeg.org/incoming/ "
                "and contact the ffmpeg-devel mailing list. (ffmpeg-devel@ffmpeg.org)");

    return theta;
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
    if (!avFormatContext) {
        return AVData();
    }

    AVData avData;
    AVPacket *pkt = av_packet_alloc();
    int re = av_read_frame(avFormatContext, pkt);
    if (re != 0) {
        if (re == AVERROR_EOF) {
//            while(!isExit) {
//                xsleep(100);
            AVData avData;
            MediaEvent mediaEvent((Object &) *this, writeVideoPts, writeAudioPts, readVideoPts,
                                  readAudioPts);
            EventBus::FireEvent(mediaEvent);
//            }
            return avData;
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


//    pkt->pts = pkt->pts * (1000 * r2d(avFormatContext->streams[pkt->stream_index]->time_base));
//    pkt->dts = pkt->dts * (1000 * r2d(avFormatContext->streams[pkt->stream_index]->time_base));
//    avData.duration = pkt->duration =
//            pkt->duration * (1000 * r2d(avFormatContext->streams[pkt->stream_index]->time_base));
//    avData.pts=pkt->pts;
    if (avData.isAudio) {
        LOGD("read audio media data size:%d pts:%lld ", avData.size, pkt->pts);
    } else {
        LOGD("read video media data size:%d pts:%lld ", avData.size, pkt->pts);
    }

    if (avData.isAudio) {
        //AAC PTS=sample rate/1024*n;
        //MP3 PTS= ....../1152
//        audioPts += 1024;
        avData.pts = pkt->pts;
        LOGD("audio pts:%lld   pts:%lld", avData.pts, audioPts);

    } else {
        if (pkt->pts > pkt->dts) {//B Frame
            pkt->pts = pkt->dts;
        }
        avData.pts = pkt->pts;
//         = videoPtsRatio;
        LOG_D("read packet size:%d  pts:%lld", pkt->size, pkt->pts);
        LOGD("video pts:%lld   pts:%lld", avData.pts, videoPts);
    }
    return avData;
}

void FFmpegDemux::setStreamer(RtmpStreamer *pStreamer) {
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
    this->videoStreamIndex = videoStreamIndex;
}

int FFmpegDemux::getAudioStreamIndex() const {
    return audioStreamIndex;
}

void FFmpegDemux::setAudioStreamIndex(int audioStreamIndex) {
    this->audioStreamIndex = audioStreamIndex;
}

AVStream *FFmpegDemux::getAudioStream() const {
    return audioStream;
}

AVStream *FFmpegDemux::getVideoStream() const {
    return videoStream;
}

void FFmpegDemux::addVideoDecode(FFmpegDecode *pDecode) {
    this->mVideoDecode = pDecode;
}

void FFmpegDemux::addAudioDecode(FFmpegDecode *pDecode) {
    this->mAudioDecode = pDecode;
}

