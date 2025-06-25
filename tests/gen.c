//
// Created by zhangchi on 2024-November-27.
//

#include "./gen.h"
/**
 * Generates a random multi-precision integer with a specified bit length.
 *
 * This function creates a random multi-precision integer (`result`) of the specified bit length (`bit_len`),
 * ensuring that the highest bit is set to 1. It uses the provided random state (`grt`) from the GNU Multiple Precision Arithmetic Library (GMP) to generate the random number.
 *
 * @param result[out] Pointer to a multi-precision integer where the generated random number will be stored.
 * @param bit_len[in] The bit length of the random multi-precision integer to be generated.
 * @param grt[in] The random state initialized using GMP functions for random number generation.
 */
void gen_random(mpz_t result, int bit_len,gmp_randstate_t grt){
    mpz_urandomb(result,grt, bit_len - 1);
    mpz_setbit(result,bit_len - 1);
}

/**
 * Generates a random prime multi-precision integer with a specified bit length.
 *
 * This function creates a random prime multi-precision integer (`result`) of the specified bit length (`bit_len`),
 * ensuring that the highest bit is set to 1. It first generates a random number with the specified bit length minus one,
 * sets the highest bit, and then checks if the number is probably prime using a probabilistic test with 80 iterations.
 * If the generated number is not prime, it finds the next prime number using `mpz_nextprime`.
 *
 * @param result[out] Pointer to a multi-precision integer where the generated random prime number will be stored.
 * @param bit_len[in] The bit length of the random prime multi-precision integer to be generated.
 * @param grt[in] The random state initialized using GMP functions for random number generation.
 */
void gen_random_prime(mpz_t result, int bit_len, gmp_randstate_t grt){
    mpz_urandomb(result,grt, bit_len - 1);
    mpz_setbit(result,bit_len - 1);
    if(mpz_probab_prime_p(result,80) == 0){
        mpz_nextprime(result,result);
    }
}

/**
 * Converts a binary multi-precision integer to its Montgomery representation modulo a given modulus.
 *
 * This function takes a multi-precision integer `data` represented in binary form and converts it to its
 * Montgomery representation with respect to the modulus `mod`. The Montgomery representation is calculated
 * by multiplying `data` by 2 raised to the power of `base` and then taking the result modulo `mod`.
 *
 * @param result[out] Pointer to a multi-precision integer where the Montgomery representation will be stored.
 * @param data[in] The multi-precision integer in binary form to be converted.
 * @param mod[in] The modulus used for the Montgomery reduction.
 * @param base[in] The exponent used in the multiplication (2^base).
 */
void bin2mont(mpz_t result, mpz_t data, mpz_t mod, int base){
    mpz_mul_2exp(result,data,base);
    mpz_mod(result,result,mod);
}

/**
 * Converts a Montgomery representation multi-precision integer to a binary representation.
 *
 * This function takes a multi-precision integer (`data`) in Montgomery representation and converts it
 * into a standard binary representation, storing the result in `result`. The conversion is performed
 * within the context of the modulus (`mod`) and considering the specified base (`base`).
 *
 * @param result[out] Pointer to a multi-precision integer where the converted binary result will be stored.
 * @param data[in] The multi-precision integer in Montgomery representation to be converted.
 * @param mod[in] The modulus used in the Montgomery arithmetic.
 * @param base[in] The base relevant for the Montgomery to binary conversion calculation.
 */
void mont2bin(mpz_t result, mpz_t data, mpz_t mod, int base){
    mpz_t r;
    mpz_init(r);
    mpz_setbit(r,base);
    mpz_invert(r,r,mod);
    mpz_mul(result,r,data);
    mpz_mod(result,result,mod);
}

/**
 * Tears apart a multi-precision integer into an array of smaller multi-precision integers based on the specified word length and bit length.
 *
 * This function takes a large multi-precision integer (`data`) and splits it into an array (`words`) of smaller multi-precision integers,
 * each having the same word bit length. It is particularly useful in scenarios where the original data needs to be processed in fixed-size chunks.
 *
 * @param words[out] Pointer to an array of multi-precision integers where the split data will be stored.
 * @param data[in] The original multi-precision integer to be split.
 * @param word_len[in] The number of words to split the `data` into.
 * @param word_bit_len[in] The bit length of each word in the resulting array.
 */
