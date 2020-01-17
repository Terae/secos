.text

.global switch_stack
.type   switch_stack,"function"

switch_stack:
    # save old ebp
    pushl   %ebp

    # switch kernel stacks
    movl    %esp, (%ecx)
    movl    %edx, %esp

    # load new ebp
    popl    %ebp
    ret
