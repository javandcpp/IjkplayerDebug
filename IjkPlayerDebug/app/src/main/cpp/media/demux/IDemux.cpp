//
// Created by developer on 2018/8/27.
//

#include "IDemux.h"
#include "../global_header.h"

void IDemux::main() {
#if 0
    int videoFrameCount=0;
    int audioFrameCount=1;
    struct  timeval    videoStart;
    struct  timeval    audioStart;
    gettimeofday(&videoStart,NULL);
    gettimeofday(&audioStart,NULL);
#endif
    while (!isExit) {
        AVData data = readMediaData();
        if(data.size>0) {
            if (data.isAudio) {
                LOGD("audio stream   pts:%ld", data.pts);

                NativeThread::readAudioPts = data.pts;
                if (mAudioDecode) {
                    mAudioDecode->update(data);
                }
                LOG_D("readaudio pts %lld", readAudioPts);
                xsleep(3);
            }

#if 0
                struct  timeval    endval;
                gettimeofday(&endval,NULL);
                long diff = 1000000 * (endval.tv_sec-audioStart.tv_sec)+ endval.tv_usec-audioStart.tv_usec;
                LOG_E("diff:%ld",diff);
                if(diff/1000000>=3){
                    long i = diff / 1000000;
                    LOG_E("interval time:%ld  audio decodeframecount:%d",i,audioFrameCount/i);
                    audioFrameCount=0;
                    audioStart.tv_sec=endval.tv_sec;
                    audioStart.tv_usec=endval.tv_usec;
                }

                audioFrameCount++;
#endif
            else {
                LOGD("video stream   pts:%ld", data.pts);
                NativeThread::readVideoPts = data.pts;
                if (mVideoDecode) {
                    mVideoDecode->update(data);
                }
                LOG_D("read video pts %lld", readVideoPts);
                xsleep(2);
            }
        }

#if 0
            struct  timeval    endval;
            gettimeofday(&endval,NULL);
            long diff = 1000000 * (endval.tv_sec-videoStart.tv_sec)+ endval.tv_usec-videoStart.tv_usec;
            LOG_E("diff:%ld",diff);
            if(diff/1000000>=3){
                long i = diff / 1000000;
                LOG_E("interval time:%ld  video decodeframecount:%d",i,videoFrameCount/i);
                videoFrameCount=0;
                videoStart.tv_usec=endval.tv_usec;
                videoStart.tv_sec=endval.tv_sec;
            }

            videoFrameCount++;
#endif

        }

//        if (data.size > 0) {
//            notifyObserver(data);
//        }

}
