#include <cr.h>
#include <debug.h>
#include <paging.h>
#include <print.h>

pde32_t* pgd_init(uint32_t addr) {
    pde32_t* pgd = (pde32_t*)addr;
    memset((void*)pgd, 0, PAGE_SIZE);
    set_cr3(pgd);
    return pgd;
}

void pgd_print(const pde32_t* pgd) {
    int i, j;
    debug("\n▓▓▓▓ G D T ▓▓▓▓▒▒▒ (@='%p') ▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
          "▓  -- @ PTB --\t-- Physical address --\t-- Virtual address --\t░\n", pgd);
    for(i = 0; i < 1024; ++i) {
        if(pg_present(&pgd[i])) {
            // PTB found
            const pte32_t* ptb = (pte32_t*)pg_4K_addr(pgd[i].addr);
            bool_t is_linear = true;
            uint32_t prev_addr = ptb[0].addr;

            for(j = 1; j < 1024 && is_linear; ++j) {
                is_linear = (is_linear && pg_present(&ptb[j]) && (ptb[j].addr - prev_addr) == 1);
                prev_addr = ptb[j].addr;
            }
            debug("▓ (@=%p)\t0x%x--- -> 0x%x---\t", pg_4K_addr(pgd[i].addr), i << 10, (i << 10) + 1023);
            if(is_linear) {
                debug("0x%x -> 0x%x\t░\n", pg_4K_addr(ptb[0].addr), pg_4K_addr(ptb[1023].addr));
            } else {
                // Printing PTB details
                debug("\t<not linear>\t░\n");

                uint32_t ptb_prev_addr = ptb[0].addr;
                int ptb_prev_idx = 0;
                for(j = 1; j < 1024; ++j) {
                    if((pg_present(&ptb[j]) && ptb[j].addr - ptb_prev_addr != 1) || j == 1023) {
                        debug("▓\t|pe:\t.0x%x--- -> 0x%x---\t.0x%x -> 0x%x\t░\n", (i << 10) + ptb_prev_idx, (i << 10) + j - 1, pg_4K_addr(ptb[ptb_prev_idx].addr), pg_4K_addr(ptb[j - 1].addr));
                        ptb_prev_idx = j;
                    }
                        ptb_prev_addr = ptb[j].addr;
                }
            }
        }
    }
    debug("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░\n\n");
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
    /* uint32_t offset = pt32_idx(physical_addr); */
    physical_addr &= 0xFFFFF;
    /* physical_addr &= 0xFFF; */
    attr          &= 0xFFF;
    pg_set_entry(&ptb[index], attr | PG_P, physical_addr);
    /* pg_set_entry(&ptb[index], attr | PG_P, offset); */
}

pte32_t* pgd_create_page(pde32_t* pgd, uint32_t virt_addr, uint32_t physical_addr, uint16_t attr) {
    debug("virt_addr=%p\n", virt_addr);
    uint32_t pgd_index = pd32_idx(virt_addr);
    uint32_t ptb_index = pt32_idx(virt_addr);
    uint16_t ptb_addr  = (uint32_t)(pgd[pgd_index].addr << 12);
    pte32_t* ptb = pgd_register_ptb(pgd, ptb_addr, pgd_index, attr);
    ptb_register_page(ptb, pt32_idx(physical_addr), ptb_index, attr);
    /* ptb_register_page(ptb, physical_addr, ptb_index, attr); */
    debug("Creation of a page with pgd=%p, pgd_index=%d, ptb_addr=%p, ptb=%p, ptb_index=%d (%p)\n", pgd, pgd_index, ptb_addr, ptb, ptb_index, &ptb[ptb_index]);
    return ptb;
}

void activate_cr0(void) {
    uint32_t cr0 = get_cr0();
    set_cr0(cr0 | CR0_PG);
}
