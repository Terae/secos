#include <syscall.h>

void __attribute__((section(".sys_print"))) sys_print_msg(const char* msg) {
    asm volatile ("int $48"::"S"(msg));
}

void __attribute__((section(".sys_count"))) sys_counter(uint32_t* counter) {
    asm volatile ("mov %0, %%esi" :: "m"(counter));
    asm volatile ("int $80");
}
