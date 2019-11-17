#ifndef __TASK_H__
#define __TASK_H__

#include <intr.h>
#include <pagemem.h>

#define VERBOSE 0

typedef union task_descriptor task_t;

extern task_t* current_task;

#define init_krn(_task_,_pgd_,_next_)               \
    ({                                              \
        (_task_)->pgd       = (pde32_t*)(_pgd_);    \
        (_task_)->next_task = (task_t*)(_next_);    \
    })

#define task_restore_ctx(_gpr_)     \
    asm volatile(                   \
            "movl %0, %%edi \n"     \
            "movl %1, %%esi \n"     \
            "movl %2, %%ebx \n"     \
            "movl %3, %%edx \n"     \
            "movl %4, %%ecx \n"     \
            "movl %5, %%eax \n"     \
            :                       \
            "=m"((_gpr_).edi.raw),  \
            "=m"((_gpr_).esi.raw),  \
            "=m"((_gpr_).ebx.raw),  \
            "=m"((_gpr_).edx.raw),  \
            "=m"((_gpr_).ecx.raw),  \
            "=m"((_gpr_).eax.raw)   \
            :)

typedef union task_descriptor
{
    struct
    {
        uint32_t* ebp_krn_stack;
        uint32_t* esp_krn_stack;
        pde32_t* pgd;
        task_t* next_task;
    };

    raw32_t;

} task_t;

void jump_to_next_task(int_ctx_t* ctx);

void task_init(task_t* task, uint32_t eip, uint32_t* stack_kernel, uint32_t* stack_user, pde32_t* pgd, task_t* next);

void print_task(const task_t* task);

#endif
