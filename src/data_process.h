//
// Created by zhangchi on 24-11-26.
//

#ifndef DATA_PROCESS_H
#define DATA_PROCESS_H
#include <gmp.h>
#define SMALL_END     0
#define BIG_END       1

#ifdef __cplusplus
extern "C"{
#endif

void mpz2ram_buf(unsigned char* buf, const mpz_t num, int buf_byte_size, int num_byte_len, int endian);
void mpz2fifo_buf(unsigned char* buf, unsigned int base, const mpz_t num, int buf_byte_size, int num_byte_len, int endian);

#ifdef __cplusplus
}
#endif

#endif //DATA_PROCESS_H
