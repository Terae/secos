/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h> // get_gdtr(), seg_desc_t et gdt_reg_t
#include <string.h>

extern info_t *info;

#define KERNEL_LAND 0
#define USER_LAND 3

#define SIZE_DGR 4
seg_desc_t GDT[SIZE_DGR];

/* Create a string of binary digits based on the input value.
   Input:
       val:  value to convert.
       buff: buffer to write to must be >= sz+1 chars.
       sz:   size of buffer.
   Returns address of string or NULL if not enough space provided.
*/
static char *binrep (unsigned int val, char *buff, int sz) {
    char *pbuff = buff;

    /* Must be able to store one character at least. */
    if (sz < 1) return NULL;

    /* Special case for zero to ensure some output. */
    if (val == 0) {
        *pbuff++ = '0';
        *pbuff = '\0';
        return buff;
    }

    /* Work from the end of the buffer back. */
    pbuff += sz;
    *pbuff-- = '\0';

    /* For each bit (going backwards) store character. */
    while (val != 0) {
        if (sz-- == 0) return NULL;
        *pbuff-- = ((val & 1) == 1) ? '1' : '0';

        /* Get next bit. */
        val >>= 1;
    }

    /* Padding */
    while (sz-- > 0) {
        *pbuff-- = '0';
    }

    return pbuff+1;
}

void print_gdt(void) {
    gdt_reg_t dgtr;
    get_gdtr(dgtr);
    debug("### GDT ###\n");

    debug("Size of GDT:\t\t%d\n", dgtr.limit);

    uint32_t n = (dgtr.limit + 1) / sizeof(seg_desc_t);

    char buff[5];
    for (uint32_t i = 0; i < n; ++i) {
        seg_desc_t* seg_desc = &dgtr.desc[i];
        uint32_t base = (seg_desc->base_3 << 24) | (seg_desc->base_2 << 16) | (seg_desc->base_1);
        uint32_t limit = (seg_desc->limit_2 << 16) | (seg_desc->limit_1);

        debug("@GDT[%d] = 0x%lx | limit=0x%x | base=0x%x | type=0b%s\n", i, seg_desc, limit, base, binrep(seg_desc->type, buff, 4));
        debug("seg_desc->s:      \t0b%s\n", binrep(seg_desc->s,    buff, 1));
        debug("seg_desc->dpl:    \t0b%s\n", binrep(seg_desc->dpl,  buff, 2));
        debug("seg_desc->p:      \t0b%s\n", binrep(seg_desc->p,    buff, 1));
        debug("seg_desc->avl:    \t0b%s\n", binrep(seg_desc->avl,  buff, 1));
        debug("seg_desc->l:      \t0b%s\n", binrep(seg_desc->l,    buff, 1));
        debug("seg_desc->d:      \t0b%s\n", binrep(seg_desc->d,    buff, 1));
        debug("seg_desc->g:      \t0b%s\n", binrep(seg_desc->g,    buff, 1));
    }
    debug("---\n\n");
}

void gdt_set_seg_desc_with_params(seg_desc_t* seg, uint8_t type, uint8_t dpl, uint32_t base, uint32_t limit) {
    limit &= 0xFFFFF;

    seg->base_1 = base & 0xFFFF;
    seg->base_2 = (base >> 16) & 0xFF;
    seg->base_3 = (base >> 24) & 0xFF;
    seg->limit_1 = limit & 0xFFFF;
    seg->limit_2 = (limit >> 16) & 0xF;
    seg->type = type & 0xf;
    seg->dpl = dpl & 0x3;
    seg->d = 1;
    seg->g = 1;
    seg->s = 1;
    seg->p = 1;
    seg->l = 0;
}

void gdt_set_seg_desc(seg_desc_t* seg, uint8_t type, uint8_t dpl) {
    gdt_set_seg_desc_with_params(seg, type, dpl, 0x00000000, 0xFFFFF);
}

void init_gdt() {
    GDT[0].raw = 0ULL;

    gdt_set_seg_desc(&GDT[1], SEG_DESC_CODE_XR, KERNEL_LAND);
    gdt_set_seg_desc(&GDT[2], SEG_DESC_DATA_RW, KERNEL_LAND);

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

void memset_usage() {
    // 0x600000: 6Mo
    gdt_set_seg_desc_with_params(&GDT[3], SEG_DESC_DATA_RW, KERNEL_LAND, 0x600000, 31);

    print_gdt();

    char src[64];
    char *dst = 0;
    memset(src, 0xFF, 32);

    set_es(gdt_krn_seg_sel(3));
    _memcpy8(dst, src, 64);
}

void tp()
{
    debug("Initial GDT\n");
    print_gdt();
    init_gdt();

    debug("\nGDT updated\n");
    print_gdt();

    memset_usage();
    printf("End of TP1\n");
}
