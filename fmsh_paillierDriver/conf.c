#include "conf.h"

void conf_cluster_result_path(
    off_t clusteri_result_path_pingOrPong_conf_addr,    
    int len, int number, int group,
    int* clusteri_len, int* clusteri_number){
    
    int i, rc;
    int size;
    int host2fpga;
    unsigned char* conf_bytes;

    conf_bytes = (unsigned char *)malloc(sizeof(unsigned char) * (4 + CLUSTER_NUMBER * 2));
    if(conf_bytes == NULL){
        printf("conf_cluster_result_path :: malloc wrong \n ");
        return;
    }
    conf_bytes[0] = (unsigned char)len;
    conf_bytes[1] = (unsigned char)(len >> 8);
    conf_bytes[2] = (unsigned char)number;
    conf_bytes[3] = (unsigned char)group;
    for(i = 0; i < CLUSTER_NUMBER; i++){
        size = clusteri_len[i] * clusteri_number[i];
        conf_bytes[4 + i*2 + 0] = (unsigned char)size;
        conf_bytes[4 + i*2 + 1] = (unsigned char)(size >> 8);
    }

    host2fpga = open_host2FpgaChannel(1);
    if(host2fpga == -1) {
        printf("conf_cluster_result_path :: open host2fpga failed\n");
        return;
    }
    rc = write_from_host_to_fpga(host2fpga, clusteri_result_path_pingOrPong_conf_addr, conf_bytes, 4 + CLUSTER_NUMBER * 2);
    if(rc == -1) {
        printf("conf_cluster_result_path :: write data error\n");
        return;
    }
    free(conf_bytes);
    close(host2fpga);
}

void conf_control(
    off_t conf_control_pingOrPong_conf_addr, 
    off_t conf_control_pingOrPong_state_addr,   
    int* clusteri_len, int* clusteri_number, int cluster_cnt,
    unsigned char *result, int last){
    static unsigned int offset = 0;
    static unsigned int axi_pingpong_high32 = (unsigned int)((long long)BRAM_CONTROL_BRAM2AXI >> 32);

    int i, j, rc;
    int size;
    int host2fpga, fpga2host;
    unsigned char* conf_bytes;
    unsigned char cluster_cnt_bytes[16] = { 0 };
    if(last != 1){

        conf_bytes = (unsigned char* )malloc(sizeof(unsigned char) * 16 * CLUSTER_NUMBER);
        if(conf_bytes == NULL){
            printf("conf_control :: malloc wrong \n ");
            return;
        }    
        memset(conf_bytes, 0, 16 * CLUSTER_NUMBER);

        cluster_cnt_bytes[0] = (unsigned char)cluster_cnt;
        for(i =  CLUSTER_NUMBER - cluster_cnt; i < CLUSTER_NUMBER; i++){
            size = clusteri_len[i] * clusteri_number[i]; 
            if(offset >= ddr_half_limit){
                axi_pingpong_high32 = axi_pingpong_high32 ^ 0x00000003;
                offset = 0;  
            } 
            for(j = 0; j < 4; j++){
                conf_bytes[i * 16 + j] = (unsigned char)(offset >> (j * 8));
            }
            for(j = 0; j < 4; j++){
                conf_bytes[i * 16 + 4 + j] = (unsigned char)(axi_pingpong_high32 >> (j * 8));
            }   
            for(j = 0; j < 4; j++){
                conf_bytes[i * 16 + 8 + j] = (unsigned char)(size >> (j * 8));
            }
            for(j = 0; j < 4; j++){
                conf_bytes[i * 16 + 12 + j] = (unsigned char)0;
            }           
            offset += (size << 4);
        }

        host2fpga = open_host2FpgaChannel(1);
        if(host2fpga == -1){
            printf("conf_control :: open host2fpga failed\n");
            return;
        } 
        rc = write_from_host_to_fpga(host2fpga, conf_control_pingOrPong_conf_addr, conf_bytes, 16 * CLUSTER_NUMBER);
        if(rc == -1) {
            printf("conf_control :: write data error\n");
            return;
        }
        rc = write_from_host_to_fpga(host2fpga, conf_control_pingOrPong_state_addr, cluster_cnt_bytes, 16);
        if(rc == -1) {
            printf("conf_control :: write data error\n");
            return;
        }

        free(conf_bytes);
        close(host2fpga);
    }
    else{
        // const char *out_filename = "outdata.bin";
        // int outfile_fd = open(out_filename, O_RDWR | O_CREAT | O_TRUNC);
        // if(outfile_fd == -1){
        //     perror("no file\n");
        //     return;
        // }
        fpga2host = open_fpga2HostChannel(1);
        read_from_fpga_to_host(fpga2host, DDR0_ADDR, result, offset);
        // rc = write(outfile_fd, result, offset);
        // if(rc == -1) 
        // {
        //     printf("write data from buffer to outdata Error: %s\n", strerror(errno));
        // } 
        offset = 0;
        // close(outfile_fd);
        close(fpga2host);
    }
}

