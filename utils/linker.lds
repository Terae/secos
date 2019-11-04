/* GPLv2 (c) Airbus */
OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386");
OUTPUT_ARCH("i386")

ENTRY(entry)

PHDRS
{
   phboot  PT_LOAD FLAGS (7);
   phstack PT_LOAD FLAGS (6);
   phsetup PT_LOAD FLAGS (7);
}

SECTIONS
{
   . = 0x300000;
   .mbh      : { KEEP(*(.mbh)) . = ALIGN(4);     } : phboot
   .stack    : { KEEP(*(.stack))                 } : phstack

   __kernel_start__ = .;

   .idt_jmp  : { KEEP(*(.idt_jmp))               } : phsetup
   .text     : { *(.text)                        } : phsetup
   .rodata   : { *(.rodata)                      } : phsetup
   .data     : { *(.data)                        } : phsetup
   .bss      : { *(.bss COMMON)                  } : phsetup
   /DISCARD/ : { *(.note* .indent .comment)      } : phsetup

   __kernel_end__ = .;

   . = 0xC10000;
   .sys_print : { KEEP(*(.sys_print)) . = ALIGN(4); } : phsetup
   . = 0xC20000;
   .sys_count : { KEEP(*(.sys_count)) . = ALIGN(4); } : phsetup

   . = 0x400000;
   .user1     : { KEEP(*(.user1))     . = ALIGN(4); } : phsetup

   . = 0x401000;
   .user1_cpt : { KEEP(*(.user1_cpt)) . = ALIGN(4); } : phsetup

   . = 0x403000;
   .user1_sem1 : { KEEP(*(.user1_sem1)) . = ALIGN(4); } : phsetup

   . = 0x405000;
   .user1_sem2 : { KEEP(*(.user1_sem2)) . = ALIGN(4); } : phsetup

   . = 0x800000;
   .user2     : { KEEP(*(.user2))     . = ALIGN(4); } : phsetup

   . = 0x801000;
   .user2_cpt : { KEEP(*(.user2_cpt)) . = ALIGN(4); } : phsetup

   . = 0x803000;
   .user2_sem1 : { KEEP(*(.user2_sem1)) . = ALIGN(4); } : phsetup

   . = 0x805000;
   .user2_sem2 : { KEEP(*(.user2_sem2)) . = ALIGN(4); } : phsetup
}
