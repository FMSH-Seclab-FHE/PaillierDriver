
//
// Created by douwei lei on 2025-June-16.
//

#include "test__fmsh_paillierDriver.h"

void test_paillier_enc_ncrt_final(gmp_randstate_t grt, int data_len, const unsigned char * n, const unsigned char* hs){
    int i;
    int cnt = 0;
    unsigned char* m;
    unsigned char* r;
    unsigned char* c_fpga;
    unsigned char* c_right;
    int flag = 1;

    m =       (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);
    r =       (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);
    c_fpga  = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);
    c_right = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);

    if(m == NULL || r == NULL || c_fpga == NULL || c_right == NULL){
        perror("malloc failed");
        return;
    }

    int c_right_fd = open("/home/zhangchi/ldw_prj/fmsh_paillier/data/enc_ncrt_right.bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    int c_fpga_fd  = open("/home/zhangchi/ldw_prj/fmsh_paillier/data/enc_ncrt_fpga.bin",  O_RDWR | O_CREAT | O_TRUNC, 0666);

    if(c_right_fd < 0 || c_fpga_fd < 0){
        perror("open file failed");
        return;
    }

    //initial core
    paillier_enc_ncrt_inital(n, hs);

    //golden data
    mpz_t N             ;       mpz_init(N              );
    mpz_t res           ;       mpz_init(res            );
    mpz_t HS            ;       mpz_init(HS              );
    mpz_set_str(N, n, 16);
    mpz_set_str(HS ,hs, 16);
    mpz_t R                 ;   mpz_init(R                  );  mpz_setbit(R,2048);
    mpz_t N2                ;   mpz_init(N2                 );  mpz_mul(N2,N,N);
    mpz_t R2                ;   mpz_init(R2                 );  mpz_mul(R2,R,R);
    mpz_t lowest;               mpz_init(lowest);
    mpz_t plaintext;            mpz_init(plaintext);
    mpz_t randexp  ;            mpz_init(randexp); 
    mpz_t gm;                   mpz_init(gm); 
    for(i = 0; i < data_len; i++){
        gen_random(plaintext, 2048, grt);
        gen_random(randexp, 1024, grt);
        mpz2ram_buf(m + i * 512, plaintext, 1, 512,SMALL_END);
        mpz2ram_buf(r + i * 512, randexp, 1, 512,SMALL_END);
        mpz_powm(res,HS,randexp,N2);
        mpz_mul(gm,plaintext,N);mpz_add_ui(gm,gm,1);
        mpz_mul(res,res,gm); mpz_mod(res,res,N2);
        mpz2ram_buf(c_right + i * 512, res, 1, 512, SMALL_END);
    }

    //fpga computation
    paillier_enc_ncrt_final(c_fpga, m, r, data_len);
    for(i = 0; i < data_len * 512; i++){
        if(c_right[i] != c_fpga[i]){
            printf("right : %d. fpga: %d\n", c_right[i], c_fpga[i]);
            flag = 0;
        }
    }
    if(flag == 1){
        printf("test_paillier_enc_ncrt_final : right\n");
    }
    else{
        printf("test_paillier_enc_ncrt_final : wrong\n");
    }

    write(c_right_fd, c_right, data_len * 512);
    write(c_fpga_fd,  c_fpga,  data_len * 512);
    free(m);
    free(r);
    free(c_fpga);
    free(c_right);
    mpz_clear(N);
    mpz_clear(res);
    mpz_clear(HS);
    mpz_clear(R);
    mpz_clear(N2);
    mpz_clear(R2);
    mpz_clear(lowest);
    mpz_clear(plaintext);
    mpz_clear(randexp); 
    mpz_clear(gm); 
    close(c_right_fd);
    close(c_fpga_fd);
}

