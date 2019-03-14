//
// Created by developer on 2018/12/22.
//

#include "RtmpStreamer.h"
#include "../encode/AudioEncoder.h"
#include "../encode/VideoEncoder.h"
#include <iostream>
#include <thread>

using namespace std;


void RtmpStreamer::update(AVData avData) {
    std::lock_guard<std::mutex> lk(mtx);
    while (!isExit) {
        if (avData.isAudio) {
            mAudioframeQueue.push(avData);
            LOGD("push audio to streamer");
            break;
        } else if (!avData.isAudio) {
            mVideoframeQueue.push(avData);
            LOGD("push video to streamer");
            break;
        }
//    xsleep(1);//减少CPU时间片
//    }
    }
}

RtmpStreamer::RtmpStreamer() {

//    pFile=fopen("/mnt/sdcard/test.h264","wb+");
}

void RtmpStreamer::release() {
    LOG_E("RTMP STREAMER RELEASE");
    if (audioStream) {
        av_free(audioStream);
        audioStream = NULL;
    }
    if (videoStream) {
        av_free(videoStream);
        videoStream = NULL;
    }
    if (mAudioCodecContext) {
        avcodec_free_context(&mAudioCodecContext);
        mAudioCodecContext = NULL;
    }
    if (mVideoCodecContext) {
        avcodec_free_context(&mVideoCodecContext);
        mVideoCodecContext = NULL;
    }
    if (iAvFormatContext) {
        avformat_free_context(iAvFormatContext);
        iAvFormatContext = NULL;
    }
}

RtmpStreamer::~RtmpStreamer() {


}

RtmpStreamer *RtmpStreamer::Get() {
    static RtmpStreamer RtmpStreamer;
    return &RtmpStreamer;
}

int RtmpStreamer::InitStreamer(const char *url) {
    std::lock_guard<std::mutex> lk(mtx);
    this->outputUrl = url;
    int ret = 0;
    ret = avformat_alloc_output_context2(&iAvFormatContext, NULL, "flv", url);

    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGD("avformat alloc output context2 failed: %s", buf);

        return -1;
    }

    if (videoEncoder) {
        videoStreamIndex = AddStream(videoEncoder->getVideoCodecContext());
    }
    if (audioEncoder) {
        audioStreamIndex = AddStream(audioEncoder->getAudioCodecContext());
    }
    LOGD("InitStreamer Success!");
    return 0;
}


static void flush_if_needed(AVFormatContext *s) {
    if (s->pb && s->pb->error >= 0) {
        if (s->flush_packets == 1 || s->flags & AVFMT_FLAG_FLUSH_PACKETS)
            avio_flush(s->pb);
        else if (s->flush_packets && !(s->oformat->flags & AVFMT_NOFILE))
            avio_write_marker(s->pb, AV_NOPTS_VALUE, AVIO_DATA_MARKER_FLUSH_POINT);
    }
}

static void frac_add(FFFrac *f, int64_t incr) {
    int64_t num, den;

    num = f->num + incr;
    den = f->den;
    if (num < 0) {
        f->val += num / den;
        num = num % den;
        if (num < 0) {
            num += den;
            f->val--;
        }
    } else if (num >= den) {
        f->val += num / den;
        num = num % den;
    }
    f->num = num;
}

#if FF_API_COMPUTE_PKT_FIELDS2 && FF_API_LAVF_AVCTX

