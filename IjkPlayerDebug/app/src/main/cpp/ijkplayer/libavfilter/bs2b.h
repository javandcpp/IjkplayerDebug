#ifndef BS2B_H
#define BS2B_H

/* Number of crossfeed levels */
#define BS2B_CLEVELS           3
#define BS2B_HIGH_CLEVEL       3
#define BS2B_MIDDLE_CLEVEL     2
#define BS2B_LOW_CLEVEL        1
#define BS2B_HIGH_ECLEVEL      BS2B_HIGH_CLEVEL    + BS2B_CLEVELS
#define BS2B_MIDDLE_ECLEVEL    BS2B_MIDDLE_CLEVEL  + BS2B_CLEVELS
#define BS2B_LOW_ECLEVEL       BS2B_LOW_CLEVEL     + BS2B_CLEVELS

/* Default crossfeed levels */
#define BS2B_DEFAULT_CLEVEL    BS2B_HIGH_ECLEVEL
/* Default sample rate (Hz) */
#define BS2B_DEFAULT_SRATE     44100
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
struct bs2b {
    int level;  /* Crossfeed level */
    int srate;   /* Sample rate (Hz) */

    /* Lowpass IIR filter coefficients */
    double a0_lo;
    double b1_lo;

    /* Highboost IIR filter coefficients */
    double a0_hi;
    double a1_hi;
    double b1_hi;

    /* Global gain against overloading */
    float gain;
    /* Buffer of last filtered sample.
66      * [0] - first channel, [1] - second channel
67      */
    struct t_last_sample {
        double asis[2];
        double lo[2];
        double hi[2];
    } last_sample;
};
/* Clear buffers and set new coefficients with new crossfeed level value.
  76  * level - crossfeed level of *LEVEL values.
  77  */
void bs2b_set_level(struct bs2b *bs2b, int level);
/* Return current crossfeed level value */
int bs2b_get_level(struct bs2b *bs2b);

/* Clear buffers and set new coefficients with new sample rate value.
  84  * srate - sample rate by Hz.
  85  */
void bs2b_set_srate(struct bs2b *bs2b, int srate);
/* Return current sample rate value */
int bs2b_get_srate(struct bs2b *bs2b);
/* Clear buffer */
void bs2b_clear(struct bs2b *bs2b);
/* Crossfeeds one stereo sample that are pointed by sample.
   95  * [0] - first channel, [1] - second channel.
   96  * Returns crossfided samle by sample pointer.
   97  */
/* sample poits to floats */
void bs2b_cross_feed(struct bs2b *bs2b, float *sample);
#ifdef __cplusplus
}    /* extern "C" */
#endif /* __cplusplus */
#endif /* BS2B_H */