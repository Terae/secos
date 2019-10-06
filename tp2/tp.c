/* GPLv2 (c) Airbus */
#include <debug.h>
#include <excp.h>
#include <info.h>
#include <intr.h>

extern info_t *info;
extern void resume_from_intr(void);

void bp_handler(void) {
    debug("db_handler called\n");
    asm("leave; iret;");
}

void bp_trigger(void) {
    asm ("int3;");
    debug("inside bp_trigger\n");
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