void test_paillier_dec_crt_final(gmp_randstate_t grt, int data_len, const unsigned char *p, const unsigned char *q, const unsigned char *n, const unsigned char *hs){
    int i;
    int cnt = 0;
    int flag = 1;
    unsigned char* m_right;
    unsigned char* m_fpga;
    unsigned char* c;

    m_right = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 256);
    m_fpga  = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 256);
    c       = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);
    if(m_right == NULL || m_fpga == NULL || c == NULL){
        printf("MALLOC ERROR\n");
        return;
    }

    int right_fd = open("/home/zhangchi/ldw_prj/fmsh_paillier/data/dec_crt_right.bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    int fpga_fd  = open("/home/zhangchi/ldw_prj/fmsh_paillier/data/dec_crt_fpga.bin",  O_RDWR | O_CREAT | O_TRUNC, 0666);

    if(right_fd < 0 || fpga_fd < 0){
        perror("open file failed");
        return;
    }

    //initial core
    paillier_dec_crt_inital(p, q, hs);

    //golden data
    mpz_t N  ;       mpz_init(N              );
    mpz_t res;       mpz_init(res            );
    mpz_t HS ;       mpz_init(HS              );
    mpz_set_str(N, n, 16);
    mpz_set_str(HS ,hs, 16);
    mpz_t R ;        mpz_init(R                  );  mpz_setbit(R,2048);
    mpz_t N2;        mpz_init(N2                 );  mpz_mul(N2,N,N);
    mpz_t R2;        mpz_init(R2                 );  mpz_mul(R2,R,R);
    mpz_t lowest;    mpz_init(lowest);
    mpz_t plaintext; mpz_init(plaintext);
    mpz_t randexp  ; mpz_init(randexp); 
    mpz_t gm;        mpz_init(gm); 
    for(i = 0; i < data_len; i++){
        gen_random(plaintext, 2048, grt);
        gen_random(randexp, 1024, grt);
        mpz_set_str(plaintext, "1", 10);
        mpz2ram_buf(m_right + i * 256, plaintext, 1, 256,SMALL_END);
        mpz_powm(res,HS,randexp,N2);
        mpz_mul(gm,plaintext,N);mpz_add_ui(gm,gm,1);
        mpz_mul(res,res,gm); mpz_mod(res,res,N2);
        mpz2ram_buf(c + i * 512, res, 1, 512,SMALL_END);
    }

    //fpga computation and compare
    paillier_dec_crt_final(m_fpga, c, data_len);
    for(i = 0; i < data_len * 256; i++){
        if(m_fpga[i] != m_right[i]){
            printf("right : %d. fpga: %d\n", m_right[i], m_fpga[i]);
            flag = 0;
        }
    }
    if(flag == 1){
        printf("test_paillier_dec_crt_final right\n");
    }
    else{
        printf("test_paillier_dec_crt_final wrong\n");
    }

    write(right_fd, m_right, data_len * 256);
    write(fpga_fd,  m_fpga,  data_len * 256);

    free(m_right);
    free(m_fpga);
    free(c);
    mpz_clear(N);
    mpz_clear(res);
    mpz_clear(HS);
    mpz_clear(R);
    mpz_clear(N2);
    mpz_clear(R2);
    mpz_clear(lowest);
    mpz_clear(plaintext);
    mpz_clear(randexp); 
    mpz_clear(gm); 
    close(right_fd);
    close(fpga_fd);
}

