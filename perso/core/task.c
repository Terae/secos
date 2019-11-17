#include <cr.h>
#include <debug.h>
#include <info.h>
#include <mapping.h>
#include <paging.h>
#include <segmentation.h>
#include <task.h>
#include <utils.h>

extern void resume_from_intr();

extern info_t* info;
extern tss_t TSS;
task_t* current_task;

uint8_t check_interrupted_task(uint32_t eip);

void jump_to_next_task(int_ctx_t* int_ctx) {
    int_ctx_t* ctx = (int_ctx_t*)&current_task->esp_krn_stack;
    uint8_t prev = check_interrupted_task(int_ctx->eip.raw);
    uint32_t* prev_krn_esp = (uint32_t*)int_ctx->gpr.esp.raw;
    uint32_t* prev_usr_esp = (uint32_t*)int_ctx->esp.raw;

    if(VERBOSE) {
        print_int_ctx_t(int_ctx, "Incoming int_ctx");

        debug("\n### Previous task:\n");
        print_task(current_task);
        print_stack((uint32_t)&current_task->ebp_krn_stack, (uint32_t)&current_task->esp_krn_stack);

        debug("\n\n\nctx = %p\n", ctx);
        debug("prev_krn_esp = %p\n", prev_krn_esp);
        debug("prev_usr_esp = %p\n", prev_usr_esp);
        debug("current_task->esp = %p\n", current_task->esp_krn_stack);
    }

    current_task = current_task->next_task;
    if(prev == 1 || prev == 2) {
        /* current_task->esp_krn_stack = prev_krn_esp; */
    }

    uint32_t* next_krn_esp = current_task->esp_krn_stack;

    if(VERBOSE) {
        debug("next_krn_esp = %p\n", next_krn_esp);

        pgd_print(current_task->pgd);
        debug("### Next task:\n");
        print_task(current_task);

        ctx = (int_ctx_t*)current_task->esp_krn_stack;

        print_int_ctx_t(ctx, "Stack reconstruction");
        debug("\n### Print stack\n");
        print_stack((uint32_t)current_task->ebp_krn_stack, (uint32_t)current_task->esp_krn_stack);

        asm volatile("nop; nop; nop; nop;");
    }

    set_cr3(current_task->pgd);
    TSS.s0.esp = (uint32_t)next_krn_esp;

    asm volatile(
            "pushl %%ebp;"
            "movl %%esp, (%[prev_esp]);"
            "movl %[next_esp], %%esp;"
            /* "mov %[next_ebp], %%ebp;" */
            "popl %%ebp;"
            /* "push %[next_eip];" */
            "ret;"
            ::
            [prev_esp] "a" (prev_krn_esp),
            [next_esp] "b" (next_krn_esp)/*,
            [next_eip] "r" (next_eip),
            [next_ebp] "r" (current_task->ebp_krn_stack)*/);

    debug("%p\n", int_ctx->eip.raw);
}

void task_init(task_t* task, uint32_t eip, uint32_t* stack_kernel, uint32_t* stack_user, pde32_t* pgd, task_t* next) {
    task->ebp_krn_stack     = stack_kernel;
    task->esp_krn_stack     = task->ebp_krn_stack;

    task->esp_krn_stack     = (uint32_t*)(task->esp_krn_stack - sizeof(int_ctx_t) / 4);

    int_ctx_t* ctx          = (int_ctx_t*)task->esp_krn_stack;
    debug("ctx:\t%p\n", ctx);
    ctx->eip.raw            = eip;
    ctx->cs.raw             = usr_code;
    ctx->eflags.raw         = EFLAGS_IF;
    ctx->esp.raw            = (uint32_t)stack_user;
    ctx->ss.raw             = usr_data;
    ctx->gpr.eax.raw        = 0xA;
    ctx->gpr.ebx.raw        = 0xB;
    ctx->gpr.ecx.raw        = 0xC;
    ctx->gpr.edx.raw        = 0xD;
    ctx->gpr.esp.raw        = 0xE;
    ctx->gpr.ebp.raw        = (uint32_t)stack_user;
    ctx->gpr.esi.raw        = 0x1;
    ctx->gpr.edi.raw        = 0x2;
    memcpy((void*)task->ebp_krn_stack, (void*)ctx, sizeof(int_ctx_t));

    *(--task->esp_krn_stack)= (offset_t)resume_from_intr;
    *(--task->esp_krn_stack)= 0xfade; // fake EBP
    /* *(--task->esp_krn_stack)=eip; */

    debug("esp:\t%p\n", task->esp_krn_stack);

    /* print_stack((uint32_t)task->ebp_krn_stack, (uint32_t)task->esp_krn_stack); */

    task->pgd               = pgd;
    task->next_task         = next;
}

void print_task(const task_t* task) {
    const int_ctx_t* ctx = (int_ctx_t*)task->esp_krn_stack;
    debug("ebp_krn_stack = %p\n"
          "esp_krn_stack = %p\n"
          "   -> eip = %p\n"
          "   -> cs  = %p\n"
          "   -> efl = %p\n"
          "   -> esp = %p\n"
          "   -> ss  = %p\n"
          "next_task = %p\n",
          task->ebp_krn_stack,
          ctx,
          ctx->eip.raw,
          ctx->cs.raw,
          ctx->eflags.raw,
          ctx->esp.raw,
          ctx->ss.raw,
          task->next_task);
}

uint8_t check_interrupted_task(uint32_t eip) {
    uint8_t prev_task;

    if(eip >= USR1_BEGIN && eip <= USR1_STACK) {
        prev_task = 1;
    } else if(eip >= USR2_BEGIN && eip <= USR2_STACK) {
        prev_task = 2;
    } else if(eip >= KRN_CODE && eip <= KRN_STACK_END) {
        prev_task = 0;
    } else if(eip >= SYS_PRINT && eip < SYS_PRINT + 0x10000) {
        prev_task = 3;
    } else if(eip >= SYS_COUNT && eip < SYS_COUNT + 0x10000) {
        prev_task = 4;
    } else {
        panic("\n -= Interruption from an unauthorized memory area: eip=%p /!\\\n", eip);
    }

    if(VERBOSE) {
        debug(C_RED S_BOLD "\n\n\n -= Interruption of '");
        if(prev_task == 0) {
            debug("the kernel");
        } else if(prev_task == 1) {
            debug("user1()");
        } else if(prev_task == 2) {
            debug("user2()");
        } else if(prev_task == 3) {
            debug("sys_print()");
        } else if(prev_task == 4) {
            debug("sys_count()");
        } else {
            panic("\nUnknown prev_task type: %d\n", prev_task);
        }
        debug("' from eip=%p =-\n" S_RST, eip);
    }

    return prev_task;
}
