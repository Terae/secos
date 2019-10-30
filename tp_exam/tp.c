/* GPLv2 (c) Airbus */
#include <cr.h>
#include <debug.h>
#include <excp.h>
#include <info.h>
#include <mapping.h>
#include <segmem.h>
#include <syscall.h>
#include <paging.h>
#include <segmentation.h>

extern info_t *info;

uint32_t* __attribute__((section(".user1_cpt"))) counter_user1 = (uint32_t*)USR1_COUNT;
uint32_t* __attribute__((section(".user2_cpt"))) counter_user2 = (uint32_t*)USR2_COUNT;

pde32_t* pgd;

#define debug_usr(_msg_) asm volatile("int $48"::"S"((_msg_)))

void user2(void) __attribute__((section(".user2")));
void __attribute__((section(".user1"))) user1() {
    debug_usr("Inside user1()");
    /* while(1) { */
        (*counter_user1)++;

    uint32_t ustack = USR2_STACK;
    perso_ring0_to_ring3_ustack(&user2, ustack);
    while(1) {
    }
}

void __attribute__((section(".user2"))) user2() {
    debug_usr("Inside user2()");
    /* while(1) { */
        sys_counter(counter_user2);

    uint32_t ustack = USR1_STACK;
    perso_ring0_to_ring3_ustack(&user1, ustack);
    while(1) {
    }
}

void init_excp_handlers(void) {
    // Handlers called from ASM code
    // See `kernel.core.idt.s`, `kernel/core/excp.c`
    idt_reg_t idtr;
    get_idtr(idtr);
    idtr.desc[SYS_INT48].dpl = SEG_SEL_USR;
    idtr.desc[SYS_INT80].dpl = SEG_SEL_USR;
}

void init_paging(void) {
    debug("init_paging\n");

    // PGD
    pgd = pgd_init(PGD_KERNEL);
    memset((void*)pgd, 0, PAGE_SIZE);

    // PTBs
    pte32_t* ptb_kernel = pgd_register_ptb(pgd, PTB_KERNEL, 0, PG_KRN | PG_RW);
    pte32_t* ptb_user1  = pgd_register_ptb(pgd, PTB_USER1,  1, PG_USR | PG_RW);
    pte32_t* ptb_user2  = pgd_register_ptb(pgd, PTB_USER2,  2, PG_USR | PG_RW);
    pte32_t* ptb_shared = pgd_register_ptb(pgd, PTB_SHARED, 3, PG_USR | PG_RO);

    // Identity mapping
    for(int i = 0; i < 1024; ++i) {
        ptb_register_page(ptb_kernel, i,        i, PG_KRN | PG_RW);
        ptb_register_page(ptb_user1,  i + 1024, i, PG_USR | PG_RW);
        ptb_register_page(ptb_user2,  i + 2048, i, PG_USR | PG_RW);
        ptb_register_page(ptb_shared, i + 3072, i, PG_USR | PG_RO);
    }

    // Shared memory
    pg_set_entry(&ptb_user1 [pt32_idx(USR1_COUNT)], PG_USR | PG_RW | PG_P, page_nr(SHARED_COUNTER));
    pg_set_entry(&ptb_user2 [pt32_idx(USR2_COUNT)], PG_USR | PG_RW | PG_P, page_nr(SHARED_COUNTER));

    // Activation, set_cr3() is done on `pgd_init()`
    activate_cr0();

    pgd_print(pgd);
    debug("init_paging done\n");
}

void init_tasks(void) {
    debug("init_tasks()\n");

    set_ds(usr_data);
    set_es(usr_data);
    set_fs(usr_data);
    set_gs(usr_data);

    force_interrupts_on();
    uint32_t ustack = USR2_STACK;
    perso_ring0_to_ring3_ustack(&user2, ustack);
}

void tp() {
    // Exceptions
    init_excp_handlers();

    // Segmentation
    perso_init_gdt();

    perso_init_tss();

    // Paging
    init_paging();

    // Tasks
    init_tasks();

    debug("End of file - I shouldn't be there\n");
}
