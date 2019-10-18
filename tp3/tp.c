/* GPLv2 (c) Airbus */
#include <asm.h>
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <utilities.h>

extern info_t *info;

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
    perso_init_tss();

    debug("## iret: ring 0 to ring 3\n");
    perso_ring0_to_ring3(&userland);
}

void tp()
{
    perso_init_gdt();

    debug("\nGDT updated\n");
    perso_print_gdt();

    debug("Loading ring 3\n");
    question_3_1();
    //question_3_2();
    //question_3_3();
    question_3_4_2();

    printf("End of TP3\n");
}
