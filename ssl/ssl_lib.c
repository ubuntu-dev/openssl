/*
 * ! \file ssl/ssl_lib.c \brief Version independent SSL functions.
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2007 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 * ECC cipher suite support in OpenSSL originally developed by
 * SUN MICROSYSTEMS, INC., and contributed to the OpenSSL project.
 */
/* ====================================================================
 * Copyright 2005 Nokia. All rights reserved.
 *
 * The portions of the attached software ("Contribution") is developed by
 * Nokia Corporation and is licensed pursuant to the OpenSSL open source
 * license.
 *
 * The Contribution, originally written by Mika Kousa and Pasi Eronen of
 * Nokia Corporation, consists of the "PSK" (Pre-Shared Key) ciphersuites
 * support (see RFC 4279) to OpenSSL.
 *
 * No patent licenses or other rights except those expressly stated in
 * the OpenSSL open source license shall be deemed granted or received
 * expressly, by implication, estoppel, or otherwise.
 *
 * No assurances are provided by Nokia that the Contribution does not
 * infringe the patent or other intellectual property rights of any third
 * party or that the license provides you with all the necessary rights
 * to make use of the Contribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. IN
 * ADDITION TO THE DISCLAIMERS INCLUDED IN THE LICENSE, NOKIA
 * SPECIFICALLY DISCLAIMS ANY LIABILITY FOR CLAIMS BROUGHT BY YOU OR ANY
 * OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS OR
 * OTHERWISE.
 */

#ifdef REF_DEBUG
# include <assert.h>
#endif
#include <stdio.h>
#include "ssl_locl.h"
#include <openssl/objects.h>
#include <openssl/lhash.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/ocsp.h>
#ifndef OPENSSL_NO_DH
# include <openssl/dh.h>
#endif
#ifndef OPENSSL_NO_ENGINE
# include <openssl/engine.h>
#endif
#include <openssl/async.h>
#ifndef OPENSSL_NO_CT
# include <openssl/ct.h>
#endif

const char SSL_version_str[] = OPENSSL_VERSION_TEXT;

SSL3_ENC_METHOD ssl3_undef_enc_method = {
    /*
     * evil casts, but these functions are only called if there's a library
     * bug
     */
    (int (*)(SSL *, int))ssl_undefined_function,
    (int (*)(SSL *, unsigned char *, int))ssl_undefined_function,
    ssl_undefined_function,
    (int (*)(SSL *, unsigned char *, unsigned char *, int))
        ssl_undefined_function,
    (int (*)(SSL *, int))ssl_undefined_function,
    (int (*)(SSL *, const char *, int, unsigned char *))
        ssl_undefined_function,
    0,                          /* finish_mac_length */
    NULL,                       /* client_finished_label */
    0,                          /* client_finished_label_len */
    NULL,                       /* server_finished_label */
    0,                          /* server_finished_label_len */
    (int (*)(int))ssl_undefined_function,
    (int (*)(SSL *, unsigned char *, size_t, const char *,
             size_t, const unsigned char *, size_t,
             int use_context))ssl_undefined_function,
};

struct ssl_async_args {
    SSL *s;
    void *buf;
    int num;
    enum { READFUNC, WRITEFUNC,  OTHERFUNC} type;
    union {
        int (*func_read)(SSL *, void *, int);
        int (*func_write)(SSL *, const void *, int);
        int (*func_other)(SSL *);
    } f;
};

static const struct {
    uint8_t mtype;
    uint8_t ord;
    int     nid;
} dane_mds[] = {
    { DANETLS_MATCHING_FULL, 0, NID_undef },
    { DANETLS_MATCHING_2256, 1, NID_sha256 },
    { DANETLS_MATCHING_2512, 2, NID_sha512 },
};

