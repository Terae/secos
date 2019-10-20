/* GPLv2 (c) Airbus */
#include <cr.h>
#include <debug.h>
#include <info.h>
#include <pagemem.h>

extern info_t *info;

// Question 1
void q1_print_cr3(void) {
    cr3_reg_t cr3;
    cr3.raw = get_cr3();
    debug("cr3 = %p\n", cr3.raw);
}

// Question 2
pde32_t* q2_create_pgd(void) {
    pde32_t* pgd = (pde32_t*)0x600000;
    memset((void*)pgd, 0, PAGE_SIZE);
    set_cr3(pgd);
    return pgd;
}

// Question 3
void q3_activate_cr0(void) {
    uint32_t cr0 = get_cr0();
    // Reboot: identity mapping not set
    set_cr0(cr0 | CR0_PG);
}

// Question 4
pte32_t* q4_create_ptb(pde32_t* pgd, uint32_t address, uint32_t first_index) {
    pte32_t* ptb = (pte32_t*)address;
    for(uint32_t i = 0; i < 1024; ++i) {
        // Question 6: mapping of PTBs
        pg_set_entry(&ptb[i], PG_KRN | PG_RW, i + first_index);
    }
    pg_set_entry(pgd, PG_KRN | PG_RW, page_nr(ptb));
    return ptb;
}

// Question 5
void q5_print_ptb(int pgd, pte32_t* ptb) {
    debug("PGD[%d]  \t-> PTB[0] mapped with physical %p (%p)\n", pgd, ptb[0].raw, ptb);
}

// Question 7
pte32_t* q7_make_ptb_editable(pde32_t* pgd, uint32_t address, uint32_t* modifier) {
    pte32_t* ptb = (pte32_t*)address;
    memset((void*)ptb, 0, PAGE_SIZE);

    int pgd_index = pd32_idx(modifier);
    int ptb_index = pt32_idx(modifier);

    pg_set_entry(&ptb[ptb_index], PG_KRN | PG_RW, page_nr(pgd));
    pg_set_entry(&pgd[pgd_index], PG_KRN | PG_RW, page_nr(ptb));

    debug("PGD[0] = %p | modifier = %p\n", pgd[0].raw, *modifier);

    return ptb;
}

pte32_t* _virt_addr_to_ptb(pde32_t* pgd, uint32_t virt_addr) {
    uint32_t pgd_index = pd32_idx(virt_addr);
    uint32_t ptb_index = pt32_idx(virt_addr);
    pte32_t* ptb = (pte32_t*)(pgd[pgd_index].addr << 0xC);
    return &ptb[ptb_index];
}

void q8_shared_memory(pde32_t* pgd, uint32_t virt_addr_1, uint32_t virt_addr_2, uint32_t addr_physical) {
    uint32_t* ptr = (uint32_t*)addr_physical;
    int ptb_index = pt32_idx(ptr);

    pte32_t* ptb_v1 = _virt_addr_to_ptb(pgd, virt_addr_1);
    pg_set_entry(ptb_v1, PG_KRN | PG_RW, ptb_index);

    pte32_t* ptb_v2 = _virt_addr_to_ptb(pgd, virt_addr_2);
    pg_set_entry(ptb_v2, PG_KRN | PG_RW, ptb_index);

    debug("shared_memory: V1: %p = %s\n"
          "               V2: %p = %s\n",
          virt_addr_1, virt_addr_1, virt_addr_2, virt_addr_2);
}

void q9_drop_pdg(uint32_t* modifier) {
    *modifier = 0;
    // LB are cached
    //invalidate(modifier);
}

void tp()
{
    q1_print_cr3();

    pde32_t* pgd = q2_create_pgd();

    // #PF
    //q3_activate_cr0();

    pte32_t* ptb = q4_create_ptb(&pgd[0], 0x601000, 0);

    pte32_t* ptb_q6 = q4_create_ptb(&pgd[1], 0x602000, 1024);

    q3_activate_cr0();
    // #PF: virtual address 0x700000 isn't mapped
    q5_print_ptb(0, ptb);
    q5_print_ptb(1, ptb_q6);

    uint32_t* modifier = (uint32_t*)0xC0000000;
    pte32_t* ptb_edit = q7_make_ptb_editable(pgd, 0x603000, modifier);
    q5_print_ptb(pd32_idx(0xC0000000), ptb_edit);

    q8_shared_memory(pgd, 0x700000, 0x7FF000, 0x2000);

    q9_drop_pdg(modifier);
}
