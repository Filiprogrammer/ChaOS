; start.asm

[BITS 32]
extern __bss_start
extern __end
extern _main
extern _exit
global _start

_start:
    mov edi, __bss_start
    mov ecx, __end
    sub ecx, __bss_start
    mov al, 0
    rep stosb   ; repeats instruction decrementing ECX until zero
                ; and stores value from AL incrementing ES:EDI

    call _main
    call _exit
    jmp $
