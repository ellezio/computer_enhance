global TESTJumpTakeJmp
global TESTJumpNotTakeJmp

section .text

TESTJumpTakeJmp:
    xor rax, rax
.loop:
    cmp rax, rax
    jz .after_nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
.after_nop:
    inc rax
    cmp rax, rdi
    jb .loop

TESTJumpNotTakeJmp:
    xor rax, rax
.loop:
    cmp rax, rax
    jb .after_nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
.after_nop:
    inc rax
    cmp rax, rdi
    jb .loop