static int compute_muxer_pkt_fields(AVFormatContext *s, AVStream *st, AVPacket *pkt) {
    int delay = FFMAX(st->codecpar->video_delay, st->internal->avctx->max_b_frames > 0);
    int num, den, i;
    int frame_size;

    if (!s->internal->missing_ts_warning &&
        !(s->oformat->flags & AVFMT_NOTIMESTAMPS) &&
        (!(st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
         (st->disposition & AV_DISPOSITION_TIMED_THUMBNAILS)) &&
        (pkt->pts == AV_NOPTS_VALUE || pkt->dts == AV_NOPTS_VALUE)) {
        av_log(s, AV_LOG_WARNING,
               "Timestamps are unset in a packet for stream %d. "
                       "This is deprecated and will stop working in the future. "
                       "Fix your code to set the timestamps properly\n", st->index);
        s->internal->missing_ts_warning = 1;
    }

    if (s->debug & FF_FDEBUG_TS)
        av_log(s, AV_LOG_TRACE,
               "compute_muxer_pkt_fields: pts:%s dts:%s cur_dts:%s b:%d size:%d st:%d\n",
               av_ts2str(pkt->pts), av_ts2str(pkt->dts), av_ts2str(st->cur_dts), delay, pkt->size,
               pkt->stream_index);

    if (pkt->duration < 0 && st->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
        av_log(s, AV_LOG_WARNING, "Packet with invalid duration %" PRId64" in stream %d\n",
               pkt->duration, pkt->stream_index);
        pkt->duration = 0;
    }

    /* duration field */
    if (pkt->duration == 0) {
        ff_compute_frame_duration(s, &num, &den, st, NULL, pkt);
        if (den && num) {
            pkt->duration = av_rescale(1, num * (int64_t) st->time_base.den *
                                          st->codec->ticks_per_frame,
                                       den * (int64_t) st->time_base.num);
        }
    }

    if (pkt->pts == AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE && delay == 0)
        pkt->pts = pkt->dts;

    //XXX/FIXME this is a temporary hack until all encoders output pts
    if ((pkt->pts == 0 || pkt->pts == AV_NOPTS_VALUE) && pkt->dts == AV_NOPTS_VALUE && !delay) {
        static int warned;
        if (!warned) {
            av_log(s, AV_LOG_WARNING, "Encoder did not produce proper pts, making some up.\n");
            warned = 1;
        }
        if (!st->priv_pts) {
            LOG_E("IS NULL");
        }
        pkt->dts =
//        pkt->pts= st->cur_dts;
        pkt->pts = st->priv_pts->val;
    }

    //calculate dts from pts
    if (pkt->pts != AV_NOPTS_VALUE && pkt->dts == AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY) {
        st->pts_buffer[0] = pkt->pts;
        for (i = 1; i < delay + 1 && st->pts_buffer[i] == AV_NOPTS_VALUE; i++)
            st->pts_buffer[i] = pkt->pts + (i - delay - 1) * pkt->duration;
        for (i = 0; i < delay && st->pts_buffer[i] > st->pts_buffer[i + 1]; i++)
            FFSWAP(int64_t, st->pts_buffer[i], st->pts_buffer[i + 1]);

        pkt->dts = st->pts_buffer[0];
    }

    if (st->cur_dts && st->cur_dts != AV_NOPTS_VALUE &&
        ((!(s->oformat->flags & AVFMT_TS_NONSTRICT) &&
          st->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE &&
          st->codecpar->codec_type != AVMEDIA_TYPE_DATA &&
          st->cur_dts >= pkt->dts) || st->cur_dts > pkt->dts)) {
        av_log(s, AV_LOG_ERROR,
               "Application provided invalid, non monotonically increasing dts to muxer in stream %d: %s >= %s\n",
               st->index, av_ts2str(st->cur_dts), av_ts2str(pkt->dts));
        return AVERROR(EINVAL);
    }
    if (pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts) {
        av_log(s, AV_LOG_ERROR,
               "pts (%s) < dts (%s) in stream %d\n",
               av_ts2str(pkt->pts), av_ts2str(pkt->dts),
               st->index);
        return AVERROR(EINVAL);
    }

    if (s->debug & FF_FDEBUG_TS)
        av_log(s, AV_LOG_TRACE, "av_write_frame: pts2:%s dts2:%s\n",
               av_ts2str(pkt->pts), av_ts2str(pkt->dts));

    st->cur_dts = pkt->dts;
    st->priv_pts->val = pkt->dts;

    /* update pts */
    switch (st->codecpar->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            frame_size = (pkt->flags & AV_PKT_FLAG_UNCODED_FRAME) ?
                         ((AVFrame *) pkt->data)->nb_samples :
                         av_get_audio_frame_duration(st->codec, pkt->size);

            /* HACK/FIXME, we skip the initial 0 size packets as they are most
             * likely equal to the encoder delay, but it would be better if we
             * had the real timestamps from the encoder */
            if (frame_size >= 0 &&
                (pkt->size || st->priv_pts->num != st->priv_pts->den >> 1 || st->priv_pts->val)) {
                frac_add(st->priv_pts, (int64_t) st->time_base.den * frame_size);
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            frac_add(st->priv_pts, (int64_t) st->time_base.den * st->time_base.num);
            break;
        case AVMEDIA_TYPE_UNKNOWN:
            break;
        case AVMEDIA_TYPE_DATA:
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            break;
        case AVMEDIA_TYPE_ATTACHMENT:
            break;
        case AVMEDIA_TYPE_NB:
            break;
    }
    return 0;
}

#endif


static int check_packet(AVFormatContext *s, AVPacket *pkt) {
    if (!pkt)
        return 0;

    if (pkt->stream_index < 0 || pkt->stream_index >= s->nb_streams) {
        av_log(s, AV_LOG_ERROR, "Invalid packet stream index: %d\n",
               pkt->stream_index);
        return AVERROR(EINVAL);
    }

    if (s->streams[pkt->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT) {
        av_log(s, AV_LOG_ERROR, "Received a packet for an attachment stream.\n");
        return AVERROR(EINVAL);
    }

    return 0;
}


static int prepare_input_packet(AVFormatContext *s, AVPacket *pkt) {
    int ret;

    ret = check_packet(s, pkt);
    if (ret < 0)
        return ret;

#if !FF_API_COMPUTE_PKT_FIELDS2 || !FF_API_LAVF_AVCTX
    /* sanitize the timestamps */
    if (!(s->oformat->flags & AVFMT_NOTIMESTAMPS)) {
        AVStream *st = s->streams[pkt->stream_index];

        /* when there is no reordering (so dts is equal to pts), but
         * only one of them is set, set the other as well */
        if (!st->internal->reorder) {
            if (pkt->pts == AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE)
                pkt->pts = pkt->dts;
            if (pkt->dts == AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE)
                pkt->dts = pkt->pts;
        }

        /* check that the timestamps are set */
        if (pkt->pts == AV_NOPTS_VALUE || pkt->dts == AV_NOPTS_VALUE) {
            av_log(s, AV_LOG_ERROR,
                   "Timestamps are unset in a packet for stream %d\n", st->index);
            return AVERROR(EINVAL);
        }

        /* check that the dts are increasing (or at least non-decreasing,
         * if the format allows it */
        if (st->cur_dts != AV_NOPTS_VALUE &&
            ((!(s->oformat->flags & AVFMT_TS_NONSTRICT) && st->cur_dts >= pkt->dts) ||
             st->cur_dts > pkt->dts)) {
            av_log(s, AV_LOG_ERROR,
                   "Application provided invalid, non monotonically increasing "
                   "dts to muxer in stream %d: %" PRId64 " >= %" PRId64 "\n",
                   st->index, st->cur_dts, pkt->dts);
            return AVERROR(EINVAL);
        }

        if (pkt->pts < pkt->dts) {
            av_log(s, AV_LOG_ERROR, "pts %" PRId64 " < dts %" PRId64 " in stream %d\n",
                   pkt->pts, pkt->dts, st->index);
            return AVERROR(EINVAL);
        }
    }
#endif

    return 0;
}

static int write_header_internal(AVFormatContext *s) {
    if (!(s->oformat->flags & AVFMT_NOFILE) && s->pb)
        avio_write_marker(s->pb, AV_NOPTS_VALUE, AVIO_DATA_MARKER_HEADER);
    if (s->oformat->write_header) {
        int ret = s->oformat->write_header(s);
        if (ret >= 0 && s->pb && s->pb->error < 0)
            ret = s->pb->error;
        s->internal->write_header_ret = ret;
        if (ret < 0)
            return ret;
        flush_if_needed(s);
    }
    s->internal->header_written = 1;
    if (!(s->oformat->flags & AVFMT_NOFILE) && s->pb)
        avio_write_marker(s->pb, AV_NOPTS_VALUE, AVIO_DATA_MARKER_UNKNOWN);
    return 0;
}

static int write_packet(AVFormatContext *s, AVPacket *pkt) {
    int ret, did_split;
    int64_t pts_backup, dts_backup;

    pts_backup = pkt->pts;
    dts_backup = pkt->dts;

    // If the timestamp offsetting below is adjusted, adjust
    // ff_interleaved_peek similarly.
    if (s->output_ts_offset) {
        AVStream *st = s->streams[pkt->stream_index];
        int64_t offset = av_rescale_q(s->output_ts_offset, AV_TIME_BASE_Q, st->time_base);

        if (pkt->dts != AV_NOPTS_VALUE)
            pkt->dts += offset;
        if (pkt->pts != AV_NOPTS_VALUE)
            pkt->pts += offset;
    }

    if (s->avoid_negative_ts > 0) {
        AVStream *st = s->streams[pkt->stream_index];
        int64_t offset = st->mux_ts_offset;
        int64_t ts = s->internal->avoid_negative_ts_use_pts ? pkt->pts : pkt->dts;

        if (s->internal->offset == AV_NOPTS_VALUE && ts != AV_NOPTS_VALUE &&
            (ts < 0 || s->avoid_negative_ts == AVFMT_AVOID_NEG_TS_MAKE_ZERO)) {
            s->internal->offset = -ts;
            s->internal->offset_timebase = st->time_base;
        }

        if (s->internal->offset != AV_NOPTS_VALUE && !offset) {
            offset = st->mux_ts_offset =
                    av_rescale_q_rnd(s->internal->offset,
                                     s->internal->offset_timebase,
                                     st->time_base,
                                     AV_ROUND_UP);
        }

        if (pkt->dts != AV_NOPTS_VALUE)
            pkt->dts += offset;
        if (pkt->pts != AV_NOPTS_VALUE)
            pkt->pts += offset;

        if (s->internal->avoid_negative_ts_use_pts) {
            if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < 0) {
                av_log(s, AV_LOG_WARNING, "failed to avoid negative "
                               "pts %s in stream %d.\n"
                               "Try -avoid_negative_ts 1 as a possible workaround.\n",
                       av_ts2str(pkt->pts),
                       pkt->stream_index
                );
            }
        } else {
            av_assert2(pkt->dts == AV_NOPTS_VALUE || pkt->dts >= 0 || s->max_interleave_delta > 0);
            if (pkt->dts != AV_NOPTS_VALUE && pkt->dts < 0) {
                av_log(s, AV_LOG_WARNING,
                       "Packets poorly interleaved, failed to avoid negative "
                               "timestamp %s in stream %d.\n"
                               "Try -max_interleave_delta 0 as a possible workaround.\n",
                       av_ts2str(pkt->dts),
                       pkt->stream_index
                );
            }
        }
    }

#if FF_API_LAVF_MERGE_SD
            FF_DISABLE_DEPRECATION_WARNINGS
    did_split = av_packet_split_side_data(pkt);
    FF_ENABLE_DEPRECATION_WARNINGS
#endif

    if (!s->internal->header_written) {
        ret = s->internal->write_header_ret ? s->internal->write_header_ret : write_header_internal(
                s);
        if (ret < 0)
            goto fail;
    }

    if ((pkt->flags & AV_PKT_FLAG_UNCODED_FRAME)) {
        AVFrame *frame = (AVFrame *) pkt->data;
        av_assert0(pkt->size == UNCODED_FRAME_PACKET_SIZE);
        ret = s->oformat->write_uncoded_frame(s, pkt->stream_index, &frame, 0);
        av_frame_free(&frame);
    } else {
        ret = s->oformat->write_packet(s, pkt);
    }

    if (s->pb && ret >= 0) {
        flush_if_needed(s);
        if (s->pb->error < 0)
            ret = s->pb->error;
    }

    fail:
#if FF_API_LAVF_MERGE_SD
    FF_DISABLE_DEPRECATION_WARNINGS
    if (did_split)
        av_packet_merge_side_data(pkt);
            FF_ENABLE_DEPRECATION_WARNINGS
#endif

    if (ret < 0) {
        pkt->pts = pts_backup;
        pkt->dts = dts_backup;
    }

    return ret;
}

static int interleave_compare_dts(AVFormatContext *s, AVPacket *next,
                                  AVPacket *pkt) {
    AVStream *st = s->streams[pkt->stream_index];
    AVStream *st2 = s->streams[next->stream_index];
    int comp = av_compare_ts(next->dts, st2->time_base, pkt->dts,
                             st->time_base);
    if (s->audio_preload && ((st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) !=
                             (st2->codecpar->codec_type == AVMEDIA_TYPE_AUDIO))) {
        int64_t ts = av_rescale_q(pkt->dts, st->time_base, AV_TIME_BASE_Q) -
                     s->audio_preload * (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO);
        int64_t ts2 = av_rescale_q(next->dts, st2->time_base, AV_TIME_BASE_Q) -
                      s->audio_preload * (st2->codecpar->codec_type == AVMEDIA_TYPE_AUDIO);
        if (ts == ts2) {
            ts = (pkt->dts * st->time_base.num * AV_TIME_BASE -
                  s->audio_preload * (int64_t) (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) *
                  st->time_base.den) * st2->time_base.den
                 - (next->dts * st2->time_base.num * AV_TIME_BASE -
                    s->audio_preload * (int64_t) (st2->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) *
                    st2->time_base.den) * st->time_base.den;
            ts2 = 0;
        }
        comp = (ts > ts2) - (ts < ts2);
    }

    if (comp == 0)
        return pkt->stream_index < next->stream_index;
    return comp > 0;
}

int av_packet_add_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                            uint8_t *data, size_t size) {
    AVPacketSideData *tmp;
    int i, elems = pkt->side_data_elems;

    for (i = 0; i < elems; i++) {
        AVPacketSideData *sd = &pkt->side_data[i];

        if (sd->type == type) {
            av_free(sd->data);
            sd->data = data;
            sd->size = size;
            return 0;
        }
    }

    if ((unsigned) elems + 1 > AV_PKT_DATA_NB)
        return AVERROR(ERANGE);

    tmp = (AVPacketSideData *) av_realloc(pkt->side_data, (elems + 1) * sizeof(*tmp));
    if (!tmp)
        return AVERROR(ENOMEM);

    pkt->side_data = tmp;
    pkt->side_data[elems].data = data;
    pkt->side_data[elems].size = size;
    pkt->side_data[elems].type = type;
    pkt->side_data_elems++;

    return 0;
}

uint8_t *av_packet_new_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                                 int size) {
    int ret;
    uint8_t *data;

    if ((unsigned) size > INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
        return NULL;
    data = (uint8_t *) av_mallocz(size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!data)
        return NULL;

    ret = av_packet_add_side_data(pkt, type, data, size);
    if (ret < 0) {
        av_freep(&data);
        return NULL;
    }

    return data;
}


static int packet_alloc(AVBufferRef **buf, int size) {
    int ret;
    if (size < 0 || size >= INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
        return AVERROR(EINVAL);

    ret = av_buffer_realloc(buf, size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (ret < 0)
        return ret;

    memset((*buf)->data + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    return 0;
}

int av_packet_ref(AVPacket *dst, const AVPacket *src) {
    int ret;

    ret = av_packet_copy_props(dst, src);
    if (ret < 0)
        return ret;

    if (!src->buf) {
        ret = packet_alloc(&dst->buf, src->size);
        if (ret < 0)
            goto fail;
        if (src->size)
            memcpy(dst->buf->data, src->data, src->size);

        dst->data = dst->buf->data;
    } else {
        dst->buf = av_buffer_ref(src->buf);
        if (!dst->buf) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        dst->data = src->data;
    }

    dst->size = src->size;

    return 0;
    fail:
    av_packet_free_side_data(dst);
    return ret;
}

int av_packet_copy_props(AVPacket *dst, const AVPacket *src) {
    int i;

    dst->pts = src->pts;
    dst->dts = src->dts;
    dst->pos = src->pos;
    dst->duration = src->duration;
#if FF_API_CONVERGENCE_DURATION
    FF_DISABLE_DEPRECATION_WARNINGS
    dst->convergence_duration = src->convergence_duration;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif
    dst->flags = src->flags;
    dst->stream_index = src->stream_index;

    for (i = 0; i < src->side_data_elems; i++) {
        enum AVPacketSideDataType type = src->side_data[i].type;
        int size = src->side_data[i].size;
        uint8_t *src_data = src->side_data[i].data;
        uint8_t *dst_data = av_packet_new_side_data(dst, type, size);

        if (!dst_data) {
            av_packet_free_side_data(dst);
            return AVERROR(ENOMEM);
        }
        memcpy(dst_data, src_data, size);
    }

    return 0;
}


int ff_interleave_add_packet(AVFormatContext *s, AVPacket *pkt,
                             int (*compare)(AVFormatContext *, AVPacket *, AVPacket *)) {
    int ret;
    AVPacketList **next_point, *this_pktl;
    AVStream *st = s->streams[pkt->stream_index];
    int chunked = s->max_chunk_size || s->max_chunk_duration;

    this_pktl = (AVPacketList *) av_mallocz(sizeof(AVPacketList));
    if (!this_pktl)
        return AVERROR(ENOMEM);
    if ((pkt->flags & AV_PKT_FLAG_UNCODED_FRAME)) {
        av_assert0(pkt->size == UNCODED_FRAME_PACKET_SIZE);
        av_assert0(((AVFrame *) pkt->data)->buf);
        this_pktl->pkt = *pkt;
        pkt->buf = NULL;
        pkt->side_data = NULL;
        pkt->side_data_elems = 0;
    } else {
        if ((ret = av_packet_ref(&this_pktl->pkt, pkt)) < 0) {
            av_free(this_pktl);
            return ret;
        }
    }

    if (s->streams[pkt->stream_index]->last_in_packet_buffer) {
        next_point = &(st->last_in_packet_buffer->next);
    } else {
        next_point = &s->internal->packet_buffer;
    }

    if (chunked) {
        uint64_t max = av_rescale_q_rnd(s->max_chunk_duration, AV_TIME_BASE_Q, st->time_base,
                                        AV_ROUND_UP);
        st->interleaver_chunk_size += pkt->size;
        st->interleaver_chunk_duration += pkt->duration;
        if ((s->max_chunk_size && st->interleaver_chunk_size > s->max_chunk_size)
            || (max && st->interleaver_chunk_duration > max)) {
            st->interleaver_chunk_size = 0;
            this_pktl->pkt.flags |= CHUNK_START;
            if (max && st->interleaver_chunk_duration > max) {
                int64_t syncoffset = (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) * max / 2;
                int64_t syncto = av_rescale(pkt->dts + syncoffset, 1, max) * max - syncoffset;

                st->interleaver_chunk_duration += (pkt->dts - syncto) / 8 - max;
            } else
                st->interleaver_chunk_duration = 0;
        }
    }
    if (*next_point) {
        if (chunked && !(this_pktl->pkt.flags & CHUNK_START))
            goto next_non_null;

        if (compare(s, &s->internal->packet_buffer_end->pkt, pkt)) {
            while (*next_point
                   && ((chunked && !((*next_point)->pkt.flags & CHUNK_START))
                       || !compare(s, &(*next_point)->pkt, pkt)))
                next_point = &(*next_point)->next;
            if (*next_point)
                goto next_non_null;
        } else {
            next_point = &(s->internal->packet_buffer_end->next);
        }
    }
    av_assert1(!*next_point);

    s->internal->packet_buffer_end = this_pktl;
    next_non_null:

    this_pktl->next = *next_point;

    s->streams[pkt->stream_index]->last_in_packet_buffer =
    *next_point = this_pktl;

    av_packet_unref(pkt);

    return 0;
}


int ff_interleave_packet_per_dts(AVFormatContext *s, AVPacket *out,
                                 AVPacket *pkt, int flush) {
    AVPacketList *pktl;
    int stream_count = 0;
    int noninterleaved_count = 0;
    int i, ret;
    int eof = flush;

    if (pkt) {
        if ((ret = ff_interleave_add_packet(s, pkt, interleave_compare_dts)) < 0)
            return ret;
    }

    for (i = 0; i < s->nb_streams; i++) {
        if (s->streams[i]->last_in_packet_buffer) {
            ++stream_count;
        } else if (s->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_ATTACHMENT &&
                   s->streams[i]->codecpar->codec_id != AV_CODEC_ID_VP8 &&
                   s->streams[i]->codecpar->codec_id != AV_CODEC_ID_VP9) {
            ++noninterleaved_count;
        }
    }

    if (s->internal->nb_interleaved_streams == stream_count)
        flush = 1;

    if (s->max_interleave_delta > 0 &&
        s->internal->packet_buffer &&
        !flush &&
        s->internal->nb_interleaved_streams == stream_count + noninterleaved_count
            ) {
        AVPacket *top_pkt = &s->internal->packet_buffer->pkt;
        int64_t delta_dts = INT64_MIN;
        int64_t top_dts = av_rescale_q(top_pkt->dts,
                                       s->streams[top_pkt->stream_index]->time_base,
                                       AV_TIME_BASE_Q);

        for (i = 0; i < s->nb_streams; i++) {
            int64_t last_dts;
            const AVPacketList *last = s->streams[i]->last_in_packet_buffer;

            if (!last)
                continue;

            last_dts = av_rescale_q(last->pkt.dts,
                                    s->streams[i]->time_base,
                                    AV_TIME_BASE_Q);
            delta_dts = FFMAX(delta_dts, last_dts - top_dts);
        }

        if (delta_dts > s->max_interleave_delta) {
            av_log(s, AV_LOG_DEBUG,
                   "Delay between the first packet and last packet in the "
                           "muxing queue is %" PRId64" > %" PRId64": forcing output\n",
                   delta_dts, s->max_interleave_delta);
            flush = 1;
        }
    }

    if (s->internal->packet_buffer &&
        eof &&
        (s->flags & AVFMT_FLAG_SHORTEST) &&
        s->internal->shortest_end == AV_NOPTS_VALUE) {
        AVPacket *top_pkt = &s->internal->packet_buffer->pkt;

        s->internal->shortest_end = av_rescale_q(top_pkt->dts,
                                                 s->streams[top_pkt->stream_index]->time_base,
                                                 AV_TIME_BASE_Q);
    }

    if (s->internal->shortest_end != AV_NOPTS_VALUE) {
        while (s->internal->packet_buffer) {
            AVPacket *top_pkt = &s->internal->packet_buffer->pkt;
            AVStream *st;
            int64_t top_dts = av_rescale_q(top_pkt->dts,
                                           s->streams[top_pkt->stream_index]->time_base,
                                           AV_TIME_BASE_Q);

            if (s->internal->shortest_end + 1 >= top_dts)
                break;

            pktl = s->internal->packet_buffer;
            st = s->streams[pktl->pkt.stream_index];

            s->internal->packet_buffer = pktl->next;
            if (!s->internal->packet_buffer)
                s->internal->packet_buffer_end = NULL;

            if (st->last_in_packet_buffer == pktl)
                st->last_in_packet_buffer = NULL;

            av_packet_unref(&pktl->pkt);
            av_freep(&pktl);
            flush = 0;
        }
    }

    if (stream_count && flush) {
        AVStream *st;
        pktl = s->internal->packet_buffer;
        *out = pktl->pkt;
        st = s->streams[out->stream_index];

        s->internal->packet_buffer = pktl->next;
        if (!s->internal->packet_buffer)
            s->internal->packet_buffer_end = NULL;

        if (st->last_in_packet_buffer == pktl)
            st->last_in_packet_buffer = NULL;
        av_freep(&pktl);

        return 1;
    } else {
        av_init_packet(out);
        return 0;
    }
}


static int interleave_packet(AVFormatContext *s, AVPacket *out, AVPacket *in, int flush) {
    if (s->oformat->interleave_packet) {
        int ret = s->oformat->interleave_packet(s, out, in, flush);
        if (in)
            av_packet_unref(in);
        return ret;
    } else
        return ff_interleave_packet_per_dts(s, out, in, flush);
}


static int do_packet_auto_bsf(AVFormatContext *s, AVPacket *pkt) {
    AVStream *st = s->streams[pkt->stream_index];
    int i, ret;

    if (!(s->flags & AVFMT_FLAG_AUTO_BSF))
        return 1;

    if (s->oformat->check_bitstream) {
        if (!st->internal->bitstream_checked) {
            if ((ret = s->oformat->check_bitstream(s, pkt)) < 0)
                return ret;
            else if (ret == 1)
                st->internal->bitstream_checked = 1;
        }
    }

#if FF_API_LAVF_MERGE_SD
            FF_DISABLE_DEPRECATION_WARNINGS
    if (st->internal->nb_bsfcs) {
        ret = av_packet_split_side_data(pkt);
        if (ret < 0)
            av_log(s, AV_LOG_WARNING, "Failed to split side data before bitstream filter\n");
    }FF_ENABLE_DEPRECATION_WARNINGS
#endif

    for (i = 0; i < st->internal->nb_bsfcs; i++) {
        AVBSFContext *ctx = st->internal->bsfcs[i];
        // TODO: when any bitstream filter requires flushing at EOF, we'll need to
        // flush each stream's BSF chain on write_trailer.
        if ((ret = av_bsf_send_packet(ctx, pkt)) < 0) {
            av_log(ctx, AV_LOG_ERROR,
                   "Failed to send packet to filter %s for stream %d\n",
                   ctx->filter->name, pkt->stream_index);
            return ret;
        }
        // TODO: when any automatically-added bitstream filter is generating multiple
        // output packets for a single input one, we'll need to call this in a loop
        // and write each output packet.
        if ((ret = av_bsf_receive_packet(ctx, pkt)) < 0) {
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return 0;
            av_log(ctx, AV_LOG_ERROR,
                   "Failed to send packet to filter %s for stream %d\n",
                   ctx->filter->name, pkt->stream_index);
            return ret;
        }
    }
    return 1;
}


int k_av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt) {
    int ret, flush = 0;

    LOGE("prepare_input_packet   streamindex:%d size:%d pts:%lld", pkt->stream_index, pkt->size,
         pkt->pts);
    ret = prepare_input_packet(s, pkt);
    if (ret < 0)
        goto fail;

    if (pkt) {
        AVStream *st = s->streams[pkt->stream_index];
        st->priv_pts;
        LOGI("priv_pts:%p streamindex:%d", st->priv_pts, st->index);
        LOGI("do_packet_auto_bsf");
        ret = do_packet_auto_bsf(s, pkt);
        if (ret == 0)
            return 0;
        else if (ret < 0)
            goto fail;

        if (s->debug & FF_FDEBUG_TS)
            av_log(s, AV_LOG_TRACE, "av_interleaved_write_frame size:%d dts:%s pts:%s\n",
                   pkt->size, av_ts2str(pkt->dts), av_ts2str(pkt->pts));

#if FF_API_COMPUTE_PKT_FIELDS2 && FF_API_LAVF_AVCTX
        LOGI("compute_muxer_pkt_fields");
        if ((ret = compute_muxer_pkt_fields(s, st, pkt)) < 0 &&
            !(s->oformat->flags & AVFMT_NOTIMESTAMPS))
            goto fail;
#endif
        LOGI("kt->dts == AV_NOPTS_VALUE");
        if (pkt->dts == AV_NOPTS_VALUE && !(s->oformat->flags & AVFMT_NOTIMESTAMPS)) {
            ret = AVERROR(EINVAL);
            goto fail;
        }
    } else {
        av_log(s, AV_LOG_TRACE, "av_interleaved_write_frame FLUSH\n");
        flush = 1;
    }

    for (;;) {
        AVPacket opkt;
        int ret = interleave_packet(s, &opkt, pkt, flush);
        if (pkt) {
            memset(pkt, 0, sizeof(*pkt));
            av_init_packet(pkt);
            pkt = NULL;
        }
        LOGE("interleave_packet ret=%d  streamIndex:%d", ret, opkt.stream_index);
        if (ret <= 0) //FIXME cleanup needed for ret<0 ?
            return ret;


        ret = write_packet(s, &opkt);
        if (ret >= 0)
            s->streams[opkt.stream_index]->nb_frames++;
        LOGI("av_packet_unref");
        av_packet_unref(&opkt);
        LOGE("write_packet ret=%d  streamIndex:%d", ret, opkt.stream_index);
        if (ret < 0)
            return ret;
        if (s->pb && s->pb->error)
            return s->pb->error;
    }
    fail:
    LOGI("fail");
    av_packet_unref(pkt);
    return ret;
}

//int RtmpStreamer::setAudioCodecContext(AVCodecParameters *avCodecParameters) {
//    avcodec_parameters_to_context(mVideoCodecContext,avCodecParameters);
//}
//
//int RtmpStreamer::setVideoCodecContext(AVCodecParameters *avCodecParameters) {
//    avcodec_alloc_context3(avCodecParameters->codec)
//    avcodec_parameters_to_context(mVideoCodecContext,avCodecParameters);
//}

int RtmpStreamer::AddStream(AVCodecContext *avCodecContext) {
//    std::lock_guard<std::mutex> lk(mtx);
    AVStream *pStream = avformat_new_stream(iAvFormatContext, avCodecContext->codec);
    if (!pStream) {
        LOGD("avformat_new_stream failed!");
        return -1;
    }
    LOGD("avformat new stream success!");
    int ret = 0;
    ret = avcodec_parameters_from_context(pStream->codecpar, avCodecContext);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGD("avcodec_parameters_from_context failed :%s", buf);
        return -1;
    }
    LOGD("avcodec_parameters_from_context success!");
    pStream->codecpar->codec_tag = 0;
    if (avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        LOGD("Add video stream success!");
        videoStream = pStream;
//        FFFrac *p= (FFFrac *) malloc(sizeof(FFFrac));
//        p->den=videoStream->time_base.den;
//        p->num=videoStream->time_base.num;
//        videoStream->priv_pts= p;

        LOGI("video priv_pts:%p", videoStream->priv_pts);
        mVideoCodecContext = avCodecContext;
//        videoStream->start_time=0;
//        videoStream->time_base=(AVRational){1,1000};
//        videoStream->duration=43655L;
    } else if (avCodecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        LOGD("Add audio stream success!");
        audioStream = pStream;
//        FFFrac *p= (FFFrac *) malloc(sizeof(FFFrac));
//        p->den=audioStream->time_base.den;
//        p->num=audioStream->time_base.num;
//        audioStream->priv_pts= p;
        LOGI("audio priv_pts:%p", audioStream->priv_pts);
        mAudioCodecContext = avCodecContext;
//        audioStream->start_time=0;
//        audioStream->time_base=(AVRational){1,1000};
//        audioStream->duration=av_rescale_q_rnd(42647L,AVRational{1,1000},AVRational{1,48000},AV_ROUND_NEAR_INF);
    }
    return pStream->index;
}


//int RtmpStreamer::SendAudioFrame(AVData *originData, int streamIndex) {
//    std::lock_guard<std::mutex> lk(mut1);
//    AVRational stime;
//    AVRational dtime;
//
//    AVPacket *packet = originData->avPacket;
//    packet->stream_index = streamIndex;
//#ifdef SHOW_DEBUG_INFO
//    LOGD("packet index:%d    index:%d", packet->stream_index, streamIndex);
//#endif
//    stime = audioCodecContext->time_base;
//    dtime = audioStream->time_base;
//    //push
//    packet->pts = av_rescale_q(packet->pts, stime, dtime);
//    packet->dts = av_rescale_q(packet->dts, stime, dtime);
//    packet->duration = av_rescale_q(packet->duration, stime, dtime);
//#ifdef SHOW_DEBUG_INFO
//    LOGD("writer frame stream Index:%d   size:%d",
//          packet->stream_index,
//          packet->size);
//#endif
//    int ret = av_interleaved_write_frame(iAvFormatContext, packet);
//    if (ret == 0) {
//        LOGD("write ++++++++++++++audio frame sucess!");
//    } else {
//        char buf[1024] = {0};
//        av_strerror(ret, buf, sizeof(buf));
//        LOGD("writer******************* audio frame failed! :%s", buf);
//    }
//    delete originData;
//    return 0;
//
//}

//int RtmpStreamer::SendVideoFrame(AVData *originData, int streamIndex) {
//    std::lock_guard<std::mutex> lk(mut1);
//    AVRational stime;
//    AVRational dtime;
//
//    AVPacket *packet = originData->avPacket;
//    packet->stream_index = streamIndex;
//#ifdef SHOW_DEBUG_INFO
//    LOGD("video packet index:%d    index:%d", packet->stream_index, streamIndex);
//#endif
//    stime = videoCodecContext->time_base;
//    dtime = videoStream->time_base;
//    packet->pts = originData->pts;
//    packet->dts = packet->pts;
//    packet->pts = av_rescale_q_rnd(packet->pts, stime, dtime,
//                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//    packet->dts = av_rescale_q_rnd(packet->dts, stime, dtime,
//                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//    packet->duration = av_rescale_q(packet->duration, stime, dtime);
//#ifdef SHOW_DEBUG_INFO
//    LOGD("writer frame stream Index:%d   size:%d",
//          packet->stream_index,
//          packet->size);
//
//#endif
//    int ret = av_interleaved_write_frame(iAvFormatContext, packet);
//    if (ret == 0) {
//        LOGD("write ------------------video frame success!");
//    } else {
//        char buf[1024] = {0};
//        av_strerror(ret, buf, sizeof(buf));
//        LOGD("writer*******************video frame failed! :%s", buf);
//    }
//    delete originData;
//    return 0;
//
//}


/**
 * 音频推流任务
 */
void *RtmpStreamer::
PushAudioStreamTask(void *pObj) {
    RtmpStreamer *rtmpStreamer = (RtmpStreamer *) pObj;
//    RtmpStreamer->isPushStream = true;

//    if (NULL == RtmpStreamer->audioEncoder) {
//        return 0;
//    }
//    int64_t beginTime = av_gettime();

//    while (!RtmpStreamer->isExit) {
    if (rtmpStreamer->mAudioframeQueue.empty()) {
//            continue;
        return 0;
    }
//        pthread_mutex_lock(&(RtmpStreamer->mutex));
    LOG_E("push audio queue size:%d",rtmpStreamer->mAudioframeQueue.Size());
    const shared_ptr<AVData> &ptr = rtmpStreamer->mAudioframeQueue.wait_and_pop();
    AVData *pData = ptr.get();
    if (pData && pData->avPacket) {
        AVPacket *packet = pData->avPacket;
//            av_packet_ref(dst, packet);
//            av_copy_packet(dst,packet);
//            if (packet->size > 0) {
//                RtmpStreamer->SendFrame(packet, RtmpStreamer->audioStreamIndex);
        //notice:双路流需保证音视频PTS递增
//            if(RtmpStreamer->videoPts>packet->pts){
//                packet->pts=packet->pts+(RtmpStreamer->videoPts-packet->pts)+1;
//            }
        packet->pts = pData->pts;

        packet->dts = packet->pts;
//        RtmpStreamer->audioPts = packet->pts;
//            packet->pts = av_rescale_q(packet->pts, RtmpStreamer->audioStream->time_base,AV_TIME_BASE_Q);
//            packet->dts = av_rescale_q(packet->pts, RtmpStreamer->audioStream->time_base,AV_TIME_BASE_Q);
//        LOGE("send audio pts:%lld", RtmpStreamer->audioPts);
        packet->stream_index = rtmpStreamer->audioStreamIndex;
        packet->duration = pData->duration;
        rtmpStreamer->SendFrame(packet, rtmpStreamer->audioStreamIndex);
//            }
//            av_packet_free(&(*(pData->avPacket)));
//            av_packet_unref(packet);
    }
//        pthread_mutex_unlock(&(RtmpStreamer->mutex));
//        av_usleep(1000000);
//    }

    return 0;
}

/**
* 音频推流任务
*/
void *RtmpStreamer::PushVideoStreamTask(void *pObj) {
    RtmpStreamer *rtmpStreamer = (RtmpStreamer *) pObj;
    rtmpStreamer->isPushStream = true;

//    if (NULL == RtmpStreamer->videoEncoder) {
//        return 0;
//    }
//    int64_t beginTime = av_gettime();
//    while (!RtmpStreamer->isExit) {
    if (rtmpStreamer->mVideoframeQueue.empty()) {
        return 0;
//            continue;
    }
//
    LOG_E("push video queue size:%d",rtmpStreamer->mVideoframeQueue.Size());
    const shared_ptr<AVData> &ptr = rtmpStreamer->mVideoframeQueue.wait_and_pop();
    AVData *pData = ptr.get();


    if (pData && pData->avPacket) {
//            AVPacket *dst = av_packet_alloc();
        AVPacket *packet = pData->avPacket;
//            av_packet_move_ref(dst,packet);
//            av_packet_free(&packet);

//            fwrite(packet->data,1,packet->size,RtmpStreamer->pFile);
//            fflush(RtmpStreamer->pFile);
//            av_packet_ref(dst, packet);
//            av_copy_packet(dst,packet);
//            if (packet->data && packet->size > 0) {
//            AVPacket *avPacket = av_packet_alloc();
//            av_packet_move_ref(avPacket, packet);

        //notice:双路流需保证音视频PTS递增
//            if(RtmpStreamer->audioPts>packet->pts){
//                packet->pts=RtmpStreamer->audioPts+1;
//            }
        packet->pts = pData->pts;
        packet->dts = packet->pts;
        packet->duration = pData->duration;
//        RtmpStreamer->videoPts = packet->pts;
        packet->stream_index = rtmpStreamer->videoStreamIndex;


//            packet->pts = av_rescale_q(packet->pts, RtmpStreamer->videoStream->time_base,AV_TIME_BASE_Q);
//            packet->dts = av_rescale_q(packet->pts, RtmpStreamer->videoStream->time_base,AV_TIME_BASE_Q);
//        LOGE("send video pts:%lld", RtmpStreamer->videoPts);
//        packet->duration = 0;
        rtmpStreamer->SendFrame(packet, rtmpStreamer->videoStreamIndex);
//            int ret = av_interleaved_write_frame(RtmpStreamer->iAvFormatContext,packet);
//            }
//            av_packet_free(&packet);
//            av_packet_free(&packet);
    }
//        pthread_mutex_unlock(&(RtmpStreamer->mutex));
//        av_usleep(1000000);
//    }
    return 0;
}


void RtmpStreamer::main() {

//    WriteHead(this);
    while (!isExit) {
        if (writeHeadFinish) {
            PushAudioStreamTask(this);
            PushVideoStreamTask(this);
        }
    }

}

void RtmpStreamer::setMetaData(MetaData data) {
    this->metaData = data;
}


int vflush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret = 0;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities
          & CODEC_CAP_DELAY))
        return 0;
    av_init_packet(&enc_pkt);
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec,
                                    &enc_pkt, NULL, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        av_free_packet(&enc_pkt);
        if (ret < 0)
            break;
    }

    return ret;
}

