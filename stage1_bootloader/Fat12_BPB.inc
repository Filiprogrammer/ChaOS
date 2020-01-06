%ifndef FAT12_BPB_INC
%define FAT12_BPB_INC

[BITS 16]

;##############################################################################
; "FAT 12" file system is build up by four areas at Floppy Disk:
; - Boot Sector (also called BIOS Parameter Block, BPB) 
; - File Allocation Table, FAT 
; - Root Directory
; - Subdirectories and Files

; boot sector info block

%define VOLUMESERIALNUM 0x192A2CF5

OperatingSystemName db "ChaOS   "      ;  8 bytes
BytesPerSec         dw 512
SecPerClus          db 1
ReservedSec         dw 1
NumFATs             db 2
RootEntries         dw 224
TotSec              dw 2880
MediaType           db 0xF0            ; 0xF0 3.5-inch 1.44 MB Floppy    0xF8 Fixed disk
FATSize             dw 9
SecPerTrack         dw 18
NumHeads            dw 2
HiddenSec           dd 0
TotSec32            dd 0
DriveNum            db 0
Reserved            db 0
BootSig             db 0x29
VolumeSerialNum     dd VOLUMESERIALNUM
VolumeLabel         db "           "   ; 11 bytes (Not used anymore)
FileSys             db "FAT12   "      ;  8 bytes
;##############################################################################

%endif
