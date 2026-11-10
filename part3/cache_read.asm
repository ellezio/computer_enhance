global read_mask

section .text

read_mask:
    xor rax, rax
    mov r8, rsi
    align 64
.loop:
    vmovdqu64 zmm0, [r8] 
    vmovdqu64 zmm0, [r8 + 64] 
    vmovdqu64 zmm0, [r8 + 128] 
    vmovdqu64 zmm0, [r8 + 192] 

    add rax, 256
    mov r8, rax
    and r8, rdx
    add r8, rsi

    cmp rax, rdi
    jb .loop
    ret
