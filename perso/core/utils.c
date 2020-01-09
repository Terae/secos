#include <debug.h>
#include <utils.h>

void print_stack(uint32_t ebp, uint32_t esp) {
    debug("\n### ESP:\n");
    while(esp < ebp) {
        debug("%p: %p\n", esp, *(uint32_t*)esp);
        esp += 4;
    }
    debug ("### EBP (%p)\n", ebp);
}
