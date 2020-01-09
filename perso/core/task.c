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

void jump_to_next_task(int_ctx_t* ctx) {
    current_task->usr_ctx = ctx;

    {
        uint32_t prev_eip = current_task->usr_ctx->eip.raw;
        uint8_t prev_task;

        if(prev_eip >= USR1_BEGIN && prev_eip <= USR1_STACK) {
            prev_task = 1;
        } else if(prev_eip >= USR2_BEGIN && prev_eip <= USR2_STACK) {
            prev_task = 2;
        } else if(prev_eip >= KRN_CODE && prev_eip <= KRN_STACK_END) {
            prev_task = 0;
        } else if(prev_eip >= SYS_PRINT && prev_eip < SYS_PRINT + 0x10000) {
            prev_task = 3;
        } else if(prev_eip >= SYS_COUNT && prev_eip < SYS_COUNT + 0x10000) {
            prev_task = 4;
        } else {
            panic("\n -= Interruption from an unauthorized memory area: eip=%p /!\\\n", prev_eip);
        }

        if(VERBOSE) {
            debug("\n\n\n -= Interruption of '");
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
            debug("' from eip=%p =-\n", prev_eip);

            debug("\n### Previous task:\n");
            print_task(current_task);
        }
    }

    current_task = current_task->next_task;

    if(VERBOSE) {
        pgd_print(current_task->pgd);
        debug("### Next task:\n");
        print_task(current_task);
    }

    set_cr3(current_task->pgd);
    TSS.s0.esp = (uint32_t)current_task->krn_stack;

    asm volatile (
        // Stack reconstruction
        "push  %0;"                 // Data segment selector
        "push  %[next_esp];"        // User stack
        "pushl %[next_fla];"        // Flags with `sti`
        "push  %1;"                 // Code segment selector
        "pushl %[next_eip];"        // eip
        // %ebp
        "mov %[next_ebp], %%ebp;"
        // inter return
        "iret;"
        ::
        "i" usr_data,
        "i" usr_code,
        [next_esp] "m" (current_task->usr_ctx->esp.raw),
        [next_fla] "r" (current_task->usr_ctx->eflags.raw | EFLAGS_IF),
        [next_eip] "m" (current_task->usr_ctx->eip.raw),
        [next_ebp] "m" (current_task->usr_ctx->gpr.ebp.raw));
}

void task_init(task_t* task, uint32_t eip, uint32_t* stack_kernel, uint32_t* stack_user, pde32_t* pgd, task_t* next) {
    task->krn_stack                     = stack_kernel;
    task->usr_ctx                       = (int_ctx_t*)task->krn_stack - 1;
    task->usr_ctx->gpr.ebp.raw          = (uint32_t)stack_user;
    task->usr_ctx->eip.raw              = eip;
    task->usr_ctx->cs.raw               = usr_code;
    task->usr_ctx->eflags.raw           = EFLAGS_IF;
    task->usr_ctx->esp.raw              = (uint32_t)stack_user;
    task->usr_ctx->ss.raw               = usr_data;
    task->pgd                           = pgd;
    task->next_task                     = next;
}

void print_task(const task_t* task) {
    const int_ctx_t* ctx = (int_ctx_t*)task->usr_ctx;
    debug("ebp_krn_stack = %p\n"
          "esp_krn_stack = %p\n"
          "   -> eip = %p\n"
          "   -> cs  = %p\n"
          "   -> efl = %p\n"
          "   -> esp = %p\n"
          "   -> ss  = %p\n"
          "next_task = %p\n",
          task->krn_stack,
          ctx,
          ctx->eip.raw,
          ctx->cs.raw,
          ctx->eflags.raw,
          ctx->esp.raw,
          ctx->ss.raw,
          task->next_task);
}
