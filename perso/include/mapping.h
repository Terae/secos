#ifndef __MAPPING_H__
#define __MAPPING_H__

//////////////////////////
// PGD[0] - Kernel land //
//////////////////////////
#define KRN_BEGIN       0x200000

#define PGD_KERNEL      0x200000
#define PTB_KRN_KERNEL  0x201000

#define PGD_USER1       0x210000
#define PTB_U1_KERNEL   0x211000
#define PTB_U1_TASK1    0x212000
#define PTB_U1_SHARED   0x214000

#define PGD_USER2       0x220000
#define PTB_U2_KERNEL   0x221000
#define PTB_U2_TASK2    0x223000
#define PTB_U2_SHARED   0x224000

#define KRN_CODE        0x300000
#define KRN_STACK_END   0x3A0000
#define KRN_T1_STACK    0x3B0000
#define KRN_T2_STACK    0x3C0000

//////////////////////////
// PGD[1] - User 1 land //
//////////////////////////
#define USR1_BEGIN      0x400000
#define USR1_COUNT      0x40A000
#define USR1_SEM_1      0x40B000
#define USR1_SEM_2      0x40C000
#define USR1_STACK      0x7FF000

//////////////////////////
// PGD[2] - User 2 land //
//////////////////////////
#define USR2_BEGIN      0x800000
#define USR2_COUNT      0x80A000
#define USR2_SEM_1      0x80B000
#define USR2_SEM_2      0x80C000
#define USR2_STACK      0xBFF000

////////////////////////////
// PGD[3] - Shared memory //
////////////////////////////
// Syscalls
#define SYS_BEGIN       0xC00000
#define SYS_PRINT       0xC10000
#define SYS_COUNT       0xC20000

#define SHARED_COUNTER  0xE10000
#define SEM_RUN_1       0xE30000
#define SEM_RUN_2       0xE50000

#endif
