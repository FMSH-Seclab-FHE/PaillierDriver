//
// Created by zhangchi on 24-11-27.
//

#ifndef PAILLIER_INTF_H
#define PAILLIER_INTF_H



#ifdef __cplusplus
extern "C"{
#endif

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
void paillier_InitWindowsPath(const char *n, const char *hs);
void paillier_enc_ncrt_inital(const char *n, const char *hs);
int paillier_enc_ncrt_final(unsigned char* c, const unsigned char* m, const unsigned char* r, int len);

void paillier_dec_crt_inital(const char *p, const char *q, const char *hs);
int paillier_dec_crt_final(unsigned char* m, const unsigned char* c, int len);

void paillier_scale_mul_inital(const char *n, const char *hs);
int paillier_scale_mul_final(unsigned char* c, const unsigned char* c_old, const unsigned char* m, int len);
#ifdef __cplusplus
}
#endif

#endif //PAILLIER_INTF_H
