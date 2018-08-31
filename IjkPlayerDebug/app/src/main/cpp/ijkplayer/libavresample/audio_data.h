/*
 * Copyright (c) 2012 Justin Ruggles <justin.ruggles@gmail.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVRESAMPLE_AUDIO_DATA_H
#define AVRESAMPLE_AUDIO_DATA_H

#include <stdint.h>

#include "libavutil/audio_fifo.h"
#include "libavutil/log.h"
#include "libavutil/samplefmt.h"
#include "avresample.h"
#include "internal.h"

int ff_sample_fmt_is_planar(enum AVSampleFormat sample_fmt, int channels);

/**
 * Audio buffer used for intermediate storage between conversion phases.
 */
struct AudioData {
    const AVClass *class;               /**< AVClass for logging            */
    uint8_t *data[AVRESAMPLE_MAX_CHANNELS]; /**< data plane pointers        */
    uint8_t *buffer;                    /**< data buffer                    */
    unsigned int buffer_size;           /**< allocated buffer size          */
    int allocated_samples;              /**< number of samples the buffer can hold */
    int nb_samples;                     /**< current number of samples      */
    enum AVSampleFormat sample_fmt;     /**< sample format                  */
    int channels;                       /**< channel count                  */
    int allocated_channels;             /**< allocated channel count        */
    int is_planar;                      /**< sample format is planar        */
    int planes;                         /**< number of data planes          */
    int sample_size;                    /**< bytes per sample               */
    int stride;                         /**< sample byte offset within a plane */
    int read_only;                      /**< data is read-only              */
    int allow_realloc;                  /**< realloc is allowed             */
    int ptr_align;                      /**< minimum data pointer alignment */
    int samples_align;                  /**< allocated samples alignment    */
    const char *name;                   /**< name for debug logging         */
};

int ff_audio_data_set_channels(struct AudioData *a, int channels);

/**
 * Initialize struct AudioData using a given source.
 *
 * This does not allocate an internal buffer. It only sets the data pointers
 * and audio parameters.
 *
 * @param a               struct AudioData struct
 * @param src             source data pointers
 * @param plane_size      plane size, in bytes.
 *                        This can be 0 if unknown, but that will lead to
 *                        optimized functions not being used in many cases,
 *                        which could slow down some conversions.
 * @param channels        channel count
 * @param nb_samples      number of samples in the source data
 * @param sample_fmt      sample format
 * @param read_only       indicates if buffer is read only or read/write
 * @param name            name for debug logging (can be NULL)
 * @return                0 on success, negative AVERROR value on error
 */
int ff_audio_data_init(struct AudioData *a, uint8_t * const *src, int plane_size,
                       int channels, int nb_samples,
                       enum AVSampleFormat sample_fmt, int read_only,
                       const char *name);

/**
 * Allocate struct AudioData.
 *
 * This allocates an internal buffer and sets audio parameters.
 *
 * @param channels        channel count
 * @param nb_samples      number of samples to allocate space for
 * @param sample_fmt      sample format
 * @param name            name for debug logging (can be NULL)
 * @return                newly allocated struct AudioData struct, or NULL on error
 */
struct AudioData *ff_audio_data_alloc(int channels, int nb_samples,
                               enum AVSampleFormat sample_fmt,
                               const char *name);

/**
 * Reallocate struct AudioData.
 *
 * The struct AudioData must have been previously allocated with ff_audio_data_alloc().
 *
 * @param a           struct AudioData struct
 * @param nb_samples  number of samples to allocate space for
 * @return            0 on success, negative AVERROR value on error
 */
int ff_audio_data_realloc(struct AudioData *a, int nb_samples);

/**
 * Free struct AudioData.
 *
 * The struct AudioData must have been previously allocated with ff_audio_data_alloc().
 *
 * @param a  struct AudioData struct
 */
void ff_audio_data_free(struct AudioData **a);

/**
 * Copy data from one struct AudioData to another.
 *
 * @param out  output struct AudioData
 * @param in   input struct AudioData
 * @param map  channel map, NULL if not remapping
 * @return     0 on success, negative AVERROR value on error
 */
int ff_audio_data_copy(struct AudioData *out, struct AudioData *in, ChannelMapInfo *map);

/**
 * Append data from one struct AudioData to the end of another.
 *
 * @param dst         destination struct AudioData
 * @param dst_offset  offset, in samples, to start writing, relative to the
 *                    start of dst
 * @param src         source struct AudioData
 * @param src_offset  offset, in samples, to start copying, relative to the
 *                    start of the src
 * @param nb_samples  number of samples to copy
 * @return            0 on success, negative AVERROR value on error
 */
int ff_audio_data_combine(struct AudioData *dst, int dst_offset, struct AudioData *src,
                          int src_offset, int nb_samples);

/**
 * Drain samples from the start of the struct AudioData.
 *
 * Remaining samples are shifted to the start of the struct AudioData.
 *
 * @param a           struct AudioData struct
 * @param nb_samples  number of samples to drain
 */
void ff_audio_data_drain(struct AudioData *a, int nb_samples);

/**
 * Add samples in struct AudioData to an AVAudioFifo.
 *
 * @param af          Audio FIFO Buffer
 * @param a           struct AudioData struct
 * @param offset      number of samples to skip from the start of the data
 * @param nb_samples  number of samples to add to the FIFO
 * @return            number of samples actually added to the FIFO, or
 *                    negative AVERROR code on error
 */
int ff_audio_data_add_to_fifo(AVAudioFifo *af, struct AudioData *a, int offset,
                              int nb_samples);

/**
 * Read samples from an AVAudioFifo to struct AudioData.
 *
 * @param af          Audio FIFO Buffer
 * @param a           struct AudioData struct
 * @param nb_samples  number of samples to read from the FIFO
 * @return            number of samples actually read from the FIFO, or
 *                    negative AVERROR code on error
 */
int ff_audio_data_read_from_fifo(AVAudioFifo *af, struct AudioData *a, int nb_samples);

#endif /* AVRESAMPLE_AUDIO_DATA_H */
