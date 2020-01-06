; boot.asm
[Bits 16]

%define BASE 0x7C00
%define DEST 0x0600

org BASE

;******************************************************************************
;   bootloader entry point
;******************************************************************************
entry_point:
    xor     sp, sp                      ; set registers
    mov     ds, sp
    mov     es, sp
    mov     fs, sp
    mov     gs, sp

    mov     ax, BASE                    ; set the stack
    mov     ss, ax

    ; copy this bootloader to 0x0600
    mov di, DEST
    mov si, BASE
    mov cx, 256
    cld
    rep movsw

    jmp SKIP

SKIP: EQU ($ - BASE + DEST)

    mov  [bootdevice_id], dl            ; store boot device
    mov si, msgLoading_id
    call print_string

    ; find active partition
    mov bx, DEST + 0x01BE
Find_Active_Partition:
    mov al, [bx]
    test al, 0x80
    jnz .SUCCESS

    cmp bx, DEST + 0x01FE
    je .FAILURE

    add bx, 0x10
    jmp Find_Active_Partition

.SUCCESS:
    mov ax, WORD [bx + 0x08]
    mov bx, 0x7C00
    mov cx, 1
    call ReadSectors
    dec ax
    mov [0x7C1C], ax                    ; write the number of hidden sectors into the BIOS Parameter Block
    mov dl, BYTE [bootdevice_id]        ; load the bootdevice into dl
    jmp 0x0000:0x7C00                   ; jump to the actual 1. stage bootloader

.FAILURE:
    mov si, msgFailure_id
    call print_string
    mov ah, 0x00
    int 0x16                            ; wait for keypress
    int 0x19                            ; warm boot reset

;******************************************************************************
;   Convert LBA to CHS
;   AX    LBA Address to convert
;
;   sector = (logical sector / sectors per track) + 1
;   head   = (logical sector / sectors per track) MOD number of heads
;   track  = logical sector / (sectors per track * number of heads)
;
;******************************************************************************
Convert_LBA_to_CHS:
    xor dx, dx                          ; prepare dx:ax for operation
    div WORD [SecPerTrack_id]           ; calculate
    inc dl                              ; adjust for sector 0
    mov BYTE [Sector_id], dl
    xor dx, dx                          ; prepare dx:ax for operation
    div WORD [NumHeads_id]              ; calculate
    mov BYTE [Head_id], dl
    mov BYTE [Cylinder_id], al
    ret

;******************************************************************************
;   Reads sectors
;   CX     Number of sectors to read
;   AX     Starting sector
;   ES:BX  Buffer to read to
;******************************************************************************
ReadSectors:
.NEXTSECTOR:
    mov di, 5                           ; five retries for error
.LOOP:
    push ax
    push bx
    push cx

    mov dl, BYTE [bootdevice_id]

    test dl, 0x80
    jz .FLOPPY

    ; Hard Drive
    mov si, dapBuffer_id
    mov [dapBuffer_id+4], bx
    mov [dapBuffer_id+8], ax
    mov ah, 0x42
    mov cl, 1
    int 0x13

    jmp .RESET

.FLOPPY:
    ; Floppy Disk
    call Convert_LBA_to_CHS             ; convert starting sector from LBA to CHS
    mov  dl, BYTE [bootdevice_id]
    mov  ah, 2                          ; INT 0x13, AH=2 --> read in CHS mode
    mov  al, 1                          ; read one sector
    mov  ch, BYTE [Cylinder_id]         ; track/cylinder
    mov  cl, BYTE [Sector_id]           ; sector
    mov  dh, BYTE [Head_id]             ; head
    int  0x13

.RESET:
    jnc  .SUCCESS                       ; check read error

    xor  ax, ax                         ; INT 0x13, AH=0 --> reset floppy/hard disk
    int  0x13
    dec  di                             ; decrement error counter
    pop  cx
    pop  bx
    pop  ax
    jnz  .LOOP                          ; read again
    int  0x18
.SUCCESS:
    pop  cx
    pop  bx
    pop  ax
    add  bx, 512                        ; queue next buffer
    inc  ax                             ; queue next sector
    loop .NEXTSECTOR                    ; read next sector
    ret

;******************************************************************************
;   Print String
;   DS:SI   null-terminated string
;******************************************************************************
print_string:
    mov ah, 0x0E                        ; BIOS function 0x0E: teletype
.loop:
    lodsb                               ; grab a byte from SI
    test al, al                         ; NUL?
    jz .done                            ; if the result is zero: get out
    int 0x10                            ; else: print out the character
    jmp .loop
.done:
    ret

;******************************************************************************
;   Parameters
;******************************************************************************
Sector         db 0
Head           db 0
Cylinder       db 0
SecPerTrack    dw 18
NumHeads       dw 2
bootdevice     db 0
msgLoading     db "Loading ChaOS Partition", 0x0D, 0x0A, 0
msgFailure     db "No Active Partition Found"
msgCRLF        db 0x0D, 0x0A, 0
dapBuffer:                              ; Disk Address Packet buffer
    db 0x10                             ; size
    db 0                                ; (Unused)
    db 1                                ; Number of sectors
    db 0                                ; (Unused)
    dd 0                                ; Buffer offset
    dd 0                                ; LBA start
    dd 0

dapBuffer_id: EQU dapBuffer - BASE + DEST
msgFailure_id: EQU msgFailure - BASE + DEST
msgLoading_id: EQU msgLoading - BASE + DEST
Sector_id: EQU Sector - BASE + DEST
Head_id: EQU Head - BASE + DEST
Cylinder_id: EQU Cylinder - BASE + DEST
SecPerTrack_id: EQU SecPerTrack - BASE + DEST
NumHeads_id: EQU NumHeads - BASE + DEST
bootdevice_id: EQU bootdevice - BASE + DEST

TIMES 440-($-$$) hlt
TIMES 510-($-$$) db 0x00
db 0x55                                 ; boot signature
db 0xAA                                 ; boot signature
