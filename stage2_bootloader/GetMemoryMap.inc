%ifndef GETMEMORYMAP_INC
%define GETMEMORYMAP_INC

;******************************************************************************
;    INT 0x15, eax = 0xE820 BIOS function to get a memory map
;    es:di -> destination buffer for 24 byte entries
;    bp = entry count, trashes all registers except esi
;******************************************************************************

    get_memory_by_int15_e820:
    xor ebx, ebx                        ; ebx must be 0 to start
    xor bp, bp                          ; keep an entry count in bp
    mov edx, 0x0534D4150                ; Place "SMAP" into edx
    mov eax, 0xe820
    mov [es:di + 20], dword 1           ; force a valid ACPI 3.X entry
    mov ecx, 24                         ; ask for 24 bytes
    int 0x15
    jc short .failed                    ; carry set on first call means "unsupported function"
    mov edx, 0x0534D4150                ; Some BIOSes apparently trash this register?
    cmp eax, edx                        ; on success, eax must have been reset to "SMAP"
    jne short .failed
    test ebx, ebx                       ; ebx = 0 implies list is only 1 entry long (worthless)
    je short .failed
    jmp short .jmpin
.e820lp:
    mov eax, 0xe820                     ; eax, ecx get trashed on every int 0x15 call
    mov [es:di + 20], dword 1           ; force a valid ACPI 3.X entry
    mov ecx, 24                         ; ask for 24 bytes again
    int 0x15
    jc short .e820f                     ; carry set means "end of list already reached"
    mov edx, 0x0534D4150                ; repair potentially trashed register
.jmpin:
    jcxz .skipent                       ; skip any 0 length entries
    cmp cl, 20                          ; got a 24 byte ACPI 3.X response?
    jbe short .notext
    test byte [es:di + 20], 1           ; if so: is the "ignore this data" bit clear?
    je short .skipent
.notext:
    mov ecx, [es:di + 8]                ; get lower dword of memory region length
    test ecx, ecx                       ; is the qword == 0?
    jne short .goodent
    mov ecx, [es:di + 12]               ; get upper dword of memory region length
    jecxz .skipent                      ; if length qword is 0, skip entry
.goodent:
    inc bp                              ; got a good entry: ++count, move to next storage spot
    add di, 24
.skipent:
    test ebx, ebx                       ; if ebx resets to 0, list is complete
    jne short .e820lp
.e820f:
    mov WORD [0x0FFE], bp               ; store the entry count at 0x0FFE
    ret                                 ; test opcode cleared carry flag
.failed:
    stc                                 ; "function unsupported" error exit
    ret

%endif
