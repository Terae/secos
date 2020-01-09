#include <cr.h>
#include <pagination.h>
#include <print.h>

pde32_t* pgd_init(uint32_t addr) {
    pde32_t* pgd = (pde32_t*)addr;
    memset((void*)pgd, 0, PAGE_SIZE);
    set_cr3(pgd);
    return pgd;
}

pte32_t* pgd_register_ptb(pde32_t* pgd, uint32_t ptb_addr, uint16_t index, uint16_t attr) {
    if(index > 1023) {
        panic("== invalid PGD index: '%d > 1023'\n", index);
    }
    pg_set_entry(&pgd[index], (attr & 0xFFF) | PG_P, page_nr(ptb_addr));
    return (pte32_t*)ptb_addr;
}

void ptb_register_page(pte32_t* ptb, uint32_t physical_addr, uint16_t index, uint16_t attr) {
    if(index > 1023) {
        panic("== invalid PTB index: '%d > 1023'\n", index);
    }
    physical_addr &= 0xFFF;
    attr          &= 0xFFF;
    pg_set_entry(&ptb[index], attr | PG_P, physical_addr);
}

pte32_t* pgd_create_page(pde32_t* pgd, uint32_t virt_addr, uint32_t physical_addr, uint16_t attr) {
    uint32_t pgd_index = pd32_idx(virt_addr);
    uint32_t ptb_index = pt32_idx(virt_addr);
    uint16_t ptb_addr  = (uint32_t)(pgd[pgd_index].addr << 12);
    pte32_t* ptb = pgd_register_ptb(pgd, ptb_addr, pgd_index, attr);
    ptb_register_page(ptb, pt32_idx(physical_addr), ptb_index, attr);
    return ptb;
}

void activate_cr0(void) {
    uint32_t cr0 = get_cr0();
    set_cr0(cr0 | CR0_PG);
}
