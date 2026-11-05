global write_x1
global write_x2
global write_x3
global write_x4

section .text

write_x1:
    align 64
.loop:
    mov [rsi], 1 
    sub rdi, 1
    jnle .loop
    ret

write_x2:
    align 64
.loop:
    mov [rsi], 1 
    mov [rsi], 1 
    sub rdi, 2
    jnle .loop
    ret

write_x3:
    align 64
.loop:
    mov [rsi], 1 
    mov [rsi], 1 
    mov [rsi], 1 
    sub rdi, 3
    jnle .loop
    ret

write_x4:
    align 64
.loop:
    mov [rsi], 1 
    mov [rsi], 1 
    mov [rsi], 1 
    mov [rsi], 1 
    sub rdi, 4
    jnle .loop
    ret