static int dane_ctx_enable(struct dane_ctx_st *dctx)
{
    const EVP_MD **mdevp;
    uint8_t *mdord;
    uint8_t mdmax = DANETLS_MATCHING_LAST;
    int n = ((int) mdmax) + 1;          /* int to handle PrivMatch(255) */
    size_t i;

    mdevp = OPENSSL_zalloc(n * sizeof(*mdevp));
    mdord = OPENSSL_zalloc(n * sizeof(*mdord));

    if (mdord == NULL || mdevp == NULL) {
        OPENSSL_free(mdevp);
        SSLerr(SSL_F_DANE_CTX_ENABLE, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    /* Install default entries */
    for (i = 0; i < OSSL_NELEM(dane_mds); ++i) {
        const EVP_MD *md;

        if (dane_mds[i].nid == NID_undef ||
            (md = EVP_get_digestbynid(dane_mds[i].nid)) == NULL)
            continue;
        mdevp[dane_mds[i].mtype] = md;
        mdord[dane_mds[i].mtype] = dane_mds[i].ord;
    }

    dctx->mdevp = mdevp;
    dctx->mdord = mdord;
    dctx->mdmax = mdmax;

    return 1;
}

static void dane_ctx_final(struct dane_ctx_st *dctx)
{
    OPENSSL_free(dctx->mdevp);
    dctx->mdevp = NULL;

    OPENSSL_free(dctx->mdord);
    dctx->mdord = NULL;
    dctx->mdmax = 0;
}

static void tlsa_free(danetls_record *t)
{
    if (t == NULL)
        return;
    OPENSSL_free(t->data);
    EVP_PKEY_free(t->spki);
    OPENSSL_free(t);
}

static void dane_final(struct dane_st *dane)
{
    sk_danetls_record_pop_free(dane->trecs, tlsa_free);
    dane->trecs = NULL;

    sk_X509_pop_free(dane->certs, X509_free);
    dane->certs = NULL;

    X509_free(dane->mcert);
    dane->mcert = NULL;
    dane->mtlsa = NULL;
    dane->mdpth = -1;
    dane->pdpth = -1;
}

/*
 * dane_copy - Copy dane configuration, sans verification state.
 */
static int ssl_dane_dup(SSL *to, SSL *from)
{
    int num;
    int i;

    if (!DANETLS_ENABLED(&from->dane))
        return 1;

    dane_final(&to->dane);

    num  = sk_danetls_record_num(from->dane.trecs);
    for (i = 0; i < num; ++i) {
        danetls_record *t = sk_danetls_record_value(from->dane.trecs, i);
        if (SSL_dane_tlsa_add(to, t->usage, t->selector, t->mtype,
                              t->data, t->dlen) <= 0)
            return 0;
    }
    return 1;
}

static int dane_mtype_set(
    struct dane_ctx_st *dctx,
    const EVP_MD *md,
    uint8_t mtype,
    uint8_t ord)
{
    int i;

    if (mtype == DANETLS_MATCHING_FULL && md != NULL) {
        SSLerr(SSL_F_DANE_MTYPE_SET,
                SSL_R_DANE_CANNOT_OVERRIDE_MTYPE_FULL);
        return 0;
    }

    if (mtype > dctx->mdmax) {
        const EVP_MD **mdevp;
        uint8_t *mdord;
        int n = ((int) mtype) + 1;

        mdevp = OPENSSL_realloc(dctx->mdevp, n * sizeof(*mdevp));
        if (mdevp == NULL) {
            SSLerr(SSL_F_DANE_MTYPE_SET, ERR_R_MALLOC_FAILURE);
            return -1;
        }
        dctx->mdevp = mdevp;

        mdord = OPENSSL_realloc(dctx->mdord, n * sizeof(*mdord));
        if (mdord == NULL) {
            SSLerr(SSL_F_DANE_MTYPE_SET, ERR_R_MALLOC_FAILURE);
            return -1;
        }
        dctx->mdord = mdord;

        /* Zero-fill any gaps */
        for (i = dctx->mdmax+1; i < mtype; ++i) {
            mdevp[i] = NULL;
            mdord[i] = 0;
        }

        dctx->mdmax = mtype;
    }

    dctx->mdevp[mtype] = md;
    /* Coerce ordinal of disabled matching types to 0 */
    dctx->mdord[mtype] = (md == NULL) ? 0 : ord;

    return 1;
}

static const EVP_MD *tlsa_md_get(struct dane_st *dane, uint8_t mtype)
{
    if (mtype > dane->dctx->mdmax)
        return NULL;
    return dane->dctx->mdevp[mtype];
}

static int dane_tlsa_add(
    struct dane_st *dane,
    uint8_t usage,
    uint8_t selector,
    uint8_t mtype,
    unsigned char *data,
    size_t dlen)
{
    danetls_record *t;
    const EVP_MD *md = NULL;
    int ilen = (int)dlen;
    int i;

    if (dane->trecs == NULL) {
        SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_NOT_ENABLED);
        return -1;
    }

    if (ilen < 0 || dlen != (size_t)ilen) {
        SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_DATA_LENGTH);
        return 0;
    }

    if (usage > DANETLS_USAGE_LAST) {
        SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_CERTIFICATE_USAGE);
        return 0;
    }

    if (selector > DANETLS_SELECTOR_LAST) {
        SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_SELECTOR);
        return 0;
    }

    if (mtype != DANETLS_MATCHING_FULL) {
        md = tlsa_md_get(dane, mtype);
        if (md == NULL) {
            SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_MATCHING_TYPE);
            return 0;
        }
    }

    if (md != NULL && dlen != (size_t)EVP_MD_size(md)) {
        SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_DIGEST_LENGTH);
        return 0;
    }
    if (!data) {
        SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_NULL_DATA);
        return 0;
    }

    if ((t = OPENSSL_zalloc(sizeof(*t))) == NULL) {
        SSLerr(SSL_F_DANE_TLSA_ADD, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    t->usage = usage;
    t->selector = selector;
    t->mtype = mtype;
    t->data = OPENSSL_malloc(ilen);
    if (t->data == NULL) {
        tlsa_free(t);
        SSLerr(SSL_F_DANE_TLSA_ADD, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    memcpy(t->data, data, ilen);
    t->dlen = ilen;

    /* Validate and cache full certificate or public key */
    if (mtype == DANETLS_MATCHING_FULL) {
        const unsigned char *p = data;
        X509 *cert = NULL;
        EVP_PKEY *pkey = NULL;

        switch (selector) {
        case DANETLS_SELECTOR_CERT:
            if (!d2i_X509(&cert, &p, dlen) || p < data ||
                dlen != (size_t)(p - data)) {
                tlsa_free(t);
                SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_CERTIFICATE);
                return 0;
            }
            if (X509_get0_pubkey(cert) == NULL) {
                tlsa_free(t);
                SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_CERTIFICATE);
                return 0;
            }

            if ((DANETLS_USAGE_BIT(usage) & DANETLS_TA_MASK) == 0) {
                X509_free(cert);
                break;
            }

            /*
             * For usage DANE-TA(2), we support authentication via "2 0 0" TLSA
             * records that contain full certificates of trust-anchors that are
             * not present in the wire chain.  For usage PKIX-TA(0), we augment
             * the chain with untrusted Full(0) certificates from DNS, in case
             * they are missing from the chain.
             */
            if ((dane->certs == NULL &&
                 (dane->certs = sk_X509_new_null()) == NULL) ||
                !sk_X509_push(dane->certs, cert)) {
                SSLerr(SSL_F_DANE_TLSA_ADD, ERR_R_MALLOC_FAILURE);
                X509_free(cert);
                tlsa_free(t);
                return -1;
            }
            break;

        case DANETLS_SELECTOR_SPKI:
            if (!d2i_PUBKEY(&pkey, &p, dlen) || p < data ||
                dlen != (size_t)(p - data)) {
                tlsa_free(t);
                SSLerr(SSL_F_DANE_TLSA_ADD, SSL_R_DANE_TLSA_BAD_PUBLIC_KEY);
                return 0;
            }

            /*
             * For usage DANE-TA(2), we support authentication via "2 1 0" TLSA
             * records that contain full bare keys of trust-anchors that are
             * not present in the wire chain.
             */
            if (usage == DANETLS_USAGE_DANE_TA)
                t->spki = pkey;
            else
                EVP_PKEY_free(pkey);
            break;
        }
    }

    /*-
     * Find the right insertion point for the new record.
     *
     * See crypto/x509/x509_vfy.c.  We sort DANE-EE(3) records first, so that
     * they can be processed first, as they require no chain building, and no
     * expiration or hostname checks.  Because DANE-EE(3) is numerically
     * largest, this is accomplished via descending sort by "usage".
     *
     * We also sort in descending order by matching ordinal to simplify
     * the implementation of digest agility in the verification code.
     *
     * The choice of order for the selector is not significant, so we
     * use the same descending order for consistency.
     */
    for (i = 0; i < sk_danetls_record_num(dane->trecs); ++i) {
        danetls_record *rec = sk_danetls_record_value(dane->trecs, i);
        if (rec->usage > usage)
            continue;
        if (rec->usage < usage)
            break;
        if (rec->selector > selector)
            continue;
        if (rec->selector < selector)
            break;
        if (dane->dctx->mdord[rec->mtype] > dane->dctx->mdord[mtype])
            continue;
        break;
    }

    if (!sk_danetls_record_insert(dane->trecs, t, i)) {
        tlsa_free(t);
        SSLerr(SSL_F_DANE_TLSA_ADD, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    dane->umask |= DANETLS_USAGE_BIT(usage);

    return 1;
}

static void clear_ciphers(SSL *s)
{
    /* clear the current cipher */
    ssl_clear_cipher_ctx(s);
    ssl_clear_hash_ctx(&s->read_hash);
    ssl_clear_hash_ctx(&s->write_hash);
}

int SSL_clear(SSL *s)
{
    if (s->method == NULL) {
        SSLerr(SSL_F_SSL_CLEAR, SSL_R_NO_METHOD_SPECIFIED);
        return (0);
    }

    if (ssl_clear_bad_session(s)) {
        SSL_SESSION_free(s->session);
        s->session = NULL;
    }

    s->error = 0;
    s->hit = 0;
    s->shutdown = 0;

    if (s->renegotiate) {
        SSLerr(SSL_F_SSL_CLEAR, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    ossl_statem_clear(s);

    s->version = s->method->version;
    s->client_version = s->version;
    s->rwstate = SSL_NOTHING;

    BUF_MEM_free(s->init_buf);
    s->init_buf = NULL;
    clear_ciphers(s);
    s->first_packet = 0;

    /* Reset DANE verification result state */
    s->dane.mdpth = -1;
    s->dane.pdpth = -1;
    X509_free(s->dane.mcert);
    s->dane.mcert = NULL;
    s->dane.mtlsa = NULL;

    /* Clear the verification result peername */
    X509_VERIFY_PARAM_move_peername(s->param, NULL);

    /*
     * Check to see if we were changed into a different method, if so, revert
     * back if we are not doing session-id reuse.
     */
    if (!ossl_statem_get_in_handshake(s) && (s->session == NULL)
        && (s->method != s->ctx->method)) {
        s->method->ssl_free(s);
        s->method = s->ctx->method;
        if (!s->method->ssl_new(s))
            return (0);
    } else
        s->method->ssl_clear(s);

    RECORD_LAYER_clear(&s->rlayer);

    return (1);
}

/** Used to change an SSL_CTXs default SSL method type */
int SSL_CTX_set_ssl_version(SSL_CTX *ctx, const SSL_METHOD *meth)
{
    STACK_OF(SSL_CIPHER) *sk;

    ctx->method = meth;

    sk = ssl_create_cipher_list(ctx->method, &(ctx->cipher_list),
                                &(ctx->cipher_list_by_id),
                                SSL_DEFAULT_CIPHER_LIST, ctx->cert);
    if ((sk == NULL) || (sk_SSL_CIPHER_num(sk) <= 0)) {
        SSLerr(SSL_F_SSL_CTX_SET_SSL_VERSION,
               SSL_R_SSL_LIBRARY_HAS_NO_CIPHERS);
        return (0);
    }
    return (1);
}

SSL *SSL_new(SSL_CTX *ctx)
{
    SSL *s;

    if (ctx == NULL) {
        SSLerr(SSL_F_SSL_NEW, SSL_R_NULL_SSL_CTX);
        return (NULL);
    }
    if (ctx->method == NULL) {
        SSLerr(SSL_F_SSL_NEW, SSL_R_SSL_CTX_HAS_NO_DEFAULT_SSL_VERSION);
        return (NULL);
    }

    s = OPENSSL_zalloc(sizeof(*s));
    if (s == NULL)
        goto err;

    RECORD_LAYER_init(&s->rlayer, s);

    s->options = ctx->options;
    s->min_proto_version = ctx->min_proto_version;
    s->max_proto_version = ctx->max_proto_version;
    s->mode = ctx->mode;
    s->max_cert_list = ctx->max_cert_list;
    s->references = 1;

    /*
     * Earlier library versions used to copy the pointer to the CERT, not
     * its contents; only when setting new parameters for the per-SSL
     * copy, ssl_cert_new would be called (and the direct reference to
     * the per-SSL_CTX settings would be lost, but those still were
     * indirectly accessed for various purposes, and for that reason they
     * used to be known as s->ctx->default_cert). Now we don't look at the
     * SSL_CTX's CERT after having duplicated it once.
     */
    s->cert = ssl_cert_dup(ctx->cert);
    if (s->cert == NULL)
        goto err;

    RECORD_LAYER_set_read_ahead(&s->rlayer, ctx->read_ahead);
    s->msg_callback = ctx->msg_callback;
    s->msg_callback_arg = ctx->msg_callback_arg;
    s->verify_mode = ctx->verify_mode;
    s->not_resumable_session_cb = ctx->not_resumable_session_cb;
    s->sid_ctx_length = ctx->sid_ctx_length;
    OPENSSL_assert(s->sid_ctx_length <= sizeof s->sid_ctx);
    memcpy(&s->sid_ctx, &ctx->sid_ctx, sizeof(s->sid_ctx));
    s->verify_callback = ctx->default_verify_callback;
    s->generate_session_id = ctx->generate_session_id;

    s->param = X509_VERIFY_PARAM_new();
    if (s->param == NULL)
        goto err;
    X509_VERIFY_PARAM_inherit(s->param, ctx->param);
    s->quiet_shutdown = ctx->quiet_shutdown;
    s->max_send_fragment = ctx->max_send_fragment;

    CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
    s->ctx = ctx;
    s->tlsext_debug_cb = 0;
    s->tlsext_debug_arg = NULL;
    s->tlsext_ticket_expected = 0;
    s->tlsext_status_type = -1;
    s->tlsext_status_expected = 0;
    s->tlsext_ocsp_ids = NULL;
    s->tlsext_ocsp_exts = NULL;
    s->tlsext_ocsp_resp = NULL;
    s->tlsext_ocsp_resplen = -1;
    CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
    s->initial_ctx = ctx;
# ifndef OPENSSL_NO_EC
    if (ctx->tlsext_ecpointformatlist) {
        s->tlsext_ecpointformatlist =
            OPENSSL_memdup(ctx->tlsext_ecpointformatlist,
                           ctx->tlsext_ecpointformatlist_length);
        if (!s->tlsext_ecpointformatlist)
            goto err;
        s->tlsext_ecpointformatlist_length =
            ctx->tlsext_ecpointformatlist_length;
    }
    if (ctx->tlsext_ellipticcurvelist) {
        s->tlsext_ellipticcurvelist =
            OPENSSL_memdup(ctx->tlsext_ellipticcurvelist,
                           ctx->tlsext_ellipticcurvelist_length);
        if (!s->tlsext_ellipticcurvelist)
            goto err;
        s->tlsext_ellipticcurvelist_length =
            ctx->tlsext_ellipticcurvelist_length;
    }
# endif
# ifndef OPENSSL_NO_NEXTPROTONEG
    s->next_proto_negotiated = NULL;
# endif

    if (s->ctx->alpn_client_proto_list) {
        s->alpn_client_proto_list =
            OPENSSL_malloc(s->ctx->alpn_client_proto_list_len);
        if (s->alpn_client_proto_list == NULL)
            goto err;
        memcpy(s->alpn_client_proto_list, s->ctx->alpn_client_proto_list,
               s->ctx->alpn_client_proto_list_len);
        s->alpn_client_proto_list_len = s->ctx->alpn_client_proto_list_len;
    }

    s->verified_chain = NULL;
    s->verify_result = X509_V_OK;

    s->default_passwd_callback = ctx->default_passwd_callback;
    s->default_passwd_callback_userdata = ctx->default_passwd_callback_userdata;

    s->method = ctx->method;

    if (!s->method->ssl_new(s))
        goto err;

    s->server = (ctx->method->ssl_accept == ssl_undefined_function) ? 0 : 1;

    if (!SSL_clear(s))
        goto err;

    CRYPTO_new_ex_data(CRYPTO_EX_INDEX_SSL, s, &s->ex_data);

#ifndef OPENSSL_NO_PSK
    s->psk_client_callback = ctx->psk_client_callback;
    s->psk_server_callback = ctx->psk_server_callback;
#endif

    s->job = NULL;

#ifndef OPENSSL_NO_CT
    if (!SSL_set_ct_validation_callback(s, ctx->ct_validation_callback,
            ctx->ct_validation_callback_arg))
        goto err;
#endif

    return (s);
 err:
    SSL_free(s);
    SSLerr(SSL_F_SSL_NEW, ERR_R_MALLOC_FAILURE);
    return (NULL);
}

void SSL_up_ref(SSL *s)
{
    CRYPTO_add(&s->references, 1, CRYPTO_LOCK_SSL);
}

int SSL_CTX_set_session_id_context(SSL_CTX *ctx, const unsigned char *sid_ctx,
                                   unsigned int sid_ctx_len)
{
    if (sid_ctx_len > sizeof ctx->sid_ctx) {
        SSLerr(SSL_F_SSL_CTX_SET_SESSION_ID_CONTEXT,
               SSL_R_SSL_SESSION_ID_CONTEXT_TOO_LONG);
        return 0;
    }
    ctx->sid_ctx_length = sid_ctx_len;
    memcpy(ctx->sid_ctx, sid_ctx, sid_ctx_len);

    return 1;
}

int SSL_set_session_id_context(SSL *ssl, const unsigned char *sid_ctx,
                               unsigned int sid_ctx_len)
{
    if (sid_ctx_len > SSL_MAX_SID_CTX_LENGTH) {
        SSLerr(SSL_F_SSL_SET_SESSION_ID_CONTEXT,
               SSL_R_SSL_SESSION_ID_CONTEXT_TOO_LONG);
        return 0;
    }
    ssl->sid_ctx_length = sid_ctx_len;
    memcpy(ssl->sid_ctx, sid_ctx, sid_ctx_len);

    return 1;
}

int SSL_CTX_set_generate_session_id(SSL_CTX *ctx, GEN_SESSION_CB cb)
{
    CRYPTO_w_lock(CRYPTO_LOCK_SSL_CTX);
    ctx->generate_session_id = cb;
    CRYPTO_w_unlock(CRYPTO_LOCK_SSL_CTX);
    return 1;
}

int SSL_set_generate_session_id(SSL *ssl, GEN_SESSION_CB cb)
{
    CRYPTO_w_lock(CRYPTO_LOCK_SSL);
    ssl->generate_session_id = cb;
    CRYPTO_w_unlock(CRYPTO_LOCK_SSL);
    return 1;
}

int SSL_has_matching_session_id(const SSL *ssl, const unsigned char *id,
                                unsigned int id_len)
{
    /*
     * A quick examination of SSL_SESSION_hash and SSL_SESSION_cmp shows how
     * we can "construct" a session to give us the desired check - ie. to
     * find if there's a session in the hash table that would conflict with
     * any new session built out of this id/id_len and the ssl_version in use
     * by this SSL.
     */
    SSL_SESSION r, *p;

    if (id_len > sizeof r.session_id)
        return 0;

    r.ssl_version = ssl->version;
    r.session_id_length = id_len;
    memcpy(r.session_id, id, id_len);

    CRYPTO_r_lock(CRYPTO_LOCK_SSL_CTX);
    p = lh_SSL_SESSION_retrieve(ssl->ctx->sessions, &r);
    CRYPTO_r_unlock(CRYPTO_LOCK_SSL_CTX);
    return (p != NULL);
}

int SSL_CTX_set_purpose(SSL_CTX *s, int purpose)
{
    return X509_VERIFY_PARAM_set_purpose(s->param, purpose);
}

int SSL_set_purpose(SSL *s, int purpose)
{
    return X509_VERIFY_PARAM_set_purpose(s->param, purpose);
}

int SSL_CTX_set_trust(SSL_CTX *s, int trust)
{
    return X509_VERIFY_PARAM_set_trust(s->param, trust);
}

int SSL_set_trust(SSL *s, int trust)
{
    return X509_VERIFY_PARAM_set_trust(s->param, trust);
}

int SSL_set1_host(SSL *s, const char *hostname)
{
    return X509_VERIFY_PARAM_set1_host(s->param, hostname, 0);
}

int SSL_add1_host(SSL *s, const char *hostname)
{
    return X509_VERIFY_PARAM_add1_host(s->param, hostname, 0);
}

void SSL_set_hostflags(SSL *s, unsigned int flags)
{
    X509_VERIFY_PARAM_set_hostflags(s->param, flags);
}

const char *SSL_get0_peername(SSL *s)
{
    return X509_VERIFY_PARAM_get0_peername(s->param);
}

int SSL_CTX_dane_enable(SSL_CTX *ctx)
{
    return dane_ctx_enable(&ctx->dane);
}

int SSL_dane_enable(SSL *s, const char *basedomain)
{
    struct dane_st *dane = &s->dane;

    if (s->ctx->dane.mdmax == 0) {
        SSLerr(SSL_F_SSL_DANE_ENABLE, SSL_R_CONTEXT_NOT_DANE_ENABLED);
        return 0;
    }
    if (dane->trecs != NULL) {
        SSLerr(SSL_F_SSL_DANE_ENABLE, SSL_R_DANE_ALREADY_ENABLED);
        return 0;
    }

    /*
     * Default SNI name.  This rejects empty names, while set1_host below
     * accepts them and disables host name checks.  To avoid side-effects with
     * invalid input, set the SNI name first.
     */
    if (s->tlsext_hostname == NULL) {
	if (!SSL_set_tlsext_host_name(s, basedomain)) {
            SSLerr(SSL_F_SSL_DANE_ENABLE, SSL_R_ERROR_SETTING_TLSA_BASE_DOMAIN);
	    return -1;
        }
    }

    /* Primary RFC6125 reference identifier */
    if (!X509_VERIFY_PARAM_set1_host(s->param, basedomain, 0)) {
        SSLerr(SSL_F_SSL_DANE_ENABLE, SSL_R_ERROR_SETTING_TLSA_BASE_DOMAIN);
        return -1;
    }

    dane->mdpth = -1;
    dane->pdpth = -1;
    dane->dctx = &s->ctx->dane;
    dane->trecs = sk_danetls_record_new_null();

    if (dane->trecs == NULL) {
        SSLerr(SSL_F_SSL_DANE_ENABLE, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    return 1;
}

int SSL_get0_dane_authority(SSL *s, X509 **mcert, EVP_PKEY **mspki)
{
    struct dane_st *dane = &s->dane;

    if (!DANETLS_ENABLED(dane) || s->verify_result != X509_V_OK)
        return -1;
    if (dane->mtlsa) {
        if (mcert)
            *mcert = dane->mcert;
        if (mspki)
            *mspki = (dane->mcert == NULL) ? dane->mtlsa->spki : NULL;
    }
    return dane->mdpth;
}

int SSL_get0_dane_tlsa(SSL *s, uint8_t *usage, uint8_t *selector,
                       uint8_t *mtype, unsigned const char **data, size_t *dlen)
{
    struct dane_st *dane = &s->dane;

    if (!DANETLS_ENABLED(dane) || s->verify_result != X509_V_OK)
        return -1;
    if (dane->mtlsa) {
        if (usage)
            *usage = dane->mtlsa->usage;
        if (selector)
            *selector = dane->mtlsa->selector;
        if (mtype)
            *mtype = dane->mtlsa->mtype;
        if (data)
            *data = dane->mtlsa->data;
        if (dlen)
            *dlen = dane->mtlsa->dlen;
    }
    return dane->mdpth;
}

struct dane_st *SSL_get0_dane(SSL *s)
{
    return &s->dane;
}

int SSL_dane_tlsa_add(SSL *s, uint8_t usage, uint8_t selector,
                      uint8_t mtype, unsigned char *data, size_t dlen)
{
    return dane_tlsa_add(&s->dane, usage, selector, mtype, data, dlen);
}

int SSL_CTX_dane_mtype_set(SSL_CTX *ctx, const EVP_MD *md, uint8_t mtype, uint8_t ord)
{
    return dane_mtype_set(&ctx->dane, md, mtype, ord);
}

int SSL_CTX_set1_param(SSL_CTX *ctx, X509_VERIFY_PARAM *vpm)
{
    return X509_VERIFY_PARAM_set1(ctx->param, vpm);
}

int SSL_set1_param(SSL *ssl, X509_VERIFY_PARAM *vpm)
{
    return X509_VERIFY_PARAM_set1(ssl->param, vpm);
}

X509_VERIFY_PARAM *SSL_CTX_get0_param(SSL_CTX *ctx)
{
    return ctx->param;
}

X509_VERIFY_PARAM *SSL_get0_param(SSL *ssl)
{
    return ssl->param;
}

void SSL_certs_clear(SSL *s)
{
    ssl_cert_clear_certs(s->cert);
}

void SSL_free(SSL *s)
{
    int i;

    if (s == NULL)
        return;

    i = CRYPTO_add(&s->references, -1, CRYPTO_LOCK_SSL);
    REF_PRINT_COUNT("SSL", s);
    if (i > 0)
        return;
    REF_ASSERT_ISNT(i < 0);

    X509_VERIFY_PARAM_free(s->param);
    dane_final(&s->dane);
    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_SSL, s, &s->ex_data);

    if (s->bbio != NULL) {
        /* If the buffering BIO is in place, pop it off */
        if (s->bbio == s->wbio) {
            s->wbio = BIO_pop(s->wbio);
        }
        BIO_free(s->bbio);
        s->bbio = NULL;
    }
    BIO_free_all(s->rbio);
    if (s->wbio != s->rbio)
        BIO_free_all(s->wbio);

    BUF_MEM_free(s->init_buf);

    /* add extra stuff */
    sk_SSL_CIPHER_free(s->cipher_list);
    sk_SSL_CIPHER_free(s->cipher_list_by_id);

    /* Make the next call work :-) */
    if (s->session != NULL) {
        ssl_clear_bad_session(s);
        SSL_SESSION_free(s->session);
    }

    clear_ciphers(s);

    ssl_cert_free(s->cert);
    /* Free up if allocated */

    OPENSSL_free(s->tlsext_hostname);
    SSL_CTX_free(s->initial_ctx);
#ifndef OPENSSL_NO_EC
    OPENSSL_free(s->tlsext_ecpointformatlist);
    OPENSSL_free(s->tlsext_ellipticcurvelist);
#endif                         /* OPENSSL_NO_EC */
    sk_X509_EXTENSION_pop_free(s->tlsext_ocsp_exts, X509_EXTENSION_free);
    sk_OCSP_RESPID_pop_free(s->tlsext_ocsp_ids, OCSP_RESPID_free);
#ifndef OPENSSL_NO_CT
    SCT_LIST_free(s->scts);
    OPENSSL_free(s->ct_validation_callback_arg);
    OPENSSL_free(s->tlsext_scts);
#endif
    OPENSSL_free(s->tlsext_ocsp_resp);
    OPENSSL_free(s->alpn_client_proto_list);

    sk_X509_NAME_pop_free(s->client_CA, X509_NAME_free);

    sk_X509_pop_free(s->verified_chain, X509_free);

    if (s->method != NULL)
        s->method->ssl_free(s);

    RECORD_LAYER_release(&s->rlayer);

    SSL_CTX_free(s->ctx);

#if !defined(OPENSSL_NO_NEXTPROTONEG)
    OPENSSL_free(s->next_proto_negotiated);
#endif

#ifndef OPENSSL_NO_SRTP
    sk_SRTP_PROTECTION_PROFILE_free(s->srtp_profiles);
#endif

    OPENSSL_free(s);
}

void SSL_set_rbio(SSL *s, BIO *rbio)
{
    if (s->rbio != rbio)
        BIO_free_all(s->rbio);
    s->rbio = rbio;
}

void SSL_set_wbio(SSL *s, BIO *wbio)
{
    /*
     * If the output buffering BIO is still in place, remove it
     */
    if (s->bbio != NULL) {
        if (s->wbio == s->bbio) {
            s->wbio = s->wbio->next_bio;
            s->bbio->next_bio = NULL;
        }
    }
    if (s->wbio != wbio && s->rbio != s->wbio)
        BIO_free_all(s->wbio);
    s->wbio = wbio;
}

void SSL_set_bio(SSL *s, BIO *rbio, BIO *wbio)
{
    SSL_set_wbio(s, wbio);
    SSL_set_rbio(s, rbio);
}

BIO *SSL_get_rbio(const SSL *s)
{
    return (s->rbio);
}

BIO *SSL_get_wbio(const SSL *s)
{
    return (s->wbio);
}

int SSL_get_fd(const SSL *s)
{
    return (SSL_get_rfd(s));
}

int SSL_get_rfd(const SSL *s)
{
    int ret = -1;
    BIO *b, *r;

    b = SSL_get_rbio(s);
    r = BIO_find_type(b, BIO_TYPE_DESCRIPTOR);
    if (r != NULL)
        BIO_get_fd(r, &ret);
    return (ret);
}

int SSL_get_wfd(const SSL *s)
{
    int ret = -1;
    BIO *b, *r;

    b = SSL_get_wbio(s);
    r = BIO_find_type(b, BIO_TYPE_DESCRIPTOR);
    if (r != NULL)
        BIO_get_fd(r, &ret);
    return (ret);
}

#ifndef OPENSSL_NO_SOCK
int SSL_set_fd(SSL *s, int fd)
{
    int ret = 0;
    BIO *bio = NULL;

    bio = BIO_new(BIO_s_socket());

    if (bio == NULL) {
        SSLerr(SSL_F_SSL_SET_FD, ERR_R_BUF_LIB);
        goto err;
    }
    BIO_set_fd(bio, fd, BIO_NOCLOSE);
    SSL_set_bio(s, bio, bio);
    ret = 1;
 err:
    return (ret);
}

int SSL_set_wfd(SSL *s, int fd)
{
    int ret = 0;
    BIO *bio = NULL;

    if ((s->rbio == NULL) || (BIO_method_type(s->rbio) != BIO_TYPE_SOCKET)
        || ((int)BIO_get_fd(s->rbio, NULL) != fd)) {
        bio = BIO_new(BIO_s_socket());

        if (bio == NULL) {
            SSLerr(SSL_F_SSL_SET_WFD, ERR_R_BUF_LIB);
            goto err;
        }
        BIO_set_fd(bio, fd, BIO_NOCLOSE);
        SSL_set_bio(s, SSL_get_rbio(s), bio);
    } else
        SSL_set_bio(s, SSL_get_rbio(s), SSL_get_rbio(s));
    ret = 1;
 err:
    return (ret);
}

int SSL_set_rfd(SSL *s, int fd)
{
    int ret = 0;
    BIO *bio = NULL;

    if ((s->wbio == NULL) || (BIO_method_type(s->wbio) != BIO_TYPE_SOCKET)
        || ((int)BIO_get_fd(s->wbio, NULL) != fd)) {
        bio = BIO_new(BIO_s_socket());

        if (bio == NULL) {
            SSLerr(SSL_F_SSL_SET_RFD, ERR_R_BUF_LIB);
            goto err;
        }
        BIO_set_fd(bio, fd, BIO_NOCLOSE);
        SSL_set_bio(s, bio, SSL_get_wbio(s));
    } else
        SSL_set_bio(s, SSL_get_wbio(s), SSL_get_wbio(s));
    ret = 1;
 err:
    return (ret);
}
#endif

/* return length of latest Finished message we sent, copy to 'buf' */
size_t SSL_get_finished(const SSL *s, void *buf, size_t count)
{
    size_t ret = 0;

    if (s->s3 != NULL) {
        ret = s->s3->tmp.finish_md_len;
        if (count > ret)
            count = ret;
        memcpy(buf, s->s3->tmp.finish_md, count);
    }
    return ret;
}

/* return length of latest Finished message we expected, copy to 'buf' */
size_t SSL_get_peer_finished(const SSL *s, void *buf, size_t count)
{
    size_t ret = 0;

    if (s->s3 != NULL) {
        ret = s->s3->tmp.peer_finish_md_len;
        if (count > ret)
            count = ret;
        memcpy(buf, s->s3->tmp.peer_finish_md, count);
    }
    return ret;
}

int SSL_get_verify_mode(const SSL *s)
{
    return (s->verify_mode);
}

int SSL_get_verify_depth(const SSL *s)
{
    return X509_VERIFY_PARAM_get_depth(s->param);
}

int (*SSL_get_verify_callback(const SSL *s)) (int, X509_STORE_CTX *) {
    return (s->verify_callback);
}

int SSL_CTX_get_verify_mode(const SSL_CTX *ctx)
{
    return (ctx->verify_mode);
}

int SSL_CTX_get_verify_depth(const SSL_CTX *ctx)
{
    return X509_VERIFY_PARAM_get_depth(ctx->param);
}

int (*SSL_CTX_get_verify_callback(const SSL_CTX *ctx)) (int, X509_STORE_CTX *) {
    return (ctx->default_verify_callback);
}

void SSL_set_verify(SSL *s, int mode,
                    int (*callback) (int ok, X509_STORE_CTX *ctx))
{
    s->verify_mode = mode;
    if (callback != NULL)
        s->verify_callback = callback;
}

void SSL_set_verify_depth(SSL *s, int depth)
{
    X509_VERIFY_PARAM_set_depth(s->param, depth);
}

void SSL_set_read_ahead(SSL *s, int yes)
{
    RECORD_LAYER_set_read_ahead(&s->rlayer, yes);
}

int SSL_get_read_ahead(const SSL *s)
{
    return RECORD_LAYER_get_read_ahead(&s->rlayer);
}

int SSL_pending(const SSL *s)
{
    /*
     * SSL_pending cannot work properly if read-ahead is enabled
     * (SSL_[CTX_]ctrl(..., SSL_CTRL_SET_READ_AHEAD, 1, NULL)), and it is
     * impossible to fix since SSL_pending cannot report errors that may be
     * observed while scanning the new data. (Note that SSL_pending() is
     * often used as a boolean value, so we'd better not return -1.)
     */
    return (s->method->ssl_pending(s));
}

X509 *SSL_get_peer_certificate(const SSL *s)
{
    X509 *r;

    if ((s == NULL) || (s->session == NULL))
        r = NULL;
    else
        r = s->session->peer;

    if (r == NULL)
        return (r);

    X509_up_ref(r);

    return (r);
}

STACK_OF(X509) *SSL_get_peer_cert_chain(const SSL *s)
{
    STACK_OF(X509) *r;

    if ((s == NULL) || (s->session == NULL))
        r = NULL;
    else
        r = s->session->peer_chain;

    /*
     * If we are a client, cert_chain includes the peer's own certificate; if
     * we are a server, it does not.
     */

    return (r);
}

/*
 * Now in theory, since the calling process own 't' it should be safe to
 * modify.  We need to be able to read f without being hassled
 */
int SSL_copy_session_id(SSL *t, const SSL *f)
{
    /* Do we need to to SSL locking? */
    if (!SSL_set_session(t, SSL_get_session(f))) {
        return 0;
    }

    /*
     * what if we are setup for one protocol version but want to talk another
     */
    if (t->method != f->method) {
        t->method->ssl_free(t);
        t->method = f->method;
        if (t->method->ssl_new(t) == 0)
            return 0;
    }

    CRYPTO_add(&f->cert->references, 1, CRYPTO_LOCK_SSL_CERT);
    ssl_cert_free(t->cert);
    t->cert = f->cert;
    if (!SSL_set_session_id_context(t, f->sid_ctx, f->sid_ctx_length)) {
        return 0;
    }

    return 1;
}

/* Fix this so it checks all the valid key/cert options */
int SSL_CTX_check_private_key(const SSL_CTX *ctx)
{
    if ((ctx == NULL) ||
        (ctx->cert->key->x509 == NULL)) {
        SSLerr(SSL_F_SSL_CTX_CHECK_PRIVATE_KEY,
               SSL_R_NO_CERTIFICATE_ASSIGNED);
        return (0);
    }
    if (ctx->cert->key->privatekey == NULL) {
        SSLerr(SSL_F_SSL_CTX_CHECK_PRIVATE_KEY,
               SSL_R_NO_PRIVATE_KEY_ASSIGNED);
        return (0);
    }
    return (X509_check_private_key
            (ctx->cert->key->x509, ctx->cert->key->privatekey));
}

/* Fix this function so that it takes an optional type parameter */
int SSL_check_private_key(const SSL *ssl)
{
    if (ssl == NULL) {
        SSLerr(SSL_F_SSL_CHECK_PRIVATE_KEY, ERR_R_PASSED_NULL_PARAMETER);
        return (0);
    }
    if (ssl->cert->key->x509 == NULL) {
        SSLerr(SSL_F_SSL_CHECK_PRIVATE_KEY, SSL_R_NO_CERTIFICATE_ASSIGNED);
        return (0);
    }
    if (ssl->cert->key->privatekey == NULL) {
        SSLerr(SSL_F_SSL_CHECK_PRIVATE_KEY, SSL_R_NO_PRIVATE_KEY_ASSIGNED);
        return (0);
    }
    return (X509_check_private_key(ssl->cert->key->x509,
                                   ssl->cert->key->privatekey));
}

int SSL_waiting_for_async(SSL *s)
{
    if(s->job)
        return 1;

    return 0;
}

int SSL_get_async_wait_fd(SSL *s)
{
    if (!s->job)
        return -1;

    return ASYNC_get_wait_fd(s->job);
}

int SSL_accept(SSL *s)
{
    if (s->handshake_func == 0) {
        /* Not properly initialized yet */
        SSL_set_accept_state(s);
    }

    return SSL_do_handshake(s);
}

int SSL_connect(SSL *s)
{
    if (s->handshake_func == 0) {
        /* Not properly initialized yet */
        SSL_set_connect_state(s);
    }

    return SSL_do_handshake(s);
}

long SSL_get_default_timeout(const SSL *s)
{
    return (s->method->get_timeout());
}

static int ssl_start_async_job(SSL *s, struct ssl_async_args *args,
                          int (*func)(void *)) {
    int ret;
    switch(ASYNC_start_job(&s->job, &ret, func, args,
        sizeof(struct ssl_async_args))) {
    case ASYNC_ERR:
        s->rwstate = SSL_NOTHING;
        SSLerr(SSL_F_SSL_START_ASYNC_JOB, SSL_R_FAILED_TO_INIT_ASYNC);
        return -1;
    case ASYNC_PAUSE:
        s->rwstate = SSL_ASYNC_PAUSED;
        return -1;
    case ASYNC_FINISH:
        s->job = NULL;
        return ret;
    default:
        s->rwstate = SSL_NOTHING;
        SSLerr(SSL_F_SSL_START_ASYNC_JOB, ERR_R_INTERNAL_ERROR);
        /* Shouldn't happen */
        return -1;
    }
}

static int ssl_io_intern(void *vargs)
{
    struct ssl_async_args *args;
    SSL *s;
    void *buf;
    int num;

    args = (struct ssl_async_args *)vargs;
    s = args->s;
    buf = args->buf;
    num = args->num;
    switch (args->type) {
    case READFUNC:
        return args->f.func_read(s, buf, num);
    case WRITEFUNC:
        return args->f.func_write(s, buf, num);
    case OTHERFUNC:
        return args->f.func_other(s);
    }
    return -1;
}

int SSL_read(SSL *s, void *buf, int num)
{
    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_READ, SSL_R_UNINITIALIZED);
        return -1;
    }

    if (s->shutdown & SSL_RECEIVED_SHUTDOWN) {
        s->rwstate = SSL_NOTHING;
        return (0);
    }

    if((s->mode & SSL_MODE_ASYNC) && ASYNC_get_current_job() == NULL) {
        struct ssl_async_args args;

        args.s = s;
        args.buf = buf;
        args.num = num;
        args.type = READFUNC;
        args.f.func_read = s->method->ssl_read;

        return ssl_start_async_job(s, &args, ssl_io_intern);
    } else {
        return s->method->ssl_read(s, buf, num);
    }
}

int SSL_peek(SSL *s, void *buf, int num)
{
    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_PEEK, SSL_R_UNINITIALIZED);
        return -1;
    }

    if (s->shutdown & SSL_RECEIVED_SHUTDOWN) {
        return (0);
    }
    if((s->mode & SSL_MODE_ASYNC) && ASYNC_get_current_job() == NULL) {
        struct ssl_async_args args;

        args.s = s;
        args.buf = buf;
        args.num = num;
        args.type = READFUNC;
        args.f.func_read = s->method->ssl_peek;

        return ssl_start_async_job(s, &args, ssl_io_intern);
    } else {
        return s->method->ssl_peek(s, buf, num);
    }
}

int SSL_write(SSL *s, const void *buf, int num)
{
    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_WRITE, SSL_R_UNINITIALIZED);
        return -1;
    }

    if (s->shutdown & SSL_SENT_SHUTDOWN) {
        s->rwstate = SSL_NOTHING;
        SSLerr(SSL_F_SSL_WRITE, SSL_R_PROTOCOL_IS_SHUTDOWN);
        return (-1);
    }

    if((s->mode & SSL_MODE_ASYNC) && ASYNC_get_current_job() == NULL) {
        struct ssl_async_args args;

        args.s = s;
        args.buf = (void *)buf;
        args.num = num;
        args.type = WRITEFUNC;
        args.f.func_write = s->method->ssl_write;

        return ssl_start_async_job(s, &args, ssl_io_intern);
    } else {
        return s->method->ssl_write(s, buf, num);
    }
}

