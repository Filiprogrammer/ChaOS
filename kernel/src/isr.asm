section .text

%macro IR_ROUTINE 1
    _ir%1:
        cli
        %if ( %1 != 8 ) && ( %1 < 10 || %1 > 14 )
            push dword 0
        %endif
        push dword %1
        jmp ir_common_stub
%endmacro

; Create the 48 interrupt-routines
%assign routine_nr 0
%rep 48
    IR_ROUTINE routine_nr
    %assign routine_nr routine_nr+1
%endrep

IR_ROUTINE 127 ; Software Interrupt Sys Call
IR_ROUTINE 126

; Call of the C function irq_handler(...)
extern _irq_handler

; Common ISR stub saves processor state, sets up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
ir_common_stub:
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp              ; parameter of _fault_handler 
    call _irq_handler
    global _irq_tail
    _irq_tail:
    mov esp, eax          ; return value: changed or unchanged esp

    pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
    iret

global _idt_install

_idt_install:
    %macro DO_IDT_ENTRY 3
        mov ecx, _ir%1
        mov [_idt_table+ %1*8], cx
        mov [_idt_table+ %1*8+2], WORD %2
        mov [_idt_table+ %1*8+4], WORD %3
        shr ecx, 16
        mov [_idt_table+ %1*8+6], cx
    %endmacro

    %assign COUNTER 0
    %rep 48
        DO_IDT_ENTRY COUNTER, 0x0008, 0x8E00
        %assign COUNTER COUNTER+1
    %endrep

    DO_IDT_ENTRY 127, 0x0008, 0xEE00
    DO_IDT_ENTRY 126, 0x0008, 0xEE00

    ; Remap IRQ 0-15 to 32-47
    %macro putport 2
        mov al, %2
        out %1, al
    %endmacro
    putport 0x20, 0x11
    putport 0xA0, 0x11
    putport 0x21, 0x20
    putport 0xA1, 0x28
    putport 0x21, 0x04
    putport 0xA1, 0x02
    putport 0x21, 0x01
    putport 0xA1, 0x01
    putport 0x21, 0x00
    putport 0xA1, 0x00

    lidt [idt_descriptor]
    ret

section .data

_idt_table:
    times 256*8 db 0

idt_descriptor:
    dw 0x07FF
    dd _idt_table
