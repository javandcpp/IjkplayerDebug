//
// Created by developer on 2018/12/9.
//

#ifndef IJKPLAYERDEBUG_GLOBAL_HEADER_H
#define IJKPLAYERDEBUG_GLOBAL_HEADER_H

#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#include "libavformat/avformat.h"
#include "libavformat/internal.h"
#include "libavutil/timestamp.h"
#include "libavutil/avassert.h"
#include "libavutil/internal.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavcodec/jni.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include <stdio.h>
};

#include <mutex>
#define TAG "hw_media"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

#endif

#endif //IJKPLAYERDEBUG_GLOBAL_HEADER_H
