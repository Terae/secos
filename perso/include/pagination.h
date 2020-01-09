#ifndef __PAGINATION_H__
#define __PAGINATION_H__

#include <pagemem.h>

pde32_t* pgd_init(uint32_t addr);

pte32_t* pgd_register_ptb(pde32_t* pgd, uint32_t ptb_addr, uint16_t index, uint16_t attr);

void ptb_register_page(pte32_t* ptb, uint32_t physical_addr, uint16_t index, uint16_t attr);

pte32_t* pgd_create_page(pde32_t* pgd, uint32_t virt_addr, uint32_t physical_addr, uint16_t attr);

void activate_cr0(void);

#endif
