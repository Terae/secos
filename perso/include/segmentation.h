#include <asm.h>
#include <debug.h>
#include <segmem.h> // get_gdtr(), seg_desc_t, gdt_reg_t

#define krn_code_idx    1
#define krn_data_idx    2
#define usr_code_idx    3
#define usr_data_idx    4
#define tss_idx         5

#define krn_code    gdt_krn_seg_sel(krn_code_idx)
#define krn_data    gdt_krn_seg_sel(krn_data_idx)
#define usr_code    gdt_usr_seg_sel(usr_code_idx)
#define usr_data    gdt_usr_seg_sel(usr_data_idx)

#define perso_set_seg_desc(_seg_,_type_,_dpl_,_base_,_limit_)   \
    ({                                                          \
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
        (_seg_)->l       = 0;                                   \
     })

#define perso_set_tss_seg(_seg_,_base_)             \
    ({                                              \
        uint32_t _limit_ = sizeof(tss_t);           \
        (_seg_)->base_1  = (_base_) & 0xFFFF;       \
        (_seg_)->base_2  = ((_base_) >> 16) & 0xFF; \
        (_seg_)->base_3  = ((_base_) >> 24) & 0xFF; \
        (_seg_)->limit_1 = (_limit_) & 0xFFFF;      \
        (_seg_)->limit_2 = ((_limit_) >> 16) & 0xF; \
        (_seg_)->type    = SEG_DESC_SYS_TSS_AVL_32; \
        (_seg_)->dpl     = SEG_SEL_KRN;             \
        (_seg_)->d       = 1;                       \
        (_seg_)->g       = 1;                       \
        (_seg_)->s       = 0;                       \
        (_seg_)->p       = 1;                       \
        (_seg_)->l       = 0;                       \
     })

#define GDT_SIZE 6

extern seg_desc_t GDT[GDT_SIZE];
extern tss_t TSS;

void perso_print_gdt(void);

void perso_init_gdt(void);

void perso_init_tss(void);

// push ss, esp, eflags, cs, eip
#define perso_ring0_to_ring3(_userland_)    \
    asm volatile("push %0;"                 \
                 "push %%ebp;"              \
                 "pushf;"                   \
                 "push %1;"                 \
                 "push %2;"                 \
                 "iret;"                    \
                 :: "i"(usr_data),          \
                    "i"(usr_code),          \
                    "r"((_userland_))       \
            );

#define perso_ring0_to_ring3_ustack(_userland_,_ustack_)    \
    asm volatile("push %0;"                                 \
                 "push %1;"                                 \
                 "pushf;"                                   \
                 "push %2;"                                 \
                 "push %3;"                                 \
                 "iret;"                                    \
                 :: "i"(usr_data),                          \
                    "m"((_ustack_)),                        \
                    "i"(usr_code),                          \
                    "r"((_userland_))                       \
            );
