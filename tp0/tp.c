/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t   *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

void tp() {
   debug("kernel mem [0x%x - 0x%x]\n", &__kernel_start__, &__kernel_end__);
   debug("MBI flags 0x%x\n", info->mbi->flags);

   multiboot_uint32_t start, stop;
   start = info->mbi->mmap_addr;
   stop = start + info->mbi->mmap_length;

   multiboot_memory_map_t* entry;

   while (start < stop) {
       entry = (multiboot_memory_map_t*) start;
       debug("begin: 0x%0X\tend: 0x%0X\ttype: %d\n", entry->addr, entry->addr + entry->len, entry->type);
       start += sizeof(multiboot_memory_map_t);
   }

   // Initialization from an available space: no seg fault
   int* ptr = (int*) 0x100005;
   debug("@: 0x%0X\tvalue: %d\n", ptr, *ptr);
   *ptr = 12;
   debug("@: 0x%0X\tvalue: %d\n", ptr, *ptr);


}
