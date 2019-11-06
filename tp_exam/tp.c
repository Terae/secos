/* GPLv2 (c) Airbus */
#include <cr.h>
#include <debug.h>
#include <excp.h>
#include <info.h>
#include <mapping.h>
#include <paging.h>
#include <segmem.h>
#include <segmentation.h>
#include <syscall.h>
#include <task.h>
#include <utils.h>

extern info_t *info;

task_t task_krn;
task_t task_user1;
task_t task_user2;
extern task_t* current_task;

uint32_t* __attribute__((section(".user1_cpt"))) counter_user1 = (uint32_t*)USR1_COUNT;
uint32_t* __attribute__((section(".user2_cpt"))) counter_user2 = (uint32_t*)USR2_COUNT;

bool_t* __attribute__((section(".user1_sem1"))) user1_sem1 = (bool_t*)USR1_SEM_1;
bool_t* __attribute__((section(".user1_sem2"))) user1_sem2 = (bool_t*)USR1_SEM_2;
bool_t* __attribute__((section(".user2_sem1"))) user2_sem1 = (bool_t*)USR2_SEM_1;
bool_t* __attribute__((section(".user2_sem2"))) user2_sem2 = (bool_t*)USR2_SEM_2;

pde32_t* pgd_kernel;
pde32_t* pgd_user1;
pde32_t* pgd_user2;

void __attribute__((section(".user1"))) user1() {
    while(1) {
        if(*user1_sem1) {
            (*counter_user1)++;
            *user1_sem1 = false;
            *user1_sem2 = true;
        }
    }
}

void __attribute__((section(".user2"))) user2() {
    while(1) {
        if(*user2_sem2) {
            sys_counter(counter_user2);
            *user2_sem1 = true;
            *user2_sem2 = false;
        }
    }
}

void init_excp_handlers(void) {
    debug("\n# Initialization of exceptions handlers\n");
    // Handlers called from ASM code
    // See `kernel.core.idt.s`, `kernel/core/excp.c`
    idt_reg_t idtr;
    get_idtr(idtr);

    debug("-> IRQ 32: counter clock, DPL=%d\n", idtr.desc[IRQ32_EXCP].dpl);
    idtr.desc[SYS_INT48].dpl = SEG_SEL_USR;
    debug("-> INT 48: syscall debug, DPL=%d\n", idtr.desc[SYS_INT48].dpl);
    idtr.desc[SYS_INT80].dpl = SEG_SEL_USR;
    debug("-> INT 80; syscall count, DPL=%d\n", idtr.desc[SYS_INT80].dpl);
}

void init_paging(void) {
    debug("\n# Initialization of paging\n");

    // PGD
    pgd_kernel = pgd_init(PGD_KERNEL);
    pgd_user1  = pgd_init(PGD_USER1);
    pgd_user2  = pgd_init(PGD_USER2);
    memset((void*)pgd_kernel, 0, PAGE_SIZE);
    memset((void*)pgd_user1,  0, PAGE_SIZE);
    memset((void*)pgd_user2,  0, PAGE_SIZE);

    // PTBs
    pte32_t* ptb_krn_kernel = pgd_register_ptb(pgd_kernel, PTB_KRN_KERNEL, 0, PG_KRN | PG_RW);

    pte32_t* ptb_u1_kernel  = pgd_register_ptb(pgd_user1, PTB_U1_KERNEL,  0, PG_USR | PG_RW);
    pte32_t* ptb_u1_task1   = pgd_register_ptb(pgd_user1, PTB_U1_TASK1,   1, PG_USR | PG_RW);
    pte32_t* ptb_u1_shared  = pgd_register_ptb(pgd_user1, PTB_U1_SHARED,  3, PG_USR | PG_RW);

    pte32_t* ptb_u2_kernel  = pgd_register_ptb(pgd_user2, PTB_U2_KERNEL,  0, PG_USR | PG_RW);
    pte32_t* ptb_u2_task2   = pgd_register_ptb(pgd_user2, PTB_U2_TASK2,   2, PG_USR | PG_RW);
    pte32_t* ptb_u2_shared  = pgd_register_ptb(pgd_user2, PTB_U2_SHARED,  3, PG_USR | PG_RW);

    // Identity mapping
    for(int i = 0; i < 1024; ++i) {
        ptb_register_page(ptb_krn_kernel, i,        i, PG_KRN | PG_RW);
        ptb_register_page(ptb_u1_kernel,  i,        i, PG_USR | PG_RW);
        ptb_register_page(ptb_u2_kernel,  i,        i, PG_USR | PG_RW);

        ptb_register_page(ptb_u1_task1,   i + 1024, i, PG_USR | PG_RW);
        ptb_register_page(ptb_u2_task2,   i + 2048, i, PG_USR | PG_RW);

        ptb_register_page(ptb_u1_shared,  i + 3072, i, PG_USR | PG_RO);
        ptb_register_page(ptb_u2_shared,  i + 3072, i, PG_USR | PG_RO);
    }

    // Shared memory
    pg_set_entry(&ptb_u1_task1[pt32_idx(USR1_COUNT)], PG_USR | PG_RW | PG_P, page_nr(SHARED_COUNTER));
    pg_set_entry(&ptb_u2_task2[pt32_idx(USR2_COUNT)], PG_USR | PG_RW | PG_P, page_nr(SHARED_COUNTER));

    // Semaphores
    pg_set_entry(&ptb_u1_task1[pt32_idx(USR1_SEM_1)], PG_USR | PG_RW | PG_P, page_nr(SEM_RUN_1));
    pg_set_entry(&ptb_u1_task1[pt32_idx(USR1_SEM_2)], PG_USR | PG_RW | PG_P, page_nr(SEM_RUN_2));
    pg_set_entry(&ptb_u2_task2[pt32_idx(USR2_SEM_1)], PG_USR | PG_RW | PG_P, page_nr(SEM_RUN_1));
    pg_set_entry(&ptb_u2_task2[pt32_idx(USR2_SEM_2)], PG_USR | PG_RW | PG_P, page_nr(SEM_RUN_2));

    // Activation, set_cr3() is done on `pgd_init()`
    activate_cr0();

    debug("== KERNEL ==");
    pgd_print(pgd_kernel);
    debug("== USER 1 ==");
    pgd_print(pgd_user1);
    debug("== USER 2 ==");
    pgd_print(pgd_user2);
}

void init_tasks(void) {
    debug("# Initialization of tasks\n");
    init_krn(&task_krn, pgd_kernel, &task_user2);
    debug("-> ........: task_krn,   first process\n");
    task_init(&task_user1, (uint32_t) &user1, (uint32_t*) KRN_T1_STACK, (uint32_t*) USR1_STACK, pgd_user1,  &task_user2);
    debug("-> %p: task_user1, increments the counter\n", task_user1);
    task_init(&task_user2, (uint32_t) &user2, (uint32_t*) KRN_T2_STACK, (uint32_t*) USR2_STACK, pgd_user2,  &task_user1);
    debug("-> %p: task_user2, calls the syscall INT 80\n", task_user2);

    set_ds(usr_data);
    set_es(usr_data);
    set_fs(usr_data);
    set_gs(usr_data);

    current_task = &task_krn;
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

    // Initial values
    *counter_user2 = 0;
    *user2_sem1 = false;
    *user2_sem2 = true;

    debug("\n################################\n# Let's turn interruptions on! #\n################################\n\n");
    force_interrupts_on();

    while(1);
    panic("End of file - I shouldn't be there\n");
}
