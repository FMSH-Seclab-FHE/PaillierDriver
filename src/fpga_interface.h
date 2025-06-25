//
// Created by zhangchi, douwei lei on 2024-November-26.
//

#ifndef FPGA_INTERFACE_H
#define FPGA_INTERFACE_H
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "global_define.h"

extern long long int clusteri_result_path_conf_pingpong_addr[2][2];
extern unsigned int ddr_half_limit;  //don't overflow 32 bits
extern off_t clusteri_corei_bram_addr[3][16];
extern off_t cluster_core_broadcast_bram_addr;
extern off_t clusteri_corei_fifo_addr[3][16];
extern off_t cluster_core_broadcast_fifo_addr;
extern off_t clusteri_windows_path_start_addr[3];
extern off_t clusteri_windows_path_config_addr[3];


typedef struct {
    //128bit 0
    uint8_t  coreReqNum;     // Byte   0
    uint8_t  nextReqNum;     // Byte   1
    uint16_t nextWdsLen;     // Byte   2~3
    uint16_t startAddr;      // Byte   4~5
    uint8_t  reserved0[10];  // Byte   6~15

    //128bit 1 
    uint32_t axiAddrMask;    // Byte   0~3
    uint32_t axiAddrAdd;     // Byte   4~7
    uint32_t axiAddrLow;     // Byte   8~11
    uint8_t  reserved1[4];   // Byte  12~15
} __attribute__((packed)) ConfWindowPath;
extern ConfWindowPath ConfWindowPath_data_0;
extern ConfWindowPath ConfWindowPath_data_1;

#define PING 0
#define PONG 1
#define WORD_BYTES_LEN 16

#define NUMBER_OF_WORD_ENC_NCRT 32
#define ALL_BYTES_LEN_ENC_NCRT (WORD_BYTES_LEN * NUMBER_OF_WORD_ENC_NCRT)

#define NUMBER_OF_WORD_DEC_CRT 16
#define ALL_BYTES_LEN_DEC_CRT (WORD_BYTES_LEN * NUMBER_OF_WORD_DEC_CRT)

#define NUMBER_OF_WORD_SCALE_MUL 32
#define ALL_BYTES_LEN_SCALE_MUL (WORD_BYTES_LEN * NUMBER_OF_WORD_SCALE_MUL)

#define R_BIT_LEN 1024
#define WINDOWS_NUM (R_BIT_LEN >> 3)

#define CLUSTER0_BRAM_ADDR                  0x0000000000000000ULL
#define CLUSTER0_FIFO_ADDR                  0x0000000000400000ULL
#define CLUSTER0_RESULT_PATH_CONF_PING_ADDR 0x0000000000800000ULL
#define CLUSTER0_RESULT_PATH_CONF_PONG_ADDR 0x0000000000800400ULL
#define CLUSTER1_RESULT_PATH_CONF_PING_ADDR 0x0000000001800000ULL
#define CLUSTER1_RESULT_PATH_CONF_PONG_ADDR 0x0000000001800400ULL


#define CLUSTER_NUMBER                      1
#define CORE_NUMBER                         1
#define CONF_CONTROL_RESULT_PING_ADDR       0x0000000100000000ULL
#define CONF_CONTROL_RESULT_PONG_ADDR       0x0000000100001000ULL
#define CONF_CONTROL_STATE_PING_ADDR        0x0000000100010000ULL
#define CONF_CONTROL_STATE_PONG_ADDR        0x0000000100020000ULL
#define CONF_CONTROL_RESET_PINGPONG_ADDR    0x00000000ff810000ULL
#define CONF_CONTROL_RESET_CORE_ADDR        0x0000000100040000ULL
#define DDR0_ADDR                           0x0000000200000000ULL
#define DDR1_ADDR                           0x0000000300000000ULL
#define STATE_PING                          0x0000000400000000ULL
#define STATE_PONG                          0x0000000400000020ULL
#define BRAM_CONTROL_BRAM2AXI               0x0000000500000000ULL

#define PAILLIER_PADDING                    0x00
#define PAILLIER_REGSET_ADDR                0x02
#define PAILLIER_MODE_ADDR                  0x00 
#define PAILLIER_LEN_ADDR                   0x01 
#define PAILLIER_PING_TRIGGER_ADDR          0x02 
#define PAILLIER_PONG_TRIGGER_ADDR          0x03 
// #define PAILLIER_N_PRIME_ADDR               0x04 
// #define PAILLIER_N2_PRIME_ADDR              0x05 
// #define PAILLIER_P2_PRIME_ADDR              0x06 
// #define PAILLIER_Q2_PRIME_ADDR              0x07 
#define PAILLIER_PRMT_ADDR                  0x08 
#define PAILLIER_THREAD_NUM_ADDR            0x09 
#define PAILLIER_TRIGGER_START_ADDR         0x0A 
#define PAILLIER_PLAIN_LENGTH               0x0F
#define PAILLIER_CIPHER_LENGTH              0x1F
      
