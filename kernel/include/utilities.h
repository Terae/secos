#include <asm.h>
#include <debug.h>
#include <segmem.h> // get_gdtr(), seg_desc_t, gdt_reg_t

#define perso_set_seg_desc(_seg_, _type_, _dpl_, _base_, _limit_) \
    (_seg_)->base_1  = (_base_) & 0xFFFF;                   \
    (_seg_)->base_2  = ((_base_) >> 16) & 0xFF;             \
    (_seg_)->base_3  = ((_base_) >> 24) & 0xFF;             \
    (_seg_)->limit_1 = (_limit_) & 0xFFFF;                  \
    (_seg_)->limit_2 = ((_limit_) >> 16) & 0xF;             \
    (_seg_)->type    = (_type_) & 0xF;                      \
    (_seg_)->dpl     = (_dpl_) & 0x3;                       \
    (_seg_)->d       = 1;                                   \
    (_seg_)->g       = 1;                                   \
    (_seg_)->s       = 1;                                   \
    (_seg_)->p       = 1;                                   \
    (_seg_)->l       = 0;

#define BEGIN_USR_SPACE 0ULL
#define BEGIN_KRN_SPACE 0xC0000000

#define GDT_SIZE 6

extern seg_desc_t GDT[GDT_SIZE];
extern tss_t TSS;

void perso_print_gdt(void);

void perso_init_gdt(void);

void perso_init_tss(void);

// push ss, esp, eflags, cs, eip
#define perso_ring0_to_ring3(_userland_)       \
    asm volatile("push %0;"                    \
                 "push %%ebp;"                 \
                 "pushf;"                      \
                 "push %1;"                    \
                 "push %2;"                    \
                 "iret;"                       \
                 :: "i"(gdt_usr_seg_sel(4)),   \
                    "i"(gdt_usr_seg_sel(3)),   \
                    "r"((_userland_)));

#define perso_ring0_to_ring3_ustack(_userland_, _ustack_)       \
    asm volatile("push %0;"                                     \
                 "push %1;"                                     \
                 "pushf;"                                       \
                 "push %2;"                                     \
                 "push %3;"                                     \
                 "iret;"                                        \
                 :: "i"(gdt_usr_seg_sel(4)),                    \
                    "m"((_ustack_)),                            \
                    "i"(gdt_usr_seg_sel(3)),                    \
                    "r"((_userland_)));