void av_opt_free(void *obj) {
    const AVOption *o = NULL;
    while ((o = av_opt_next(obj, o))) {
        switch (o->type) {
            case AV_OPT_TYPE_STRING:
            case AV_OPT_TYPE_BINARY:
                av_freep((uint8_t *) obj + o->offset);
                break;

            case AV_OPT_TYPE_DICT:
                av_dict_free((AVDictionary **) (((uint8_t *) obj) + o->offset));
                break;

            default:
                break;
        }
    }
}

//int av_write_trailer(AVFormatContext *s) {
//    int ret, i;
//
//    for (;;) {
//        AVPacket pkt;
//        ret = interleave_packet(s, &pkt, NULL, 1);
//        if (ret < 0)
//            goto fail;
//        if (!ret)
//            break;
//
//        ret = write_packet(s, &pkt);
//        if (ret >= 0)
//            s->streams[pkt.stream_index]->nb_frames++;
//
//        av_packet_unref(&pkt);
//
//        if (ret < 0)
//            goto fail;
//        if (s->pb && s->pb->error)
//            goto fail;
//    }
//
//    if (!s->internal->header_written) {
//        ret = s->internal->write_header_ret ? s->internal->write_header_ret : write_header_internal(
//                s);
//        if (ret < 0)
//            goto fail;
//    }
//
//    fail:
//    if (s->internal->header_written && s->oformat->write_trailer) {
//        if (!(s->oformat->flags & AVFMT_NOFILE) && s->pb)
//            avio_write_marker(s->pb, AV_NOPTS_VALUE, AVIO_DATA_MARKER_TRAILER);
//        if (ret >= 0) {
//            ret = s->oformat->write_trailer(s);
//        } else {
//            s->oformat->write_trailer(s);
//        }
//    }
//
//    if (s->oformat->deinit)
//        s->oformat->deinit(s);
//
//    s->internal->header_written =
//    s->internal->initialized =
//    s->internal->streams_initialized = 0;
//
//    if (s->pb)
//        avio_flush(s->pb);
//    if (ret == 0)
//        ret = s->pb ? s->pb->error : 0;
//    for (i = 0; i < s->nb_streams; i++) {
//        av_freep(&s->streams[i]->priv_data);
//        av_freep(&s->streams[i]->index_entries);
//    }
//    if (s->oformat->priv_class)
//        av_opt_free(s->priv_data);
//    av_freep(&s->priv_data);
//    return ret;
//}

