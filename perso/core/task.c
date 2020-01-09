#include <cr.h>
#include <debug.h>
#include <info.h>
#include <mapping.h>
#include <paging.h>
#include <segmentation.h>
#include <task.h>
#include <utils.h>

#define VERBOSE 0

extern void resume_from_intr();

extern info_t* info;
extern tss_t TSS;
task_t* current_task;

void jump_to_next_task(int_ctx_t* ctx) {
    if(VERBOSE) {
        uint32_t ctx_eip = ctx->eip.raw;
        debug("\n -= Interruption of ");
        if(ctx_eip >= USR1_BEGIN && ctx_eip <= USR1_STACK) {
            debug("user1()");
        } else if(ctx_eip >= USR2_BEGIN && ctx_eip <= USR2_STACK) {
            debug("user2()");
        } else if(ctx_eip >= KRN_CODE && ctx_eip <= KRN_STACK_END) {
            debug("the kernel");
        } else if(ctx_eip >= SYS_PRINT && ctx_eip < SYS_PRINT + 0x10000) {
            debug("sys_print()");
        } else if(ctx_eip >= SYS_COUNT && ctx_eip < SYS_COUNT + 0x10000) {
            debug("sys_count()");
        } else {
            panic("an unauthorized memory area: eip=%p /!\\\n", ctx_eip);
        }
        debug(" from eip=%p =-\n", ctx_eip);

    debug("\n### Previous task:\n");
    task_print(current_task);
    }

    uint32_t prev_ebp = (uint32_t)current_task->usr_ctx->gpr.ebp.raw;
    uint32_t prev_esp = (uint32_t)current_task->usr_ctx->esp.raw;
    uint32_t prev_eip = (uint32_t)current_task->usr_ctx->eip.raw;
    uint32_t prev_fla = (uint32_t)current_task->usr_ctx->eflags.raw;

    // PTBs switch
    current_task->pgd[current_task->ptb_idx].p = 0;
    {
        const pte32_t* ptb = (pte32_t*)pg_4K_addr(current_task->pgd[current_task->ptb_idx].addr);
        uint32_t addr = pg_4K_addr(ptb[0].addr);
        invalidate(addr);
    }
    current_task = current_task->next_task;
    current_task->pgd[current_task->ptb_idx].p = 1;

    if(VERBOSE) {
        pgd_print(current_task->pgd);
        debug("Next task:\n");
        task_print(current_task);
    }

    set_cr3(current_task->pgd);
    TSS.s0.esp = (uint32_t)current_task->krn_stack;

    uint32_t next_ebp = (uint32_t)current_task->usr_ctx->gpr.ebp.raw;
    uint32_t next_esp = (uint32_t)current_task->usr_ctx->esp.raw;
    uint32_t next_eip = (uint32_t)current_task->usr_ctx->eip.raw;
    uint32_t next_fla = (uint32_t)current_task->usr_ctx->eflags.raw | EFLAGS_IF;

    if(VERBOSE) {
        debug("switch ctx:\n"
            "  -> prev_ebp = %p\n"
            "  -> next_ebp = %p\n"
            "  -> prev_esp = %p\n"
            "  -> next_esp = %p\n"
            "  -> prev_eip = %p\n"
            "  -> next_eip = %p\n"
            "  -> prev_fla = %p\n"
            "  -> next_fla = %p\n",
            prev_ebp,
            next_ebp,
            prev_esp,
            next_esp,
            prev_eip,
            next_eip,
            prev_fla,
            next_fla);
    }

    asm volatile (
        "push  %0;" // Data segment selector
        "push  %[next_esp];" // stack
        "pushl %[next_fla];"
        "push  %1;" // Code segment selector
        "pushl %[next_eip];" // eip
        "pushl %[next_ebp];" // ebp
        "popl  %%ebp;"
        "iret;"
        ::
        "i" usr_data, // DATA segment
        "i" usr_code,
        [next_ebp] "m" (next_ebp),
        [next_esp] "m" (next_esp),
        [next_fla] "m" (next_fla),
        [next_eip] "m" (next_eip));

    asm volatile("pushfl;"                  // save flags
                 /* "pop %%eax;" */
                 /* "or $512, %%eax;" */
                 /* "push %%eax;" */
                 "pushl %%ebp;"             // save old %ebp
                 "movl %%esp, %[prev_esp];" // save    ESP
                 "movl %[next_esp], %%esp;" // restore ESP
                 "movl $1f, %[prev_eip];"   // save    EIP
                 "pushl %[next_eip];"       // restore EIP
                 "1:\t"
                 "popl %%ebp;"              // load new %ebp
                 /* "popfl;"                   // restore flags */
                 /* "sti;" */
                 "iret;"

                 // output parameters
                 : [prev_esp] "=m" (prev_esp),
                   [prev_eip] "=m" (prev_eip)
                 // input  parameters
                 : [next_esp]  "m" (next_esp),
                   [next_eip]  "m" (next_eip)
                 // reloaded segment registers
                 : "memory"
                );
}

