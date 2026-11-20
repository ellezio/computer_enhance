global double_loop_read

section .text

; rdi - 1st argument - inner loop counter
; rsi - 2nd argument - outer loop counter
; rdx - 3rd argument - base data pointer

double_loop_read:
    align 64
.outer_loop:
    mov r9, rdi
    mov r8, rdx
.inner_loop:
    ; read 256 bytes
    vmovdqu64 zmm0, [r8] 
    vmovdqu64 zmm0, [r8 + 64] 
    vmovdqu64 zmm0, [r8 + 128] 
    vmovdqu64 zmm0, [r8 + 192] 

    add r8, 256

    dec r9
    jnz .inner_loop

    dec rsi
    jnz .outer_loop
    ret
