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
    const char msg[] = "int80_hdlr - count = ";
    if (USR2_COUNT <= (uint32_t)counter && (uint32_t)counter + sizeof(*counter) <= USR2_COUNT + PG_4K_SIZE) {
        if(VERBOSE) {
            debug("%s%d\n", msg, *counter);
        } else {
            debug("\r");
            for(uint8_t i = 0; i < sizeof(msg); ++i) {
                if(i > 0 && (*counter % sizeof(msg) + 1 == i)) {
                    debug(C_BLUE "%c" S_RST, msg[i]);
                } else if(*counter % sizeof(msg) == i) {
                    debug(S_BOLD C_CYAN "%c" S_RST, msg[i]);
                } else if(i < sizeof(msg) - 1 && (*counter % sizeof(msg) - 1 == i)) {
                    debug(C_BLUE "%c" S_RST, msg[i]);
                } else {
                    debug("%c", msg[i]);
                }
            }
            debug("\x1b[3%dm%d" S_RST, (*counter / 10) % 8 + 1, *counter);
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
            print_int_ctx_t(ctx, "IDT event");

        if(vector < NR_EXCP) {
            excp_hdlr(ctx);
        } else {
            debug("ignore IRQ %d\n", vector);
        }
    }
}

void print_int_ctx_t(const int_ctx_t* ctx, const char* msg) {
    debug(
        "\n#### %s ####\n"
        "  === GPR ===\n"
        "  -> eax = %p\n"
        "  -> ecx = %p\n"
        "  -> edx = %p\n"
        "  -> ebx = %p\n"
        "  -> esp = %p\n"
        "  -> ebp = %p\n"
        "  -> esi = %p\n"
        "  -> edi = %p\n"
        "  === CPU ctx ===\n"
        "  -> int     #%d\n"
        "  -> error  = %p\n"
        "  -> eip    = %p\n"
        "  -> cs     = %p\n"
        "  -> eflags = %p\n"
        "  -> esp    = %p\n"
        "  -> ss     = %p\n",
        msg,
        ctx->gpr.eax.raw,
        ctx->gpr.ecx.raw,
        ctx->gpr.edx.raw,
        ctx->gpr.ebx.raw,
        ctx->gpr.esp.raw,
        ctx->gpr.ebp.raw,
        ctx->gpr.esi.raw,
        ctx->gpr.edi.raw,
        ctx->nr.raw,
        ctx->err.raw,
        ctx->eip.raw,
        ctx->cs.raw,
        ctx->eflags.raw,
        ctx->esp.raw,
        ctx->ss.raw);
}