int RtmpStreamer::ClosePushStream() {
    isExit = true;

    if (iAvFormatContext) {
//        vflush_encoder(iAvFormatContext,videoStreamIndex);
        av_write_trailer(iAvFormatContext);
        avio_close(iAvFormatContext->pb);
        LOG_D("close");
    }
    writeHeadFinish = false;
    writeVideoPts = 0;
    writeAudioPts = 0;
    readAudioPts = 0;
    readVideoPts = 0;
    if (&mFunctionPoniter) {
        mFunctionPoniter(p);
    }
    return 0;
}

int writeTraier(struct AVFormatContext *avFormatContext) {
    LOG_E("write traier");

    return 0;
}


/**
 * notice:AVStream创建完成开始写头信息
 */
void *RtmpStreamer::WriteHead(void *pObj) {
    RtmpStreamer *rtmpStreamer = (RtmpStreamer *) pObj;
    int ret = 0;
    ret = avio_open(&rtmpStreamer->iAvFormatContext->pb, rtmpStreamer->outputUrl,
                    AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGI("avio open failed: %s", buf);
        return 0;
    }
    LOGI("avio open success!");


//    AVDictionary *opt = NULL;
//    av_dict_set_int(&opt, "video_track_timescale", 12800, 0);
    ret = avformat_write_header(rtmpStreamer->iAvFormatContext, NULL);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGI("avformat write header failed!: %s", buf);
        return 0;
    }
