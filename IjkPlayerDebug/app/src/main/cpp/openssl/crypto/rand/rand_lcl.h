/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_RAND_LCL_H
# define HEADER_RAND_LCL_H

# include <openssl/aes.h>
# include <openssl/evp.h>
# include <openssl/sha.h>
# include <openssl/hmac.h>
# include <openssl/ec.h>
# include "internal/rand.h"

/* How many times to read the TSC as a randomness source. */
# define TSC_READ_COUNT                 4

/* Maximum count allowed in reseeding */
# define MAX_RESEED                     (1 << 24)

/* Max size of additional input and personalization string. */
# define DRBG_MAX_LENGTH                4096

/*
 * The quotient between max_{entropy,nonce}len and min_{entropy,nonce}len
 *
 * The current factor is large enough that the RAND_POOL can store a
 * random input which has a lousy entropy rate of 0.0625 bits per byte.
 * This input will be sent through the derivation function which 'compresses'
 * the low quality input into a high quality output.
 */
# define DRBG_MINMAX_FACTOR              128


/* DRBG status values */
typedef enum drbg_status_e {
    DRBG_UNINITIALISED,
    DRBG_READY,
    DRBG_ERROR
} DRBG_STATUS;


/*
 * The state of a DRBG AES-CTR.
 */
typedef struct rand_drbg_ctr_st {
    AES_KEY ks;
    size_t keylen;
    unsigned char K[32];
    unsigned char V[16];
    /* Temp variables used by derivation function */
    AES_KEY df_ks;
    AES_KEY df_kxks;
    /* Temporary block storage used by ctr_df */
    unsigned char bltmp[16];
    size_t bltmp_pos;
    unsigned char KX[48];
} RAND_DRBG_CTR;


/*
 * The state of all types of DRBGs, even though we only have CTR mode
 * right now.
 */
struct rand_drbg_st {
    CRYPTO_RWLOCK *lock;
    RAND_DRBG *parent;
    int nid; /* the underlying algorithm */
    int fork_count;
    unsigned short flags; /* various external flags */

    /*
     * The random pool is used by RAND_add()/drbg_add() to attach random
     * data to the global drbg, such that the rand_drbg_get_entropy() callback
     * can pull it during instantiation and reseeding. This is necessary to
     * reconcile the different philosophies of the RAND and the RAND_DRBG
     * with respect to how randomness is added to the RNG during reseeding
     * (see PR #4328).
     */
    RAND_POOL *pool;

    /*
     * The following parameters are setup by the per-type "init" function.
     *
     * Currently the only type is CTR_DRBG, its init function is ctr_init().
     *
     * The parameters are closely related to the ones described in
     * section '10.2.1 CTR_DRBG' of [NIST SP 800-90Ar1], with one
     * crucial difference: In the NIST standard, all counts are given
     * in bits, whereas in OpenSSL entropy counts are given in bits
     * and buffer lengths are given in bytes.
     *
     * Since this difference has lead to some confusion in the past,
     * (see [GitHub Issue #2443], formerly [rt.openssl.org #4055])
     * the 'len' suffix has been added to all buffer sizes for
     * clarification.
     */

    int strength;
    size_t max_request;
    size_t min_entropylen, max_entropylen;
    size_t min_noncelen, max_noncelen;
    size_t max_perslen, max_adinlen;
    unsigned int reseed_counter;
    unsigned int reseed_interval;
    size_t seedlen;
    DRBG_STATUS state;

    /* Application data, mainly used in the KATs. */
    CRYPTO_EX_DATA ex_data;

    /* Implementation specific structures; was a union, but inline for now */
    RAND_DRBG_CTR ctr;

    /* Callback functions.  See comments in rand_lib.c */
    RAND_DRBG_get_entropy_fn get_entropy;
    RAND_DRBG_cleanup_entropy_fn cleanup_entropy;
    RAND_DRBG_get_nonce_fn get_nonce;
    RAND_DRBG_cleanup_nonce_fn cleanup_nonce;
};

/* The global RAND method, and the global buffer and DRBG instance. */
extern RAND_METHOD rand_meth;

/* How often we've forked (only incremented in child). */
extern int rand_fork_count;

/* Hardware-based seeding functions. */
size_t rand_acquire_entropy_from_tsc(RAND_POOL *pool);
size_t rand_acquire_entropy_from_cpu(RAND_POOL *pool);

/* DRBG entropy callbacks. */
size_t rand_drbg_get_entropy(RAND_DRBG *drbg,
                             unsigned char **pout,
                             int entropy, size_t min_len, size_t max_len);
void rand_drbg_cleanup_entropy(RAND_DRBG *drbg,
                               unsigned char *out, size_t outlen);

/* DRBG helpers */
int rand_drbg_restart(RAND_DRBG *drbg,
                      const unsigned char *buffer, size_t len, size_t entropy);

/* DRBG functions implementing AES-CTR */
int ctr_init(RAND_DRBG *drbg);
int ctr_uninstantiate(RAND_DRBG *drbg);
int ctr_instantiate(RAND_DRBG *drbg,
                    const unsigned char *entropy, size_t entropylen,
                    const unsigned char *nonce, size_t noncelen,
                    const unsigned char *pers, size_t perslen);
int ctr_reseed(RAND_DRBG *drbg,
               const unsigned char *entropy, size_t entropylen,
               const unsigned char *adin, size_t adinlen);
int ctr_generate(RAND_DRBG *drbg,
                 unsigned char *out, size_t outlen,
                 const unsigned char *adin, size_t adinlen);

#endif
