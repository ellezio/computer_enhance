;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 146
;  ========================================================================

global Read_1x2
global Read_8x2

section .text

;
; NOTE(casey): These ASM routines are written for the Windows
; 64-bit ABI. They expect the count in rcx and the data pointer in rdx.
;

;
; Note(ellezio): There is something like partial register stall which has
; to merge partial register to "full one". Eg. al must be merged to rax.
; This merge creates dependency on register to which partial register will
; be merged, makeing it wait for then next cycle for register to be up-to-date.
;
Read_1x2:
	align 64
.loop:
    mov al, [rsi]
    mov al, [rsi]
    sub rdi, 2
    jnle .loop
    ret

Read_8x2:
	align 64
.loop:
    mov rax, [rsi]
    mov rax, [rsi]
    sub rdi, 2
    jnle .loop
    ret

