; boot.asm
[Bits 16]
org 0x7C00                                  ; start address of bootloader
jmp entry_point                             ; jump to bootloader entry point

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Memory Management:
; org                  7C00
; data/extra segments     0
; stack segment        7C00
; root dir -> memory   7E00
; boot2    -> memory    500
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "Fat12_BPB.inc"

;******************************************************************************
;   bootloader entry point
;******************************************************************************
entry_point:
    xor     sp, sp                          ; set registers
    mov     ds, sp
    mov     es, sp
    mov     fs, sp
    mov     gs, sp

    mov     ax, 0x7C00                      ; set the stack
    mov     ss, ax

    mov  [DriveNum], dl                     ; store boot device
    mov si, msgLoading
    call print_string

    ; set 80x50 text mode and 8x8 font
    mov ax, 0x1112
    xor bl, bl
    int 0x10

Load_Root_Directory_Table:
    ; compute size of root directory and store in "cx"
    xor cx, cx
    xor dx, dx
    mov ax, 0x20                            ; 32 byte directory entry
    mul WORD [RootEntries]                  ; total size of directory
    div WORD [BytesPerSec]                  ; sectors used by directory
    xchg ax, cx

    ; compute location of root directory and store in "ax"
    mov al, BYTE [NumFATs]                  ; number of FATs
    mul WORD [FATSize]                      ; sectors used by FATs
    add ax, WORD [ReservedSec]              ; adjust for bootsector
    mov WORD [datasector], ax               ; base of root directory
    add WORD [datasector], cx

    ; read root directory into memory (7E00h)
    mov bx, 0x7E00                          ; copy root dir above bootcode
    call ReadSectors

;******************************************************************************
;   Find stage2 bootloader
;******************************************************************************
    ; browse root directory for binary image
    mov cx, WORD [RootEntries]              ; load loop counter
    mov di, 0x7E00                          ; locate first root entry
.LOOP:
    push cx
    mov cx, 0xB                             ; name has 11 characters
    mov si, ImageName                       ; look for this image name
    push di
    rep cmpsb                               ; test for entry match
    pop di
    je Load_FAT
    pop cx
    add di, 0x20                            ; queue next directory entry
    loop .LOOP
    jmp FAILURE

;******************************************************************************
;   Load File Allocation Table (FAT)
;******************************************************************************
Load_FAT:
    ; save starting cluster of boot image
    mov dx, WORD [di + 0x001A]
    mov WORD [cluster], dx                  ; file's first cluster

    ; store the size of FAT1 "cx"
    mov cx, WORD [FATSize]

    ; store the location of FAT in "ax"
    mov ax, WORD [ReservedSec]              ; adjust for bootsector

    ; read FAT into memory (7E00h)
    mov bx, 0x7E00                          ; copy FAT above bootcode
    call ReadSectors

    ; read image file into memory (0500h)
    xor ax, ax
    mov es, ax                              ; destination for image
    mov bx, 0x0500                          ; destination for image
    push bx

;******************************************************************************
;   Load stage2 bootloader
;******************************************************************************
Load_Image:
    mov ax, WORD [cluster]                  ; cluster to read
    pop bx                                  ; buffer to read into
    call Convert_Cluster_to_LBA             ; convert cluster to LBA
    xor cx, cx
    mov cl, BYTE [SecPerClus]               ; sectors to read
    call ReadSectors
    push bx

    ; compute next cluster
    mov ax, WORD [cluster]                  ; identify current cluster
    mov cx, ax                              ; copy current cluster
    mov dx, ax                              ; copy current cluster
    shr dx, 1                               ; divide by two
    add cx, dx                              ; sum for (3/2)
    mov bx, 0x7E00                          ; location of FAT in memory
    add bx, cx                              ; index into FAT
    mov dx, WORD [bx]                       ; read two bytes from FAT
    test ax, 1
    jnz .ODD_CLUSTER

.EVEN_CLUSTER:
    and dx, 0000111111111111b               ; take low twelve bits
    jmp .DONE

.ODD_CLUSTER:
    shr dx, 4                               ; take high twelve bits

.DONE:
    mov WORD [cluster], dx                  ; store new cluster
    cmp dx, 0x0FF0                          ; test for EOF
    jb Load_Image