void test_paillier_scale_mul_final(gmp_randstate_t grt, int data_len, const unsigned char * n, const unsigned char* hs){
    int i;
    int cnt = 0;
    unsigned char* m;
    unsigned char* c_old;
    unsigned char* c_fpga;
    unsigned char* c_right;
    int flag = 1;

    m       = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 256);
    c_old   = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);
    c_fpga  = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);
    c_right = (unsigned char* )malloc(sizeof(unsigned char) * data_len * 512);

    if(m == NULL || c_old == NULL || c_fpga == NULL || c_right == NULL){
        perror("malloc failed");
        return;
    }

    int c_right_fd = open("/home/zhangchi/ldw_prj/fmsh_paillier/data/scale_mul_right.bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    int c_fpga_fd  = open("/home/zhangchi/ldw_prj/fmsh_paillier/data/scale_mul_fpga.bin",  O_RDWR | O_CREAT | O_TRUNC, 0666);

    if(c_right_fd < 0 || c_fpga_fd < 0){
        perror("open file failed");
        return;
    }

    //initial core
    paillier_scale_mul_inital(n, hs);

    //golden data
    mpz_t N             ;       mpz_init(N              );
    mpz_t res           ;       mpz_init(res            );
    mpz_t HS            ;       mpz_init(HS              );
    mpz_set_str(N, n, 16);
    mpz_set_str(HS ,hs, 16);
    mpz_t N2                ;   mpz_init(N2                 );  mpz_mul(N2,N,N);
    mpz_t plaintext         ;   mpz_init(plaintext);
    mpz_t cipher_old        ;   mpz_init(cipher_old); 
    for(i = 0; i < data_len; i++){
        //gen_random(plaintext,  2048, grt);
        //gen_random(cipher_old, 4096, grt);
        mpz_set_str(plaintext, "1", 16);
        mpz_set_str(cipher_old, "ff", 16);
        mpz2ram_buf(m     + i * 256, plaintext,  1, 256,SMALL_END);
        mpz2ram_buf(c_old + i * 512, cipher_old, 1, 512,SMALL_END);
        mpz_powm(res,cipher_old,plaintext,N2);
        mpz2ram_buf(c_right + i * 512, res, 1, 512, SMALL_END);
    }

    //fpga computation
    paillier_scale_mul_final(c_fpga, c_old, m, data_len);
    for(i = 0; i < data_len * 512; i++){
        if(c_right[i] != c_fpga[i]){
            printf("right : %d. fpga: %d\n", c_right[i], c_fpga[i]);
            flag = 0;
        }
    }
    if(flag == 1){
        printf("test_paillier_scale_mul_final : right\n");
    }
    else{
        printf("test_paillier_scale_mul_final : wrong\n");
    }

    write(c_right_fd, c_right, data_len * 512);
    write(c_fpga_fd,  c_fpga,  data_len * 512);
    free(m);
    free(c_old);
    free(c_fpga);
    free(c_right);
    mpz_clear(N);
    mpz_clear(res);
    mpz_clear(HS);
    mpz_clear(N2);
    mpz_clear(plaintext);
    mpz_clear(cipher_old); 
    close(c_right_fd);
    close(c_fpga_fd);
}

void test_paillier_windowsPath(void){
    unsigned char bytes_buffer[256 * ALL_BYTES_LEN_ENC_NCRT];
    unsigned char nothing[16] = {0};
    int i, j, device, rc;
    mpz_t one_word;
    mpz_t one;
    mpz_init(one_word);
    mpz_init(one);
    mpz_set_str(one_word, "0", 16);
    mpz_set_str(one,      "1", 16);
    device = open_host2FpgaChannel(1);
    if(device == -1){
        printf("[Error] paillier_encrypt_ncrt: Device open error\n");
        return;
    }
    for(i = 0; i < WINDOWS_NUM; i++){
        for(j = 0; j < 256 * 32; j++){
            mpz2ram_buf(bytes_buffer+ (j << 4), one_word, 1, 16, SMALL_END);
            mpz_add(one_word, one_word, one);
        }
        rc = write_from_host_to_fpga(device, ((uint64_t)DDR1_ADDR + ((uint64_t)i * 256ULL * ALL_BYTES_LEN_ENC_NCRT)), bytes_buffer, 256 * ALL_BYTES_LEN_ENC_NCRT);
        if(rc == -1) { 
            printf("write core bram_addr wrong\n\n");
        }
    }
    rc = write_from_host_to_fpga(device, clusteri_windows_path_config_addr[0], (unsigned char*)&ConfWindowPath_data_0, 32);
    if(rc == -1) { 
        printf("write core bram_addr wrong\n\n");
    }
    rc = write_from_host_to_fpga(device, clusteri_windows_path_config_addr[0] + (off_t)32, (unsigned char*)&ConfWindowPath_data_1, 32);
    if(rc == -1) { 
        printf("write core bram_addr wrong\n\n");
    }
    rc = write_from_host_to_fpga(device, clusteri_windows_path_start_addr[0], nothing, 16);
    if(rc == -1) { 
        printf("write core bram_addr wrong\n\n");
    }
}