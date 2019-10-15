/* GPLv2 (c) Airbus */
#include <asm.h>
#include <debug.h>
#include <excp.h>
#include <info.h>
#include <intr.h>

extern info_t *info;
extern void resume_from_intr(void);

void bp_handler(void) {
    force_interrupts_off();
    asm volatile (".align 16;"
                  "pushl $-1;" // #BP does not take any argument
                  "pushl $3;"  // interruption code
                  "pusha;");   // saving the %eax register

    debug("db_handler called\n");

    asm volatile ("popa;"           // restore the %eax register
                  "add $8, %esp;"); // poping 2 #BP arguments ($-1, $3)
    force_interrupts_on();
    asm volatile ("leave;"        // `asm volatile ("mov %ebp, %esp; pop %ebp");` see https://stackoverflow.com/questions/29790175/assembly-x86-leave-instruction
                  "iret;");
}

void bp_trigger(void) {
    int a = 0;
    asm volatile ("int3;");
    a = 4;
    debug("a equals to %d\n", a);
}

void init_bp_handler(void) {
    idt_reg_t idtr;
    get_idtr(idtr);
    debug("@IDT:\t0x%x\n", &idtr);

    int_desc(&idtr.desc[BP_EXCP], gdt_krn_seg_sel(1), (unsigned int)&bp_handler);
}

void tp()
{
    init_bp_handler();
    bp_trigger();
    debug("END OF TP2\n");
}