void task_init(task_t* task, uint32_t eip, uint32_t* stack_kernel, uint32_t* stack_user, pde32_t* pgd, uint16_t ptb_idx, task_t* next) {
    task->krn_stack                     = stack_kernel;
    /* ////////////////// */
    /* task->usr_ctx                       = (int_ctx_t*)0xdead; */
    /* task->usr_ctx--; */
    /* task->usr_ctx                       = (int_ctx_t*)resume_from_intr; */
    /* task->usr_ctx--; */
    /* ////////////////// */
    task->usr_ctx                       = (int_ctx_t*)task->krn_stack - 1;
    task->usr_ctx->gpr.ebp.raw          = (uint32_t)stack_user;
    task->usr_ctx->eip.raw              = eip;
    task->usr_ctx->cs.raw               = usr_code;
    task->usr_ctx->eflags.raw           = EFLAGS_IF;
    task->usr_ctx->esp.raw              = (uint32_t)stack_user;
    task->usr_ctx->ss.raw               = usr_data;
    /* *(((uint32_t*)--(task)->usr_ctx))   = (offset_t)resume_from_intr; */
    /* *(((uint32_t*)--(task)->usr_ctx))   = 0xdead; // Fake %ebp */
    task->pgd                           = pgd;
    task->ptb_idx                       = ptb_idx;
    task->next_task                     = next;

    //////////////////
    *((uint32_t*)task->usr_ctx->esp.raw) = 0xdead;
    debug("esp.raw: %p\n", task->usr_ctx->esp.raw);
    task->usr_ctx->esp.raw -= 4;
    debug("esp.raw--: %p\n", task->usr_ctx->esp.raw);
    /* *((uint32_t*)task->usr_ctx->esp.raw) = (uint32_t)resume_from_intr; */

    // EIP
    *((uint32_t*)task->usr_ctx->esp.raw) = eip;
    task->usr_ctx->esp.raw -= 4;
    /* // building iret stack /////////////// */
    /* // CS */
    /* *((uint32_t*)task->usr_ctx->esp.raw) = usr_code; */
    /* task->usr_ctx->esp.raw -= 4; */
    /* // FLAGS */
    /* *((uint32_t*)task->usr_ctx->esp.raw) = EFLAGS_IF; */
    /* task->usr_ctx->esp.raw -= 4; */
    /* // ESP3 */
    /* *((uint32_t*)task->usr_ctx->esp.raw) = (uint32_t)stack_user; */
    /* task->usr_ctx->esp.raw -= 4; */
    /* // SS3 */
    /* *((uint32_t*)task->usr_ctx->esp.raw) = usr_data; */
    /* task->usr_ctx->esp.raw -= 4; */
    /* /// building iret stack /////////////// */
    //////////////////
    print_stack((uint32_t)stack_user, (uint32_t)task->usr_ctx->esp.raw);
}

void task_print(const task_t* task) {
    debug("krn_stack = %p\n"
          "usr_ctx   = %p\n"
          "   -> eip = %p\n"
          "   -> cs  = %p\n"
          "   -> efl = %p\n"
          "   -> esp = %p\n"
          "   -> ss  = %p\n"
          "ptb_idx   = %d\n"
          "next_task = %p\n",
          task->krn_stack,
          task->usr_ctx,
          task->usr_ctx->eip.raw,
          task->usr_ctx->cs.raw,
          task->usr_ctx->eflags.raw,
          task->usr_ctx->esp.raw,
          task->usr_ctx->ss.raw,
          task->ptb_idx,
          task->next_task);
}
