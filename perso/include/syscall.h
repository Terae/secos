#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>

// #INT 48
void sys_print_msg(const char* msg) __attribute__((section(".sys_print")));

// #INT 80
void sys_counter(uint32_t* counter) __attribute__((section(".sys_count")));

#endif
