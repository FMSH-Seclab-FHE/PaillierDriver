//
// Created by douwei lei on 2025-June-16.
//

#ifndef TEST_PAILLIER_DRIVER
#define TEST_PAILLIER_DRIVER

#include <gmp.h>
#include <stdio.h>
#include <fcntl.h>
#include "../src/paillier_intf.h"
#include "../src/data_process.h"
#include "../src/fpga_interface.h"
#include "gen.h"

#ifdef __cplusplus
extern "C"{
#endif

#define RESET_BUS

void test_paillier_enc_ncrt_final(gmp_randstate_t grt, int data_len, const unsigned char* n, const unsigned char* hs);

void test_paillier_dec_crt_final(gmp_randstate_t grt, int data_len, const unsigned char *p, const unsigned char *q, const unsigned char *n, const unsigned char *hs);

void test_paillier_scale_mul_final(gmp_randstate_t grt, int data_len, const unsigned char * n, const unsigned char* hs);

void test_paillier_windowsPath(void);

#ifdef __cplusplus
}
#endif

#endif //TEST_PAILLIER_DRIVER
