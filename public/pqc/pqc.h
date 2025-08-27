#ifndef PQAPI_H_
#define PQAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Defines supported signature algorithm list. The resulting
// ID of an algorithm is PQC_ALG_SIG_(NAME_AS_BELOW)
#define PQC_SUPPORTED_SIGS(_) \
    _(DILITHIUM2)                \
    _(DILITHIUM3)                \
    _(DILITHIUM5)                \
    _(FALCON512)                 \
    _(FALCON1024)                \
    _(SPHINCSSHAKE256128FSIMPLE) \
    _(SPHINCSSHAKE256128SSIMPLE) \
    _(SPHINCSSHAKE256128FROBUST) \
    _(SPHINCSSHAKE256128SROBUST) \
    _(SPHINCSSHAKE256192FSIMPLE) \
    _(SPHINCSSHAKE256192SSIMPLE) \
    _(SPHINCSSHAKE256192FROBUST) \
    _(SPHINCSSHAKE256192SROBUST) \
    _(SPHINCSSHAKE256256FSIMPLE) \
    _(SPHINCSSHAKE256256SSIMPLE) \
    _(SPHINCSSHAKE256256FROBUST) \
    _(SPHINCSSHAKE256256SROBUST) \
    _(SPHINCSSHA256128FSIMPLE)   \
    _(SPHINCSSHA256128SSIMPLE)   \
    _(SPHINCSSHA256128FROBUST)   \
    _(SPHINCSSHA256128SROBUST)   \
    _(SPHINCSSHA256192FSIMPLE)   \
    _(SPHINCSSHA256192SSIMPLE)   \
    _(SPHINCSSHA256192FROBUST)   \
    _(SPHINCSSHA256192SROBUST)   \
    _(SPHINCSSHA256256FSIMPLE)   \
    _(SPHINCSSHA256256SSIMPLE)   \
    _(SPHINCSSHA256256FROBUST)   \
    _(SPHINCSSHA256256SROBUST)

// Defines supported kem algorithm list. The resulting
// ID of an algorithm is PQC_ALG_KEM_(NAME_AS_BELOW)
#define PQC_SUPPORTED_KEMS(_)\
    _(KYBER512)          \
    _(KYBER768)          \
    _(KYBER1024)         \
    _(HQCRMRS128)        \
    _(HQCRMRS192)        \
    _(HQCRMRS256)        \
    _(MCELIECE348864)    \
    _(MCELIECE460896)    \
    _(MCELIECE6688128)   \
    _(MCELIECE6960119)   \
    _(MCELIECE8192128)   \
    _(MCELIECE348864F)   \
    _(MCELIECE460896F)   \
    _(MCELIECE6688128F)  \
    _(MCELIECE6960119F)  \
    _(MCELIECE8192128F)

// Defines IDs for each algorithm. The
// PQC_ALG_SIG/KEM_MAX indicates number
// of KEM and signature schemes supported.
#define DEFNUM_SIG(N) PQC_ALG_SIG_##N,
#define DEFNUM_KEM(N) PQC_ALG_KEM_##N,
enum { PQC_SUPPORTED_SIGS(DEFNUM_SIG) PQC_ALG_SIG_MAX };
enum { PQC_SUPPORTED_KEMS(DEFNUM_KEM) PQC_ALG_KEM_MAX };
#undef DEFNUM_SIG
#undef DEFNUM_KEM

// Parameters of the scheme
typedef struct pqc_ctx_t {
    const uint8_t alg_id;
    const char* alg_name;
    const uint32_t prv_key_bsz;
    const uint32_t pub_key_bsz;
    const bool is_kem;

    int (*keygen)(uint8_t *sk, uint8_t *pk);
} pqc_ctx_t;

typedef struct pqc_kem_ctx_t {
    pqc_ctx_t p;
    const uint32_t ciphertext_bsz;
    const uint32_t secret_bsz;

    int (*encapsulate)(uint8_t *ct, uint8_t *ss, const uint8_t *pk);
    int (*decapsulate)(uint8_t *ss, const uint8_t *ct, const uint8_t *sk);
} pqc_kem_ctx_t;

typedef struct pqc_sig_ctx_t {
    pqc_ctx_t p;
    const uint32_t sign_bsz;
    int (*sign)(uint8_t *sig, uint64_t *siglen, const uint8_t *m, uint64_t mlen, const uint8_t *sk);
    int (*verify)(const uint8_t *sig, uint64_t siglen, const uint8_t *m, uint64_t mlen, const uint8_t *pk);
} pqc_sig_ctx_t;

bool pqc_keygen(
    const pqc_ctx_t *p,
    uint8_t *pk, uint8_t *sk);

bool pqc_kem_encapsulate(
    const pqc_ctx_t *p,
    uint8_t *ct, uint8_t *ss,
    const uint8_t *pk);

bool pqc_kem_decapsulate(
    const pqc_ctx_t *p,
    uint8_t *ss, const uint8_t *ct,
    const uint8_t *sk);

bool pqc_sig_create(
    const pqc_ctx_t *p,
    uint8_t *sig, uint64_t *siglen,
    const uint8_t *m, uint64_t mlen,
    const uint8_t *sk);

bool pqc_sig_verify(
    const pqc_ctx_t *p,
    const uint8_t *sig, uint64_t siglen,
    const uint8_t *m, uint64_t mlen,
    const uint8_t *pk);


const pqc_ctx_t *pqc_kem_alg_by_id(uint8_t id);
const pqc_ctx_t *pqc_sig_alg_by_id(uint8_t id);

uint32_t pqc_ciphertext_bsz(const pqc_ctx_t *p);
uint32_t pqc_shared_secret_bsz(const pqc_ctx_t *p);
uint32_t pqc_signature_bsz(const pqc_ctx_t *p);
uint32_t pqc_public_key_bsz(const pqc_ctx_t *p);
uint32_t pqc_private_key_bsz(const pqc_ctx_t *p);

#ifdef __cplusplus
}
#endif

#endif
