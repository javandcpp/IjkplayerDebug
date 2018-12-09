//
// Created by developer on 2018/9/16.
//

#include "FFmpegAudioResample.h"
#include "../global_header.h"

/**
 * 重采样初始化
 * @param in
 * @param out
 * @return
 */
bool FFmpegAudioResample::Init(AVParameters in, AVParameters out) {

    //音频重采样上下文初始化
    actx = swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(out.channels),
                              AV_SAMPLE_FMT_S16, out.sample_rate,
                              av_get_default_channel_layout(in.codecParameters->channels),
                              (AVSampleFormat) in.codecParameters->format,
                              in.codecParameters->sample_rate,
                              0, 0);

    int re = swr_init(actx);
    if (re != 0) {
        LOGE("swr_init failed!");
        return false;
    } else {
        LOGD("swr_init success!");
    }
    outChannels = in.codecParameters->channels;
    outFormat = AV_SAMPLE_FMT_S16;
    return true;

}


/**
 * 重采样audio
 * @param indata
 * @return
 */
AVData FFmpegAudioResample::Resample(AVData indata) {
    if (indata.size <= 0 || !indata.data) return AVData();
    if (!actx)
        return AVData();
    //XLOGE("indata pts is %d",indata.pts);
    AVFrame *frame = (AVFrame *) indata.data;

    //输出空间的分配
    AVData out;
    int outsize =
            outChannels * frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat) outFormat);
    if (outsize <= 0)return AVData();
    out.Alloc(outsize);
    uint8_t *outArr[2] = {0};
    outArr[0] = out.data;
    int len = swr_convert(actx, outArr, frame->nb_samples, (const uint8_t **) frame->data,
                          frame->nb_samples);
    if (len <= 0) {
        out.Drop();
        return AVData();
    }
    out.pts = indata.pts;
    LOGD("reample audio");
    return out;
}
