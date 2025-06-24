//
// Created by zhangchi on 24-11-27.
//

#include "paillier_intf.h"
#include "conf.h"
#include "data_process.h"
#include "fpga_interface.h"
#include "global_define.h"
#include "init.h"
#include "ram_addr_list.h"
void paillier_InitWindowsPath(const char *n, const char *hs){
  unsigned char bytes_buffer[256 * ALL_BYTES_LEN_ENC_NCRT];
  unsigned char nothing[16] = {0};
  int i, j, rc, host2Fpga;
  mpz_t HS_EXP            ;   mpz_init(HS_EXP             );  mpz_set_str(HS_EXP,hs,16);
  mpz_t N                 ;   mpz_init(N                  );  mpz_set_str(N, n, 16);
  mpz_t N2                ;   mpz_init(N2                 );  mpz_mul(N2,N,N);
  mpz_t DATA256           ;   mpz_init(DATA256            );  mpz_set_str(DATA256,"100",16);
  mpz_t TEMP              ;   mpz_init(TEMP               );  
  host2Fpga = open_host2FpgaChannel(1);
  if(host2Fpga == -1){
    printf("[Error] paillier_InitWindowsPath: Device open error\n");
      return;
  }
  //reset TEMP to 1
  mpz_set_str(TEMP, "1", 16);
  for(i = 0; i < WINDOWS_NUM; i++){
    for(j = 0; j < 256; j++){
      //transfer to bytes
      mpz2ram_buf(bytes_buffer + (j << 9), TEMP , 1, 512,SMALL_END);
      //mul
      mpz_mul(TEMP, TEMP, HS_EXP);
      mpz_mod(TEMP, TEMP, N2);
    }
    //reset TEMP to 1
    mpz_set_str(TEMP, "1", 16);
    //change the multiplier， HS_EXP = HS^（i << 8）
    mpz_powm(HS_EXP, HS_EXP, DATA256, N2);

    rc = write_from_host_to_fpga(host2Fpga, ((uint64_t)DDR1_ADDR + ((uint64_t)i * 256ULL * ALL_BYTES_LEN_ENC_NCRT)), bytes_buffer, 256 * ALL_BYTES_LEN_ENC_NCRT);
    if(rc == -1) { 
      printf("write core bram_addr wrong\n\n");
    }
  }
  rc = write_from_host_to_fpga(host2Fpga, clusteri_windows_path_config_addr[0], (unsigned char*)&ConfWindowPath_data_0, 32);
  if(rc == -1) { 
    printf("write core bram_addr wrong\n\n");
  }
  rc = write_from_host_to_fpga(host2Fpga, clusteri_windows_path_config_addr[0] + (off_t)32, (unsigned char*)&ConfWindowPath_data_1, 32);
  if(rc == -1) { 
    printf("write core bram_addr wrong\n\n");
  }
  rc = write_from_host_to_fpga(host2Fpga, clusteri_windows_path_start_addr[0], nothing, 16);
  if(rc == -1) { 
    printf("write core bram_addr wrong\n\n");
  }
  close_host2FpgaChannel(host2Fpga);
}
void paillier_enc_ncrt_inital(const char *n, const char *hs){
    unsigned char ram_in_buf[8192];
    int rc;
    unsigned char enc_ncrt_init_cfg[] = {
      0x01, 0x00,PAILLIER_PRMT_ADDR,PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
     
    int device = open_host2FpgaChannel(1);
    if(device == -1){
        printf("[Error] paillier_encrypt_ncrt: Device open error\n");
        return;
    }
    pk_initialize(ram_in_buf, n, hs);
    rc = write_from_host_to_fpga(device, cluster_core_broadcast_bram_addr, ram_in_buf, 8192);
    if(rc == -1) { 
        printf("write core bram_addr wrong\n\n");
    }
    write_from_host_to_fpga(device, cluster_core_broadcast_fifo_addr, enc_ncrt_init_cfg, 16);
    if(rc == -1) { 
        printf("write core fifo wrong\n\n");
    }
}
static void paillier_enc_ncrt_one_pingpong(unsigned char* c, const unsigned char* m, const unsigned char* r, int len, int last){   
    static int first_ping = 1, first_pong = 1;
    static int pingpong = PING;
    int i, j, k, len_now;
    int used_cluster = len / (int)CORE_NUMBER; 
    int last_cluster_used_core = len % (int)CORE_NUMBER;
    int device_fpga2host = open_fpga2HostChannel(0);
    int device_host2fpga = open_host2FpgaChannel(0);
    unsigned char enc_ncrt_cfg[] = {
      MODE_ENC_NCRT,          0x00,                PAILLIER_MODE_ADDR,          PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config mode
      PAILLIER_CIPHER_LENGTH, 0x00,                PAILLIER_LEN_ADDR,           PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config length
      TRIGGER_ADDR_ENC_NCRT,  TRIGGER_OFFSET_PING, PAILLIER_PING_TRIGGER_ADDR,  PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config thread & auto trigger
      TRIGGER_ADDR_ENC_NCRT,  TRIGGER_OFFSET_PONG, PAILLIER_PONG_TRIGGER_ADDR,  PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config thread & auto trigger
      PAILLIER_TRIGGER_START, 0x00,                PAILLIER_TRIGGER_START_ADDR, PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };//config trigger addr
    unsigned char enc_ncrt_end[] = {
      PAILLIER_TRIGGER_END,   0x00,                PAILLIER_TRIGGER_START_ADDR, PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };//config trigger addr
    int clusteri_len[CLUSTER_NUMBER], clusteri_number[CLUSTER_NUMBER], clusteri_group[CLUSTER_NUMBER], cluster_cnt;
    for(i = 0; i < CLUSTER_NUMBER; i++){
      clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
      clusteri_number[i] = CORE_NUMBER;
      clusteri_group[i] = i + 1;
    }
    cluster_cnt = CLUSTER_NUMBER;
    unsigned char zero[32] = {0};
    unsigned char state[32] = {0};

    //start
    if((first_ping == 1) && (first_pong == 1)){
      write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, enc_ncrt_cfg, 80);
    }
    //ping
    if(pingpong == PING){
      //full use
      if(used_cluster == CLUSTER_NUMBER){
        //first use
        if(first_ping == 1){
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            conf_cluster_result_path(
              (off_t)clusteri_result_path_conf_pingpong_addr[i][0], 
              clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                m + i * CORE_NUMBER * 512 + j * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PING << 4), 
                r + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_ping = 0;
          pingpong = PONG;
        }
        //not first use
        else{
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                m + i * CORE_NUMBER * 512 + j * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PING << 4), 
                r + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_ping = 0;
          pingpong = PONG;
        }
        //last computation, read result
        if(last == 1){
          //wait all computation over
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          //read data
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 1);

          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, enc_ncrt_end, 16);

          //reset
          //add the result_path reset.
          first_ping = 1; 
          first_pong = 1;
          pingpong = PING;
        }
      }
      //not full use
      else{
        if(last_cluster_used_core != 0) used_cluster += 1;
        j = 1;
        len_now = len;
        for(i = CLUSTER_NUMBER - used_cluster; i < CLUSTER_NUMBER; i++){
          clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
          clusteri_number[i] = len_now >= CORE_NUMBER ? CORE_NUMBER : len_now;
          clusteri_group[i] = j;
          printf("PING : clusteri_len[%d], %d, clusteri_number[%d], %d,clusteri_group[%d], %d\n", 
          i, clusteri_len[i], i, clusteri_number[i], i, clusteri_group[i]);
          
          j += 1;
          len_now -= CORE_NUMBER;
        }
        cluster_cnt = used_cluster;
        if(first_ping != 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        }
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 0);
        k = 0;
        for(i = 0; i < CLUSTER_NUMBER; i++){
          conf_cluster_result_path(
            (off_t)clusteri_result_path_conf_pingpong_addr[i][0], 
            clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
          if(i >= CLUSTER_NUMBER - used_cluster){
            for(j = 0; j < clusteri_number[i]; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                m + k * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PING << 4), 
                r + k * 512, 512);
              k += 1;
            }
          }
        }
        do{
          read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
        }while(state[0] != 0xff);
        //read data
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PING_ADDR, CONF_CONTROL_STATE_PING_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 1);
        write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, enc_ncrt_end, 16);
        first_ping = 1; 
        first_pong = 1;
        pingpong = PING;
      }
    }
    //pong
    else{
      //full use
      if(used_cluster == CLUSTER_NUMBER){
        //first use
        if(first_pong == 1){
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            conf_cluster_result_path(
              (off_t)clusteri_result_path_conf_pingpong_addr[i][1], 
              clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
                m + i * CORE_NUMBER * 512 + j * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PONG << 4), 
                r + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_pong = 0;
          pingpong = PING;
        }
        //not first use
        else{
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, (off_t)CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            for(j = 0; j < CORE_NUMBER; j++){
            write_from_host_to_fpga(
              device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
              m + i * CORE_NUMBER * 512 + j * 512, 512);
            write_from_host_to_fpga(
              device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PONG << 4), 
              r + i * CORE_NUMBER * 512 + j * 512, 512);  
            }
          }
          first_pong = 0;
          pingpong = PING;
        }
        //last computation, read result
        if(last == 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          
          //read data
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 1);

          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, enc_ncrt_end, 16);
          //reset
          //add the result_path reset.
          first_ping = 1; 
          first_pong = 1;
          pingpong = PING;
        }
      }
      //not full use
      else{
        if(last_cluster_used_core != 0) used_cluster += 1;
        j = 1;
        len_now = len;
        for(i = CLUSTER_NUMBER - used_cluster; i < CLUSTER_NUMBER; i++){
          clusteri_len[i] = NUMBER_OF_WORD_ENC_NCRT;
          clusteri_number[i] = len_now >= CORE_NUMBER ? CORE_NUMBER : len_now;
          clusteri_group[i] = j;
          printf("PONG : clusteri_len[%d], %d, clusteri_number[%d], %d,clusteri_group[%d], %d\n", 
          i, clusteri_len[i], i, clusteri_number[i], i, clusteri_group[i]);
          
          j += 1;
          len_now -= CORE_NUMBER;
        }
        cluster_cnt = used_cluster;
        if(first_pong != 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        }
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PONG_ADDR, (off_t)CONF_CONTROL_STATE_PONG_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 0);
        k = 0;
        for(i = 0; i < CLUSTER_NUMBER; i++){
          conf_cluster_result_path(
            (off_t)clusteri_result_path_conf_pingpong_addr[i][1], 
            clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
          if(i >= CLUSTER_NUMBER - used_cluster){
            for(j = 0; j < clusteri_number[i]; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
                m + k * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PONG << 4), 
                r + k * 512, 512);
              k += 1;
            }
          }
        }
        do{
          read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
        }while(state[0] != 0xff);
        //read data
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 1);
        write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, enc_ncrt_end, 16);
        first_ping = 1; 
        first_pong = 1;
        pingpong = PING;
      }
    }
  close(device_fpga2host);
  close(device_host2fpga);

  return;
}
int paillier_enc_ncrt_final(unsigned char* c, const unsigned char* m, const unsigned char* r, int len){
  int host2fpga;
  int all_core_number = (int)CLUSTER_NUMBER * (int)CORE_NUMBER;
  int remainder = len % all_core_number;
  int cycle = len / all_core_number;
  int i, j;
  unsigned char nothing[32] = { 0 };
  int last = 0;

  int fd = open("testfile.txt", O_WRONLY | O_CREAT, 0666);
  if (fd == -1) {
      perror("open");
      return -1;
  }

  struct flock lock;
  lock.l_type = F_WRLCK;   // set write lock
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // lock whole file

  //unlock
  lock.l_type = F_UNLCK;
  if (fcntl(fd, F_SETLK, &lock) == -1) {
      perror("fcntl unlock");
      return -1;
  }

  //lock
  if (fcntl(fd, F_SETLK, &lock) == -1) {
      perror("fcntl lock");
      printf("File has already locked.\n");
      close(fd);
      return -1;
  }

  printf("cycle: %d, remainder: %d\n", cycle, remainder);

  for(i = 0; i < cycle; i++){
    if(remainder == 0 && i == (cycle - 1)) last = 1;
    printf("begin cycle: %d\n", i);
    paillier_enc_ncrt_one_pingpong(c, m + 512 * i * all_core_number, r + 512 * i * all_core_number, all_core_number, last);
    if(last == 1){
      host2fpga = open_host2FpgaChannel(1);
      write_from_host_to_fpga(host2fpga, CONF_CONTROL_RESET_PINGPONG_ADDR, nothing, 32);
      close_host2FpgaChannel(host2fpga);
    }
  }
  if(remainder != 0){
    last  = 1;
    printf("not full\n");
    paillier_enc_ncrt_one_pingpong(c, m + 512 * i * all_core_number, r + 512 * i * all_core_number, remainder, last);
    host2fpga = open_host2FpgaChannel(1);
    write_from_host_to_fpga(host2fpga, CONF_CONTROL_RESET_PINGPONG_ADDR, nothing, 32);
    close_host2FpgaChannel(host2fpga);
  }

  //unlock
  lock.l_type = F_UNLCK;
  if (fcntl(fd, F_SETLK, &lock) == -1) {
      perror("fcntl unlock");
      return -1;
  }

  close(fd);
}

