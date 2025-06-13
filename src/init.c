//
// Created by zhangchi on 24-11-26.
//
#include <stdio.h>
#include <string.h>
#include "gmp.h"

#include "init.h"
#include "data_process.h"
#include "ram_addr_list.h"
#include "reg_addr_list.h"

/**
 * Initializes SK parameters and buffers them into memory.
 *
 * This function computes various cryptographic parameters based on the provided
 * prime strings `p_str` and `q_str`, and then stores these computed values into
 * a buffer pointed to by `ram_in_buf`. The computations involve initializing and
 * manipulating large integers (mpz_t types) using GMP library functions.
 *
 * @param ram_in_buf A pointer to a buffer where the computed parameters will be stored.
 *                  Must be allocated to a size of at least 8192 bytes.
 * @param p_str     A hexadecimal string representing the first prime number.
 * @param q_str     A hexadecimal string representing the second prime number.
 *
 * @return int: Returns an integer status code indicating success (0) or error codes otherwise.
 *              It is assumed that error checking and handling mechanisms are in place
 *              elsewhere in the codebase to interpret these return values.
 *
 * Note:
 * - Ensure `ram_in_buf` has sufficient space before calling this function.
 * - The function assumes `p_str` and `q_str` represent valid prime numbers.
 * - Error handling is not detailed within this function; ensure external checks validate inputs.
 */
