/* GPLv2 (c) Airbus */
#include <intr.h>
#include <debug.h>
#include <info.h>
#include <mapping.h>
#include <task.h>

extern info_t *info;
extern void idt_trampoline();
static int_desc_t IDT[IDT_NR_DESC];

void intr_init()
{
   idt_reg_t idtr;
   offset_t  isr;
   size_t    i;

   isr = (offset_t)idt_trampoline;

   /* re-use default grub GDT code descriptor */
   for(i=0 ; i<IDT_NR_DESC ; i++, isr += IDT_ISR_ALGN)
      int_desc(&IDT[i], gdt_krn_seg_sel(1), isr);

   idtr.desc  = IDT;
   idtr.limit = sizeof(IDT) - 1;
   set_idtr(idtr);
}

void __regparm__(1) irq32_hdlr(int_ctx_t* ctx) {
    /* asm volatile("int $48"::"S"("int32_hdlr called")); */
    jump_to_next_task(ctx);
}

void __regparm__(1) int48_hdlr(int_ctx_t *ctx) {
    // Q3: print %eax
    // Q4: print "%s" located in "ESI"
    debug("SYSCALL eax = %p, \ttext = '%s'\n", ctx->gpr.eax, ctx->gpr.esi);
}

void __regparm__(1) int80_hdlr(int_ctx_t* ctx) {
    uint32_t* counter = (uint32_t*) (ctx->gpr.esi.raw);
    if (USR2_COUNT <= (uint32_t)counter && (uint32_t)counter + sizeof(*counter) <= USR2_COUNT + PG_4K_SIZE) {
        if(VERBOSE) {
            debug("int80_hdlr - count = %d\n", *counter);
        } else {
            debug("\rint80_hdlr - count = %d", *counter);
        }
    } else {
        panic("INT80_hdlr - bad address: counter=%p, *counter=%d\n", counter, *counter);
    }
}

void __regparm__(1) intr_hdlr(int_ctx_t *ctx) {
    uint8_t vector = ctx->nr.blow;

    switch(vector) {
        case IRQ32_EXCP:
            irq32_hdlr(ctx);
            break;

        case SYS_INT48:
            int48_hdlr(ctx);
            break;

        case SYS_INT80:
            int80_hdlr(ctx);
            break;

        default:
            debug("\nIDT event\n"
                " . int    #%d\n"
                " . error  0x%x\n"
                " . cs:eip 0x%x:0x%x\n"
                " . ss:esp 0x%x:0x%x\n"
                " . eflags 0x%x\n"
                "\n- GPR\n"
                "eax     : 0x%x\n"
                "ecx     : 0x%x\n"
                "edx     : 0x%x\n"
                "ebx     : 0x%x\n"
                "esp     : 0x%x\n"
                "ebp     : 0x%x\n"
                "esi     : 0x%x\n"
                "edi     : 0x%x\n"
                ,ctx->nr.raw, ctx->err.raw
                ,ctx->cs.raw, ctx->eip.raw
                ,ctx->ss.raw, ctx->esp.raw
                ,ctx->eflags.raw
                ,ctx->gpr.eax.raw
                ,ctx->gpr.ecx.raw
                ,ctx->gpr.edx.raw
                ,ctx->gpr.ebx.raw
                ,ctx->gpr.esp.raw
                ,ctx->gpr.ebp.raw
                ,ctx->gpr.esi.raw
                ,ctx->gpr.edi.raw);

        if(vector < NR_EXCP) {
            excp_hdlr(ctx);
        } else {
            debug("ignore IRQ %d\n", vector);
        }
    }
}