//    av_dump_format(RtmpStreamer->iAvFormatContext, 0, RtmpStreamer->outputUrl, 1);
//    RtmpStreamer->iAvFormatContext->oformat->write_trailer = writeTraier;
//    RtmpStreamer->iAvFormatContext->duration=42627L;
//    RtmpStreamer->iAvFormatContext->streams[RtmpStreamer->audioStreamIndex]->duration=av_rescale_q_rnd(42647L,AVRational{1,1000},AVRational{1,48000},AV_ROUND_NEAR_INF);
//    RtmpStreamer->iAvFormatContext->streams[RtmpStreamer->audioStreamIndex]->time_base=(AVRational){1,48000};

    rtmpStreamer->writeHeadFinish = true;
    return 0;
}

int RtmpStreamer::SendFrame(AVPacket *packet, int streamIndex) {
    int64_t pts = 0;
    int64_t duration = packet->duration;
    packet->duration = 0;
    if (packet->pts == 0) {
        packet->pts = 1;
    }
    LOG_E("---------->pts:%lld", packet->pts);

    if (streamIndex == videoStreamIndex) {
        if (&metaData && metaData.video_rotate) {
            int i = av_dict_set(&iAvFormatContext->streams[streamIndex]->metadata, "rotate",
                                metaData.video_rotate, 0);
            if (i < 0) {
                LOG_E("set error");
            }
        }


        pts = packet->pts;
        packet->pts = av_rescale_q_rnd(packet->pts, inVideoTimeBase, videoStream->time_base,
                                       AV_ROUND_NEAR_INF);
        packet->dts = packet->pts;//移除B帧
        packet->duration = av_rescale_q(packet->duration, inVideoTimeBase, videoStream->time_base);
    } else if (streamIndex == audioStreamIndex) {
        pts = packet->pts;
        packet->pts = av_rescale_q_rnd(packet->pts, inAudioTimeBase, audioStream->time_base,
                                       AV_ROUND_NEAR_INF);
        packet->dts = packet->pts;
        packet->duration = av_rescale_q(packet->duration, inAudioTimeBase, audioStream->time_base);

    }

    packet->pos = -1;
    int ret;
    ret = av_interleaved_write_frame(iAvFormatContext, packet);
    if (ret == 0) {
        if (streamIndex == audioStreamIndex) {

            writeAudioPts = pts;
            LOG_D("---------->write @@@@@@@@@ frame success------->! pts:%lld duration:%lld",
                  writeAudioPts,
                  duration);
        } else if (streamIndex == videoStreamIndex) {
            writeVideoPts = pts;
            if(progressCall){
                progressCall(metaData.duration,writeVideoPts);
            }
            LOG_D("---------->write ######### frame success------->! pts:%lld duration:%lld",
                  writeVideoPts,
                  duration);
        }
    } else {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOG_D("stream index %d writer frame failed! :%s", streamIndex, buf);
    }


    return 0;
}


void RtmpStreamer::setVideoEncoder(VideoEncoder *pEncoder) {
    this->videoEncoder = pEncoder;
}

void RtmpStreamer::setAudioEncoder(AudioEncoder *pEncoder) {
    this->audioEncoder = pEncoder;
}

void RtmpStreamer::setCloseCallBack(void (*fun)(void *), void *p) {
    mFunctionPoniter = fun;
    this->p = p;
}

void RtmpStreamer::setProgressCallBack(functionP p) {
    this->progressCall=p;
}