void paillier_dec_crt_inital(const char *p, const char *q, const char *hs){
  unsigned char ram_in_buf[8192];
  unsigned char fifo_in_buf[8192];
  int rc;
  unsigned char dec_crt_init_cfg[] = {
    0x01, 0x00,PAILLIER_PRMT_ADDR,PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  int device = open_host2FpgaChannel(1);
  if(device == -1){
      printf("[Error] paillier_encrypt_ncrt: Device open error\n");
      return;
  }
  sk_initialize(ram_in_buf, p, q, hs);
  rc = write_from_host_to_fpga(device, cluster_core_broadcast_bram_addr, ram_in_buf, 8192);
  if(rc == -1) { 
      printf("write core bram_addr wrong\n\n");
  }
  rc = write_from_host_to_fpga(device, cluster_core_broadcast_fifo_addr, dec_crt_init_cfg, 16);
  if(rc == -1) { 
      printf("write core fifo wrong\n\n");
  }
}
static void paillier_dec_crt_one_pingpong(unsigned char* m, const unsigned char* c, int len, int last)
{   
    static int first_ping = 1, first_pong = 1;
    static int pingpong = PING;
    int i, j, k, len_now;
    int used_cluster = len / (int)CORE_NUMBER; 
    int last_cluster_used_core = len % (int)CORE_NUMBER;
    int device_fpga2host = open_fpga2HostChannel(0);
    int device_host2fpga = open_host2FpgaChannel(0);
    unsigned char dec_crt_cfg[] = {
      MODE_DEC_CRT,           0x00,                 PAILLIER_MODE_ADDR,          PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config mode
      PAILLIER_CIPHER_LENGTH, 0x00,                 PAILLIER_LEN_ADDR,           PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config length
      TRIGGER_ADDR_DEC_CRT,   TRIGGER_OFFSET_PING,  PAILLIER_PING_TRIGGER_ADDR,  PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config thread & auto trigger
      TRIGGER_ADDR_DEC_CRT,   TRIGGER_OFFSET_PONG,  PAILLIER_PONG_TRIGGER_ADDR,  PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config thread & auto trigger
      PAILLIER_TRIGGER_START, 0x00,                 PAILLIER_TRIGGER_START_ADDR, PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };//config trigger addr
    unsigned char dec_crt_end[] = {
      PAILLIER_TRIGGER_END,   0x00,                 PAILLIER_TRIGGER_START_ADDR, PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };//config trigger addr
    int clusteri_len[CLUSTER_NUMBER], clusteri_number[CLUSTER_NUMBER], clusteri_group[CLUSTER_NUMBER], cluster_cnt;
    for(i = 0; i < CLUSTER_NUMBER; i++){
      clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
      clusteri_number[i] = CORE_NUMBER;
      clusteri_group[i] = i + 1;
    }
    cluster_cnt = CLUSTER_NUMBER;
    unsigned char zero[32] = {0};
    unsigned char state[32] = {0};

    //start
    if((first_ping == 1) && (first_pong == 1)){
      write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, dec_crt_cfg, 80);
    }
    //ping
    if(pingpong == PING){
      //full use
      if(used_cluster == CLUSTER_NUMBER){
        //first use
        if(first_ping == 1){
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, m, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            conf_cluster_result_path(
              (off_t)clusteri_result_path_conf_pingpong_addr[i][0], 
              clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                c + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_ping = 0;
          pingpong = PONG;
        }
        //not first use
        else{
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, m, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                c + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_ping = 0;
          pingpong = PONG;
        }
        //last computation, read result
        if(last == 1){
          //wait all computation over
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          //read data
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, m, 1);

          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, dec_crt_end, 16);

          //reset
          //add the result_path reset.
          first_ping = 1; 
          first_pong = 1;
          pingpong = PING;
        }
      }
      //not full use
      else{
        if(last_cluster_used_core != 0) used_cluster += 1;
        j = 1;
        len_now = len;
        for(i = CLUSTER_NUMBER - used_cluster; i < CLUSTER_NUMBER; i++){
          clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
          clusteri_number[i] = len_now >= CORE_NUMBER ? CORE_NUMBER : len_now;
          clusteri_group[i] = j;
          printf("PING : clusteri_len[%d], %d, clusteri_number[%d], %d,clusteri_group[%d], %d\n", 
          i, clusteri_len[i], i, clusteri_number[i], i, clusteri_group[i]);
          
          j += 1;
          len_now -= CORE_NUMBER;
        }
        cluster_cnt = used_cluster;
        if(first_ping != 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        }
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, m, 0);
        k = 0;
        for(i = 0; i < CLUSTER_NUMBER; i++){
          conf_cluster_result_path(
            (off_t)clusteri_result_path_conf_pingpong_addr[i][0], 
            clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
          if(i >= CLUSTER_NUMBER - used_cluster){
            for(j = 0; j < clusteri_number[i]; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                c + k * 512, 512);
              k += 1;
            }
          }
        }
        do{
          read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
        }while(state[0] != 0xff);
        //read data
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PING_ADDR, CONF_CONTROL_STATE_PING_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, m, 1);
        write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, dec_crt_end, 16);
        first_ping = 1; 
        first_pong = 1;
        pingpong = PING;
      }
    }
    //pong
    else{
      //full use
      if(used_cluster == CLUSTER_NUMBER){
        //first use
        if(first_pong == 1){
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, m, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            conf_cluster_result_path(
              (off_t)clusteri_result_path_conf_pingpong_addr[i][1], 
              clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
                c + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_pong = 0;
          pingpong = PING;
        }
        //not first use
        else{
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, (off_t)CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, m, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            for(j = 0; j < CORE_NUMBER; j++){
            write_from_host_to_fpga(
              device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
              c + i * CORE_NUMBER * 512 + j * 512, 512);
            }
          }
          first_pong = 0;
          pingpong = PING;
        }
        //last computation, read result
        if(last == 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          
          //read data
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, m, 1);

          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, dec_crt_end, 16);
          //reset
          //add the result_path reset.
          first_ping = 1; 
          first_pong = 1;
          pingpong = PING;
        }
      }
      //not full use
      else{
        if(last_cluster_used_core != 0) used_cluster += 1;
        j = 1;
        len_now = len;
        for(i = CLUSTER_NUMBER - used_cluster; i < CLUSTER_NUMBER; i++){
          clusteri_len[i] = NUMBER_OF_WORD_DEC_CRT;
          clusteri_number[i] = len_now >= CORE_NUMBER ? CORE_NUMBER : len_now;
          clusteri_group[i] = j;
          printf("PONG : clusteri_len[%d], %d, clusteri_number[%d], %d,clusteri_group[%d], %d\n", 
          i, clusteri_len[i], i, clusteri_number[i], i, clusteri_group[i]);
          
          j += 1;
          len_now -= CORE_NUMBER;
        }
        cluster_cnt = used_cluster;
        if(first_pong != 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        }
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PONG_ADDR, (off_t)CONF_CONTROL_STATE_PONG_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, m, 0);
        k = 0;
        for(i = 0; i < CLUSTER_NUMBER; i++){
          conf_cluster_result_path(
            (off_t)clusteri_result_path_conf_pingpong_addr[i][1], 
            clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
          if(i >= CLUSTER_NUMBER - used_cluster){
            for(j = 0; j < clusteri_number[i]; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
                c + k * 512, 512);
              k += 1;
            }
          }
        }
        do{
          read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
        }while(state[0] != 0xff);
        //read data
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, m, 1);
        write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, dec_crt_end, 16);
        first_ping = 1; 
        first_pong = 1;
        pingpong = PING;
      }
    }
  close(device_fpga2host);
  close(device_host2fpga);
  return;
}
int paillier_dec_crt_final(unsigned char* m, const unsigned char* c, int len){
  int host2fpga;
  int all_core_number = (int)CLUSTER_NUMBER * (int)CORE_NUMBER;
  int remainder = len % all_core_number;
  int cycle = len / all_core_number;
  int i, j;
  unsigned char nothing[32] = { 0 };
  int last = 0;

  printf("cycle: %d, remainder: %d\n", cycle, remainder);
  for(i = 0; i < cycle; i++){
    if(remainder == 0 && i == (cycle - 1)) last = 1;
    printf("begin cycle: %d\n", i);
    paillier_dec_crt_one_pingpong(m, c + 512 * i * all_core_number, all_core_number, last);
    if(last == 1){
      host2fpga = open_host2FpgaChannel(1);
      write_from_host_to_fpga(host2fpga, CONF_CONTROL_RESET_PINGPONG_ADDR, nothing, 32);
      close_host2FpgaChannel(host2fpga);
    }
  }
  if(remainder != 0){
    last  = 1;
    printf("not full\n");
    paillier_dec_crt_one_pingpong(m, c + 512 * i * all_core_number, remainder, last);
    host2fpga = open_host2FpgaChannel(1);
    write_from_host_to_fpga(host2fpga, CONF_CONTROL_RESET_PINGPONG_ADDR, nothing, 32);
    close_host2FpgaChannel(host2fpga);
  }
}

