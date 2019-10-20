/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <intr.h>
#include <utilities.h>

extern info_t *info;
extern void resume_from_intr(void);

void __regparm__(1) syscall_handler(int_ctx_t *ctx) {
    // Q3: print %eax
    // Q4: print "%s" located in "ESI"
    debug("SYSCALL eax = %p, \ttext = '%s'\n", ctx->gpr.eax, ctx->gpr.esi);
}

// Q3: called on kernel/core/idt.s
void syscall_isr() {
    debug("### #int48 handling from C ###\n");
    asm volatile (
        "leave ; pusha        \n"
        "mov %esp, %eax      \n"
        "call syscall_handler \n"
        "popa ; iret"
    );
}

void q1_trigger_int48(uint32_t arg) {
    asm volatile("int $48"::"a"(arg));
}

int_desc_t* q2_init_int48_handler(void) {
    idt_reg_t idtr;
    get_idtr(idtr);
    int_desc_t* desc = &idtr.desc[48];
    desc->dpl = SEG_SEL_USR;
    return desc;
}

void q3_setup_syscall_handler(int_desc_t* desc, bool_t asm_implementation) {
    if(!asm_implementation) {
        raw32_t addr = {.raw = (uint32_t)syscall_isr};
        desc->offset_1 = addr.wlow;
        desc->offset_2 = addr.whigh;
    }
}

void q4_syscall_print_text(const char* msg) {
    asm volatile("int $48"::"S"(msg));
}

void q5_attack() {
   asm volatile ("int $48"::"S"(0x303cf9));
}

void userland(void) {
    debug("# Inside userland\n");

    // Q1: #GP, dpl = 0
    // Q2: %eax <- 0xdeadbeef
    debug("trigger int48\n");
    q1_trigger_int48(0xdeadbeef);

    // Q4: print text
    q4_syscall_print_text("-= I am a text variable from ESI =-\n");

    // Q5: Hack service
    q5_attack();

    // Q0: never return
    while(1);
}

void tp()
{
    // TP3 Initialization
    perso_init_gdt();

    perso_print_gdt();
    set_ds(gdt_usr_seg_sel(4));
    set_es(gdt_usr_seg_sel(4));
    set_fs(gdt_usr_seg_sel(4));
    set_gs(gdt_usr_seg_sel(4));

    perso_init_tss();

    bool_t asm_implementation = true;
    int_desc_t* desc = q2_init_int48_handler();
    q3_setup_syscall_handler(desc, asm_implementation);

    uint32_t ustack = 0x600000;
    perso_ring0_to_ring3_ustack(&userland, ustack);

    debug("End of TP5\n");
}