DONE:
    mov si, msgCRLF
    call print_string
    mov dl, [DriveNum]

    ; copy the BIOS Parameter Block to the 2. Stage bootloader
    mov di, 0x56C                           ; destination
    mov si, 0x7C03                          ; source
    mov cx, 0x3C                            ; number of bytes to copy
    cld
    rep movsb
    push WORD 0x0000
    push WORD 0x0500
    retf

FAILURE:
    mov si, msgFailure
    call print_string
    mov ah, 0x00
    int 0x16                                ; wait for keypress
    int 0x19                                ; warm boot reset


;******************************************************************************
;   Convert CHS to LBA
;   LBA = (cluster - 2) * sectors per cluster
;******************************************************************************
Convert_Cluster_to_LBA:
    sub ax, 2                               ; zero base cluster number
    xor cx, cx
    mov cl, BYTE [SecPerClus]               ; convert byte to word
    mul cx
    add ax, WORD [datasector]               ; base data sector
    ret

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
    xor dx, dx                              ; prepare dx:ax for operation
    div WORD [SecPerTrack]                  ; calculate
    inc dl                                  ; adjust for sector 0
    mov BYTE [Sector], dl
    xor dx, dx                              ; prepare dx:ax for operation
    div WORD [NumHeads]                     ; calculate
    mov BYTE [Head], dl
    mov BYTE [Cylinder], al
    ret

;******************************************************************************
;   Reads sectors 
;   CX     Number of sectors to read
;   AX     Starting sector
;   ES:BX  Buffer to read to
;******************************************************************************
ReadSectors:
    ; add the number of hidden sectors to AX
    add ax, WORD [HiddenSec]
.NEXTSECTOR:
    mov di, 5                               ; five retries for error
.LOOP:
    push ax
    push bx
    push cx

    mov dl, BYTE [DriveNum]

    test dl, 0x80
    jz .FLOPPY

    ; Hard Drive
    mov si, dapBuffer
    mov [dapBuffer+4], bx
    mov [dapBuffer+8], ax
    mov ah, 0x42
    mov cl, 1
    int 0x13

    jmp .RESET

.FLOPPY:
    ; Floppy Disk
    call Convert_LBA_to_CHS                 ; convert starting sector from LBA to CHS
    mov  dl, BYTE [DriveNum]
    mov  ah, 2                              ; INT 0x13, AH=2 --> read in CHS mode
    mov  al, 1                              ; read one sector
    mov  ch, BYTE [Cylinder]                ; track/cylinder
    mov  cl, BYTE [Sector]                  ; sector
    mov  dh, BYTE [Head]                    ; head
    int  0x13

.RESET:
    jnc  .SUCCESS                           ; check read error

    xor  ax, ax                             ; INT 0x13, AH=0 --> reset floppy/hard disk
    int  0x13
    dec  di                                 ; decrement error counter
    pop  cx
    pop  bx
    pop  ax
    jnz  .LOOP                              ; read again
    int  0x18
.SUCCESS:
    mov  si, msgProgress
    call print_string
    pop  cx
    pop  bx
    pop  ax
    add  bx, WORD [BytesPerSec]             ; queue next buffer
    inc  ax                                 ; queue next sector
    loop .NEXTSECTOR                        ; read next sector
    ret

;******************************************************************************
;   Print String
;   DS:SI   null-terminated string
;******************************************************************************
print_string:
    mov ah, 0x0E                            ; BIOS function 0x0E: teletype
.loop:   
    lodsb                                   ; grab a byte from SI
    test al, al                             ; NUL?
    jz .done                                ; if the result is zero: get out
    int 0x10                                ; else: print out the character
    jmp .loop
.done:
    ret

;******************************************************************************
;   Parameters
;******************************************************************************
Sector         db 0
Head           db 0
Cylinder       db 0
datasector     dw 0
cluster        dw 0
ImageName      db "BOOT2   SYS"
msgProgress    db "*", 0
msgLoading     db "Loading 2. Bootloader", 0x0D, 0x0A, 0
msgFailure     db "Fail"
msgCRLF        db 0x0D, 0x0A, 0
dapBuffer:                                  ; Disk Address Packet buffer
    db 0x10                                 ; size
    db 0                                    ; (Unused)
    db 1                                    ; Number of sectors
    db 0                                    ; (Unused)
    dd 0                                    ; Buffer offset
    dd 0                                    ; LBA start
    dd 0

TIMES 510-($-$$) hlt                        ; fill bytes until boot signature
db 0x55                                     ; boot signature
db 0xAA                                     ; boot signature 