void tear(mpz_t* words, mpz_t data, int word_len, int word_bit_len){
    int i;
    mpz_t cur_data;
    mpz_init_set(cur_data,data);
    for (i = 0; i < word_len; i++) {
        mpz_mod_2exp(words[i], cur_data, word_bit_len);
        mpz_div_2exp(cur_data, cur_data, word_bit_len);
    }
}

/**
 * Combines an array of smaller multi-precision integers into a single multi-precision integer.
 *
 * This function takes an array (`words`) of multi-precision integers and combines them into a larger multi-precision integer (`data`),
 * effectively reversing the process of splitting data into smaller chunks. Each element in the array is assumed to have the same bit length.
 *
 * @param[out] data Pointer to a multi-precision integer where the combined result will be stored.
 * @param[in] words Pointer to an array of multi-precision integers to be combined.
 * @param[in] word_len The number of elements in the `words` array.
 * @param[in] word_bit_len The bit length of each multi-precision integer in the `words` array.
 */
void clip(mpz_t data, mpz_t* words, int word_len, int word_bit_len){
    int i;
    mpz_set_ui(data,0);
    for (i = word_len - 1; i >=0; i--) {
        mpz_mul_2exp(data, data, word_bit_len);
        mpz_add(data, data, words[i]);
    }
}

/**
 * Generates parameters required for a cryptographic operation.
 *
 * This function sets up various multi-precision integers used as parameters in a cryptographic scheme,
 * including large prime numbers P and Q, and related values derived from them. It also initializes
 * a random state for subsequent use in secure computations and performs modular arithmetic operations
 * to compute intermediate variables necessary for cryptographic protocols such as key generation or encryption.
 *
 * The function assumes a fixed bit length for certain parameters and splits computed values into words
 * suitable for processing within a specific cryptographic algorithm, demonstrating a common practice in
 * asymmetric cryptography setups like RSA or similar schemes.
 *
 * Note: The function does not return any value but modifies variables declared within its scope.
 * Memory is allocated for an array of multi-precision integers (`a_w`), which must be freed after use to avoid leaks.
 *
 * Dependencies:
 * - GMP (GNU Multiple Precision Arithmetic Library) for multi-precision arithmetic operations.
 */