int SSL_shutdown(SSL *s)
{
    /*
     * Note that this function behaves differently from what one might
     * expect.  Return values are 0 for no success (yet), 1 for success; but
     * calling it once is usually not enough, even if blocking I/O is used
     * (see ssl3_shutdown).
     */

    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_SHUTDOWN, SSL_R_UNINITIALIZED);
        return -1;
    }

    if (!SSL_in_init(s)) {
        if((s->mode & SSL_MODE_ASYNC) && ASYNC_get_current_job() == NULL) {
            struct ssl_async_args args;

            args.s = s;
            args.type = OTHERFUNC;
            args.f.func_other = s->method->ssl_shutdown;

            return ssl_start_async_job(s, &args, ssl_io_intern);
        } else {
            return s->method->ssl_shutdown(s);
        }
    } else {
        SSLerr(SSL_F_SSL_SHUTDOWN, SSL_R_SHUTDOWN_WHILE_IN_INIT);
        return -1;
    }
}

int SSL_renegotiate(SSL *s)
{
    if (s->renegotiate == 0)
        s->renegotiate = 1;

    s->new_session = 1;

    return (s->method->ssl_renegotiate(s));
}

int SSL_renegotiate_abbreviated(SSL *s)
{
    if (s->renegotiate == 0)
        s->renegotiate = 1;

    s->new_session = 0;

    return (s->method->ssl_renegotiate(s));
}

int SSL_renegotiate_pending(SSL *s)
{
    /*
     * becomes true when negotiation is requested; false again once a
     * handshake has finished
     */
    return (s->renegotiate != 0);
}

long SSL_ctrl(SSL *s, int cmd, long larg, void *parg)
{
    long l;

    switch (cmd) {
    case SSL_CTRL_GET_READ_AHEAD:
        return (RECORD_LAYER_get_read_ahead(&s->rlayer));
    case SSL_CTRL_SET_READ_AHEAD:
        l = RECORD_LAYER_get_read_ahead(&s->rlayer);
        RECORD_LAYER_set_read_ahead(&s->rlayer, larg);
        return (l);

    case SSL_CTRL_SET_MSG_CALLBACK_ARG:
        s->msg_callback_arg = parg;
        return 1;

    case SSL_CTRL_MODE:
        return (s->mode |= larg);
    case SSL_CTRL_CLEAR_MODE:
        return (s->mode &= ~larg);
    case SSL_CTRL_GET_MAX_CERT_LIST:
        return (s->max_cert_list);
    case SSL_CTRL_SET_MAX_CERT_LIST:
        l = s->max_cert_list;
        s->max_cert_list = larg;
        return (l);
    case SSL_CTRL_SET_MAX_SEND_FRAGMENT:
        if (larg < 512 || larg > SSL3_RT_MAX_PLAIN_LENGTH)
            return 0;
        s->max_send_fragment = larg;
        return 1;
    case SSL_CTRL_GET_RI_SUPPORT:
        if (s->s3)
            return s->s3->send_connection_binding;
        else
            return 0;
    case SSL_CTRL_CERT_FLAGS:
        return (s->cert->cert_flags |= larg);
    case SSL_CTRL_CLEAR_CERT_FLAGS:
        return (s->cert->cert_flags &= ~larg);

    case SSL_CTRL_GET_RAW_CIPHERLIST:
        if (parg) {
            if (s->s3->tmp.ciphers_raw == NULL)
                return 0;
            *(unsigned char **)parg = s->s3->tmp.ciphers_raw;
            return (int)s->s3->tmp.ciphers_rawlen;
        } else {
            return TLS_CIPHER_LEN;
        }
    case SSL_CTRL_GET_EXTMS_SUPPORT:
        if (!s->session || SSL_in_init(s) || ossl_statem_get_in_handshake(s))
		return -1;
	if (s->session->flags & SSL_SESS_FLAG_EXTMS)
            return 1;
        else
            return 0;
    case SSL_CTRL_SET_MIN_PROTO_VERSION:
        return ssl_set_version_bound(s->ctx->method->version, (int)larg,
                                     &s->min_proto_version);
    case SSL_CTRL_SET_MAX_PROTO_VERSION:
        return ssl_set_version_bound(s->ctx->method->version, (int)larg,
                                     &s->max_proto_version);
    default:
        return (s->method->ssl_ctrl(s, cmd, larg, parg));
    }
}

