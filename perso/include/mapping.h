#ifndef __MAPPING_H__
#define __MAPPING_H__

//////////////////////////
// PGD[0] - Kernel land //
//////////////////////////
#define KRN_BEGIN       0x200000

#define PGD_KERNEL      0x200000

#define PTB_KERNEL      0x201000
#define PTB_USER1       0x202000
#define PTB_USER2       0x203000
#define PTB_SHARED      0x204000

#define KRN_CODE        0x300000
#define KRN_STACK_END   0x3A0000
#define KRN_T1_STACK    0x3B0000
#define KRN_T2_STACK    0x3C0000

//////////////////////////
// PGD[1] - User 1 land //
//////////////////////////
#define USR1_BEGIN      0x400000
#define USR1_CPT        0x401000
#define USR1_COUNT      0x402000
#define USR1_STACK      0x7FF000

//////////////////////////
// PGD[2] - User 2 land //
//////////////////////////
#define USR2_BEGIN      0x800000
#define USR2_CPT        0x801000
#define USR2_COUNT      0x802000
#define USR2_STACK      0xBFF000

////////////////////////////
// PGD[3] - Shared memory //
////////////////////////////
// Syscalls
#define SYS_BEGIN       0xC00000
#define SYS_PRINT       0xC10000
#define SYS_COUNT       0xC20000

#define SHARED_COUNTER  0xE10000

#endif
