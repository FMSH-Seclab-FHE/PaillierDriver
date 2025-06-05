//
// Created by zhangchi on 24-11-26.
//

#include "fpga_interface.h"

long long int clusteri_result_path_conf_pingpong_addr[2][2] = {
{                                           0x0000000000800000      ,    
                                            0x0000000000800400      },
{                                           0x0000000001800000      ,    
                                            0x0000000001800400      }};

unsigned int ddr_half_limit = 0x40000000;  //don't overflow 32 bits

off_t clusteri_corei_bram_addr[3][16] = {
    {
    0x0000000000000000, 0x0000000000010000, 0x0000000000020000, 0x0000000000030000, 
    0x0000000000040000, 0x0000000000050000, 0x0000000000060000, 0x0000000000070000, 
    0x0000000000080000, 0x0000000000090000, 0x00000000000a0000, 0x00000000000b0000, 
    0x00000000000c0000, 0x00000000000d0000, 0x00000000000e0000, 0x00000000000f0000},
    {
    0x0000000001000000, 0x0000000001010000, 0x0000000001020000, 0x0000000001030000, 
    0x0000000001040000, 0x0000000001050000, 0x0000000001060000, 0x0000000001070000, 
    0x0000000001080000, 0x0000000001090000, 0x00000000010a0000, 0x00000000010b0000, 
    0x00000000010c0000, 0x00000000010d0000, 0x00000000010e0000, 0x00000000010f0000},
    {
    0x0000000002000000, 0x0000000002010000, 0x0000000002020000, 0x0000000002030000, 
    0x0000000002040000, 0x0000000002050000, 0x0000000002060000, 0x0000000002070000, 
    0x0000000002080000, 0x0000000002090000, 0x00000000020a0000, 0x00000000020b0000, 
    0x00000000020c0000, 0x00000000020d0000, 0x00000000020e0000, 0x00000000020f0000}};

off_t cluster_core_broadcast_bram_addr = 0x00000000ff3f0000;

off_t clusteri_corei_fifo_addr[3][16] = {
    {
    0x0000000000400000, 0x0000000000410000, 0x0000000000420000, 0x0000000000430000, 
    0x0000000000440000, 0x0000000000450000, 0x0000000000460000, 0x0000000000470000, 
    0x0000000000480000, 0x0000000000490000, 0x00000000004a0000, 0x00000000004b0000, 
    0x00000000004c0000, 0x00000000004d0000, 0x00000000004e0000, 0x00000000004f0000},
    {
    0x0000000001400000, 0x0000000001410000, 0x0000000001420000, 0x0000000001430000, 
    0x0000000001440000, 0x0000000001450000, 0x0000000001460000, 0x0000000001470000, 
    0x0000000001480000, 0x0000000001490000, 0x00000000014a0000, 0x00000000014b0000, 
    0x00000000014c0000, 0x00000000014d0000, 0x00000000014e0000, 0x00000000014f0000},
    {
    0x0000000002400000, 0x0000000002410000, 0x0000000002420000, 0x0000000002430000, 
    0x0000000002440000, 0x0000000002450000, 0x0000000002460000, 0x0000000002470000, 
    0x0000000002480000, 0x0000000002490000, 0x00000000024a0000, 0x00000000024b0000, 
    0x00000000024c0000, 0x00000000024d0000, 0x00000000024e0000, 0x00000000024f0000}
};

off_t cluster_core_broadcast_fifo_addr = 0x00000000ff7f0000;


int open_host2FpgaChannel(int i){
    int rc;
    switch (i)
    {
    case 0:
        rc = open("/dev/xdma0_h2c_0", O_RDWR);
        if(rc == -1){
            perror("open host2FpgaChannel failed");
        }
        break;
    case 1:
        rc = open("/dev/xdma0_h2c_1", O_RDWR);
        if(rc == -1){
            perror("open host2FpgaChannel failed");
        }
        break;
    case 2:
        rc = open("/dev/xdma0_h2c_2", O_RDWR);
        if(rc == -1){
            perror("open host2FpgaChannel failed");
        }
        break;
    case 3:
        rc = open("/dev/xdma0_h2c_3", O_RDWR);
        if(rc == -1){
            perror("open host2FpgaChannel failed");
        }
        break;    
    default:
        rc = open("/dev/xdma0_h2c_0", O_RDWR);
        if(rc == -1){
            perror("open host2FpgaChannel failed");
        }
        break;
    }
#ifndef ERROR_LOG_ONLY
    else{
        printf("open host2FpgaChannel succeed\n");
    }
#endif
    return rc;
}

void close_host2FpgaChannel(int deviceIdentifier){
    close(deviceIdentifier);
}

int write_from_host_to_fpga(int deviceIdentifier, off_t addr, const unsigned char* data, int dataBytesLen){
    off_t rc;
    if(deviceIdentifier == -1){
        printf("input device illegal \n");
        return rc;
    }

    rc = lseek(deviceIdentifier, addr, SEEK_SET);
    if(rc == -1)
    {
        printf("addr Error: %s\n", strerror(errno));
        return rc;
    }
    rc = write(deviceIdentifier, data, dataBytesLen);
    if(rc == -1)
    {
        printf("write data Error: %s \n", strerror(errno));
        return rc;
    }
#ifndef ERROR_LOG_ONLY
    printf("write data succeed\n");
#endif
}

int open_fpga2HostChannel(int i){
    int rc;
    switch (i)
    {
    case 0:
        rc = open("/dev/xdma0_c2h_0", O_RDWR);
        if(rc == -1){
            perror("open fpga2HostChannel failed");
        }
        break;
    case 1:
        rc = open("/dev/xdma0_c2h_1", O_RDWR);
        if(rc == -1){
            perror("open fpga2HostChannel failed");
        }
        break;
    case 2:
        rc = open("/dev/xdma0_c2h_2", O_RDWR);
        if(rc == -1){
            perror("open fpga2HostChannel failed");
        }
        break;
    case 3:
        rc = open("/dev/xdma0_c2h_3", O_RDWR);
        if(rc == -1){
            perror("open fpga2HostChannel failed");
        }
        break;
    default:
        rc = open("/dev/xdma0_c2h_0", O_RDWR);
        if(rc == -1){
            perror("open fpga2HostChannel failed");
        }
        break;
    }
#ifndef ERROR_LOG_ONLY
    else{
        printf("open fpga2HostChannel succeed\n");
    }
#endif
    return rc;
}

void close_fpga2HostChannel(int deviceIdentifier){
    close(deviceIdentifier);
}

int read_from_fpga_to_host(int deviceIdentifier, off_t addr, unsigned char* data, int dataBytesLen){
    off_t rc;
    if(deviceIdentifier == -1){
        perror("input device illegal \n");
        return rc;
    }

    rc = lseek(deviceIdentifier, addr, SEEK_SET);
    if(rc == -1)
    {
        printf("addr Error: %s\n", strerror(errno));
        return rc;
    }
    rc = read(deviceIdentifier, data, dataBytesLen);
    if(rc == -1)
    {
        printf("read data Error: %s \n", strerror(errno));
        return rc;
    }
#ifndef ERROR_LOG_ONLY
    printf("read data succeed\n");
#endif

}