long SSL_callback_ctrl(SSL *s, int cmd, void (*fp) (void))
{
    switch (cmd) {
    case SSL_CTRL_SET_MSG_CALLBACK:
        s->msg_callback = (void (*)
                           (int write_p, int version, int content_type,
                            const void *buf, size_t len, SSL *ssl,
                            void *arg))(fp);
        return 1;

    default:
        return (s->method->ssl_callback_ctrl(s, cmd, fp));
    }
}

LHASH_OF(SSL_SESSION) *SSL_CTX_sessions(SSL_CTX *ctx)
{
    return ctx->sessions;
}

long SSL_CTX_ctrl(SSL_CTX *ctx, int cmd, long larg, void *parg)
{
    long l;
    /* For some cases with ctx == NULL perform syntax checks */
    if (ctx == NULL) {
        switch (cmd) {
#ifndef OPENSSL_NO_EC
        case SSL_CTRL_SET_CURVES_LIST:
            return tls1_set_curves_list(NULL, NULL, parg);
#endif
        case SSL_CTRL_SET_SIGALGS_LIST:
        case SSL_CTRL_SET_CLIENT_SIGALGS_LIST:
            return tls1_set_sigalgs_list(NULL, parg, 0);
        default:
            return 0;
        }
    }

    switch (cmd) {
    case SSL_CTRL_GET_READ_AHEAD:
        return (ctx->read_ahead);
    case SSL_CTRL_SET_READ_AHEAD:
        l = ctx->read_ahead;
        ctx->read_ahead = larg;
        return (l);

    case SSL_CTRL_SET_MSG_CALLBACK_ARG:
        ctx->msg_callback_arg = parg;
        return 1;

    case SSL_CTRL_GET_MAX_CERT_LIST:
        return (ctx->max_cert_list);
    case SSL_CTRL_SET_MAX_CERT_LIST:
        l = ctx->max_cert_list;
        ctx->max_cert_list = larg;
        return (l);

    case SSL_CTRL_SET_SESS_CACHE_SIZE:
        l = ctx->session_cache_size;
        ctx->session_cache_size = larg;
        return (l);
    case SSL_CTRL_GET_SESS_CACHE_SIZE:
        return (ctx->session_cache_size);
    case SSL_CTRL_SET_SESS_CACHE_MODE:
        l = ctx->session_cache_mode;
        ctx->session_cache_mode = larg;
        return (l);
    case SSL_CTRL_GET_SESS_CACHE_MODE:
        return (ctx->session_cache_mode);

    case SSL_CTRL_SESS_NUMBER:
        return (lh_SSL_SESSION_num_items(ctx->sessions));
    case SSL_CTRL_SESS_CONNECT:
        return (ctx->stats.sess_connect);
    case SSL_CTRL_SESS_CONNECT_GOOD:
        return (ctx->stats.sess_connect_good);
    case SSL_CTRL_SESS_CONNECT_RENEGOTIATE:
        return (ctx->stats.sess_connect_renegotiate);
    case SSL_CTRL_SESS_ACCEPT:
        return (ctx->stats.sess_accept);
    case SSL_CTRL_SESS_ACCEPT_GOOD:
        return (ctx->stats.sess_accept_good);
    case SSL_CTRL_SESS_ACCEPT_RENEGOTIATE:
        return (ctx->stats.sess_accept_renegotiate);
    case SSL_CTRL_SESS_HIT:
        return (ctx->stats.sess_hit);
    case SSL_CTRL_SESS_CB_HIT:
        return (ctx->stats.sess_cb_hit);
    case SSL_CTRL_SESS_MISSES:
        return (ctx->stats.sess_miss);
    case SSL_CTRL_SESS_TIMEOUTS:
        return (ctx->stats.sess_timeout);
    case SSL_CTRL_SESS_CACHE_FULL:
        return (ctx->stats.sess_cache_full);
    case SSL_CTRL_MODE:
        return (ctx->mode |= larg);
    case SSL_CTRL_CLEAR_MODE:
        return (ctx->mode &= ~larg);
    case SSL_CTRL_SET_MAX_SEND_FRAGMENT:
        if (larg < 512 || larg > SSL3_RT_MAX_PLAIN_LENGTH)
            return 0;
        ctx->max_send_fragment = larg;
        return 1;
    case SSL_CTRL_CERT_FLAGS:
        return (ctx->cert->cert_flags |= larg);
    case SSL_CTRL_CLEAR_CERT_FLAGS:
        return (ctx->cert->cert_flags &= ~larg);
    case SSL_CTRL_SET_MIN_PROTO_VERSION:
        return ssl_set_version_bound(ctx->method->version, (int)larg,
                                     &ctx->min_proto_version);
    case SSL_CTRL_SET_MAX_PROTO_VERSION:
        return ssl_set_version_bound(ctx->method->version, (int)larg,
                                     &ctx->max_proto_version);
    default:
        return (ctx->method->ssl_ctx_ctrl(ctx, cmd, larg, parg));
    }
}

long SSL_CTX_callback_ctrl(SSL_CTX *ctx, int cmd, void (*fp) (void))
{
    switch (cmd) {
    case SSL_CTRL_SET_MSG_CALLBACK:
        ctx->msg_callback = (void (*)
                             (int write_p, int version, int content_type,
                              const void *buf, size_t len, SSL *ssl,
                              void *arg))(fp);
        return 1;

    default:
        return (ctx->method->ssl_ctx_callback_ctrl(ctx, cmd, fp));
    }
}

int ssl_cipher_id_cmp(const SSL_CIPHER *a, const SSL_CIPHER *b)
{
    if (a->id > b->id)
        return 1;
    if (a->id < b->id)
        return -1;
    return 0;
}

int ssl_cipher_ptr_id_cmp(const SSL_CIPHER *const *ap,
                          const SSL_CIPHER *const *bp)
{
    if ((*ap)->id > (*bp)->id)
        return 1;
    if ((*ap)->id < (*bp)->id)
        return -1;
    return 0;
}

/** return a STACK of the ciphers available for the SSL and in order of
 * preference */
STACK_OF(SSL_CIPHER) *SSL_get_ciphers(const SSL *s)
{
    if (s != NULL) {
        if (s->cipher_list != NULL) {
            return (s->cipher_list);
        } else if ((s->ctx != NULL) && (s->ctx->cipher_list != NULL)) {
            return (s->ctx->cipher_list);
        }
    }
    return (NULL);
}

STACK_OF(SSL_CIPHER) *SSL_get_client_ciphers(const SSL *s)
{
    if ((s == NULL) || (s->session == NULL) || !s->server)
        return NULL;
    return s->session->ciphers;
}

STACK_OF(SSL_CIPHER) *SSL_get1_supported_ciphers(SSL *s)
{
    STACK_OF(SSL_CIPHER) *sk = NULL, *ciphers;
    int i;
    ciphers = SSL_get_ciphers(s);
    if (!ciphers)
        return NULL;
    ssl_set_client_disabled(s);
    for (i = 0; i < sk_SSL_CIPHER_num(ciphers); i++) {
        const SSL_CIPHER *c = sk_SSL_CIPHER_value(ciphers, i);
        if (!ssl_cipher_disabled(s, c, SSL_SECOP_CIPHER_SUPPORTED)) {
            if (!sk)
                sk = sk_SSL_CIPHER_new_null();
            if (!sk)
                return NULL;
            if (!sk_SSL_CIPHER_push(sk, c)) {
                sk_SSL_CIPHER_free(sk);
                return NULL;
            }
        }
    }
    return sk;
}

/** return a STACK of the ciphers available for the SSL and in order of
 * algorithm id */
STACK_OF(SSL_CIPHER) *ssl_get_ciphers_by_id(SSL *s)
{
    if (s != NULL) {
        if (s->cipher_list_by_id != NULL) {
            return (s->cipher_list_by_id);
        } else if ((s->ctx != NULL) && (s->ctx->cipher_list_by_id != NULL)) {
            return (s->ctx->cipher_list_by_id);
        }
    }
    return (NULL);
}

/** The old interface to get the same thing as SSL_get_ciphers() */
const char *SSL_get_cipher_list(const SSL *s, int n)
{
    const SSL_CIPHER *c;
    STACK_OF(SSL_CIPHER) *sk;

    if (s == NULL)
        return (NULL);
    sk = SSL_get_ciphers(s);
    if ((sk == NULL) || (sk_SSL_CIPHER_num(sk) <= n))
        return (NULL);
    c = sk_SSL_CIPHER_value(sk, n);
    if (c == NULL)
        return (NULL);
    return (c->name);
}

/** specify the ciphers to be used by default by the SSL_CTX */
int SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str)
{
    STACK_OF(SSL_CIPHER) *sk;

    sk = ssl_create_cipher_list(ctx->method, &ctx->cipher_list,
                                &ctx->cipher_list_by_id, str, ctx->cert);
    /*
     * ssl_create_cipher_list may return an empty stack if it was unable to
     * find a cipher matching the given rule string (for example if the rule
     * string specifies a cipher which has been disabled). This is not an
     * error as far as ssl_create_cipher_list is concerned, and hence
     * ctx->cipher_list and ctx->cipher_list_by_id has been updated.
     */
    if (sk == NULL)
        return 0;
    else if (sk_SSL_CIPHER_num(sk) == 0) {
        SSLerr(SSL_F_SSL_CTX_SET_CIPHER_LIST, SSL_R_NO_CIPHER_MATCH);
        return 0;
    }
    return 1;
}

/** specify the ciphers to be used by the SSL */
int SSL_set_cipher_list(SSL *s, const char *str)
{
    STACK_OF(SSL_CIPHER) *sk;

    sk = ssl_create_cipher_list(s->ctx->method, &s->cipher_list,
                                &s->cipher_list_by_id, str, s->cert);
    /* see comment in SSL_CTX_set_cipher_list */
    if (sk == NULL)
        return 0;
    else if (sk_SSL_CIPHER_num(sk) == 0) {
        SSLerr(SSL_F_SSL_SET_CIPHER_LIST, SSL_R_NO_CIPHER_MATCH);
        return 0;
    }
    return 1;
}

char *SSL_get_shared_ciphers(const SSL *s, char *buf, int len)
{
    char *p;
    STACK_OF(SSL_CIPHER) *sk;
    const SSL_CIPHER *c;
    int i;

    if ((s->session == NULL) || (s->session->ciphers == NULL) || (len < 2))
        return (NULL);

    p = buf;
    sk = s->session->ciphers;

    if (sk_SSL_CIPHER_num(sk) == 0)
        return NULL;

    for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
        int n;

        c = sk_SSL_CIPHER_value(sk, i);
        n = strlen(c->name);
        if (n + 1 > len) {
            if (p != buf)
                --p;
            *p = '\0';
            return buf;
        }
        memcpy(p, c->name, n + 1);
        p += n;
        *(p++) = ':';
        len -= n + 1;
    }
    p[-1] = '\0';
    return (buf);
}

/** return a servername extension value if provided in Client Hello, or NULL.
 * So far, only host_name types are defined (RFC 3546).
 */

const char *SSL_get_servername(const SSL *s, const int type)
{
    if (type != TLSEXT_NAMETYPE_host_name)
        return NULL;

    return s->session && !s->tlsext_hostname ?
        s->session->tlsext_hostname : s->tlsext_hostname;
}

int SSL_get_servername_type(const SSL *s)
{
    if (s->session
        && (!s->tlsext_hostname ? s->session->
            tlsext_hostname : s->tlsext_hostname))
        return TLSEXT_NAMETYPE_host_name;
    return -1;
}

/*
 * SSL_select_next_proto implements the standard protocol selection. It is
 * expected that this function is called from the callback set by
 * SSL_CTX_set_next_proto_select_cb. The protocol data is assumed to be a
 * vector of 8-bit, length prefixed byte strings. The length byte itself is
 * not included in the length. A byte string of length 0 is invalid. No byte
 * string may be truncated. The current, but experimental algorithm for
 * selecting the protocol is: 1) If the server doesn't support NPN then this
 * is indicated to the callback. In this case, the client application has to
 * abort the connection or have a default application level protocol. 2) If
 * the server supports NPN, but advertises an empty list then the client
 * selects the first protcol in its list, but indicates via the API that this
 * fallback case was enacted. 3) Otherwise, the client finds the first
 * protocol in the server's list that it supports and selects this protocol.
 * This is because it's assumed that the server has better information about
 * which protocol a client should use. 4) If the client doesn't support any
 * of the server's advertised protocols, then this is treated the same as
 * case 2. It returns either OPENSSL_NPN_NEGOTIATED if a common protocol was
 * found, or OPENSSL_NPN_NO_OVERLAP if the fallback case was reached.
 */
int SSL_select_next_proto(unsigned char **out, unsigned char *outlen,
                          const unsigned char *server,
                          unsigned int server_len,
                          const unsigned char *client,
                          unsigned int client_len)
{
    unsigned int i, j;
    const unsigned char *result;
    int status = OPENSSL_NPN_UNSUPPORTED;

    /*
     * For each protocol in server preference order, see if we support it.
     */
    for (i = 0; i < server_len;) {
        for (j = 0; j < client_len;) {
            if (server[i] == client[j] &&
                memcmp(&server[i + 1], &client[j + 1], server[i]) == 0) {
                /* We found a match */
                result = &server[i];
                status = OPENSSL_NPN_NEGOTIATED;
                goto found;
            }
            j += client[j];
            j++;
        }
        i += server[i];
        i++;
    }

    /* There's no overlap between our protocols and the server's list. */
    result = client;
    status = OPENSSL_NPN_NO_OVERLAP;

 found:
    *out = (unsigned char *)result + 1;
    *outlen = result[0];
    return status;
}

#ifndef OPENSSL_NO_NEXTPROTONEG
/*
 * SSL_get0_next_proto_negotiated sets *data and *len to point to the
 * client's requested protocol for this connection and returns 0. If the
 * client didn't request any protocol, then *data is set to NULL. Note that
 * the client can request any protocol it chooses. The value returned from
 * this function need not be a member of the list of supported protocols
 * provided by the callback.
 */
void SSL_get0_next_proto_negotiated(const SSL *s, const unsigned char **data,
                                    unsigned *len)
{
    *data = s->next_proto_negotiated;
    if (!*data) {
        *len = 0;
    } else {
        *len = s->next_proto_negotiated_len;
    }
}

/*
 * SSL_CTX_set_next_protos_advertised_cb sets a callback that is called when
 * a TLS server needs a list of supported protocols for Next Protocol
 * Negotiation. The returned list must be in wire format.  The list is
 * returned by setting |out| to point to it and |outlen| to its length. This
 * memory will not be modified, but one should assume that the SSL* keeps a
 * reference to it. The callback should return SSL_TLSEXT_ERR_OK if it
 * wishes to advertise. Otherwise, no such extension will be included in the
 * ServerHello.
 */
void SSL_CTX_set_next_protos_advertised_cb(SSL_CTX *ctx,
                                           int (*cb) (SSL *ssl,
                                                      const unsigned char
                                                      **out,
                                                      unsigned int *outlen,
                                                      void *arg), void *arg)
{
    ctx->next_protos_advertised_cb = cb;
    ctx->next_protos_advertised_cb_arg = arg;
}

