
#include <jni.h>
#include "global_header.h"
#include "VideoCompressComponent.h"
#include "streamer/FileStreamer.h"

VideoCompressComponent *videoCompressComponent = NULL;
jmethodID jMethodComplete = NULL;
JNIEnv *env = NULL;
jclass globalClazz = NULL;
jobject pJobject = NULL;


void stream_close(void *) {

    //TODO
    if (videoCompressComponent) {
        const char *path = videoCompressComponent->destPath;
//        if (globalClazz) {
//            env->CallStaticVoidMethod(globalClazz, jMethodComplete,
//                                            env->NewStringUTF(path));/**/
//            if (pJobject) {
//                env->DeleteGlobalRef(pJobject);
//            }
//        }
        delete videoCompressComponent;
    }

}

using namespace std;
mutex mtx;

static int init;


extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    jint result = -1;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    if (!init) {
        av_jni_set_java_vm(vm, NULL);
        init = 1;
        LOGE("init ffmpeg");
    }

    return JNI_VERSION_1_4;
}




extern "C" JNIEXPORT jboolean JNICALL
Java_com_stone_media_VideoCompress_videoCompress(JNIEnv *env, jobject instance, jstring url_,
                                                 jint width, jint height, jstring dest_) {

    mtx.lock();
    globalClazz = env->FindClass("com/stone/media/VideoCompress");
    pJobject = env->NewGlobalRef(globalClazz);
    jMethodComplete = env->GetStaticMethodID(globalClazz, "completeFromNative",
                                             "(Ljava/lang/String;)V");

    const char *url = env->GetStringUTFChars(url_, 0);
    const char *destPath = env->GetStringUTFChars(dest_, 0);
    long widthPram = width;
    long heightParma = height;
    if (videoCompressComponent && videoCompressComponent->isRunning) {
        if (globalClazz) {
            jmethodID pID = env->GetStaticMethodID(globalClazz, "callbackFromNative", "(Z)V");
            jboolean i = 0;
            env->CallStaticVoidMethod(globalClazz, pID, i);/**/
//            if (pJobject) {
//                env->DeleteGlobalRef(pJobject);
//            }
        }
        mtx.unlock();
        return 0;
    }
    videoCompressComponent = new VideoCompressComponent();
    if (videoCompressComponent) {
        videoCompressComponent->setMScaleWidth(widthPram);
        videoCompressComponent->setMScaleHeight(heightParma);
        videoCompressComponent->initialize();
        videoCompressComponent->setCallback(stream_close);
        videoCompressComponent->setDestPath(destPath);
        videoCompressComponent->openSource(url);
    }
    env->ReleaseStringUTFChars(url_, url);
    env->ReleaseStringUTFChars(dest_, destPath);
    mtx.unlock();

    return 0;

}

