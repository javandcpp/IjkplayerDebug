
#include <jni.h>
#include "global_header.h"
#include "VideoCompressComponent.h"
using namespace std;
mutex mtx;

VideoCompressComponent *videoCompressComponent=NULL;
static int init;



extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
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
                                                 jint width, jint height) {

    mtx.lock();
    const char *url = env->GetStringUTFChars(url_, 0);
    videoCompressComponent=new VideoCompressComponent();
    if(videoCompressComponent){
        videoCompressComponent->initialize();
        videoCompressComponent->openSource(url);
        videoCompressComponent->getDemux()->startThread();

    }

    env->ReleaseStringUTFChars(url_, url);
    mtx.unlock();
    return 0;
}