void paillier_scale_mul_inital(const char *n, const char *hs){
    unsigned char ram_in_buf[8192];
    int rc;
    unsigned char enc_ncrt_init_cfg[] = {
      0x01, 0x00,PAILLIER_PRMT_ADDR,PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
     
    int device = open_host2FpgaChannel(1);
    if(device == -1){
        printf("[Error] paillier_encrypt_ncrt: Device open error\n");
        return;
    }
    pk_initialize(ram_in_buf, n, hs);
    rc = write_from_host_to_fpga(device, cluster_core_broadcast_bram_addr, ram_in_buf, 8192);
    if(rc == -1) { 
        printf("write core bram_addr wrong\n\n");
    }
    write_from_host_to_fpga(device, cluster_core_broadcast_fifo_addr, enc_ncrt_init_cfg, 16);
    if(rc == -1) { 
        printf("write core fifo wrong\n\n");
    }
}
static void paillier_scale_mul_one_pingpong(unsigned char* c, const unsigned char* c_old, const unsigned char* m, int len, int last){   
    static int first_ping = 1, first_pong = 1;
    static int pingpong = PING;
    int i, j, k, len_now;
    int used_cluster = len / (int)CORE_NUMBER; 
    int last_cluster_used_core = len % (int)CORE_NUMBER;
    int device_fpga2host = open_fpga2HostChannel(0);
    int device_host2fpga = open_host2FpgaChannel(0);
    unsigned char scale_mul_cfg[] = {
      MODE_SCALE_MUL,         0x00,                PAILLIER_MODE_ADDR,          PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config mode
      PAILLIER_CIPHER_LENGTH, 0x00,                PAILLIER_LEN_ADDR,           PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config length
      TRIGGER_ADDR_SCALE_MUL, TRIGGER_OFFSET_PING, PAILLIER_PING_TRIGGER_ADDR,  PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config thread & auto trigger
      TRIGGER_ADDR_SCALE_MUL, TRIGGER_OFFSET_PONG, PAILLIER_PONG_TRIGGER_ADDR,  PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //config thread & auto trigger
      PAILLIER_TRIGGER_START, 0x00,                PAILLIER_TRIGGER_START_ADDR, PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };//config trigger addr
    unsigned char scale_mul_end[] = {
      PAILLIER_TRIGGER_END,   0x00,                PAILLIER_TRIGGER_START_ADDR, PAILLIER_REGSET_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };//config trigger addr
    int clusteri_len[CLUSTER_NUMBER], clusteri_number[CLUSTER_NUMBER], clusteri_group[CLUSTER_NUMBER], cluster_cnt;
    for(i = 0; i < CLUSTER_NUMBER; i++){
      clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
      clusteri_number[i] = CORE_NUMBER;
      clusteri_group[i] = i + 1;
    }
    cluster_cnt = CLUSTER_NUMBER;
    unsigned char zero[32] = {0};
    unsigned char state[32] = {0};

    //start
    if((first_ping == 1) && (first_pong == 1)){
      write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, scale_mul_cfg, 80);
    }
    //ping
    if(pingpong == PING){
      //full use
      if(used_cluster == CLUSTER_NUMBER){
        //first use
        if(first_ping == 1){
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            conf_cluster_result_path(
              (off_t)clusteri_result_path_conf_pingpong_addr[i][0], 
              clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                c_old + i * CORE_NUMBER * 512 + j * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PING << 4), 
                m + i * CORE_NUMBER * 256 + j * 256, 256);
            }
          }
          first_ping = 0;
          pingpong = PONG;
        }
        //not first use
        else{
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                c_old + i * CORE_NUMBER * 512 + j * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PING << 4), 
                m + i * CORE_NUMBER * 256 + j * 256, 256);
            }
          }
          first_ping = 0;
          pingpong = PONG;
        }
        //last computation, read result
        if(last == 1){
          //wait all computation over
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          //read data
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 1);

          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, scale_mul_end, 16);

          //reset
          //add the result_path reset.
          first_ping = 1; 
          first_pong = 1;
          pingpong = PING;
        }
      }
      //not full use
      else{
        if(last_cluster_used_core != 0) used_cluster += 1;
        j = 1;
        len_now = len;
        for(i = CLUSTER_NUMBER - used_cluster; i < CLUSTER_NUMBER; i++){
          clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
          clusteri_number[i] = len_now >= CORE_NUMBER ? CORE_NUMBER : len_now;
          clusteri_group[i] = j;
          printf("PING : clusteri_len[%d], %d, clusteri_number[%d], %d,clusteri_group[%d], %d\n", 
          i, clusteri_len[i], i, clusteri_number[i], i, clusteri_group[i]);
          
          j += 1;
          len_now -= CORE_NUMBER;
        }
        cluster_cnt = used_cluster;
        if(first_ping != 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        }
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PING_ADDR, (off_t)CONF_CONTROL_STATE_PING_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 0);
        k = 0;
        for(i = 0; i < CLUSTER_NUMBER; i++){
          conf_cluster_result_path(
            (off_t)clusteri_result_path_conf_pingpong_addr[i][0], 
            clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
          if(i >= CLUSTER_NUMBER - used_cluster){
            for(j = 0; j < clusteri_number[i]; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PING << 4), 
                c_old + k * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PING << 4), 
                m + k * 256, 256);
              k += 1;
            }
          }
        }
        do{
          read_from_fpga_to_host(device_fpga2host, STATE_PING, state, 32); 
        }while(state[0] != 0xff);
        //read data
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PING_ADDR, CONF_CONTROL_STATE_PING_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 1);
        write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, scale_mul_end, 16);
        first_ping = 1; 
        first_pong = 1;
        pingpong = PING;
      }
    }
    //pong
    else{
      //full use
      if(used_cluster == CLUSTER_NUMBER){
        //first use
        if(first_pong == 1){
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            conf_cluster_result_path(
              (off_t)clusteri_result_path_conf_pingpong_addr[i][1], 
              clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
            for(j = 0; j < CORE_NUMBER; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
                c_old + i * CORE_NUMBER * 512 + j * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PONG << 4), 
                m + i * CORE_NUMBER * 256 + j * 256, 256);
            }
          }
          first_pong = 0;
          pingpong = PING;
        }
        //not first use
        else{
          for(i = 0; i < CLUSTER_NUMBER; i++){
            clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
            clusteri_number[i] = CORE_NUMBER;
            clusteri_group[i] = i + 1;
          }
          cluster_cnt = CLUSTER_NUMBER;
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, (off_t)CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 0);
          for(i = 0; i < CLUSTER_NUMBER; i++){
            for(j = 0; j < CORE_NUMBER; j++){
            write_from_host_to_fpga(
              device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
              c_old + i * CORE_NUMBER * 512 + j * 512, 512);
            write_from_host_to_fpga(
              device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PONG << 4), 
              m + i * CORE_NUMBER * 256 + j * 256, 256);  
            }
          }
          first_pong = 0;
          pingpong = PING;
        }
        //last computation, read result
        if(last == 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          
          //read data
          conf_control(
            (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
            clusteri_len, clusteri_number, cluster_cnt, c, 1);

          write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
          write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, scale_mul_end, 16);
          //reset
          //add the result_path reset.
          first_ping = 1; 
          first_pong = 1;
          pingpong = PING;
        }
      }
      //not full use
      else{
        if(last_cluster_used_core != 0) used_cluster += 1;
        j = 1;
        len_now = len;
        for(i = CLUSTER_NUMBER - used_cluster; i < CLUSTER_NUMBER; i++){
          clusteri_len[i] = NUMBER_OF_WORD_SCALE_MUL;
          clusteri_number[i] = len_now >= CORE_NUMBER ? CORE_NUMBER : len_now;
          clusteri_group[i] = j;
          printf("PONG : clusteri_len[%d], %d, clusteri_number[%d], %d,clusteri_group[%d], %d\n", 
          i, clusteri_len[i], i, clusteri_number[i], i, clusteri_group[i]);
          
          j += 1;
          len_now -= CORE_NUMBER;
        }
        cluster_cnt = used_cluster;
        if(first_pong != 1){
          do{
            read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
          }while(state[0] != 0xff);
          write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        }
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PONG_ADDR, (off_t)CONF_CONTROL_STATE_PONG_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 0);
        k = 0;
        for(i = 0; i < CLUSTER_NUMBER; i++){
          conf_cluster_result_path(
            (off_t)clusteri_result_path_conf_pingpong_addr[i][1], 
            clusteri_len[i], clusteri_number[i], clusteri_group[i], clusteri_len, clusteri_number);
          if(i >= CLUSTER_NUMBER - used_cluster){
            for(j = 0; j < clusteri_number[i]; j++){
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP1_PONG << 4), 
                c_old + k * 512, 512);
              write_from_host_to_fpga(
                device_host2fpga, clusteri_corei_bram_addr[i][j] + (RAM_IN_OP2_PONG << 4), 
                m + k * 256, 256);
              k += 1;
            }
          }
        }
        do{
          read_from_fpga_to_host(device_fpga2host, STATE_PONG, state, 32); 
        }while(state[0] != 0xff);
        //read data
        conf_control(
          (off_t)CONF_CONTROL_RESULT_PONG_ADDR, CONF_CONTROL_STATE_PONG_ADDR,
          clusteri_len, clusteri_number, cluster_cnt, c, 1);
        write_from_host_to_fpga(device_host2fpga, STATE_PING, zero, 32);
        write_from_host_to_fpga(device_host2fpga, STATE_PONG, zero, 32);
        write_from_host_to_fpga(device_host2fpga, cluster_core_broadcast_fifo_addr, scale_mul_end, 16);
        first_ping = 1; 
        first_pong = 1;
        pingpong = PING;
      }
    }
  close(device_fpga2host);
  close(device_host2fpga);

  return;
}
int paillier_scale_mul_final(unsigned char* c, const unsigned char* c_old, const unsigned char* m, int len){
  int host2fpga;
  int all_core_number = (int)CLUSTER_NUMBER * (int)CORE_NUMBER;
  int remainder = len % all_core_number;
  int cycle = len / all_core_number;
  int i, j;
  unsigned char nothing[32] = { 0 };
  int last = 0;

  printf("cycle: %d, remainder: %d\n", cycle, remainder);
  for(i = 0; i < cycle; i++){
    if(remainder == 0 && i == (cycle - 1)) last = 1;
    printf("begin cycle: %d\n", i);
    paillier_scale_mul_one_pingpong(c, c_old + 512 * i * all_core_number, m + 256 * i * all_core_number, all_core_number, last);
    if(last == 1){
      host2fpga = open_host2FpgaChannel(1);
      write_from_host_to_fpga(host2fpga, CONF_CONTROL_RESET_PINGPONG_ADDR, nothing, 32);
      close_host2FpgaChannel(host2fpga);
    }
  }
  if(remainder != 0){
    last  = 1;
    printf("not full\n");
    paillier_scale_mul_one_pingpong(c, c_old + 512 * i * all_core_number, m + 256 * i * all_core_number, remainder, last);
    host2fpga = open_host2FpgaChannel(1);
    write_from_host_to_fpga(host2fpga, CONF_CONTROL_RESET_PINGPONG_ADDR, nothing, 32);
    close_host2FpgaChannel(host2fpga);
  }
}



