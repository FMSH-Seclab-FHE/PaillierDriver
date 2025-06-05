//
// Created by douwei lei on 24-12-04.
//

#ifndef CONF_H
#define CONF_H
#include "fpga_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * Writes configuration data to the clusters' result_path. Each cluster's result_path needs to be configured separately. Additionally, the ping and pong must be configured separately based on the address.
 *
 *
 * @param clusteri_result_path_pingOrPong_conf_addr 
 * @param len 
 * @param number 
 * @param group 
 * @param clusteri_len 
 * @param clusteri_number 
 * @param cluster_cnt 
 */
void conf_cluster_result_path(
    off_t clusteri_result_path_pingOrPong_conf_addr,    
    int len, int number, int group,
    int* clusteri_len, int* clusteri_number);

/**
 * Writes configuration data to the 
 *
 *
 * @param 
 * @param 
 * @param 
 * @param 
 */
void conf_control(
    off_t conf_control_pingOrPong_conf_addr, 
    off_t conf_control_pingOrPong_state_addr,   
    int* clusteri_len, int* clusteri_number, int cluster_cnt,
    unsigned char *result, int last);
    
#ifdef __cplusplus
}
#endif
#endif //CONF_H
