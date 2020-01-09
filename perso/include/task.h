#ifndef __TASK_H__
#define __TASK_H__

#include <intr.h>
#include <pagemem.h>

typedef union task_descriptor task_t;

extern task_t* current_task;

#define init_krn(_task_,_pgd_,_next_)                   \
    ({                                                  \
        (_task_)->pgd       = (pde32_t*) (_pgd_);       \
        (_task_)->next_task = (union task_descriptor*)(_next_);   \
    })

typedef union task_descriptor
{
    struct
    {
        uint32_t* krn_stack;
        int_ctx_t* usr_ctx;
        pde32_t* pgd;
        uint16_t ptb_idx;
        task_t* next_task;
    };

    raw32_t;

} task_t;

void jump_to_next_task(int_ctx_t* ctx);

void task_init(task_t* task, uint32_t eip, uint32_t* stack_kernel, uint32_t* stack_user, pde32_t* pgd, uint16_t ptb_idx, task_t* next);

void task_print(const task_t* task);
#endif
