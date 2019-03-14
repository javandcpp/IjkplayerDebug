
#include <jni.h>
#include "global_header.h"
#include "VideoCompressComponent.h"
#include "streamer/RtmpStreamer.h"

const char *CLASS_NAME = "com/stone/media/VideoCompress";
static VideoCompressComponent *videoCompressComponent = nullptr;
static JavaVM *g_vm;
static jclass globalClazz = NULL;
using namespace std;
mutex mtx;
static int init;
static jmethodID progress = NULL;

void stream_close(void *p) {
    JNIEnv *env = NULL;
      LOG_E("stream close");
    if (g_vm != NULL) {
        jint status = g_vm->AttachCurrentThread(&env, NULL);
        if (globalClazz) {
            jmethodID jMethodComplete = env->GetStaticMethodID(globalClazz, "completeFromNative",
                                                               "(Ljava/lang/String;)V");
            if (videoCompressComponent) {
                jstring pJstring = env->NewStringUTF(videoCompressComponent->destPath);
                env->CallStaticVoidMethod(globalClazz, jMethodComplete,
                                          pJstring);
                env->CallStaticVoidMethod(globalClazz, progress,
                                          100, (int)0);
                env->DeleteLocalRef(pJstring);
            }
            env->DeleteGlobalRef(globalClazz);
            delete videoCompressComponent;
            videoCompressComponent= NULL;

        }
        if (status == JNI_OK) {
            g_vm->DetachCurrentThread();
        }
    }
}

void stream_stop(void *p) {
    JNIEnv *env = NULL;
    if (g_vm != NULL) {
        jint status = g_vm->AttachCurrentThread(&env, NULL);
        if (globalClazz) {
            env->DeleteGlobalRef(globalClazz);
            LOGE("stream stop");
        }
    }
    if (videoCompressComponent) {
        delete videoCompressComponent;
        videoCompressComponent=NULL;
    }
}

void stream_progress(long totalMills, long currentMills) {
//    int totalSeconds = totalMills / 1000;
    int perCent = (int) ((float) currentMills  / (float) totalMills*100);
//    LOGE("total:%ld cur:%ld,progress:%d",totalMills,currentMills, perCent);
    JNIEnv *env = NULL;
    if (g_vm != NULL) {
        jint status = g_vm->AttachCurrentThread(&env, NULL);
        if (globalClazz) {
            env->CallStaticVoidMethod(globalClazz, progress,
                                      perCent, (int)currentMills);
        }
        if (status == JNI_OK) {
            g_vm->DetachCurrentThread();
        }
    }
}


extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    jint result = -1;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }
    g_vm = vm;
    if (!init) {
        av_jni_set_java_vm(vm, NULL);
        init = 1;
        LOGE("init ffmpeg");
    }

    return JNI_VERSION_1_6;
}





//
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_stone_media_VideoCompress_videoCompress(JNIEnv *env, jobject instance, jstring url_,
                                                 jint width, jint height, jstring dest_) {

    mtx.lock();
    const char *url = env->GetStringUTFChars(url_, 0);
    const char *destPath = env->GetStringUTFChars(dest_, 0);
    long widthPram = width;
    long heightParma = height;
    size_t i1 = strlen(destPath);
    char *path= (char *) malloc(i1);
    strcpy(path,destPath);

    jclass tmp = env->FindClass(CLASS_NAME);
    globalClazz = (jclass) env->NewGlobalRef(tmp);//这一步很重要必须这么写，否则报错
    progress = env->GetStaticMethodID(globalClazz, "progressFromNative",
                                      "(II)V");
    if (videoCompressComponent) {
        if(videoCompressComponent->isRunning) {
            if (globalClazz) {
                jmethodID pID = env->GetStaticMethodID(globalClazz, "callbackFromNative", "(Z)V");
                jboolean i = 0;

                env->CallStaticVoidMethod(globalClazz, pID, i);/**/
            }
            mtx.unlock();
            return 0;
        }

    }
    videoCompressComponent = new VideoCompressComponent();

    if (videoCompressComponent) {
        videoCompressComponent->setMScaleWidth(widthPram);
        videoCompressComponent->setMScaleHeight(heightParma);
        videoCompressComponent->initialize();
        videoCompressComponent->setCallback(stream_close);
        videoCompressComponent->setStopCallBack(stream_stop);
        videoCompressComponent->setProgressCallBack(stream_progress);
        videoCompressComponent->setDestPath(path);
        videoCompressComponent->openSource(url);
    }
    env->ReleaseStringUTFChars(url_, url);
    env->ReleaseStringUTFChars(dest_, destPath);
    mtx.unlock();

    return 0;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_stone_media_VideoCompress_stop(JNIEnv
                                        *env,
                                        jobject instance) {
    mtx.lock();

    if (videoCompressComponent) {
        if(videoCompressComponent->isRunning) {
            videoCompressComponent->stop();
        }
    }
    mtx.unlock();

}