/*
 * SSL_CTX_set_next_proto_select_cb sets a callback that is called when a
 * client needs to select a protocol from the server's provided list. |out|
 * must be set to point to the selected protocol (which may be within |in|).
 * The length of the protocol name must be written into |outlen|. The
 * server's advertised protocols are provided in |in| and |inlen|. The
 * callback can assume that |in| is syntactically valid. The client must
 * select a protocol. It is fatal to the connection if this callback returns
 * a value other than SSL_TLSEXT_ERR_OK.
 */
void SSL_CTX_set_next_proto_select_cb(SSL_CTX *ctx,
                                      int (*cb) (SSL *s, unsigned char **out,
                                                 unsigned char *outlen,
                                                 const unsigned char *in,
                                                 unsigned int inlen,
                                                 void *arg), void *arg)
{
    ctx->next_proto_select_cb = cb;
    ctx->next_proto_select_cb_arg = arg;
}
#endif

/*
 * SSL_CTX_set_alpn_protos sets the ALPN protocol list on |ctx| to |protos|.
 * |protos| must be in wire-format (i.e. a series of non-empty, 8-bit
 * length-prefixed strings). Returns 0 on success.
 */
int SSL_CTX_set_alpn_protos(SSL_CTX *ctx, const unsigned char *protos,
                            unsigned protos_len)
{
    OPENSSL_free(ctx->alpn_client_proto_list);
    ctx->alpn_client_proto_list = OPENSSL_malloc(protos_len);
    if (ctx->alpn_client_proto_list == NULL)
        return 1;
    memcpy(ctx->alpn_client_proto_list, protos, protos_len);
    ctx->alpn_client_proto_list_len = protos_len;

    return 0;
}

/*
 * SSL_set_alpn_protos sets the ALPN protocol list on |ssl| to |protos|.
 * |protos| must be in wire-format (i.e. a series of non-empty, 8-bit
 * length-prefixed strings). Returns 0 on success.
 */
int SSL_set_alpn_protos(SSL *ssl, const unsigned char *protos,
                        unsigned protos_len)
{
    OPENSSL_free(ssl->alpn_client_proto_list);
    ssl->alpn_client_proto_list = OPENSSL_malloc(protos_len);
    if (ssl->alpn_client_proto_list == NULL)
        return 1;
    memcpy(ssl->alpn_client_proto_list, protos, protos_len);
    ssl->alpn_client_proto_list_len = protos_len;

    return 0;
}

/*
 * SSL_CTX_set_alpn_select_cb sets a callback function on |ctx| that is
 * called during ClientHello processing in order to select an ALPN protocol
 * from the client's list of offered protocols.
 */
void SSL_CTX_set_alpn_select_cb(SSL_CTX *ctx,
                                int (*cb) (SSL *ssl,
                                           const unsigned char **out,
                                           unsigned char *outlen,
                                           const unsigned char *in,
                                           unsigned int inlen,
                                           void *arg), void *arg)
{
    ctx->alpn_select_cb = cb;
    ctx->alpn_select_cb_arg = arg;
}

/*
 * SSL_get0_alpn_selected gets the selected ALPN protocol (if any) from
 * |ssl|. On return it sets |*data| to point to |*len| bytes of protocol name
 * (not including the leading length-prefix byte). If the server didn't
 * respond with a negotiated protocol then |*len| will be zero.
 */
void SSL_get0_alpn_selected(const SSL *ssl, const unsigned char **data,
                            unsigned *len)
{
    *data = NULL;
    if (ssl->s3)
        *data = ssl->s3->alpn_selected;
    if (*data == NULL)
        *len = 0;
    else
        *len = ssl->s3->alpn_selected_len;
}


int SSL_export_keying_material(SSL *s, unsigned char *out, size_t olen,
                               const char *label, size_t llen,
                               const unsigned char *p, size_t plen,
                               int use_context)
{
    if (s->version < TLS1_VERSION)
        return -1;

    return s->method->ssl3_enc->export_keying_material(s, out, olen, label,
                                                       llen, p, plen,
                                                       use_context);
}

static unsigned long ssl_session_hash(const SSL_SESSION *a)
{
    unsigned long l;

    l = (unsigned long)
        ((unsigned int)a->session_id[0]) |
        ((unsigned int)a->session_id[1] << 8L) |
        ((unsigned long)a->session_id[2] << 16L) |
        ((unsigned long)a->session_id[3] << 24L);
    return (l);
}

/*
 * NB: If this function (or indeed the hash function which uses a sort of
 * coarser function than this one) is changed, ensure
 * SSL_CTX_has_matching_session_id() is checked accordingly. It relies on
 * being able to construct an SSL_SESSION that will collide with any existing
 * session with a matching session ID.
 */
static int ssl_session_cmp(const SSL_SESSION *a, const SSL_SESSION *b)
{
    if (a->ssl_version != b->ssl_version)
        return (1);
    if (a->session_id_length != b->session_id_length)
        return (1);
    return (memcmp(a->session_id, b->session_id, a->session_id_length));
}

/*
 * These wrapper functions should remain rather than redeclaring
 * SSL_SESSION_hash and SSL_SESSION_cmp for void* types and casting each
 * variable. The reason is that the functions aren't static, they're exposed
 * via ssl.h.
 */

SSL_CTX *SSL_CTX_new(const SSL_METHOD *meth)
{
    SSL_CTX *ret = NULL;

    if (meth == NULL) {
        SSLerr(SSL_F_SSL_CTX_NEW, SSL_R_NULL_SSL_METHOD_PASSED);
        return (NULL);
    }

    if (!OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL))
        return NULL;

    if (FIPS_mode() && (meth->version < TLS1_VERSION)) {
        SSLerr(SSL_F_SSL_CTX_NEW, SSL_R_AT_LEAST_TLS_1_0_NEEDED_IN_FIPS_MODE);
        return NULL;
    }

    if (SSL_get_ex_data_X509_STORE_CTX_idx() < 0) {
        SSLerr(SSL_F_SSL_CTX_NEW, SSL_R_X509_VERIFICATION_SETUP_PROBLEMS);
        goto err;
    }
    ret = OPENSSL_zalloc(sizeof(*ret));
    if (ret == NULL)
        goto err;

    ret->method = meth;
    ret->min_proto_version = 0;
    ret->max_proto_version = 0;
    ret->session_cache_mode = SSL_SESS_CACHE_SERVER;
    ret->session_cache_size = SSL_SESSION_CACHE_MAX_SIZE_DEFAULT;
    /* We take the system default. */
    ret->session_timeout = meth->get_timeout();
    ret->references = 1;
    ret->max_cert_list = SSL_MAX_CERT_LIST_DEFAULT;
    ret->verify_mode = SSL_VERIFY_NONE;
    if ((ret->cert = ssl_cert_new()) == NULL)
        goto err;

    ret->sessions = lh_SSL_SESSION_new(ssl_session_hash, ssl_session_cmp);
    if (ret->sessions == NULL)
        goto err;
    ret->cert_store = X509_STORE_new();
    if (ret->cert_store == NULL)
        goto err;
#ifndef OPENSSL_NO_CT
    ret->ctlog_store = CTLOG_STORE_new();
    if (ret->ctlog_store == NULL)
        goto err;
#endif
    if (!ssl_create_cipher_list(ret->method,
                           &ret->cipher_list, &ret->cipher_list_by_id,
                           SSL_DEFAULT_CIPHER_LIST, ret->cert)
       || sk_SSL_CIPHER_num(ret->cipher_list) <= 0) {
        SSLerr(SSL_F_SSL_CTX_NEW, SSL_R_LIBRARY_HAS_NO_CIPHERS);
        goto err2;
    }

    ret->param = X509_VERIFY_PARAM_new();
    if (ret->param == NULL)
        goto err;

    if ((ret->md5 = EVP_get_digestbyname("ssl3-md5")) == NULL) {
        SSLerr(SSL_F_SSL_CTX_NEW, SSL_R_UNABLE_TO_LOAD_SSL3_MD5_ROUTINES);
        goto err2;
    }
    if ((ret->sha1 = EVP_get_digestbyname("ssl3-sha1")) == NULL) {
        SSLerr(SSL_F_SSL_CTX_NEW, SSL_R_UNABLE_TO_LOAD_SSL3_SHA1_ROUTINES);
        goto err2;
    }

    if ((ret->client_CA = sk_X509_NAME_new_null()) == NULL)
        goto err;

    CRYPTO_new_ex_data(CRYPTO_EX_INDEX_SSL_CTX, ret, &ret->ex_data);

    /* No compression for DTLS */
    if (!(meth->ssl3_enc->enc_flags & SSL_ENC_FLAG_DTLS))
        ret->comp_methods = SSL_COMP_get_compression_methods();

    ret->max_send_fragment = SSL3_RT_MAX_PLAIN_LENGTH;

    /* Setup RFC4507 ticket keys */
    if ((RAND_bytes(ret->tlsext_tick_key_name, 16) <= 0)
        || (RAND_bytes(ret->tlsext_tick_hmac_key, 16) <= 0)
        || (RAND_bytes(ret->tlsext_tick_aes_key, 16) <= 0))
        ret->options |= SSL_OP_NO_TICKET;

#ifndef OPENSSL_NO_SRP
    if (!SSL_CTX_SRP_CTX_init(ret))
        goto err;
#endif
#ifndef OPENSSL_NO_ENGINE
# ifdef OPENSSL_SSL_CLIENT_ENGINE_AUTO
#  define eng_strx(x)     #x
#  define eng_str(x)      eng_strx(x)
    /* Use specific client engine automatically... ignore errors */
    {
        ENGINE *eng;
        eng = ENGINE_by_id(eng_str(OPENSSL_SSL_CLIENT_ENGINE_AUTO));
        if (!eng) {
            ERR_clear_error();
            ENGINE_load_builtin_engines();
            eng = ENGINE_by_id(eng_str(OPENSSL_SSL_CLIENT_ENGINE_AUTO));
        }
        if (!eng || !SSL_CTX_set_client_cert_engine(ret, eng))
            ERR_clear_error();
    }
# endif
#endif
    /*
     * Default is to connect to non-RI servers. When RI is more widely
     * deployed might change this.
     */
    ret->options |= SSL_OP_LEGACY_SERVER_CONNECT;
    /*
     * Disable compression by default to prevent CRIME. Applications can
     * re-enable compression by configuring
     * SSL_CTX_clear_options(ctx, SSL_OP_NO_COMPRESSION);
     * or by using the SSL_CONF library.
     */
    ret->options |= SSL_OP_NO_COMPRESSION;

    return (ret);
 err:
    SSLerr(SSL_F_SSL_CTX_NEW, ERR_R_MALLOC_FAILURE);
 err2:
    SSL_CTX_free(ret);
    return (NULL);
}

void SSL_CTX_up_ref(SSL_CTX *ctx)
{
    CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
}

void SSL_CTX_free(SSL_CTX *a)
{
    int i;

    if (a == NULL)
        return;

    i = CRYPTO_add(&a->references, -1, CRYPTO_LOCK_SSL_CTX);
    REF_PRINT_COUNT("SSL_CTX", a);
    if (i > 0)
        return;
    REF_ASSERT_ISNT(i < 0);

    X509_VERIFY_PARAM_free(a->param);
    dane_ctx_final(&a->dane);

    /*
     * Free internal session cache. However: the remove_cb() may reference
     * the ex_data of SSL_CTX, thus the ex_data store can only be removed
     * after the sessions were flushed.
     * As the ex_data handling routines might also touch the session cache,
     * the most secure solution seems to be: empty (flush) the cache, then
     * free ex_data, then finally free the cache.
     * (See ticket [openssl.org #212].)
     */
    if (a->sessions != NULL)
        SSL_CTX_flush_sessions(a, 0);

    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_SSL_CTX, a, &a->ex_data);
    lh_SSL_SESSION_free(a->sessions);
    X509_STORE_free(a->cert_store);
#ifndef OPENSSL_NO_CT
    OPENSSL_free(a->ct_validation_callback_arg);
    CTLOG_STORE_free(a->ctlog_store);
#endif
    sk_SSL_CIPHER_free(a->cipher_list);
    sk_SSL_CIPHER_free(a->cipher_list_by_id);
    ssl_cert_free(a->cert);
    sk_X509_NAME_pop_free(a->client_CA, X509_NAME_free);
    sk_X509_pop_free(a->extra_certs, X509_free);
    a->comp_methods = NULL;
#ifndef OPENSSL_NO_SRTP
    sk_SRTP_PROTECTION_PROFILE_free(a->srtp_profiles);
#endif
#ifndef OPENSSL_NO_SRP
    SSL_CTX_SRP_CTX_free(a);
#endif
#ifndef OPENSSL_NO_ENGINE
    if (a->client_cert_engine)
        ENGINE_finish(a->client_cert_engine);
#endif

#ifndef OPENSSL_NO_EC
    OPENSSL_free(a->tlsext_ecpointformatlist);
    OPENSSL_free(a->tlsext_ellipticcurvelist);
#endif
    OPENSSL_free(a->alpn_client_proto_list);

    OPENSSL_free(a);
}

void SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx, pem_password_cb *cb)
{
    ctx->default_passwd_callback = cb;
}

void SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX *ctx, void *u)
{
    ctx->default_passwd_callback_userdata = u;
}

void SSL_set_default_passwd_cb(SSL *s, pem_password_cb *cb)
{
    s->default_passwd_callback = cb;
}

void SSL_set_default_passwd_cb_userdata(SSL *s, void *u)
{
    s->default_passwd_callback_userdata = u;
}

void SSL_CTX_set_cert_verify_callback(SSL_CTX *ctx,
                                      int (*cb) (X509_STORE_CTX *, void *),
                                      void *arg)
{
    ctx->app_verify_callback = cb;
    ctx->app_verify_arg = arg;
}

void SSL_CTX_set_verify(SSL_CTX *ctx, int mode,
                        int (*cb) (int, X509_STORE_CTX *))
{
    ctx->verify_mode = mode;
    ctx->default_verify_callback = cb;
}

void SSL_CTX_set_verify_depth(SSL_CTX *ctx, int depth)
{
    X509_VERIFY_PARAM_set_depth(ctx->param, depth);
}

void SSL_CTX_set_cert_cb(SSL_CTX *c, int (*cb) (SSL *ssl, void *arg),
                         void *arg)
{
    ssl_cert_set_cert_cb(c->cert, cb, arg);
}

void SSL_set_cert_cb(SSL *s, int (*cb) (SSL *ssl, void *arg), void *arg)
{
    ssl_cert_set_cert_cb(s->cert, cb, arg);
}

void ssl_set_masks(SSL *s, const SSL_CIPHER *cipher)
{
#if !defined(OPENSSL_NO_EC) || !defined(OPENSSL_NO_GOST)
    CERT_PKEY *cpk;
#endif
    CERT *c = s->cert;
    uint32_t *pvalid = s->s3->tmp.valid_flags;
    int rsa_enc, rsa_sign, dh_tmp, dsa_sign;
    unsigned long mask_k, mask_a;
#ifndef OPENSSL_NO_EC
    int have_ecc_cert, ecdsa_ok;
    X509 *x = NULL;
    int pk_nid = 0, md_nid = 0;
#endif
    if (c == NULL)
        return;

#ifndef OPENSSL_NO_DH
    dh_tmp = (c->dh_tmp != NULL || c->dh_tmp_cb != NULL || c->dh_tmp_auto);
#else
    dh_tmp = 0;
#endif

    rsa_enc = pvalid[SSL_PKEY_RSA_ENC] & CERT_PKEY_VALID;
    rsa_sign = pvalid[SSL_PKEY_RSA_SIGN] & CERT_PKEY_SIGN;
    dsa_sign = pvalid[SSL_PKEY_DSA_SIGN] & CERT_PKEY_SIGN;
#ifndef OPENSSL_NO_EC
    have_ecc_cert = pvalid[SSL_PKEY_ECC] & CERT_PKEY_VALID;
#endif
    mask_k = 0;
    mask_a = 0;

#ifdef CIPHER_DEBUG
    fprintf(stderr, "dht=%d re=%d rs=%d ds=%d\n",
            dh_tmp, rsa_enc, rsa_sign, dsa_sign);
#endif

#ifndef OPENSSL_NO_GOST
    cpk = &(c->pkeys[SSL_PKEY_GOST12_512]);
    if (cpk->x509 != NULL && cpk->privatekey != NULL) {
        mask_k |= SSL_kGOST;
        mask_a |= SSL_aGOST12;
    }
    cpk = &(c->pkeys[SSL_PKEY_GOST12_256]);
    if (cpk->x509 != NULL && cpk->privatekey != NULL) {
        mask_k |= SSL_kGOST;
        mask_a |= SSL_aGOST12;
    }
    cpk = &(c->pkeys[SSL_PKEY_GOST01]);
    if (cpk->x509 != NULL && cpk->privatekey != NULL) {
        mask_k |= SSL_kGOST;
        mask_a |= SSL_aGOST01;
    }
#endif

    if (rsa_enc)
        mask_k |= SSL_kRSA;

    if (dh_tmp)
        mask_k |= SSL_kDHE;

    if (rsa_enc || rsa_sign) {
        mask_a |= SSL_aRSA;
    }

    if (dsa_sign) {
        mask_a |= SSL_aDSS;
    }

    mask_a |= SSL_aNULL;

    /*
     * An ECC certificate may be usable for ECDH and/or ECDSA cipher suites
     * depending on the key usage extension.
     */
#ifndef OPENSSL_NO_EC
    if (have_ecc_cert) {
        uint32_t ex_kusage;
        cpk = &c->pkeys[SSL_PKEY_ECC];
        x = cpk->x509;
        ex_kusage = X509_get_key_usage(x);
        ecdsa_ok = ex_kusage & X509v3_KU_DIGITAL_SIGNATURE;
        if (!(pvalid[SSL_PKEY_ECC] & CERT_PKEY_SIGN))
            ecdsa_ok = 0;
        OBJ_find_sigid_algs(X509_get_signature_nid(x), &md_nid, &pk_nid);
        if (ecdsa_ok) {
            mask_a |= SSL_aECDSA;
        }
    }
#endif

#ifndef OPENSSL_NO_EC
    mask_k |= SSL_kECDHE;
#endif

#ifndef OPENSSL_NO_PSK
    mask_k |= SSL_kPSK;
    mask_a |= SSL_aPSK;
    if (mask_k & SSL_kRSA)
        mask_k |= SSL_kRSAPSK;
    if (mask_k & SSL_kDHE)
        mask_k |= SSL_kDHEPSK;
    if (mask_k & SSL_kECDHE)
        mask_k |= SSL_kECDHEPSK;
#endif

    s->s3->tmp.mask_k = mask_k;
    s->s3->tmp.mask_a = mask_a;
}