void sk_initialize(unsigned char* ram_in_buf, const char* p_str, const char* q_str, const char* h_str) {

    mpz_t P                 ;   mpz_init(P                  );  mpz_set_str(P,p_str,16);
    mpz_t Q                 ;   mpz_init(Q                  );  mpz_set_str(Q,q_str,16);
    mpz_t R                 ;   mpz_init(R                  );  mpz_setbit(R,2048);
    mpz_t P_1               ;   mpz_init(P_1                );  mpz_sub_ui(P_1,P,1);
    mpz_t Q_1               ;   mpz_init(Q_1                );  mpz_sub_ui(Q_1,Q,1);
    mpz_t LAMBDA            ;   mpz_init(LAMBDA             );  mpz_lcm(LAMBDA,P_1,Q_1);
    mpz_t N                 ;   mpz_init(N                  );  mpz_mul(N,P,Q);
    mpz_t N2                ;   mpz_init(N2                 );  mpz_mul(N2,N,N);
    mpz_t P2                ;   mpz_init(P2                 );  mpz_mul(P2,P,P);
    mpz_t Q2                ;   mpz_init(Q2                 );  mpz_mul(Q2,Q,Q);
    mpz_t R2                ;   mpz_init(R2                 );  mpz_mul(R2,R,R);
    mpz_t N_MOD_N2          ;   mpz_init(N_MOD_N2           );  mpz_set(N_MOD_N2  ,N); 
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
    mpz_t N_PRIME           ;   mpz_init(N_PRIME            );  mpz_invert(N_PRIME,N,R); mpz_sub(N_PRIME,R,N_PRIME);
    mpz_t N2_PRIME          ;   mpz_init(N2_PRIME           );  mpz_invert(N2_PRIME,N2,R2); mpz_sub(N2_PRIME,R2,N2_PRIME);
    mpz_t P2_PRIME          ;   mpz_init(P2_PRIME           );  mpz_invert(P2_PRIME,P2,R); mpz_sub(P2_PRIME,R,P2_PRIME);
    mpz_t Q2_PRIME          ;   mpz_init(Q2_PRIME           );  mpz_invert(Q2_PRIME,Q2,R); mpz_sub(Q2_PRIME,R,Q2_PRIME);
    mpz_t HS                ;   mpz_init(HS                 );  mpz_set_str(HS,h_str,16); 

    memset(ram_in_buf,0,8192);

    mpz2ram_buf(ram_in_buf+(RAM_IN_N_MOD_N2          << 4),N_MOD_N2         ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_R4_MOD_N2         << 4),R4_MOD_N2        ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_QQR2_MOD_N2       << 4),QQR2_MOD_N2      ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_PPR2_MOD_N2       << 4),PPR2_MOD_N2      ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_R2_MOD_N          << 4),R2_MOD_N         ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_R2_MOD_P2         << 4),R2_MOD_P2        ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_R2_MOD_Q2         << 4),R2_MOD_Q2        ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_LAMBDA_MOD_PHI_P2 << 4),LAMBDA_MOD_PHI_P2,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_LAMBDA_MOD_PHI_Q2 << 4),LAMBDA_MOD_PHI_Q2,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N_INV_MOD_R       << 4),N_INV_MOD_R      ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_RMU_INV_MOD_N     << 4),RMU_INV_MOD_N    ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_ONE               << 4),ONE              ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N2_PRIME          << 4),N2_PRIME         ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N_PRIME           << 4),N_PRIME          ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_P2_PRIME          << 4),P2_PRIME         ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_Q2_PRIME          << 4),Q2_PRIME         ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N2                << 4),N2               ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N                 << 4),N                ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_P2                << 4),P2               ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_Q2                << 4),Q2               ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_HS                << 4),HS               ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_ZERO              << 4),ZERO             ,1,256,SMALL_END);



    // FILE* fp = fopen("data/parameter_data.txt", "w");
    // gmp_fprintf(fp, "RAM_IN_NR2_MOD_N2        is %Zx\n",NR2_MOD_N2       );
    // gmp_fprintf(fp, "RAM_IN_R4_MOD_N2         is %Zx\n",R4_MOD_N2        );
    // gmp_fprintf(fp, "RAM_IN_QQR2_MOD_N2       is %Zx\n",QQR2_MOD_N2      );
    // gmp_fprintf(fp, "RAM_IN_PPR2_MOD_N2       is %Zx\n",PPR2_MOD_N2      );
    // gmp_fprintf(fp, "RAM_IN_R2_MOD_N          is %Zx\n",R2_MOD_N         );
    // gmp_fprintf(fp, "RAM_IN_R2_MOD_P2         is %Zx\n",R2_MOD_P2        );
    // gmp_fprintf(fp, "RAM_IN_R2_MOD_Q2         is %Zx\n",R2_MOD_Q2        );
    // gmp_fprintf(fp, "RAM_IN_LAMBDA_MOD_PHI_P2 is %Zx\n",LAMBDA_MOD_PHI_P2);
    // gmp_fprintf(fp, "RAM_IN_LAMBDA_MOD_PHI_P2 is %Zx\n",LAMBDA_MOD_PHI_P2);
    // gmp_fprintf(fp, "RAM_IN_N_INV_MOD_R       is %Zx\n",N_INV_MOD_R      );
    // gmp_fprintf(fp, "RAM_IN_RMU_INV_MOD_N     is %Zx\n",RMU_INV_MOD_N    );
    // gmp_fprintf(fp, "RAM_IN_ONE               is %Zx\n",ONE              );
    // gmp_fprintf(fp, "RAM_IN_N2                is %Zx\n",N2               );
    // gmp_fprintf(fp, "RAM_IN_N                 is %Zx\n",N                );
    // gmp_fprintf(fp, "RAM_IN_P2                is %Zx\n",P2               );
    // gmp_fprintf(fp, "RAM_IN_Q2                is %Zx\n",Q2               );
    // gmp_fprintf(fp, "RAM_IN_HS                is %Zx\n",HS               );
    // gmp_fprintf(fp, "RAM_IN_ZERO              is %Zx\n",ZERO             );

}

