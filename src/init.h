//
// Created by zhangchi on 2024-November-26.
//

#ifndef INIT_H
#define INIT_H

#ifdef __cplusplus
extern "C"{
#endif

void sk_initialize(unsigned char* ram_in_buf, const char* p_str, const char* q_str, const char* h_str) ;
void pk_initialize(unsigned char* ram_in_buf, const char* n_str, const char* h_str) ;

#ifdef __cplusplus
}
#endif

#endif //INIT_H