#ifndef OPENSSL_NO_EC

int ssl_check_srvr_ecc_cert_and_alg(X509 *x, SSL *s)
{
    if (s->s3->tmp.new_cipher->algorithm_auth & SSL_aECDSA) {
        /* key usage, if present, must allow signing */
        if (!(X509_get_key_usage(x) & X509v3_KU_DIGITAL_SIGNATURE)) {
            SSLerr(SSL_F_SSL_CHECK_SRVR_ECC_CERT_AND_ALG,
                   SSL_R_ECC_CERT_NOT_FOR_SIGNING);
            return 0;
        }
    }
    return 1;                   /* all checks are ok */
}

#endif

static int ssl_get_server_cert_index(const SSL *s)
{
    int idx;
    idx = ssl_cipher_get_cert_index(s->s3->tmp.new_cipher);
    if (idx == SSL_PKEY_RSA_ENC && !s->cert->pkeys[SSL_PKEY_RSA_ENC].x509)
        idx = SSL_PKEY_RSA_SIGN;
    if (idx == SSL_PKEY_GOST_EC) {
        if (s->cert->pkeys[SSL_PKEY_GOST12_512].x509)
            idx = SSL_PKEY_GOST12_512;
        else if (s->cert->pkeys[SSL_PKEY_GOST12_256].x509)
            idx = SSL_PKEY_GOST12_256;
        else if (s->cert->pkeys[SSL_PKEY_GOST01].x509)
            idx = SSL_PKEY_GOST01;
        else
            idx = -1;
    }
    if (idx == -1)
        SSLerr(SSL_F_SSL_GET_SERVER_CERT_INDEX, ERR_R_INTERNAL_ERROR);
    return idx;
}

CERT_PKEY *ssl_get_server_send_pkey(SSL *s)
{
    CERT *c;
    int i;

    c = s->cert;
    if (!s->s3 || !s->s3->tmp.new_cipher)
        return NULL;
    ssl_set_masks(s, s->s3->tmp.new_cipher);

#ifdef OPENSSL_SSL_DEBUG_BROKEN_PROTOCOL
    /*
     * Broken protocol test: return last used certificate: which may mismatch
     * the one expected.
     */
    if (c->cert_flags & SSL_CERT_FLAG_BROKEN_PROTOCOL)
        return c->key;
#endif

    i = ssl_get_server_cert_index(s);

    /* This may or may not be an error. */
    if (i < 0)
        return NULL;

    /* May be NULL. */
    return &c->pkeys[i];
}

EVP_PKEY *ssl_get_sign_pkey(SSL *s, const SSL_CIPHER *cipher,
                            const EVP_MD **pmd)
{
    unsigned long alg_a;
    CERT *c;
    int idx = -1;

    alg_a = cipher->algorithm_auth;
    c = s->cert;

#ifdef OPENSSL_SSL_DEBUG_BROKEN_PROTOCOL
    /*
     * Broken protocol test: use last key: which may mismatch the one
     * expected.
     */
    if (c->cert_flags & SSL_CERT_FLAG_BROKEN_PROTOCOL)
        idx = c->key - c->pkeys;
    else
#endif

    if ((alg_a & SSL_aDSS) &&
            (c->pkeys[SSL_PKEY_DSA_SIGN].privatekey != NULL))
        idx = SSL_PKEY_DSA_SIGN;
    else if (alg_a & SSL_aRSA) {
        if (c->pkeys[SSL_PKEY_RSA_SIGN].privatekey != NULL)
            idx = SSL_PKEY_RSA_SIGN;
        else if (c->pkeys[SSL_PKEY_RSA_ENC].privatekey != NULL)
            idx = SSL_PKEY_RSA_ENC;
    } else if ((alg_a & SSL_aECDSA) &&
               (c->pkeys[SSL_PKEY_ECC].privatekey != NULL))
        idx = SSL_PKEY_ECC;
    if (idx == -1) {
        SSLerr(SSL_F_SSL_GET_SIGN_PKEY, ERR_R_INTERNAL_ERROR);
        return (NULL);
    }
    if (pmd)
        *pmd = s->s3->tmp.md[idx];
    return c->pkeys[idx].privatekey;
}

int ssl_get_server_cert_serverinfo(SSL *s, const unsigned char **serverinfo,
                                   size_t *serverinfo_length)
{
    CERT *c = NULL;
    int i = 0;
    *serverinfo_length = 0;

    c = s->cert;
    i = ssl_get_server_cert_index(s);

    if (i == -1)
        return 0;
    if (c->pkeys[i].serverinfo == NULL)
        return 0;

    *serverinfo = c->pkeys[i].serverinfo;
    *serverinfo_length = c->pkeys[i].serverinfo_length;
    return 1;
}

void ssl_update_cache(SSL *s, int mode)
{
    int i;

    /*
     * If the session_id_length is 0, we are not supposed to cache it, and it
     * would be rather hard to do anyway :-)
     */
    if (s->session->session_id_length == 0)
        return;

    i = s->session_ctx->session_cache_mode;
    if ((i & mode) && (!s->hit)
        && ((i & SSL_SESS_CACHE_NO_INTERNAL_STORE)
            || SSL_CTX_add_session(s->session_ctx, s->session))
        && (s->session_ctx->new_session_cb != NULL)) {
        CRYPTO_add(&s->session->references, 1, CRYPTO_LOCK_SSL_SESSION);
        if (!s->session_ctx->new_session_cb(s, s->session))
            SSL_SESSION_free(s->session);
    }

    /* auto flush every 255 connections */
    if ((!(i & SSL_SESS_CACHE_NO_AUTO_CLEAR)) && ((i & mode) == mode)) {
        if ((((mode & SSL_SESS_CACHE_CLIENT)
              ? s->session_ctx->stats.sess_connect_good
              : s->session_ctx->stats.sess_accept_good) & 0xff) == 0xff) {
            SSL_CTX_flush_sessions(s->session_ctx, (unsigned long)time(NULL));
        }
    }
}

const SSL_METHOD *SSL_CTX_get_ssl_method(SSL_CTX *ctx)
{
    return ctx->method;
}

const SSL_METHOD *SSL_get_ssl_method(SSL *s)
{
    return (s->method);
}

int SSL_set_ssl_method(SSL *s, const SSL_METHOD *meth)
{
    int ret = 1;

    if (s->method != meth) {
        const SSL_METHOD *sm = s->method;
        int (*hf)(SSL *) = s->handshake_func;

        if (sm->version == meth->version)
            s->method = meth;
        else {
            sm->ssl_free(s);
            s->method = meth;
            ret = s->method->ssl_new(s);
        }

        if (hf == sm->ssl_connect)
            s->handshake_func = meth->ssl_connect;
        else if (hf == sm->ssl_accept)
            s->handshake_func = meth->ssl_accept;
    }
    return (ret);
}

int SSL_get_error(const SSL *s, int i)
{
    int reason;
    unsigned long l;
    BIO *bio;

    if (i > 0)
        return (SSL_ERROR_NONE);

    /*
     * Make things return SSL_ERROR_SYSCALL when doing SSL_do_handshake etc,
     * where we do encode the error
     */
    if ((l = ERR_peek_error()) != 0) {
        if (ERR_GET_LIB(l) == ERR_LIB_SYS)
            return (SSL_ERROR_SYSCALL);
        else
            return (SSL_ERROR_SSL);
    }

    if ((i < 0) && SSL_want_read(s)) {
        bio = SSL_get_rbio(s);
        if (BIO_should_read(bio))
            return (SSL_ERROR_WANT_READ);
        else if (BIO_should_write(bio))
            /*
             * This one doesn't make too much sense ... We never try to write
             * to the rbio, and an application program where rbio and wbio
             * are separate couldn't even know what it should wait for.
             * However if we ever set s->rwstate incorrectly (so that we have
             * SSL_want_read(s) instead of SSL_want_write(s)) and rbio and
             * wbio *are* the same, this test works around that bug; so it
             * might be safer to keep it.
             */
            return (SSL_ERROR_WANT_WRITE);
        else if (BIO_should_io_special(bio)) {
            reason = BIO_get_retry_reason(bio);
            if (reason == BIO_RR_CONNECT)
                return (SSL_ERROR_WANT_CONNECT);
            else if (reason == BIO_RR_ACCEPT)
                return (SSL_ERROR_WANT_ACCEPT);
            else
                return (SSL_ERROR_SYSCALL); /* unknown */
        }
    }

    if ((i < 0) && SSL_want_write(s)) {
        bio = SSL_get_wbio(s);
        if (BIO_should_write(bio))
            return (SSL_ERROR_WANT_WRITE);
        else if (BIO_should_read(bio))
            /*
             * See above (SSL_want_read(s) with BIO_should_write(bio))
             */
            return (SSL_ERROR_WANT_READ);
        else if (BIO_should_io_special(bio)) {
            reason = BIO_get_retry_reason(bio);
            if (reason == BIO_RR_CONNECT)
                return (SSL_ERROR_WANT_CONNECT);
            else if (reason == BIO_RR_ACCEPT)
                return (SSL_ERROR_WANT_ACCEPT);
            else
                return (SSL_ERROR_SYSCALL);
        }
    }
    if ((i < 0) && SSL_want_x509_lookup(s)) {
        return (SSL_ERROR_WANT_X509_LOOKUP);
    }
    if ((i < 0) && SSL_want_async(s)) {
        return SSL_ERROR_WANT_ASYNC;
    }

    if (i == 0) {
        if ((s->shutdown & SSL_RECEIVED_SHUTDOWN) &&
            (s->s3->warn_alert == SSL_AD_CLOSE_NOTIFY))
            return (SSL_ERROR_ZERO_RETURN);
    }
    return (SSL_ERROR_SYSCALL);
}

static int ssl_do_handshake_intern(void *vargs)
{
    struct ssl_async_args *args;
    SSL *s;

    args = (struct ssl_async_args *)vargs;
    s = args->s;

    return s->handshake_func(s);
}

int SSL_do_handshake(SSL *s)
{
    int ret = 1;

    if (s->handshake_func == NULL) {
        SSLerr(SSL_F_SSL_DO_HANDSHAKE, SSL_R_CONNECTION_TYPE_NOT_SET);
        return -1;
    }

    s->method->ssl_renegotiate_check(s);

    if (SSL_in_init(s) || SSL_in_before(s)) {
        if((s->mode & SSL_MODE_ASYNC) && ASYNC_get_current_job() == NULL) {
            struct ssl_async_args args;

            args.s = s;

            ret = ssl_start_async_job(s, &args, ssl_do_handshake_intern);
        } else {
            ret = s->handshake_func(s);
        }
    }
    return ret;
}

void SSL_set_accept_state(SSL *s)
{
    s->server = 1;
    s->shutdown = 0;
    ossl_statem_clear(s);
    s->handshake_func = s->method->ssl_accept;
    clear_ciphers(s);
}

void SSL_set_connect_state(SSL *s)
{
    s->server = 0;
    s->shutdown = 0;
    ossl_statem_clear(s);
    s->handshake_func = s->method->ssl_connect;
    clear_ciphers(s);
}

