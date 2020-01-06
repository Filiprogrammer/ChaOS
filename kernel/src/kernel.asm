[Bits 32]
section .text           ; ld needs that for coff format
global KernelStart
extern __bss_start 
extern __bss_end
extern _main            ; entry point in ckernel.c

KernelStart:
    ; set bss to zero
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, __bss_start
    mov al, 0
    rep stosb           ; repeats instruction decrementing ECX until zero
                        ; and stores value from AL incrementing ES:EDI

    mov esp, 0x190000   ; set stack below 2 MB limit

    call _main          ; ->-> C-Kernel

    cli
    hlt
