#include <algorithm>
#include <sstream>
#include <vector>
#include <gtest/gtest.h>
#include <pqc/pqc.h>
#include <random>
extern "C" {
    #include "sign/dilithium/dilithium2/clean/reduce.h"
    #include "sign/dilithium/dilithium2/clean/poly.h"
    #include "sign/dilithium/dilithium2/clean/params.h"
}

TEST(KEM,OneOff) {

    for (int i=0; i<PQC_ALG_KEM_MAX; i++) {
        const pqc_ctx_t *p = pqc_kem_alg_by_id(i);

        std::vector<uint8_t> ct(pqc_ciphertext_bsz(p));
        std::vector<uint8_t> ss1(pqc_shared_secret_bsz(p));
        std::vector<uint8_t> ss2(pqc_shared_secret_bsz(p));
        std::vector<uint8_t> sk(pqc_private_key_bsz(p));
        std::vector<uint8_t> pk(pqc_public_key_bsz(p));

        ASSERT_TRUE(
            pqc_keygen(p, pk.data(), sk.data()));
        ASSERT_TRUE(
            pqc_kem_encapsulate(p, ct.data(), ss1.data(), pk.data()));
        ASSERT_TRUE(
            pqc_kem_decapsulate(p, ss2.data(), ct.data(), sk.data()));
        ASSERT_TRUE(
            std::equal(ss1.begin(), ss1.end(), ss2.begin()));
    }
}

TEST(SIGN,OneOff) {

    std::random_device rd;
    std::uniform_int_distribution<uint8_t> dist(0, 0xFF);
    uint8_t msg[1234] = {0};

    for (int i=0; i<PQC_ALG_SIG_MAX; i++) {
        const pqc_ctx_t *p = pqc_sig_alg_by_id(i);
        // generate some random msg
        for (auto &x : msg) {x = dist(rd);}

        std::vector<uint8_t> sig(pqc_signature_bsz(p));
        std::vector<uint8_t> sk(pqc_private_key_bsz(p));
        std::vector<uint8_t> pk(pqc_public_key_bsz(p));

        ASSERT_TRUE(
            pqc_keygen(p, pk.data(), sk.data()));
        uint64_t sigsz = sig.size();
        ASSERT_TRUE(
            pqc_sig_create(p, sig.data(), &sigsz, msg, 1234, sk.data()));
        ASSERT_TRUE(
            pqc_sig_verify(p, sig.data(), sigsz, msg, 1234, pk.data()));
    }
}

TEST(KEMSIG,PrintSizes) {

    for (int i=0; i<PQC_ALG_SIG_MAX; i++) {
        std::stringstream out;
        const pqc_ctx_t *p = pqc_sig_alg_by_id(i);
        out << std::setw(30) << std::left << p->alg_name
            << " :pk: "   << std::setw(15) << pqc_public_key_bsz(p)
            << " :sign: " << std::setw(15) << pqc_signature_bsz(p);
        std::cout << out.str() << std::endl;
    }


    for (int i=0; i<PQC_ALG_KEM_MAX; i++) {
        std::stringstream out;
        const pqc_ctx_t *p = pqc_kem_alg_by_id(i);
        out << std::setw(30) << std::left << p->alg_name
            << " :pk: " << std::setw(15) << pqc_public_key_bsz(p)
            << " :ct: " << std::setw(15) << pqc_ciphertext_bsz(p);
        std::cout << out.str() << std::endl;
    }
}

TEST(Dilithium, MontREDC) {
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(0), 0);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(Q), 0);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(Q*100), 0);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(1), -114592);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(-1), 114592);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(((uint64_t)Q<<31)-1), 114592);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(-((int64_t)Q<<31)),0);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(-((1ULL<<31)*(int64_t)Q)+1), -114592);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(-(((int64_t)Q)<<31)+1), -114592);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce((uint64_t)1<<15), -523840);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce((uint64_t)1<<31), 4190209);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(3347556), 2070606);
    ASSERT_EQ(PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(-2581810), 910169);
}

TEST(Dilithium, PolyZ) {
    std::random_device rd;
    std::uniform_int_distribution<int32_t> dist_z(-((1<<17)-1), 1<<17);
    uint8_t out[576];

    poly p1, p2;
    for (auto &x : p1.coeffs) { x = dist_z(rd); }
    PQCLEAN_DILITHIUM2_CLEAN_polyz_pack(out, &p1);
    PQCLEAN_DILITHIUM2_CLEAN_polyz_unpack(&p2, out);

    for (size_t j=0; j<256; j++) {
        ASSERT_EQ(p1.coeffs[j], p2.coeffs[j]);
    }
}
