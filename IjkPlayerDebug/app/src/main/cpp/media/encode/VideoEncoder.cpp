//
// Created by developer on 11/13/17.
//


#include "VideoEncoder.h"
#include "VideoEncodeArgs.h"
#include <stdio.h>

#include <time.h>


VideoEncoder::VideoEncoder() {

    pFILE = fopen("/mnt/sdcard/testdecode.h264", "wb+");

}



extern "C" int64_t calc_dynamic_bitrate(int quality, int width, int height, int fps) {
    static const int BasicRate[4] = {240, 400, 500, 700};

    int64_t bit_rate = 0;
    double factor = sqrt((double) (width * height * fps) / (360 * 640 * 10.0));

    if (quality <= 85) {
        //10KB
        bit_rate = (int64_t) (BasicRate[0] * factor * quality / 85 + 0.5);
    } else if (quality <= 100) {
        //16KB
        int BitrateTemp = BasicRate[0] + ((BasicRate[1] - BasicRate[0]) * (quality - 85) / 15.0);
        bit_rate = (int64_t) (BitrateTemp * factor + 0.5);
    } else if (quality <= 125) {
        //28KB
        int BitrateTemp = BasicRate[1] + ((BasicRate[2] - BasicRate[1]) * (quality - 100) / 25.0);
        bit_rate = (int64_t) (BitrateTemp * factor + 0.5);
    } else if (quality <= 150) {
        //40KB
        int BitrateTemp = BasicRate[2] + ((BasicRate[3] - BasicRate[2]) * (quality - 125) / 25.0);
        bit_rate = (int64_t) (BitrateTemp * factor + 0.5);
    } else {
        int BitrateTemp = BasicRate[3] + 16 * (quality - 150);
        bit_rate = (int64_t) (BitrateTemp * factor + 0.5);
    }
    bit_rate *= 1000;

    return bit_rate;
}

VideoEncoder::~VideoEncoder() {
    if (NULL != videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = NULL;
    }

    if (pFILE)
        fclose(pFILE);



    LOG_E("videoencoder release");

}

void VideoEncoder::update(AVData avData) {
    if (avData.isAudio) {
        return;
    }
    while (!isExit) {
//        if (aVideoframeQueue.Size()) {
        aVideoframeQueue.push(avData);
        LOGE("update push video queue data,pts:%ld   listsize:%d", avData.pts,
             aVideoframeQueue.Size());
        break;
    }
//        xsleep(1);
//    }

}

void VideoEncoder::main() {

#if 0
    struct  timeval    tv;
    gettimeofday(&tv,NULL);
    int frameCount=0;
#endif

    while (!isExit) {


        if (aVideoframeQueue.empty()) {
            xsleep(1);
            continue;
        }

        const shared_ptr<AVData> &ptr = aVideoframeQueue.wait_and_pop();
        AVData *pData = ptr.get();
        int ret = -1;
        int totalSize = pData->width * pData->height * 3 / 2;

        unsigned int ySize = (unsigned int) (pData->width * pData->height);
        memcpy(outputYUVFrame->data, pData->datas, sizeof(outputYUVFrame->data));//Y

        outputYUVFrame->linesize[0] = pData->linesize[0];
        outputYUVFrame->linesize[1] = pData->linesize[1];
        outputYUVFrame->linesize[2] = pData->linesize[2];

#if 0
        if (pFILE) {
            fwrite(outputYUVFrame->data[0], 1, ySize, pFILE);
            fwrite(outputYUVFrame->data[1], 1, ySize/4, pFILE);
            fwrite(outputYUVFrame->data[2], 1, ySize/4, pFILE);
            fflush(pFILE);
        }
#endif

//        LOGD("line size【0】 ：%d", outputYUVFrame->linesize[0]);
//        LOGD("line size【1】 ：%d", outputYUVFrame->linesize[1]);
//        LOGD("line size【2】 ：%d", outputYUVFrame->linesize[2]);
//
//        LOGE("video encode %d", pData->size);
//        if (ret < 0) {
//            LOGE("video fill frame failed!");
//            continue;
//        }
        //发送数据到解码线程，一个数据包，可能解码多个结果
//        outputYUVFrame->pts=pData->pkt_pts;


        videoPts = pData->pkt_pts;
        ret = avcodec_send_frame(videoCodecContext, outputYUVFrame);
        LOGE("video enencode avcodec_send_frame result:%d  pts:%lld", ret, pData->pts);
        if (ret == 0) {
            while (!isExit) {
                //获取解码数据
//                av_packet_unref(&videoPacket);
                ret = avcodec_receive_packet(videoCodecContext, &videoPacket);
                if (ret != 0) {
                    char buf[100] = {0};
                    av_strerror(ret, buf, sizeof(buf));
                    LOGE("video encode failed:%s", buf);
                    break;
                }
#if 0
                struct  timeval    endval;
                gettimeofday(&endval,NULL);
                long diff = 1000000 * (endval.tv_sec-tv.tv_sec)+ endval.tv_usec-tv.tv_usec;
                LOG_E("diff:%ld",diff);
                if(diff/1000000>=3){
                    long i = diff / 1000/1000;
                    LOG_E("interval time:%ld  video framecount:%d",i,frameCount/i);
                    frameCount=0;
                    gettimeofday(&tv,NULL);
                    gettimeofday(&endval,NULL);
                }

                frameCount++;
#endif

                AVData avData;

                avData.pts = pData->pts;
                AVPacket *avPacket = av_packet_alloc();
#if 0
                if(pFILE){
                    fwrite(videoPacket.data, 1, (size_t) videoPacket.size, pFILE);
                    fflush(pFILE);
                }
#endif


                av_packet_move_ref(avPacket, &videoPacket);//此处data指针指向重新分配的内存，并复制其他属性
                LOGD("video encode sucess  pts:%lld pktsize:%d  encodesize:%d", pData->pts,
                     videoPacket.size, avPacket->size);
                avData.avPacket = avPacket;
                avData.isAudio = false;
                avData.duration = pData->duration;
                this->notifyObserver(avData);
            }
        }
    }


}


