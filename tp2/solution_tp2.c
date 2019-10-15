/* GPLv2 (c) Airbus */
#include <debug.h>
#include <segmem.h>
#include <info.h>

extern info_t *info;

// 2
void test_sti()
{
   while (1) asm volatile ("sti");
}

void bp_handler()
{
   // 3.8
   asm volatile ("pusha");

   // 3.2
   debug("#BP furtif\n");

   // 3.6: affiche l'adresse de l'instruction apres int3
   uint32_t eip;
   asm volatile ("mov 4(%%ebp), %0":"=r"(eip));
   debug("EIP = %p\n", eip);

   // 3.5: termine correctement, réaligne la pile avant un "iret"
   //asm volatile ("leave ; iret");

   // 3.8: restore les GPRs avant de quitter
   asm volatile ("popa ; leave ; iret");
}

void bp_trigger()
{
   // 3.3
   asm volatile ("int3");

   // 3.7
   debug("after trigger bp\n");
}

void test_bp()
{
   idt_reg_t idtr;

   // 3.1
   get_idtr(idtr);
   debug("IDT @ %p\n", idtr.addr);

   // 3.4
   int_desc_t *bp_dsc = &idtr.desc[3];

   bp_dsc->offset_1 = (uint16_t)((uint32_t)bp_handler);
   bp_dsc->offset_2 = (uint16_t)(((uint32_t)bp_handler)>>16);

   bp_trigger();

   // le handler est incorrect, lorsqu'il termine son exécution, sa
   // pile est incorrectement alignée et il effectue un "ret" au lieu
   // d'un "iret. D'ou l'execution en EIP = 4

   /*
IDT @ 0x304fa0
#BP furtif

IDT event
 . int    #6
 . error  0xffffffff
 . cs:eip 0x8:0x4c
 . ss:esp 0x8:0x60
 . eflags 0x6

- GPR
eax     : 0x3c
ecx     : 0x9
edx     : 0xb
ebx     : 0x30
esp     : 0x301f78
ebp     : 0x304b9f
esi     : 0x60
edi     : 0x2bfc7

Exception: Invalid Opcode
   */
}

void tp()
{
   // test_sti();
   test_bp();
}
