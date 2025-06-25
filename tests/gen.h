//
// Created by zhangchi on 2024-November-27.
//

#ifndef GEN_H
#define GEN_H
#include "gmp.h"
#include "malloc.h"

#ifdef __cplusplus
extern "C"{
#endif

void gen_random(mpz_t result, int bit_len,gmp_randstate_t grt);
void gen_random_prime(mpz_t result, int bit_len,gmp_randstate_t grt);
void bin2mont(mpz_t result, mpz_t data, mpz_t mod, int base);
void mont2bin(mpz_t result, mpz_t data, mpz_t mod, int base);
void tear(mpz_t* words, mpz_t data, int word_len, int word_bit_len);
void clip(mpz_t data, mpz_t* words, int word_len, int word_bit_len);
void gen_param();
void gen_mul_data();
void gen_mod_mul_data();
void gen_mod_data();
void gen_exp_data();
void gen_enc_data();
void gen_dec_data();

#ifdef __cplusplus
}
#endif

#endif //GEN_H
