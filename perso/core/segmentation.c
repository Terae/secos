#include <cr.h>
#include <mapping.h>
#include <segmentation.h>

seg_desc_t GDT[GDT_SIZE];
tss_t TSS;

void perso_print_gdt(void) {
    gdt_reg_t dgtr;
    get_gdtr(dgtr);
    debug("### GDT ###\n");

    debug("Size of GDT:\t\t%d\n", dgtr.limit);

    uint32_t n = (dgtr.limit + 1) / sizeof(seg_desc_t);

    for (uint32_t i = 0; i < n; ++i) {
        seg_desc_t* seg_desc = &dgtr.desc[i];
        uint32_t base = (seg_desc->base_3 << 24) | (seg_desc->base_2 << 16) | (seg_desc->base_1);
        uint32_t limit = (seg_desc->limit_2 << 16) | (seg_desc->limit_1);

        debug("@GDT[%d] = 0x%lx | limit=0x%x | base=0x%x | type=0x%x\n", i, seg_desc, limit, base, seg_desc->type);
        debug("seg_desc->s:      \t0x%x\n", seg_desc->s);
        debug("seg_desc->dpl:    \t0x%x\n", seg_desc->dpl);
        debug("seg_desc->p:      \t0x%x\n", seg_desc->p);
        debug("seg_desc->avl:    \t0x%x\n", seg_desc->avl);
        debug("seg_desc->l:      \t0x%x\n", seg_desc->l);
        debug("seg_desc->d:      \t0x%x\n", seg_desc->d);
        debug("seg_desc->g:      \t0x%x\n", seg_desc->g);
    }
    debug("---\n\n");
}

void perso_init_gdt(void) {
    GDT[0].raw = 0ULL;

    perso_set_seg_desc(&GDT[krn_code_idx], SEG_DESC_CODE_XR, SEG_SEL_KRN, /*KRN_BEGIN*/ 0ULL, 0xFFFFF);
    perso_set_seg_desc(&GDT[krn_data_idx], SEG_DESC_DATA_RW, SEG_SEL_KRN, /*KRN_BEGIN*/ 0ULL, 0xFFFFF);
    perso_set_seg_desc(&GDT[usr_code_idx], SEG_DESC_CODE_XR, SEG_SEL_USR, /*USR_BEGIN*/ 0ULL, 0xFFFFF);
    perso_set_seg_desc(&GDT[usr_data_idx], SEG_DESC_DATA_RW, SEG_SEL_USR, /*USR_BEGIN*/ 0ULL, 0xFFFFF);

    gdt_reg_t gdtr;
    gdtr.desc = GDT;
    gdtr.limit = sizeof(GDT) - 1;
    set_gdtr(gdtr);

    set_cs(krn_code);
    debug("7\n");

    set_ss(krn_data);
    set_ds(krn_data);
    set_es(krn_data);
    set_fs(krn_data);
    set_gs(krn_data);
}

void perso_init_tss(void) {
    offset_t base = (offset_t) &TSS;

    TSS.s0.ss = krn_data;
    TSS.s0.esp = get_ebp();
    perso_set_tss_seg(&GDT[tss_idx], base);

    set_tr(gdt_krn_seg_sel(tss_idx));
}
