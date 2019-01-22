
#include <jni.h>
#include "global_header.h"
#include "VideoCompressComponent.h"
#include "streamer/FileStreamer.h"

VideoCompressComponent *videoCompressComponent = NULL;
static JavaVM *g_vm;
static jclass globalClazz = NULL;

void stream_close(void *p) {
    JNIEnv *env = NULL;
    if (g_vm != NULL) {
        JavaVMAttachArgs thread_args;
        thread_args.name = "close callback";
        thread_args.version = JNI_VERSION_1_6;
        thread_args.group = NULL;
        jint status = g_vm->AttachCurrentThread(&env, NULL);
        if (globalClazz) {
            jmethodID jMethodComplete = env->GetStaticMethodID(globalClazz, "completeFromNative",
                                                               "(Ljava/lang/String;)V");
            if (videoCompressComponent) {
                jstring pJstring = env->NewStringUTF(videoCompressComponent->destPath);
                env->CallStaticVoidMethod(globalClazz, jMethodComplete,
                                          pJstring);
                env->DeleteLocalRef(pJstring);
            }
            env->DeleteGlobalRef(globalClazz);
            delete videoCompressComponent;

        }
        if (status == JNI_OK) {
            g_vm->DetachCurrentThread();
        }
    }


//    if (env->ExceptionCheck()) {
//        env->ExceptionDescribe();
//    }


}

using namespace std;
mutex mtx;

static int init;


extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

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






extern "C" JNIEXPORT jboolean JNICALL
Java_com_stone_media_VideoCompress_videoCompress(JNIEnv *env, jobject instance, jstring url_,
                                                 jint width, jint height, jstring dest_) {

    mtx.lock();
    const char *url = env->GetStringUTFChars(url_, 0);
    const char *destPath = env->GetStringUTFChars(dest_, 0);
    long widthPram = width;
    long heightParma = height;

    jclass tmp = env->FindClass("com/stone/media/VideoCompress");
    globalClazz = (jclass) env->NewGlobalRef(tmp);//这一步很重要必须这么写，否则报错

    if (videoCompressComponent && videoCompressComponent->isRunning) {
        if (globalClazz) {
            jmethodID pID = env->GetStaticMethodID(globalClazz, "callbackFromNative", "(Z)V");
            jboolean i = 0;

            env->CallStaticVoidMethod(globalClazz, pID, i);/**/
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