void gen_param()
{
    const int word_bit_len = 128;
    const int cipher_word_len = 32;
    int plain_word_len = 16;
    int cipher_bit_len = word_bit_len * cipher_word_len;
    gmp_randstate_t grt;
    gmp_randinit_default (grt);
    mpz_t P                 ;   mpz_init(P                  );
    mpz_t Q                 ;   mpz_init(Q                  );
    mpz_set_str(P,"de0bbade38204e63359a46e672a8d0a2fd5300692ab48f9ef732f5c3fa212b90c98229bbb79bece734a622154c904dce9a0f53d4a88b3e558ef7612f6694ce7518f204fe6846aeb6f58174d57a3372363c0d9fcfaa3dc18b1eff7e89bf7678636580d17dd84a873b14b9c0e1680bbdc87647f3c382902d2f58d2754b39bcaea5",16);
    mpz_set_str(Q,"cb6772793755e74862e61e2ac376cfab9d61827e646421b28e9e0e2aca4625731aebbb69ea37e0fa859e499b8a186c8ee6196954170eb8068593f0d764150a6d2e5d3fea7d9d0d33ac553eecd5c3f27a310115d283e49377820195c8e67781b6f112a625b14b747fa4cc13d06eba0917246c775f5c732865701ae9349ea87d11",16);
    mpz_t R                 ;   mpz_init(R                  );  mpz_setbit(R,2048);
    mpz_t P_1               ;   mpz_init(P_1                );  mpz_sub_ui(P_1,P,1);
    mpz_t Q_1               ;   mpz_init(Q_1                );  mpz_sub_ui(Q_1,Q,1);
    mpz_t LAMBDA            ;   mpz_init(LAMBDA             );  mpz_lcm(LAMBDA,P_1,Q_1);
    mpz_t N                 ;   mpz_init(N                  );  mpz_mul(N,P,Q);
    mpz_t N2                ;   mpz_init(N2                 );  mpz_mul(N2,N,N);
    mpz_t P2                ;   mpz_init(P2                 );  mpz_mul(P2,P,P);
    mpz_t Q2                ;   mpz_init(Q2                 );  mpz_mul(Q2,Q,Q);
    mpz_t R2                ;   mpz_init(R2                 );  mpz_mul(R2,R,R);
    mpz_t NR2_MOD_N2        ;   mpz_init(NR2_MOD_N2         );  mpz_mul(NR2_MOD_N2,R2,N); mpz_mod(NR2_MOD_N2,NR2_MOD_N2,N2);
    mpz_t R4_MOD_N2         ;   mpz_init(R4_MOD_N2          );  mpz_mul(R4_MOD_N2,R2,R2); mpz_mod(R4_MOD_N2,R4_MOD_N2,N2);
    mpz_t QQR2_MOD_N2       ;   mpz_init(QQR2_MOD_N2        );  mpz_invert(QQR2_MOD_N2,Q2,P2);mpz_mul(QQR2_MOD_N2,QQR2_MOD_N2,Q2);mpz_mul(QQR2_MOD_N2,QQR2_MOD_N2,R2);mpz_mod(QQR2_MOD_N2,QQR2_MOD_N2,N2);
    mpz_t PPR2_MOD_N2       ;   mpz_init(PPR2_MOD_N2        );  mpz_invert(PPR2_MOD_N2,P2,Q2);mpz_mul(PPR2_MOD_N2,PPR2_MOD_N2,P2);mpz_mul(PPR2_MOD_N2,PPR2_MOD_N2,R2);mpz_mod(PPR2_MOD_N2,PPR2_MOD_N2,N2);
    mpz_t R2_MOD_N          ;   mpz_init(R2_MOD_N           );  mpz_mod(R2_MOD_N,R2,N);
    mpz_t R2_MOD_P2         ;   mpz_init(R2_MOD_P2          );  mpz_mod(R2_MOD_P2,R2,P2);
    mpz_t R2_MOD_Q2         ;   mpz_init(R2_MOD_Q2          );  mpz_mod(R2_MOD_Q2,R2,Q2);
    mpz_t PHI_P2            ;   mpz_init(PHI_P2             );  mpz_mul(PHI_P2,P,P_1);
    mpz_t PHI_Q2            ;   mpz_init(PHI_Q2             );  mpz_mul(PHI_Q2,Q,Q_1);
    mpz_t LAMBDA_MOD_PHI_P2 ;   mpz_init(LAMBDA_MOD_PHI_P2  );  mpz_mod(LAMBDA_MOD_PHI_P2,LAMBDA,PHI_P2);
    mpz_t LAMBDA_MOD_PHI_Q2 ;   mpz_init(LAMBDA_MOD_PHI_Q2  );  mpz_mod(LAMBDA_MOD_PHI_Q2,LAMBDA,PHI_Q2);
    mpz_t N_INV_MOD_R       ;   mpz_init(N_INV_MOD_R        );  mpz_invert(N_INV_MOD_R,N,R);
    mpz_t RMU_INV_MOD_N     ;   mpz_init(RMU_INV_MOD_N      );  mpz_add_ui(RMU_INV_MOD_N,N,1);
                                                                mpz_powm(RMU_INV_MOD_N,RMU_INV_MOD_N,LAMBDA,N2);
                                                                mpz_sub_ui(RMU_INV_MOD_N,RMU_INV_MOD_N,1);
                                                                mpz_div(RMU_INV_MOD_N,RMU_INV_MOD_N,N);
                                                                mpz_invert(RMU_INV_MOD_N,RMU_INV_MOD_N,N);
                                                                mpz_mul(RMU_INV_MOD_N,RMU_INV_MOD_N,R);
                                                                mpz_mod(RMU_INV_MOD_N,RMU_INV_MOD_N,N);
    mpz_t ZERO              ;   mpz_init(ZERO               );  mpz_set_ui(ZERO,0);
    mpz_t ONE               ;   mpz_init(ONE                );  mpz_set_ui(ONE,1);
    mpz_t PLAIN_ONE         ;   mpz_init(PLAIN_ONE          );  mpz_set_ui(PLAIN_ONE,1);

    mpz_t WORD              ;   mpz_init(WORD               );  mpz_setbit(WORD,128);
    mpz_t N_PRIME           ;   mpz_init(N_PRIME            );  mpz_invert(N_PRIME,N,WORD); mpz_sub(N_PRIME,WORD,N_PRIME);
    mpz_t N2_PRIME          ;   mpz_init(N2_PRIME           );  mpz_invert(N2_PRIME,N2,WORD); mpz_sub(N2_PRIME,WORD,N2_PRIME);
    mpz_t * a_w;
    a_w = malloc( sizeof (mpz_t) * (32 + 1));
    for (int i = 0; i < 32 + 1; ++i) {
        mpz_init_set_si(a_w[i],0);
    }
    tear(a_w,NR2_MOD_N2       ,32,128); gmp_printf("NR2_MOD_N2       :\n"); for (int i = 0; i < 32; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,R4_MOD_N2        ,32,128); gmp_printf("R4_MOD_N2        :\n"); for (int i = 0; i < 32; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,QQR2_MOD_N2      ,32,128); gmp_printf("QQR2_MOD_N2      :\n"); for (int i = 0; i < 32; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,PPR2_MOD_N2      ,32,128); gmp_printf("PPR2_MOD_N2      :\n"); for (int i = 0; i < 32; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,R2_MOD_N         ,16,128); gmp_printf("R2_MOD_N         :\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,R2_MOD_P2        ,16,128); gmp_printf("R2_MOD_P2        :\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,R2_MOD_Q2        ,16,128); gmp_printf("R2_MOD_Q2        :\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,ONE              ,16,128); gmp_printf("LAMBDA_MOD_PHI_P2:\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,ONE              ,16,128); gmp_printf("LAMBDA_MOD_PHI_Q2:\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,N_INV_MOD_R      ,16,128); gmp_printf("N_INV_MOD_R      :\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,RMU_INV_MOD_N    ,16,128); gmp_printf("RMU_INV_MOD_N    :\n"); for (int i = 0; i < 16; ++i) {gmp_printf("%032Zx\n",a_w[i]);}
    tear(a_w,ONE              ,32,128); gmp_printf("ONE              :\n"); for (int i = 0; i < 32; ++i) {gmp_printf("%032Zx\n",a_w[i]);}


    //gmp_printf("N_PRIME:\n");  gmp_printf("%032Zx\n",N_PRIME);
    //gmp_printf("N2_PRIME:\n"); gmp_printf("%032Zx\n",N2_PRIME);

    mpz_t A             ;   mpz_init(A              );  gen_random(A,4096,grt);
    mpz_t B             ;   mpz_init(B              );  gen_random(B,2048,grt);
    mpz_set_str(A,"a3f3e828532a453289bd47b363738f866debf04222abeecac1e11f980b6f115f097f4540aa7735b993f17f55083caeb6a80f80d092c59d2f895f783fab56a353b58a8c4316eacf3012c77e6fbfdb4be7ed3cd27fc1c72a98f7733050ae2a4bd8c2b356f3f81de6f56258f69355b9321117b905723db3fe533ff94c12502b145c53e61608834634eae18e60c5b991b9f8d71b2d971cbe5ac9e09f4814addab421efdcc2870d2c92c87003fcff55ccba1d4f22f5ab90950fb020f8be80ba9b4c7ca011f74c2d41581f0036d233b5e8e58b6dd5ca6db0625d764b927a43fe78844090c6843f29a331b76f8ece93e7e313eccb9bcb6ed2330923899aae43a0fd2430cb6772793755e74862e61e2ac376cfab9d61827e646421b28e9e0e2aca4625731aebbb69ea37e0fa859e499b8a186c8ee6196954170eb8068593f0d764150a6d2e5d3fea7d9d0d33ac553eecd5c3f27a310115d283e49377820195c8e67781b6f112a625b14b747fa4cc13d06eba0917246c775f5c732865701ae9349ea8729cde0bbade38204e63359a46e672a8d0a2fd5300692ab48f9ef732f5c3fa212b90c98229bbb79bece734a622154c904dce9a0f53d4a88b3e558ef7612f6694ce7518f204fe6846aeb6f58174d57a3372363c0d9fcfaa3dc18b1eff7e89bf7678636580d17dd84a873b14b9c0e1680bbdc87647f3c382902d2f58d2754b39bca874",16);
    mpz_set_str(B,"fcc716330a37576e5021ca2fd2f24b31e027c0b9bc2929f2a2a38c9d003ae5b45d153957d2d0fe1cd05a87f375d050f6341d1e83f0583276902503259190aa7b0353e99a8b404da6feabe3a3b4a54263523a3619aedffe301db8be0aa07b04b8d8c1210cbb3034856d6f46dec94cf866558439083e26bd03dc4c11a81239654b516b2f891d20d0f7fc98547fac560ab315de74e6eb71dccef15a3ac85d3daa6072603a608a1d9201d5f09ad67ed8ce94a6b25eb8a8fc7c1f2a46626cf17c40bc08e57582f9852ad12cf0ee16f2440678d35a31147278a27658a66182b41c8327a559d058a9e9df5a55fe9eeccd16fd651c2d7f13a9942e7418052b4ae1b98f8c",16);
    free(a_w);
}