#define MODE_ENC_CRT                        0x00
#define MODE_ENC_NCRT                       0x01
#define MODE_DEC_CRT                        0x02
#define MODE_SCALE_MUL                      0x03
#define MODE_EVAL_ADD                       0x04
      
#define PAILLIER_TRIGGER_START              0x01
#define PAILLIER_TRIGGER_END                0x00
#define TRIGGER_OFFSET_PING                 0x02
#define TRIGGER_OFFSET_PONG                 0x03
      
#define TRIGGER_ADDR_ENC_CRT                0x8F
#define TRIGGER_ADDR_ENC_NCRT               0x9F
#define TRIGGER_ADDR_DEC_CRT                0x1F
#define TRIGGER_ADDR_SCALE_MUL              0x8F
#define TRIGGER_ADDR_EVAL_ADD               0x9F

#ifdef __cplusplus
extern "C"{
#endif

//0000c00 4331 7af7 5458 4f46 6dd6 79f9 4efe b2eb
/**
 * Establishes a communication channel between the host and FPGA.
 *
 * This function opens a file descriptor to the specified FPGA interface, which is
 * "/dev/xdma0_h2c_0" by default, with read-write access. Upon success, it prints
 * a success message; otherwise, an error message is printed indicating the failure.
 *
 * @return The file descriptor of the opened FPGA communication channel upon success,
 *         or -1 if the operation fails, typically due to insufficient permissions or the device not being available.
 */
int open_host2FpgaChannel(int i);

/**
 * Closes the communication channel between the host and FPGA.
 *
 * This function closes the file descriptor associated with the FPGA communication channel specified by `deviceIdentifier`.
 * It is crucial to call this function when communication with the FPGA is no longer needed to release system resources properly.
 *
 * @param deviceIdentifier The file descriptor representing the open communication channel to the FPGA.
 */
void close_host2FpgaChannel(int deviceIdentifier);

/**
 * Writes data from the host to the FPGA through an established communication channel.
 *
 * This function sends a block of data to a specific address within the FPGA, utilizing the provided file descriptor
 * that represents the communication channel. It first seeks to the designated address using `lseek`, then writes
 * the data using the `write` system call. Success or failure messages are printed accordingly.
 *
 * @param deviceIdentifier The file descriptor of the opened FPGA communication channel.
 * @param addr The offset within the FPGA's memory space where data should be written.
 * @param data Pointer to an array of bytes representing the data to be written.
 * @param dataBytesLen The number of bytes to write from the data array.
 *
 * @note Ensure that the `deviceIdentifier` is valid and corresponds to an open FPGA channel.
 * @note The function assumes the channel has been successfully opened with write permissions.
 */
int write_from_host_to_fpga(int deviceIdentifier, off_t addr, const unsigned char* data, int dataBytesLen);

/**
 * Establishes a communication channel from the FPGA to the host.
 *
 * This function attempts to open a file descriptor for the FPGA-to-host interface,
 * with the default path set to "/dev/xdma0_c2h_0". It requests both read and write
 * access permissions. Upon successfully opening the channel, a success message is printed.
 * In case of failure, an error message indicating the unsuccessful open operation is displayed.
 *
 * @return The file descriptor for the FPGA-to-host communication channel if the operation succeeds,
 *         or -1 if it fails, typically due to missing permissions or the absence of the specified device.
 */
int open_fpga2HostChannel(int i);

/**
 * Closes the communication channel from FPGA to host.
 *
 * This function closes the file descriptor associated with the FPGA-to-host communication channel,
 * which was previously opened using a similar function. The `deviceIdentifier` parameter refers
 * to the file descriptor that needs to be closed.
 *
 * @param deviceIdentifier The file descriptor representing the open channel from FPGA to host that should be closed.
 */
void close_fpga2HostChannel(int deviceIdentifier);

/**
 * Reads data from the FPGA to the host through an established communication channel.
 *
 * This function reads a block of data from the FPGA, specified by the address `addr` and
 * the length `dataBytesLen`, into the buffer `data`. The communication channel is identified
 * by the file descriptor `deviceIdentifier`. It first seeks to the correct position in the FPGA memory,
 * then performs the read operation. Success or failure messages are printed to the standard output.
 *
 * @param deviceIdentifier The file descriptor of the already opened FPGA communication channel.
 * @param addr The offset from the beginning of the FPGA memory region to start reading from.
 * @param data Pointer to the buffer where the read data will be stored.
 * @param dataBytesLen The number of bytes to read from the FPGA.
 */
int read_from_fpga_to_host(int deviceIdentifier, off_t addr, unsigned char* data, int dataBytesLen);

#ifdef __cplusplus
}
#endif

#endif //FPGA_INTERFACE_H