int VideoEncoder::InitEncode(AVCodecParameters *avCodecParameters) {
    std::lock_guard<std::mutex> lk(mtx);
    int ret = 0;
    avCodec = avcodec_find_encoder_by_name("libx264");
    if (!avCodec) {
        LOGE("avcodec not found!");
        return -1;
    }
    videoCodecContext = avcodec_alloc_context3(avCodec);
    if (!videoCodecContext) {
        LOGE("avcodec alloc context failed!");
        return -1;
    }
    LOGE("avcodec alloc context success!");


    long long bitrate = 1024 * 1000;

    videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //全局参数
    videoCodecContext->codec_id = avCodec->id;
    videoCodecContext->bit_rate = bitrate;//压缩后每秒视频的bit位大小 50kB

    videoCodecContext->width = mEncodeWidth;
    videoCodecContext->height = mEncodeHeight;

    videoCodecContext->framerate = (AVRational) {30, 1};

    videoCodecContext->max_b_frames = 0;//0表示不使用B Frame
    videoCodecContext->qmin = 10;
    videoCodecContext->qmax = 40;
    videoCodecContext->qcompress = 0.5;
    videoCodecContext->time_base = (AVRational) {1, 12800};//AUDIO VIDEO 两边时间基数要相同
    videoCodecContext->time_base.den=30;
    videoCodecContext->time_base.num=1;
    videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    videoCodecContext->sample_aspect_ratio = AVRational{1, 1};
    videoCodecContext->thread_count = 8;
//    videoCodecContext->keyint_min=50;
    videoCodecContext->gop_size = 100;
    videoCodecContext->compression_level=0;

//    videoCodecContext->level = 41;
//    videoCodecContext->refs = 1;
//    videoCodecContext->chromaoffset = 2;

    videoCodecContext->bit_rate_tolerance = 8000;//CBR 固定允许的码率误差，数值越大，视频越小
    videoCodecContext->bit_rate = bitrate;
    videoCodecContext->rc_min_rate = bitrate;
    videoCodecContext->rc_max_rate = bitrate;
    videoCodecContext->bit_rate_tolerance = bitrate;
    videoCodecContext->rc_buffer_size = bitrate;
    videoCodecContext->rc_initial_buffer_occupancy = videoCodecContext->rc_buffer_size * 3 / 4;
//    videoCodecContext->rc_buffer_aggressivity= (float)1.0;
//    videoCodecContext->rc_initial_cplx= 0.5;

//    videoCodecContext->bit_rate_tolerance=bitrate;

//    }

    /**
     *选项: ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow and placebo.
     * 固定码率150K,
     * 设置slow时：编码速度：245 fps 4ms
     * medium 时：编码速度：335 fps 2ms,
     * veryslow 时：编码速度：140 fps 7ms
     */

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "preset", "ultrafast", 0);//编码器的速度会影响推流音视频同步,所以这里需要设置下
    av_dict_set(&opts, "tune", "zerolatency", 0);//如果开0延迟可能会影响视频质量
    av_dict_set(&opts, "profile", "baseline", 0);//I/P帧
//    av_dict_set(&opts, "x264opts","crf=30:vbv-maxrate=500:vbv-bufsize=3640:keyint=60",0);

    outputYUVFrame = av_frame_alloc();
    outputYUVFrame->format = AV_PIX_FMT_YUV420P;
    outputYUVFrame->width = videoCodecContext->width;//480
    outputYUVFrame->height = videoCodecContext->height;//640

    //分配yuv空间
    ret = av_frame_get_buffer(outputYUVFrame, 32);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        LOGE("input av_frame_get_buffer:%s", buf);
        return -1;
    }


    ret = avcodec_open2(videoCodecContext, avCodec, &opts);
    //avcodec open failed! info: Invalid argument
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("avcodec open failed! info: %s", buf);
        return -1;
    }
    LOGE("video avcodec open success");
    return 0;
}

int VideoEncoder::Release() {
    LOGD("Release Video Encode!");
    return 0;
}


int VideoEncoder::CloseEncode() {
    std::lock_guard<std::mutex> lk(mtx);
    if (isEncoding) {
        isEncoding = false;
        avcodec_close(videoCodecContext);
        LOGD("Close Video Encode!");
    }
    return 0;
}

AVCodecContext *VideoEncoder::getVideoCodecContext() {
    return videoCodecContext;
}

void VideoEncoder::setVideoEncodeWidth(long i) {
    this->mEncodeWidth = i;
}

void VideoEncoder::setVideoEncodeHeight(long i) {
    this->mEncodeHeight = i;
}