int ssl_undefined_function(SSL *s)
{
    SSLerr(SSL_F_SSL_UNDEFINED_FUNCTION, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
    return (0);
}

int ssl_undefined_void_function(void)
{
    SSLerr(SSL_F_SSL_UNDEFINED_VOID_FUNCTION,
           ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
    return (0);
}

int ssl_undefined_const_function(const SSL *s)
{
    return (0);
}

SSL_METHOD *ssl_bad_method(int ver)
{
    SSLerr(SSL_F_SSL_BAD_METHOD, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
    return (NULL);
}

const char *SSL_get_version(const SSL *s)
{
    if (s->version == TLS1_2_VERSION)
        return ("TLSv1.2");
    else if (s->version == TLS1_1_VERSION)
        return ("TLSv1.1");
    else if (s->version == TLS1_VERSION)
        return ("TLSv1");
    else if (s->version == SSL3_VERSION)
        return ("SSLv3");
    else if (s->version == DTLS1_BAD_VER)
        return ("DTLSv0.9");
    else if (s->version == DTLS1_VERSION)
        return ("DTLSv1");
    else if (s->version == DTLS1_2_VERSION)
        return ("DTLSv1.2");
    else
        return ("unknown");
}

SSL *SSL_dup(SSL *s)
{
    STACK_OF(X509_NAME) *sk;
    X509_NAME *xn;
    SSL *ret;
    int i;

    /* If we're not quiescent, just up_ref! */
    if (!SSL_in_init(s) || !SSL_in_before(s)) {
        CRYPTO_add(&s->references, 1, CRYPTO_LOCK_SSL);
        return s;
    }

    /*
     * Otherwise, copy configuration state, and session if set.
     */
    if ((ret = SSL_new(SSL_get_SSL_CTX(s))) == NULL)
        return (NULL);

    if (s->session != NULL) {
        /*
         * Arranges to share the same session via up_ref.  This "copies"
         * session-id, SSL_METHOD, sid_ctx, and 'cert'
         */
        if (!SSL_copy_session_id(ret, s))
            goto err;
    } else {
        /*
         * No session has been established yet, so we have to expect that
         * s->cert or ret->cert will be changed later -- they should not both
         * point to the same object, and thus we can't use
         * SSL_copy_session_id.
         */
        if (!SSL_set_ssl_method(ret, s->method))
            goto err;

        if (s->cert != NULL) {
            ssl_cert_free(ret->cert);
            ret->cert = ssl_cert_dup(s->cert);
            if (ret->cert == NULL)
                goto err;
        }

        if (!SSL_set_session_id_context(ret, s->sid_ctx, s->sid_ctx_length))
            goto err;
    }

    ssl_dane_dup(ret, s);
    ret->version = s->version;
    ret->options = s->options;
    ret->mode = s->mode;
    SSL_set_max_cert_list(ret, SSL_get_max_cert_list(s));
    SSL_set_read_ahead(ret, SSL_get_read_ahead(s));
    ret->msg_callback = s->msg_callback;
    ret->msg_callback_arg = s->msg_callback_arg;
    SSL_set_verify(ret, SSL_get_verify_mode(s), SSL_get_verify_callback(s));
    SSL_set_verify_depth(ret, SSL_get_verify_depth(s));
    ret->generate_session_id = s->generate_session_id;

    SSL_set_info_callback(ret, SSL_get_info_callback(s));

    /* copy app data, a little dangerous perhaps */
    if (!CRYPTO_dup_ex_data(CRYPTO_EX_INDEX_SSL, &ret->ex_data, &s->ex_data))
        goto err;

    /* setup rbio, and wbio */
    if (s->rbio != NULL) {
        if (!BIO_dup_state(s->rbio, (char *)&ret->rbio))
            goto err;
    }
    if (s->wbio != NULL) {
        if (s->wbio != s->rbio) {
            if (!BIO_dup_state(s->wbio, (char *)&ret->wbio))
                goto err;
        } else
            ret->wbio = ret->rbio;
    }

    ret->server = s->server;
    if (s->handshake_func) {
        if (s->server)
            SSL_set_accept_state(ret);
        else
            SSL_set_connect_state(ret);
    }
    ret->shutdown = s->shutdown;
    ret->hit = s->hit;

    ret->default_passwd_callback = s->default_passwd_callback;
    ret->default_passwd_callback_userdata = s->default_passwd_callback_userdata;

    X509_VERIFY_PARAM_inherit(ret->param, s->param);

    /* dup the cipher_list and cipher_list_by_id stacks */
    if (s->cipher_list != NULL) {
        if ((ret->cipher_list = sk_SSL_CIPHER_dup(s->cipher_list)) == NULL)
            goto err;
    }
    if (s->cipher_list_by_id != NULL)
        if ((ret->cipher_list_by_id = sk_SSL_CIPHER_dup(s->cipher_list_by_id))
            == NULL)
            goto err;

    /* Dup the client_CA list */
    if (s->client_CA != NULL) {
        if ((sk = sk_X509_NAME_dup(s->client_CA)) == NULL)
            goto err;
        ret->client_CA = sk;
        for (i = 0; i < sk_X509_NAME_num(sk); i++) {
            xn = sk_X509_NAME_value(sk, i);
            if (sk_X509_NAME_set(sk, i, X509_NAME_dup(xn)) == NULL) {
                X509_NAME_free(xn);
                goto err;
            }
        }
    }
    return ret;

 err:
    SSL_free(ret);
    return NULL;
}

void ssl_clear_cipher_ctx(SSL *s)
{
    if (s->enc_read_ctx != NULL) {
        EVP_CIPHER_CTX_free(s->enc_read_ctx);
        s->enc_read_ctx = NULL;
    }
    if (s->enc_write_ctx != NULL) {
        EVP_CIPHER_CTX_free(s->enc_write_ctx);
        s->enc_write_ctx = NULL;
    }
#ifndef OPENSSL_NO_COMP
    COMP_CTX_free(s->expand);
    s->expand = NULL;
    COMP_CTX_free(s->compress);
    s->compress = NULL;
#endif
}

X509 *SSL_get_certificate(const SSL *s)
{
    if (s->cert != NULL)
        return (s->cert->key->x509);
    else
        return (NULL);
}

EVP_PKEY *SSL_get_privatekey(const SSL *s)
{
    if (s->cert != NULL)
        return (s->cert->key->privatekey);
    else
        return (NULL);
}

X509 *SSL_CTX_get0_certificate(const SSL_CTX *ctx)
{
    if (ctx->cert != NULL)
        return ctx->cert->key->x509;
    else
        return NULL;
}

EVP_PKEY *SSL_CTX_get0_privatekey(const SSL_CTX *ctx)
{
    if (ctx->cert != NULL)
        return ctx->cert->key->privatekey;
    else
        return NULL;
}

const SSL_CIPHER *SSL_get_current_cipher(const SSL *s)
{
    if ((s->session != NULL) && (s->session->cipher != NULL))
        return (s->session->cipher);
    return (NULL);
}

const COMP_METHOD *SSL_get_current_compression(SSL *s)
{
#ifndef OPENSSL_NO_COMP
    return s->compress ? COMP_CTX_get_method(s->compress) : NULL;
#else
    return NULL;
#endif
}

const COMP_METHOD *SSL_get_current_expansion(SSL *s)
{
#ifndef OPENSSL_NO_COMP
    return s->expand ? COMP_CTX_get_method(s->expand) : NULL;
#else
    return NULL;
#endif
}

int ssl_init_wbio_buffer(SSL *s, int push)
{
    BIO *bbio;

    if (s->bbio == NULL) {
        bbio = BIO_new(BIO_f_buffer());
        if (bbio == NULL)
            return (0);
        s->bbio = bbio;
    } else {
        bbio = s->bbio;
        if (s->bbio == s->wbio)
            s->wbio = BIO_pop(s->wbio);
    }
    (void)BIO_reset(bbio);
/*      if (!BIO_set_write_buffer_size(bbio,16*1024)) */
    if (!BIO_set_read_buffer_size(bbio, 1)) {
        SSLerr(SSL_F_SSL_INIT_WBIO_BUFFER, ERR_R_BUF_LIB);
        return (0);
    }
    if (push) {
        if (s->wbio != bbio)
            s->wbio = BIO_push(bbio, s->wbio);
    } else {
        if (s->wbio == bbio)
            s->wbio = BIO_pop(bbio);
    }
    return (1);
}

void ssl_free_wbio_buffer(SSL *s)
{
    /* callers ensure s is never null */
    if (s->bbio == NULL)
        return;

    if (s->bbio == s->wbio) {
        /* remove buffering */
        s->wbio = BIO_pop(s->wbio);
#ifdef REF_DEBUG
        /*
         * not the usual REF_DEBUG, but this avoids
         * adding one more preprocessor symbol
         */
        assert(s->wbio != NULL);
#endif
    }
    BIO_free(s->bbio);
    s->bbio = NULL;
}

void SSL_CTX_set_quiet_shutdown(SSL_CTX *ctx, int mode)
{
    ctx->quiet_shutdown = mode;
}

int SSL_CTX_get_quiet_shutdown(const SSL_CTX *ctx)
{
    return (ctx->quiet_shutdown);
}

void SSL_set_quiet_shutdown(SSL *s, int mode)
{
    s->quiet_shutdown = mode;
}

int SSL_get_quiet_shutdown(const SSL *s)
{
    return (s->quiet_shutdown);
}

void SSL_set_shutdown(SSL *s, int mode)
{
    s->shutdown = mode;
}

int SSL_get_shutdown(const SSL *s)
{
    return (s->shutdown);
}

int SSL_version(const SSL *s)
{
    return (s->version);
}

SSL_CTX *SSL_get_SSL_CTX(const SSL *ssl)
{
    return (ssl->ctx);
}

SSL_CTX *SSL_set_SSL_CTX(SSL *ssl, SSL_CTX *ctx)
{
    CERT *new_cert;
    if (ssl->ctx == ctx)
        return ssl->ctx;
    if (ctx == NULL)
        ctx = ssl->initial_ctx;
    new_cert = ssl_cert_dup(ctx->cert);
    if (new_cert == NULL) {
        return NULL;
    }
    ssl_cert_free(ssl->cert);
    ssl->cert = new_cert;

    /*
     * Program invariant: |sid_ctx| has fixed size (SSL_MAX_SID_CTX_LENGTH),
     * so setter APIs must prevent invalid lengths from entering the system.
     */
    OPENSSL_assert(ssl->sid_ctx_length <= sizeof(ssl->sid_ctx));

    /*
     * If the session ID context matches that of the parent SSL_CTX,
     * inherit it from the new SSL_CTX as well. If however the context does
     * not match (i.e., it was set per-ssl with SSL_set_session_id_context),
     * leave it unchanged.
     */
    if ((ssl->ctx != NULL) &&
        (ssl->sid_ctx_length == ssl->ctx->sid_ctx_length) &&
        (memcmp(ssl->sid_ctx, ssl->ctx->sid_ctx, ssl->sid_ctx_length) == 0)) {
        ssl->sid_ctx_length = ctx->sid_ctx_length;
        memcpy(&ssl->sid_ctx, &ctx->sid_ctx, sizeof(ssl->sid_ctx));
    }

    CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
    SSL_CTX_free(ssl->ctx); /* decrement reference count */
    ssl->ctx = ctx;

    return (ssl->ctx);
}

int SSL_CTX_set_default_verify_paths(SSL_CTX *ctx)
{
    int ret = X509_STORE_set_default_paths(ctx->cert_store);
#ifndef OPENSSL_NO_CT
    if (ret == 1) {
        /*
         * Try loading the CT log list, but don't report an error if it fails
         * because we don't necessarily need it loaded (CT validation may be
         * disabled).
         */
        SSL_CTX_set_ct_verify_location_to_default(ctx);
        ERR_clear_error();
    }
#endif
    return ret;
}

int SSL_CTX_set_default_verify_dir(SSL_CTX *ctx)
{
    X509_LOOKUP *lookup;

    lookup = X509_STORE_add_lookup(ctx->cert_store, X509_LOOKUP_hash_dir());
    if (lookup == NULL)
        return 0;
    X509_LOOKUP_add_dir(lookup, NULL, X509_FILETYPE_DEFAULT);

    /* Clear any errors if the default directory does not exist */
    ERR_clear_error();

    return 1;
}

int SSL_CTX_set_default_verify_file(SSL_CTX *ctx)
{
    X509_LOOKUP *lookup;

    lookup = X509_STORE_add_lookup(ctx->cert_store, X509_LOOKUP_file());
    if (lookup == NULL)
        return 0;

    X509_LOOKUP_load_file(lookup, NULL, X509_FILETYPE_DEFAULT);

    /* Clear any errors if the default file does not exist */
    ERR_clear_error();

    return 1;
}

int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,
                                  const char *CApath)
{
    return (X509_STORE_load_locations(ctx->cert_store, CAfile, CApath));
}

void SSL_set_info_callback(SSL *ssl,
                           void (*cb) (const SSL *ssl, int type, int val))
{
    ssl->info_callback = cb;
}

/*
 * One compiler (Diab DCC) doesn't like argument names in returned function
 * pointer.
 */
void (*SSL_get_info_callback(const SSL *ssl)) (const SSL * /* ssl */ ,
                                               int /* type */ ,
                                               int /* val */ ) {
    return ssl->info_callback;
}

void SSL_set_verify_result(SSL *ssl, long arg)
{
    ssl->verify_result = arg;
}

long SSL_get_verify_result(const SSL *ssl)
{
    return (ssl->verify_result);
}

size_t SSL_get_client_random(const SSL *ssl, unsigned char *out, size_t outlen)
{
    if (outlen == 0)
        return sizeof(ssl->s3->client_random);
    if (outlen > sizeof(ssl->s3->client_random))
        outlen = sizeof(ssl->s3->client_random);
    memcpy(out, ssl->s3->client_random, outlen);
    return outlen;
}

size_t SSL_get_server_random(const SSL *ssl, unsigned char *out, size_t outlen)
{
    if (outlen == 0)
        return sizeof(ssl->s3->server_random);
    if (outlen > sizeof(ssl->s3->server_random))
        outlen = sizeof(ssl->s3->server_random);
    memcpy(out, ssl->s3->server_random, outlen);
    return outlen;
}

size_t SSL_SESSION_get_master_key(const SSL_SESSION *session,
                               unsigned char *out, size_t outlen)
{
    if (session->master_key_length < 0) {
        /* Should never happen */
        return 0;
    }
    if (outlen == 0)
        return session->master_key_length;
    if (outlen > (size_t)session->master_key_length)
        outlen = session->master_key_length;
    memcpy(out, session->master_key, outlen);
    return outlen;
}

int SSL_set_ex_data(SSL *s, int idx, void *arg)
{
    return (CRYPTO_set_ex_data(&s->ex_data, idx, arg));
}

void *SSL_get_ex_data(const SSL *s, int idx)
{
    return (CRYPTO_get_ex_data(&s->ex_data, idx));
}

int SSL_CTX_set_ex_data(SSL_CTX *s, int idx, void *arg)
{
    return (CRYPTO_set_ex_data(&s->ex_data, idx, arg));
}

void *SSL_CTX_get_ex_data(const SSL_CTX *s, int idx)
{
    return (CRYPTO_get_ex_data(&s->ex_data, idx));
}

int ssl_ok(SSL *s)
{
    return (1);
}

X509_STORE *SSL_CTX_get_cert_store(const SSL_CTX *ctx)
{
    return (ctx->cert_store);
}

void SSL_CTX_set_cert_store(SSL_CTX *ctx, X509_STORE *store)
{
    X509_STORE_free(ctx->cert_store);
    ctx->cert_store = store;
}

int SSL_want(const SSL *s)
{
    return (s->rwstate);
}

/**
 * \brief Set the callback for generating temporary DH keys.
 * \param ctx the SSL context.
 * \param dh the callback
 */

#ifndef OPENSSL_NO_DH
void SSL_CTX_set_tmp_dh_callback(SSL_CTX *ctx,
                                 DH *(*dh) (SSL *ssl, int is_export,
                                            int keylength))
{
    SSL_CTX_callback_ctrl(ctx, SSL_CTRL_SET_TMP_DH_CB, (void (*)(void))dh);
}

void SSL_set_tmp_dh_callback(SSL *ssl, DH *(*dh) (SSL *ssl, int is_export,
                                                  int keylength))
{
    SSL_callback_ctrl(ssl, SSL_CTRL_SET_TMP_DH_CB, (void (*)(void))dh);
}
#endif

#ifndef OPENSSL_NO_PSK
int SSL_CTX_use_psk_identity_hint(SSL_CTX *ctx, const char *identity_hint)
{
    if (identity_hint != NULL && strlen(identity_hint) > PSK_MAX_IDENTITY_LEN) {
        SSLerr(SSL_F_SSL_CTX_USE_PSK_IDENTITY_HINT,
               SSL_R_DATA_LENGTH_TOO_LONG);
        return 0;
    }
    OPENSSL_free(ctx->cert->psk_identity_hint);
    if (identity_hint != NULL) {
        ctx->cert->psk_identity_hint = OPENSSL_strdup(identity_hint);
        if (ctx->cert->psk_identity_hint == NULL)
            return 0;
    } else
        ctx->cert->psk_identity_hint = NULL;
    return 1;
}

int SSL_use_psk_identity_hint(SSL *s, const char *identity_hint)
{
    if (s == NULL)
        return 0;

    if (identity_hint != NULL && strlen(identity_hint) > PSK_MAX_IDENTITY_LEN) {
        SSLerr(SSL_F_SSL_USE_PSK_IDENTITY_HINT, SSL_R_DATA_LENGTH_TOO_LONG);
        return 0;
    }
    OPENSSL_free(s->cert->psk_identity_hint);
    if (identity_hint != NULL) {
        s->cert->psk_identity_hint = OPENSSL_strdup(identity_hint);
        if (s->cert->psk_identity_hint == NULL)
            return 0;
    } else
        s->cert->psk_identity_hint = NULL;
    return 1;
}

const char *SSL_get_psk_identity_hint(const SSL *s)
{
    if (s == NULL || s->session == NULL)
        return NULL;
    return (s->session->psk_identity_hint);
}

const char *SSL_get_psk_identity(const SSL *s)
{
    if (s == NULL || s->session == NULL)
        return NULL;
    return (s->session->psk_identity);
}

void SSL_set_psk_client_callback(SSL *s,
                                 unsigned int (*cb) (SSL *ssl,
                                                     const char *hint,
                                                     char *identity,
                                                     unsigned int
                                                     max_identity_len,
                                                     unsigned char *psk,
                                                     unsigned int
                                                     max_psk_len))
{
    s->psk_client_callback = cb;
}

void SSL_CTX_set_psk_client_callback(SSL_CTX *ctx,
                                     unsigned int (*cb) (SSL *ssl,
                                                         const char *hint,
                                                         char *identity,
                                                         unsigned int
                                                         max_identity_len,
                                                         unsigned char *psk,
                                                         unsigned int
                                                         max_psk_len))
{
    ctx->psk_client_callback = cb;
}

void SSL_set_psk_server_callback(SSL *s,
                                 unsigned int (*cb) (SSL *ssl,
                                                     const char *identity,
                                                     unsigned char *psk,
                                                     unsigned int
                                                     max_psk_len))
{
    s->psk_server_callback = cb;
}

void SSL_CTX_set_psk_server_callback(SSL_CTX *ctx,
                                     unsigned int (*cb) (SSL *ssl,
                                                         const char *identity,
                                                         unsigned char *psk,
                                                         unsigned int
                                                         max_psk_len))
{
    ctx->psk_server_callback = cb;
}
#endif

void SSL_CTX_set_msg_callback(SSL_CTX *ctx,
                              void (*cb) (int write_p, int version,
                                          int content_type, const void *buf,
                                          size_t len, SSL *ssl, void *arg))
{
    SSL_CTX_callback_ctrl(ctx, SSL_CTRL_SET_MSG_CALLBACK, (void (*)(void))cb);
}

void SSL_set_msg_callback(SSL *ssl,
                          void (*cb) (int write_p, int version,
                                      int content_type, const void *buf,
                                      size_t len, SSL *ssl, void *arg))
{
    SSL_callback_ctrl(ssl, SSL_CTRL_SET_MSG_CALLBACK, (void (*)(void))cb);
}

void SSL_CTX_set_not_resumable_session_callback(SSL_CTX *ctx,
                                                int (*cb) (SSL *ssl,
                                                           int
                                                           is_forward_secure))
{
    SSL_CTX_callback_ctrl(ctx, SSL_CTRL_SET_NOT_RESUMABLE_SESS_CB,
                          (void (*)(void))cb);
}

void SSL_set_not_resumable_session_callback(SSL *ssl,
                                            int (*cb) (SSL *ssl,
                                                       int is_forward_secure))
{
    SSL_callback_ctrl(ssl, SSL_CTRL_SET_NOT_RESUMABLE_SESS_CB,
                      (void (*)(void))cb);
}

/*
 * Allocates new EVP_MD_CTX and sets pointer to it into given pointer
 * vairable, freeing EVP_MD_CTX previously stored in that variable, if any.
 * If EVP_MD pointer is passed, initializes ctx with this md Returns newly
 * allocated ctx;
 */

EVP_MD_CTX *ssl_replace_hash(EVP_MD_CTX **hash, const EVP_MD *md)
{
    ssl_clear_hash_ctx(hash);
    *hash = EVP_MD_CTX_new();
    if (*hash == NULL || (md && EVP_DigestInit_ex(*hash, md, NULL) <= 0)) {
        EVP_MD_CTX_free(*hash);
        *hash = NULL;
        return NULL;
    }
    return *hash;
}

void ssl_clear_hash_ctx(EVP_MD_CTX **hash)
{

    if (*hash)
        EVP_MD_CTX_free(*hash);
    *hash = NULL;
}

/* Retrieve handshake hashes */
int ssl_handshake_hash(SSL *s, unsigned char *out, int outlen)
{
    EVP_MD_CTX *ctx = NULL;
    EVP_MD_CTX *hdgst = s->s3->handshake_dgst;
    int ret = EVP_MD_CTX_size(hdgst);
    if (ret < 0 || ret > outlen) {
        ret = 0;
        goto err;
    }
    ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        ret = 0;
        goto err;
    }
    if (!EVP_MD_CTX_copy_ex(ctx, hdgst)
        || EVP_DigestFinal_ex(ctx, out, NULL) <= 0)
        ret = 0;
 err:
    EVP_MD_CTX_free(ctx);
    return ret;
}

int SSL_session_reused(SSL *s)
{
    return s->hit;
}

int SSL_is_server(SSL *s)
{
    return s->server;
}

#if OPENSSL_API_COMPAT < 0x10100000L
void SSL_set_debug(SSL *s, int debug)
{
    /* Old function was do-nothing anyway... */
    (void)s;
    (void)debug;
}
#endif


void SSL_set_security_level(SSL *s, int level)
{
    s->cert->sec_level = level;
}

int SSL_get_security_level(const SSL *s)
{
    return s->cert->sec_level;
}

void SSL_set_security_callback(SSL *s,
                               int (*cb) (SSL *s, SSL_CTX *ctx, int op,
                                          int bits, int nid, void *other,
                                          void *ex))
{
    s->cert->sec_cb = cb;
}

int (*SSL_get_security_callback(const SSL *s)) (SSL *s, SSL_CTX *ctx, int op,
                                                int bits, int nid,
                                                void *other, void *ex) {
    return s->cert->sec_cb;
}

void SSL_set0_security_ex_data(SSL *s, void *ex)
{
    s->cert->sec_ex = ex;
}

void *SSL_get0_security_ex_data(const SSL *s)
{
    return s->cert->sec_ex;
}

void SSL_CTX_set_security_level(SSL_CTX *ctx, int level)
{
    ctx->cert->sec_level = level;
}

int SSL_CTX_get_security_level(const SSL_CTX *ctx)
{
    return ctx->cert->sec_level;
}

void SSL_CTX_set_security_callback(SSL_CTX *ctx,
                                   int (*cb) (SSL *s, SSL_CTX *ctx, int op,
                                              int bits, int nid, void *other,
                                              void *ex))
{
    ctx->cert->sec_cb = cb;
}

int (*SSL_CTX_get_security_callback(const SSL_CTX *ctx)) (SSL *s,
                                                          SSL_CTX *ctx,
                                                          int op, int bits,
                                                          int nid,
                                                          void *other,
                                                          void *ex) {
    return ctx->cert->sec_cb;
}

void SSL_CTX_set0_security_ex_data(SSL_CTX *ctx, void *ex)
{
    ctx->cert->sec_ex = ex;
}

void *SSL_CTX_get0_security_ex_data(const SSL_CTX *ctx)
{
    return ctx->cert->sec_ex;
}


/*
 * Get/Set/Clear options in SSL_CTX or SSL, formerly macros, now functions that
 * can return unsigned long, instead of the generic long return value from the
 * control interface.
 */
unsigned long SSL_CTX_get_options(const SSL_CTX *ctx)
{
    return ctx->options;
}
unsigned long SSL_get_options(const SSL* s)
{
    return s->options;
}
unsigned long SSL_CTX_set_options(SSL_CTX *ctx, unsigned long op)
{
    return ctx->options |= op;
}
unsigned long SSL_set_options(SSL *s, unsigned long op)
{
    return s->options |= op;
}
unsigned long SSL_CTX_clear_options(SSL_CTX *ctx, unsigned long op)
{
    return ctx->options &= ~op;
}
unsigned long SSL_clear_options(SSL *s, unsigned long op)
{
    return s->options &= ~op;
}

STACK_OF(X509) *SSL_get0_verified_chain(const SSL *s)
{
    return s->verified_chain;
}

IMPLEMENT_OBJ_BSEARCH_GLOBAL_CMP_FN(SSL_CIPHER, SSL_CIPHER, ssl_cipher_id);

#ifndef OPENSSL_NO_CT

/*
 * Moves SCTs from the |src| stack to the |dst| stack.
 * The source of each SCT will be set to |origin|.
 * If |dst| points to a NULL pointer, a new stack will be created and owned by
 * the caller.
 * Returns the number of SCTs moved, or a negative integer if an error occurs.
 */
static int CT_move_scts(STACK_OF(SCT) **dst, STACK_OF(SCT) *src, sct_source_t origin)
{
    int scts_moved = 0;
    SCT *sct = NULL;

    if (src == NULL)
        goto end;
    if (dst == NULL) {
        SSLerr(SSL_F_CT_MOVE_SCTS, ERR_R_PASSED_NULL_PARAMETER);
        scts_moved = -1;
        goto end;
    }

    if (*dst == NULL) {
        *dst = sk_SCT_new_null();
        if (*dst == NULL) {
            SSLerr(SSL_F_CT_MOVE_SCTS, ERR_R_MALLOC_FAILURE);
            scts_moved = -1;
            goto end;
        }
    }

    while ((sct = sk_SCT_pop(src)) != NULL) {
        if (SCT_set_source(sct, origin) != 1) {
            scts_moved = -1;
            goto end;
        }

        if (sk_SCT_push(*dst, sct) <= 0) {
            scts_moved = -1;
            goto end;
        }
        sct = NULL;
        scts_moved += 1;
    }

end:
    SCT_free(sct);
    return scts_moved;
}

/*
* Look for data collected during ServerHello and parse if found.
* Return 1 on success, 0 on failure.
*/
static int CT_extract_tls_extension_scts(SSL *s)
{
    int scts_extracted = 0;
    if (s->tlsext_scts != NULL) {
        const unsigned char *p = s->tlsext_scts;
        STACK_OF(SCT) *scts = o2i_SCT_LIST(NULL, &p, s->tlsext_scts_len);
        scts_extracted = CT_move_scts(&s->scts, scts, SCT_SOURCE_TLS_EXTENSION);
        SCT_LIST_free(scts);
    }
    return scts_extracted;
}

/*
 * Checks for an OCSP response and then attempts to extract any SCTs found if it
 * contains an SCT X509 extension. They will be stored in |s->scts|.
 * Returns:
 * - The number of SCTs extracted, assuming an OCSP response exists.
 * - 0 if no OCSP response exists or it contains no SCTs.
 * - A negative integer if an error occurs.
 */
static int CT_extract_ocsp_response_scts(SSL *s)
{
    int scts_extracted = 0;
    const unsigned char *p;
    OCSP_BASICRESP *br = NULL;
    OCSP_RESPONSE *rsp = NULL;
    STACK_OF(SCT) *scts = NULL;
    int i;

    if (s == NULL || s->tlsext_ocsp_resp == NULL || s->tlsext_ocsp_resplen == 0)
        goto err;

    p = s->tlsext_ocsp_resp;
    rsp = d2i_OCSP_RESPONSE(NULL, &p, s->tlsext_ocsp_resplen);
    if (rsp == NULL)
        goto err;

    br = OCSP_response_get1_basic(rsp);
    if (br == NULL)
        goto err;

    for (i = 0; i < OCSP_resp_count(br); ++i) {
        OCSP_SINGLERESP *single = OCSP_resp_get0(br, i);
        if (single == NULL)
            continue;

        scts =
            OCSP_SINGLERESP_get1_ext_d2i(single, NID_ct_cert_scts, NULL, NULL);
        scts_extracted =
            CT_move_scts(&s->scts, scts, SCT_SOURCE_OCSP_STAPLED_RESPONSE);
        if (scts_extracted < 0)
            goto err;

        sk_SCT_free(scts);
        scts = NULL;
    }
err:
    SCT_LIST_free(scts);
    OCSP_BASICRESP_free(br);
    OCSP_RESPONSE_free(rsp);
    return scts_extracted;
}

/*
 * Attempts to extract SCTs from the peer certificate.
 * Return the number of SCTs extracted, or a negative integer if an error
 * occurs.
 */
static int CT_extract_x509v3_extension_scts(SSL *s)
{
    int scts_extracted = 0;
    X509 *cert = SSL_get_peer_certificate(s);
    if (cert != NULL) {
        STACK_OF(SCT) *scts =
            X509_get_ext_d2i(cert, NID_ct_precert_scts, NULL, NULL);
        scts_extracted =
            CT_move_scts(&s->scts, scts, SCT_SOURCE_X509V3_EXTENSION);
        SCT_LIST_free(scts);
    }
    return scts_extracted;
}

/*
 * Attempts to find all received SCTs by checking TLS extensions, the OCSP
 * response (if it exists) and X509v3 extensions in the certificate.
 * Returns NULL if an error occurs.
 */
const STACK_OF(SCT) *SSL_get0_peer_scts(SSL *s)
{
    if (s == NULL) {
        SSLerr(SSL_F_SSL_GET0_PEER_SCTS, ERR_R_PASSED_NULL_PARAMETER);
        goto err;
    }
    if (!s->scts_parsed) {
        if (CT_extract_tls_extension_scts(s) < 0 ||
            CT_extract_ocsp_response_scts(s) < 0 ||
            CT_extract_x509v3_extension_scts(s) < 0)
            goto err;
        s->scts_parsed = 1;
    }
    return s->scts;
err:
    return NULL;
}

int SSL_set_ct_validation_callback(SSL *s, ct_validation_cb callback, void *arg)
{
    int ret = 0;
    if (s == NULL) {
        SSLerr(SSL_F_SSL_SET_CT_VALIDATION_CALLBACK, ERR_R_PASSED_NULL_PARAMETER);
        goto err;
    }
    /*
     * Since code exists that uses the custom extension handler for CT, look for
     * this and throw an error if they have already registered to use CT.
     */
    if (callback != NULL && SSL_CTX_has_client_custom_ext(s->ctx,
            TLSEXT_TYPE_signed_certificate_timestamp)) {
        SSLerr(SSL_F_SSL_SET_CT_VALIDATION_CALLBACK,
               SSL_R_CUSTOM_EXT_HANDLER_ALREADY_INSTALLED);
        goto err;
    }
    OPENSSL_free(s->ct_validation_callback_arg);
    s->ct_validation_callback = callback;
    s->ct_validation_callback_arg = arg;
    if (callback != NULL) {
        /* If we are validating CT, then we MUST accept SCTs served via OCSP */
        SSL_set_tlsext_status_type(s, TLSEXT_STATUSTYPE_ocsp);
    }
    ret = 1;
err:
    return ret;
}

int SSL_CTX_set_ct_validation_callback(SSL_CTX *ctx, ct_validation_cb callback, void *arg)
{
    int ret = 0;

    if (ctx == NULL) {
        SSLerr(SSL_F_SSL_CTX_SET_CT_VALIDATION_CALLBACK, ERR_R_PASSED_NULL_PARAMETER);
        goto err;
    }
    /*
     * Since code exists that uses the custom extension handler for CT, look for
     * this and throw an error if they have already registered to use CT.
     */
    if (callback != NULL && SSL_CTX_has_client_custom_ext(ctx,
            TLSEXT_TYPE_signed_certificate_timestamp)) {
        SSLerr(SSL_F_SSL_CTX_SET_CT_VALIDATION_CALLBACK,
               SSL_R_CUSTOM_EXT_HANDLER_ALREADY_INSTALLED);
        goto err;
    }
    OPENSSL_free(ctx->ct_validation_callback_arg);
    ctx->ct_validation_callback = callback;
    ctx->ct_validation_callback_arg = arg;
    ret = 1;
err:
    return ret;
}

ct_validation_cb SSL_get_ct_validation_callback(const SSL *s)
{
    if (s == NULL) {
        SSLerr(SSL_F_SSL_GET_CT_VALIDATION_CALLBACK, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }
    return s->ct_validation_callback;
}

ct_validation_cb SSL_CTX_get_ct_validation_callback(const SSL_CTX *ctx)
{
    if (ctx == NULL) {
        SSLerr(SSL_F_SSL_CTX_GET_CT_VALIDATION_CALLBACK, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }
    return ctx->ct_validation_callback;
}

int SSL_validate_ct(SSL *s)
{
    int ret = 0;
    X509 *cert = SSL_get_peer_certificate(s);
    X509 *issuer = NULL;
    CT_POLICY_EVAL_CTX *ctx = NULL;

    /* If no callback is set, attempt no validation - just return success */
    if (s->ct_validation_callback == NULL)
        return 1;

    if (s == NULL) {
        SSLerr(SSL_F_SSL_VALIDATE_CT, ERR_R_PASSED_NULL_PARAMETER);
        goto end;
    }
    if (cert == NULL) {
        SSLerr(SSL_F_SSL_VALIDATE_CT, SSL_R_NO_CERTIFICATE_ASSIGNED);
        goto end;
    }

    ctx = CT_POLICY_EVAL_CTX_new();
    if (ctx == NULL) {
        SSLerr(SSL_F_SSL_VALIDATE_CT, ERR_R_MALLOC_FAILURE);
        goto end;
    }

    if (s->verified_chain != NULL && sk_X509_num(s->verified_chain) > 1)
        issuer = sk_X509_value(s->verified_chain, 1);

    if (CT_POLICY_EVAL_CTX_set0_cert(ctx, cert) != 1 ||
        CT_POLICY_EVAL_CTX_set0_issuer(ctx, issuer) != 1 ||
        CT_POLICY_EVAL_CTX_set0_log_store(ctx, s->ctx->ctlog_store) != 1) {
        SSLerr(SSL_F_SSL_VALIDATE_CT, SSL_R_SET_FAILED);
        goto end;
    }

    if (SCT_LIST_validate(SSL_get0_peer_scts(s), ctx) != 1) {
        SSLerr(SSL_F_SSL_VALIDATE_CT, SSL_R_SCT_VERIFICATION_FAILED);
        goto end;
    }

    ret = s->ct_validation_callback(ctx, s->ct_validation_callback_arg);

end:
    CT_POLICY_EVAL_CTX_free(ctx);
    return ret;
}

int SSL_CTX_set_ct_verify_location_to_default(SSL_CTX *ctx)
{
    return CTLOG_STORE_load_default_file(ctx->ctlog_store);
}

int SSL_CTX_set_ct_verify_location(SSL_CTX *ctx, const char *log_list_path)
{
    return CTLOG_STORE_load_file(ctx->ctlog_store, log_list_path);
}

#endif
