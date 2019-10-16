/* GPLv2 (c) Airbus */
#include <asm.h>
#include <debug.h>
#include <info.h>
#include <segmem.h> // get_gdtr(), seg_desc_t et gdt_reg_t

extern info_t *info;

#define SIZE_DGR 6
seg_desc_t GDT[SIZE_DGR];
tss_t TSS;

void print_gdt(void) {
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

#define set_seg_desc(_seg_, _type_, _dpl_, _base_, _limit_) \
    (_seg_)->base_1  = (_base_) & 0xFFFF;                   \
    (_seg_)->base_2  = ((_base_) >> 16) & 0xFF;             \
    (_seg_)->base_3  = ((_base_) >> 24) & 0xFF;             \
    (_seg_)->limit_1 = (_limit_) & 0xFFFF;                  \
    (_seg_)->limit_2 = ((_limit_) >> 16) & 0xF;             \
    (_seg_)->type    = (_type_) & 0xF;                      \
    (_seg_)->dpl     = (_dpl_) & 0x3;                       \
    (_seg_)->d       = 1;                                   \
    (_seg_)->g       = 1;                                   \
    (_seg_)->s       = 1;                                   \
    (_seg_)->p       = 1;                                   \
    (_seg_)->l       = 0;

#define BEGIN_USR_SPACE 0ULL
#define BEGIN_KRN_SPACE 0xC0000000

void init_gdt(void) {
    GDT[0].raw = 0ULL;

    set_seg_desc(&GDT[1], SEG_DESC_CODE_XR, SEG_SEL_KRN, BEGIN_USR_SPACE, 0xFFFFF);
    set_seg_desc(&GDT[2], SEG_DESC_DATA_RW, SEG_SEL_KRN, BEGIN_USR_SPACE, 0xFFFFF);
    set_seg_desc(&GDT[3], SEG_DESC_CODE_XR, SEG_SEL_USR, BEGIN_USR_SPACE, 0xFFFFF);
    set_seg_desc(&GDT[4], SEG_DESC_DATA_RW, SEG_SEL_USR, BEGIN_USR_SPACE, 0xFFFFF);

    gdt_reg_t gdtr;
    gdtr.desc = GDT;
    gdtr.limit = sizeof(GDT) - 1;
    set_gdtr(gdtr);

    set_cs(gdt_krn_seg_sel(1));

    set_ss(gdt_krn_seg_sel(2));
    set_ds(gdt_krn_seg_sel(2));
    set_es(gdt_krn_seg_sel(2));
    set_fs(gdt_krn_seg_sel(2));
    set_gs(gdt_krn_seg_sel(2));
}

void userland(void) {
    debug("# Inside userland\n");
    asm volatile("mov %eax, %cr0");
}

void question_3_1(void) {
    debug("## Question 3.1: loading DS/ES/FS/GS\n");

    debug("ds\n");
    set_ds(gdt_usr_seg_sel(4));
    debug("es\n");
    set_es(gdt_usr_seg_sel(4));
    debug("fs\n");
    set_fs(gdt_usr_seg_sel(4));
    debug("gs\n");
    set_gs(gdt_usr_seg_sel(4));
}

void question_3_2(void) {
    debug("## Question 3.2: loading SS\n");
    // #GP
    debug("ss\n");
    set_ss(gdt_usr_seg_sel(4));
}

void question_3_3(void) {
    debug("## Question 3.3: farjump\n");
    // #GP
    fptr32_t fptr;
    fptr.segment = gdt_usr_seg_sel(3);
    fptr.offset = (offset_t)userland;
    farjump(fptr);
}

void question_3_4_1(void) {
    // Invalid TSS: `mov cr0` throws a #GP with new level
    asm volatile("push %0;"    // ss
                 "push %%ebp;" // esp
                 "pushf;"      // eflags
                 "push %1;"    // cs
                 "push %2;"    // eip
                 "iret;"
                 :: "i"(gdt_usr_seg_sel(4)),
                    "i"(gdt_usr_seg_sel(3)),
                    "r"(&userland));
}

void question_3_4_2(void) {
    debug("## Question 3.4 - Initialization of TSS\n");
    offset_t base = (offset_t) &TSS;
    uint32_t limit = sizeof(TSS) - 1;

    TSS.s0.ss = gdt_krn_seg_sel(2);
    TSS.s0.esp = get_ebp();
    set_seg_desc(&GDT[5], SEG_DESC_SYS_TSS_AVL_32, SEG_SEL_KRN, base, limit);
    GDT[5].s = 0;

    set_tr(gdt_krn_seg_sel(5));

    debug("## iret: ring 0 to ring 3\n");
    force_interrupts_off();
    asm volatile("push %0;"    // ss
                 "push %%ebp;" // esp
                 "pushf;"      // eflags
                 "push %1;"    // cs
                 "push %2;"    // eip
                 "iret;"
                 :: "i"(gdt_usr_seg_sel(4)),
                    "i"(gdt_usr_seg_sel(3)),
                    "r"(&userland));

    debug("Inside ring 3\n");
}

void tp()
{
    init_gdt();

    debug("\nGDT updated\n");
    print_gdt();

    debug("Loading ring 3\n");
    question_3_1();
    //question_3_2();
    //question_3_3();
    question_3_4_2();

    printf("End of TP3\n");
}