/**
 * Initializes public key parameters and buffers them into memory.
 *
 * This function computes various cryptographic values necessary for public key operations,
 * primarily using the provided prime number `n_str`. Computed elements include powers, modular inverses,
 * and related cryptographic components which are then stored in a buffer pointed to by `ram_in_buf`.
 * The buffer is first cleared before populating it with the computed values in little-end format.
 *
 * @param ram_in_buf A pointer to a buffer where computed public key parameters will be stored.
 *                  Must be allocated to a size of at least 8192 bytes and will be overwritten.
 * @param n_str     A hexadecimal string representing a prime number used in the computations.
 *
 * @return int: Returns an integer status code indicating success (typically 0).
 *              Specific error handling should be implemented by the caller if necessary.
 *
 * Note:
 * - Ensure `ram_in_buf` is adequately sized and can be safely overwritten.
 * - `n_str` must represent a valid prime number for the cryptographic computations to be correct.
 * - This function does not perform input validation; invalid inputs may lead to undefined behavior.
 */
void pk_initialize(unsigned char* ram_in_buf, const char* n_str, const char* h_str) {
    mpz_t R                 ;   mpz_init(R                  );  mpz_setbit(R,2048);
    mpz_t N                 ;   mpz_init(N                  );  mpz_set_str(N,n_str,16);
    mpz_t N2                ;   mpz_init(N2                 );  mpz_mul(N2,N,N);
    mpz_t R2                ;   mpz_init(R2                 );  mpz_mul(R2,R,R);
    mpz_t N_MOD_N2          ;   mpz_init(N_MOD_N2           );  mpz_set(N_MOD_N2  ,N); 
    mpz_t R4_MOD_N2         ;   mpz_init(R4_MOD_N2          );  mpz_mul(R4_MOD_N2,R2,R2); mpz_mod(R4_MOD_N2,R4_MOD_N2,N2);
    mpz_t R2_MOD_N          ;   mpz_init(R2_MOD_N           );  mpz_mod(R2_MOD_N,R2,N);
    mpz_t N_INV_MOD_R       ;   mpz_init(N_INV_MOD_R        );  mpz_invert(N_INV_MOD_R,N,R);

    mpz_t ZERO              ;   mpz_init(ZERO               );  mpz_set_ui(ZERO,0);
    mpz_t ONE               ;   mpz_init(ONE                );  mpz_set_ui(ONE,1);
    mpz_t PLAIN_ONE         ;   mpz_init(PLAIN_ONE          );  mpz_set_ui(PLAIN_ONE,1);

    mpz_t WORD              ;   mpz_init(WORD               );  mpz_setbit(WORD,128);
    mpz_t N_PRIME           ;   mpz_init(N_PRIME            );  mpz_invert(N_PRIME,N,R); mpz_sub(N_PRIME,R,N_PRIME);
    mpz_t N2_PRIME          ;   mpz_init(N2_PRIME           );  mpz_invert(N2_PRIME,N2,R2); mpz_sub(N2_PRIME,R2,N2_PRIME);
    mpz_t HS                ;   mpz_init(HS                 );  mpz_set_str(HS,h_str,16); 

    memset(ram_in_buf,0,8192);

    mpz2ram_buf(ram_in_buf+(RAM_IN_N_MOD_N2          << 4),N_MOD_N2         ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_R4_MOD_N2         << 4),R4_MOD_N2        ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_R2_MOD_N          << 4),R2_MOD_N         ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N_INV_MOD_R       << 4),N_INV_MOD_R      ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_ONE               << 4),ONE              ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N2_PRIME          << 4),N2_PRIME         ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N_PRIME           << 4),N_PRIME          ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N2                << 4),N2               ,1,512,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_N                 << 4),N                ,1,256,SMALL_END);
    mpz2ram_buf(ram_in_buf+(RAM_IN_HS                << 4),HS               ,1,512,SMALL_END);
    //for(int i = 0; i < 512; i ++){
    //    printf("%d : %x\n", i,*(ram_in_buf+(RAM_IN_HS                << 4) + i));
    //}
    mpz2ram_buf(ram_in_buf+(RAM_IN_ZERO              << 4),ZERO             ,1,256,SMALL_END);

}

