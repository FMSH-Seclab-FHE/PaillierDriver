//
// Created by zhangchi on 24-11-26.
//

#ifndef ADDR_MAP_H
#define ADDR_MAP_H
//========================================
// Paillier Unit Input RAM Address List  |
//========================================

// Data Address List
// Range from 0x000 ~ 0x0FF
//----------------------------------------
#define RAM_IN_OP1_PING          0x200
#define RAM_IN_OP2_PING          0x280
#define RAM_IN_OP1_PONG          0x300
#define RAM_IN_OP2_PONG          0x380

// #define RAM_IN_OP1_PING          0x000
// #define RAM_IN_OP2_PING          0x040
// #define RAM_IN_OP1_PONG          0x080
// #define RAM_IN_OP2_PONG          0x0C0

// Constant Data Address List
// Range from 0x180 ~ 0x1FF
//----------------------------------------
#define RAM_IN_N2_PRIME          0x100
#define RAM_IN_N_PRIME           0x120
#define RAM_IN_P2_PRIME          0x130
#define RAM_IN_Q2_PRIME          0x140
#define RAM_IN_N2                0x180
#define RAM_IN_N                 0x1A0
#define RAM_IN_P2                0x1B0
#define RAM_IN_Q2                0x1C0
#define RAM_IN_HS                0x1D0
#define RAM_IN_ZERO              0x1F0

// Initialization Parameter Address List
// Range from 0x000 ~ 0x10F
//----------------------------------------
#define RAM_IN_NR2_MOD_N2        0x000
#define RAM_IN_R4_MOD_N2         0x020
#define RAM_IN_QQR2_MOD_N2       0x040
#define RAM_IN_PPR2_MOD_N2       0x060
#define RAM_IN_R2_MOD_N          0x080
#define RAM_IN_R2_MOD_P2         0x090
#define RAM_IN_R2_MOD_Q2         0x0A0
#define RAM_IN_LAMBDA_MOD_PHI_P2 0x0B0
#define RAM_IN_LAMBDA_MOD_PHI_Q2 0x0C0
#define RAM_IN_N_INV_MOD_R       0x0D0
#define RAM_IN_RMU_INV_MOD_N     0x0E0
#define RAM_IN_ONE               0x0F0

// Pre-Calculated Data Address List
// Range from 0x100 ~ 0x17F
//----------------------------------------
#define RAM_IN_PRE_START         0x100

#endif //ADDR_MAP_H
