//
// Created by zhangchi on 24-11-26.
//

#include "data_process.h"

#include <stdio.h>

void mpz2ram_buf(unsigned char* buf, const mpz_t num, int buf_byte_size, int num_byte_len, int end) {
    mpz_t data     ;    mpz_init(data);    mpz_set(data, num);
    mpz_t buf_data;    mpz_init(buf_data);
    int shift_len = buf_byte_size * 8;
    int buf_len = (num_byte_len + buf_byte_size - 1)/buf_byte_size;
    if (end == BIG_END) {
        for (int i = 0; i < buf_len; ++i) {
            mpz_mod_2exp(buf_data,data,shift_len);
            mpz_div_2exp(data,data,shift_len);
            buf[buf_len - i - 1] = mpz_get_ui(buf_data);
        }
    }
    else if (end == SMALL_END) {
        for (int i = 0; i < buf_len; ++i) {
            mpz_mod_2exp(buf_data,data,shift_len);
            mpz_div_2exp(data,data,shift_len);
            buf[i] = mpz_get_ui(buf_data);
        }
    }

}

void mpz2fifo_buf(unsigned char* buf, unsigned int base, const mpz_t num, int buf_byte_size, int num_byte_len, int end) {
    const int slot_byte = 16;
    const int slot_data_byte = 2;
    const int slot_base_byte = 2;
    const int slot_pad_byte = 12;
    mpz_t data    ;    mpz_init(data);    mpz_set(data, num);
    mpz_t buf_data;    mpz_init(buf_data);
    int shift_len = buf_byte_size * 8;
    int buf_len = (num_byte_len + buf_byte_size - 1)/buf_byte_size;
    int slot_len = buf_len/slot_data_byte;

    if (end == BIG_END) {
        for (int slot = 0; slot < slot_len; ++slot) {
            for (int i = 0; i < slot_data_byte; ++i) {
                mpz_mod_2exp(buf_data,data,shift_len);
                mpz_div_2exp(data,data,shift_len);
                buf[(slot_len - 1 - slot) * slot_byte + i] = mpz_get_ui(buf_data);
            }
            for (int i = 0; i < slot_base_byte; ++i) {
                buf[slot * slot_byte + slot_data_byte + i] = (base >> (8 * i)) & 0xFF;
            }
            for (int i = 0; i < slot_pad_byte; ++i) {
                buf[slot * slot_byte + slot_data_byte + slot_base_byte + i] = (base >> (8 * i)) & 0xFF;
            }
        }
    }
    else if (end == SMALL_END) {
        for (int slot = 0; slot < slot_len; ++slot) {
            for (int i = 0; i < slot_data_byte; ++i) {
                mpz_mod_2exp(buf_data,data,shift_len);
                mpz_div_2exp(data,data,shift_len);
                buf[slot * slot_byte + i] = mpz_get_ui(buf_data);
            }
            for (int i = 0; i < slot_base_byte; ++i) {
                buf[slot * slot_byte + slot_data_byte + i] = (base >> (8 * i)) & 0xFF;
            }
            for (int i = 0; i < slot_pad_byte; ++i) {
                buf[slot * slot_byte + slot_data_byte + slot_base_byte + i] = (base >> (8 * i)) & 0xFF;
            }
        }

    }
}