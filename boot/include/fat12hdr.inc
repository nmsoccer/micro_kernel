BS_OEMName	db 'LeiMingC';8B

BPB_BytsPerSec	dw 512
BPB_SecPerClus	db 1
BPB_RsvdSecCnt  dw 1
BPB_NumFATS	db 2;number of fats
BPB_RootEntCnt	dw 224;Max files of root
BPB_TotSec16	dw 2880;Logic sectors in total
BPB_Media	db 0xf0;Media descriptor
BPB_FATSz16	dw 9   ;Sectors per fat
BPB_SecPerTrk	dw 18  ;Sectors per track
BPB_NumHeads	dw 2   ;number of heads
BPB_HiddSec	dd 0   ;Hidden sectors
BPB_TotSec32	dd 0   ;if BPB_TotSec16 ==0

BS_DrvNum	db 0   ;for int 13
BS_Reserved1	db 0   ;Never used
BS_BootSig	db 0x29;extending boot sector
BS_VolID	dd 0   ;Scroll
BS_VolLab	db 'LeiMinux0.1';signal 11B
BS_FileSysType  db 'FAT12    ';File system 9B
;---------BASIC------------------------------
start_rootsec	equ 19
rootsec		equ 14

base_fat	equ 0x50	;absolutely 0x500
;---------BOOT------------------------------

base_loader	equ 0x9000
offset_loader	equ 0x100
