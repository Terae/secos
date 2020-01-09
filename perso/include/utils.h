#ifndef __UTILS_H__
#define __UTILS_H__

#define debug_usr(_msg_) asm volatile("int $48"::"S"((_msg_)))

void print_secos(void);

void print_stack(uint32_t ebp, uint32_t esp);

#endif
