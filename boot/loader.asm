	org 0x100
;-------------------------
top_stack	equ 0x100
	
;-------------------------
;-----------FAT Head------
	jmp  LABEL_START

%include"fat12hdr.inc"
%include"load.inc"

LABEL_GDT:
descriptor_null:
		dd 0
		dd 0
descriptor_flat_c:
		dw 0xffff ;Limit of flat_c 0~4G
	
		dw 0	  ;Base of flat_c
		db 0 

		db 10011010b
		db 11001111b

		db 0
descriptor_flat_d:
		dw 0xffff

		dw 0
		db 0

		db 10010010b
		db 11001111b

		db 0
		
descriptor_video:
		dw 0xffff

		dw 0x8000
		db 0x0B

		db 11110010b
		db 00000000b

		db 0

gdtlen	equ $ - LABEL_GDT 
gdtptr	dw gdtlen - 1			;Length of gdt
	dd base_loaderphy + LABEL_GDT 	;Base of gdt 

selector_flat_c		equ descriptor_flat_c - LABEL_GDT
selector_flat_d 	equ descriptor_flat_d - LABEL_GDT
selector_video  	equ descriptor_video  - LABEL_GDT + 3	

;PAGING INFORMATION
pagedirbase	equ 0x100000;page dir start:1M
pagetblbase	equ 0x101000;page table start:1M+4K	
;-------------------------
LABEL_START:
	mov ax , cs
	mov ds , ax
	mov ss , ax
	mov sp , top_stack

	xor ax , ax
	mov ax , base_kernel
	mov es , ax

;	call CLRSCREEN

;	xor edx , edx
;	xor esi , esi
;	mov esi , root_message
;	mov edx , (80 * 0 + 0) * 2
;	call DISPSTR
;	nop
	
	xor ah , ah
	xor dl , dl
	int 0x13

	call GETROOTSEC
	
;	xor al , al
;	mov byte al , [item_message + 16]
;	add al , [rootsec]
;	mov [item_message + 16] , al
;	xor esi , esi
;	mov esi , item_message
;	mov edx , (80 * 1 + 0) * 2
;	call DISPSTR


	call SEARCHFILE
	
	mov edx , (80 * 1 + 0) * 2
	
	cmp word [loaderentry] , 0x0000
	jz ending
	mov esi , load_message
	call DISPSTR
	call LOADFILE

;-----------Ready to jump into Protecto Mode-------
	lgdt [gdtptr]

	cli

	in al , 0x92
	or al , 00000010b
	out 0x92 , al

	mov eax , cr0
	or  eax , 1
	mov cr0 , eax

	jmp dword selector_flat_c:(base_loaderphy + LABEL_CODE32)
;	jmp $
;        jmp base_kernel:offset_kernel
ending:
	mov esi , nofile_message
	call DISPSTR
	jmp $

;-----------------------
;change-eax,ebx,ecx,edx
CLRSCREEN:

	mov ah , 0x06;Function
	mov al , 0x00;Number scroll
	mov bh , 00000111b;attr
	mov cx , 0x0000;start row:column
	mov dx , 0x184F;end row:column
	int 0x10

	ret
;-----------------------
;str--esi addr--edx
;change-gs,eax,ebx	
DISPSTR:

	mov ax , 0xB800
	mov gs , ax

	mov ah , 00001100b

.1:
	lodsb
	cmp al , 0x00
	jz  .2
	mov [gs:edx] , ax
	add edx , 2
	jmp .1	
	
.2:     ;Set cursor
	mov ax , dx
	mov bl , 160
	div bl      ;row-al column-ah
	xor dx , dx
	mov dh , al
	add dh , 1
;	mov dh , 0x01
	mov dl , 0x00
	mov ah , 0x02
	mov bh , 0x00
	int 0x10

	ret
;---------------vars------------
rootitem 	db 0
loaderentry	dw 0

load_message:	db "Loading..." , 0x00
nofile_message	db "No kernel!"   , 0x00

file_name:	db "KERNEL  BIN"
;-------------------------------
LOADFILE:

	mov ax , base_fat
	mov fs , ax
	mov es , ax
	mov al , 0x01
	mov cl , 0x01
	call READSECTOR

	xor ax , ax
	mov ax , base_kernel
	mov es , ax

START_LOADING:
	mov di , offset_kernel

	mov dx , 0x0011;17
	add dl , rootsec;14
	add dx , [loaderentry]
	mov ax , dx
	mov cl , 0x02
	call READSECTOR;Load entry->sector

;	xor ax , ax
	mov ax , [loaderentry];using only low 8 bit
	mov bl , 0x03
	mul bl
	mov bl , 0x02
	div bl;al-quo ah-mod

	cmp ah , 0x00
	jnz NOT_EVEN_ENTRY
	add di , ax

;	xor cx , cx
	mov cx , [fs:di]
	and cx , 0x0FFF
	mov [loaderentry] , cx

	jmp CONTROL

NOT_EVEN_ENTRY:
	xor ah , ah
	add di , ax
;	xor cx , cx
	mov cx , [fs:di]
	shr cx , 4
	mov [loaderentry] , cx

CONTROL:

	cmp word [loaderentry] , 0x0FF8
	jae END_LOADING


	mov ax , es
	add ax , 0x20
	mov es , ax;set memory addr
	jmp START_LOADING	
	
END_LOADING:

;	jmp base_kernel:offset_kernel	

	ret
;-------------------------------
;change-eax,ecx,esi,edi,[rootitem]
;return-[loaderentry]
SEARCHFILE:
	mov dl , [rootitem] 

start_search_file:
	mov al , start_rootsec
	mov cl , rootsec
	call READSECTOR

	mov si , file_name
	mov di , offset_kernel
;----Checking item-----
search_file:		 
	mov ecx , 11

compare:
	lodsb
	cmp al , [es:di]
	jne not_match
	inc di
	loop compare
		
	jmp match


not_match:

	sub byte [rootitem] , 1
	cmp byte [rootitem] , 0
	jz  end_search_file	
	 
	and di , 0xFFE0	
	add di , 32
	mov si , file_name
	jmp search_file

match:
	and di , 0xFFE0
	add di , 0x001A
	mov ax , [es:di]
	mov [loaderentry] , ax
	jmp end_search_file
;------Checking item end-----	
end_search_file:
	mov  [rootitem] , dl

	ret
;-------------------------------
;change-eax,ebx,ecx,edx,esi,edi
GETROOTSEC:
	mov si , start_rootsec
;	start_rootsec == 19
	mov di , 1

	mov dx , 0	
	mov bx , offset_kernel

start_search_item:
	mov ax , si
	mov cx , di

	call READSECTOR
	
	mov ecx , 0x00000010

search_item:
	mov ah , [es:bx]
	cmp ah , 0x00
	je  end_search_item
	add bx , 32
	inc dl
	loop search_item

	inc si
	mov di , 1
	mov bx , offset_kernel
	jmp start_search_item	
	
end_search_item:
	cmp dl , 0
	mov [rootitem] , dl
;	jz  no_item

;	mov ax , dx
;	mov bl , 0x10
;	div bl
;	cmp ah , 0x00
;	jz  full_sector
;	inc al

;full_sector:
;	mov [rootsec] , al  

;no_item:

	ret

;-------------------------------
;al--start sector cl--number of sectors
;destination--es:bx
;change--ebx,edx
READSECTOR:
	push ebx
	push edx

	and ax , 0x00FF

	mov bl , [BPB_SecPerTrk]
;	[BPB_SecPerTrk] == 18
	div bl

	;al -- Quotient
	;ch -- Cylinder
	mov ch , al
	shr ch , 1
	;dh -- Head
	mov dh , al                        
	and dh , 1
	;al -- number
	mov al , cl
	;cl -- start
	mov cl , ah
	add cl , 1
	;ah -- function dl --A
	mov ah , 0x02
	mov dl , [BS_DrvNum]
;	[BS_DrvNum] == 0

	;bx -- offset_loader
	mov bx , offset_kernel

	int 0x13	


	pop  edx
	pop  ebx
	ret
;-----------------------------------
[SECTION .code32]
ALIGN 32
[BITS 32]
LABEL_CODE32:
	xor ax , ax
	mov ax , selector_video
	mov gs , ax

	xor ax , ax
	mov ax , selector_flat_d
	mov ds , ax

;	mov ah , 00001111b
;	mov al , 'P'

;	mov [gs:((80 * 0 + 17) * 2)] , ax

	xor eax , eax
	mov eax , (80 * 4 + 0) * 2
	mov [videoaddr] , eax

	call selector_flat_c:ENABLEPAGING

	xor esi , esi
	mov esi , pageinfo
	xor edi , edi
	mov edi , [videoaddr]
	call selector_flat_c:DISPSTRING
	mov [videoaddr] , edi

	xor esi , esi
	mov esi , string1
	xor edi , edi
	mov edi , [videoaddr]
	call selector_flat_c:DISPSTRING
	mov [videoaddr] , edi
	

	call selector_flat_c:SETTABLE
;	call SETTABLE

;------PRINT ENTRY
	

;	add edi , 4
	xor eax , eax
	mov eax , [entry]
	xor edi , edi
	mov edi , [videoaddr]
	call selector_flat_c:DISPINTDW
	add edi , 2

	xor esi , esi
	mov esi , str_entry
	
	call selector_flat_c:DISPSTRING

;-------PRINT PHOFF


	xor eax , eax
	mov eax , [phoff]

	call selector_flat_c:DISPINTDW
	add edi , 2

	xor esi , esi
	mov esi , str_phoff
	
	call selector_flat_c:DISPSTRING


;-------PRINT PHENTSIZE

	xor eax , eax
	mov eax , [phentsize]

	call selector_flat_c:DISPINTW
	add edi , 2

	xor esi , esi
	mov esi , str_phentsize
	
	call selector_flat_c:DISPSTRING


;-------PRINT PHNUM


	xor eax , eax
	mov eax , [phnum]

	call selector_flat_c:DISPINTW
	add edi , 2

	xor esi , esi
	mov esi , str_phnum
	
	call selector_flat_c:DISPSTRING

;-------PRINT OFFSET


	xor eax , eax
	mov eax , [offset]

	call selector_flat_c:DISPINTDW
	add edi , 2

	xor esi , esi
	mov esi , str_offset
	
	call selector_flat_c:DISPSTRING

;-------PRINT VADDR


	xor eax , eax
	mov eax , [vaddr]

	call selector_flat_c:DISPINTDW
	add edi , 2

	xor esi , esi
	mov esi , str_vaddr
	
	call selector_flat_c:DISPSTRING

;-------PRINT FILESZ


	xor eax , eax
	mov eax , [filesz]

	call selector_flat_c:DISPINTDW
	add edi , 2

	xor esi , esi
	mov esi , str_filesz
	
	call selector_flat_c:DISPSTRING
	mov [videoaddr] , edi

MOVEKERNEL:

	push word [phnum]
	xor ebx , ebx	
	mov bx , 56;the offset in first header program
STARTMOVING:
;if number of programheader >1
;	cmp word [phnum] , 0 
 ;       jz STOPMOVING

;do not load programheader whose size=0
	cmp dword [filesz] , 0
	jz FILEEMPTY
	
	mov dword eax , [offset]
	add eax , base_kernelphy
 	mov esi , eax
	mov dword ecx , [filesz]
	mov dword edi , [vaddr]

;	cmp dword ecx , 0
;	jz STOPMOVING	

	call selector_flat_c:MEMCPY

	mov edi , [videoaddr]
	mov esi , copyinfo
	call selector_flat_c:DISPSTRING
	mov [videoaddr] , edi

FILEEMPTY:
	sub word [phnum] , 1



	cmp word [phnum] , 0 	
	jz STOPMOVING
	
	add word bx , [phentsize];the addres of offset in next header program
	xor esi , esi
	mov esi , base_kernelphy
	add esi , ebx
	mov dword eax , [esi]	
	mov [offset] , eax;the offset	of next program
	add esi , 4
	mov dword eax , [esi]
	mov [vaddr] , eax;the vaddr of next program
	add esi , 8	
	mov dword eax , [esi]
	mov [filesz] , eax;the filesz of next program

	jmp STARTMOVING

STOPMOVING:
	pop word [phnum]
;	jmp $
	jmp selector_flat_c:entry_kernelphy


;--------ENABLE PAGING------
_ENABLEPAGING:				;32B length per item
	xor ax , ax			;4K memory -> per item
	mov ax , selector_flat_d
	mov es , ax

	xor edi , edi
	mov edi , pagedirbase		;edi->page dir base
	xor ecx , ecx
	mov ecx , 1024			;total 1k item
	xor eax , eax			;1k * 4K = 4G
	mov eax , pagetblbase + 1+2+4	;set page dir item
.1_p:
	stosd
	add eax , 4096
	loop .1_p

	xor edi , edi
	mov edi , pagetblbase		;edi->page table base
	xor ecx , ecx
	mov ecx , 1024 * 1024		;total 1M item
	xor eax , eax
	mov eax , 0 + 1+2+4		;line addr-> physical addr
.2_p:
	stosd
	add eax , 4096
	loop .2_p

;set cr3 -> page dir base		
	xor eax , eax
	mov eax , pagedirbase
	mov cr3 , eax
;first bite of cr0 -> enable paging
	xor eax , eax
	mov eax , cr0
	or  eax , 0x80000000
	mov cr0 , eax	

	jmp .3_p
.3_p:
	nop
	retf

ENABLEPAGING	equ _ENABLEPAGING + base_loaderphy
;--------SETTABLE----------
_SETTABLE:
	xor esi , esi
	mov esi , base_kernelphy
;------set entry,phoff	
	add esi , 24
	mov dword eax , [esi]
	mov [entry] , eax
	add esi , 4
	mov dword eax , [esi]
	mov [phoff] , eax

;------set phentsize,phnum
	mov esi , base_kernelphy	
	add esi , 42
	mov word ax , [esi]
	mov [phentsize] , ax
	add esi , 2
	mov word ax , [esi]
	mov [phnum] , ax
;------set first offset,vaddr,filesz
	mov esi , base_kernelphy
	add esi , 56
	mov dword eax , [esi]	
	mov [offset] , eax	
	add esi , 4
	mov dword eax , [esi]
	mov [vaddr] , eax
	add esi , 8	
	mov dword eax , [esi]
	mov [filesz] , eax

	retf
SETTABLE	equ _SETTABLE + base_loaderphy
;--------DISPCTRL----------
			;address-edi
;_DISPCTRL:
;	xor esi , esi
;	mov esi , ctrl	
;	call selector_flat_c:DISPSTRING
	

;	retf

;DISPCTRL	equ _DISPCTRL + base_loaderphy			 
;--------DISPSTR-----------
    	                 ;str-esi address-edi	  
  		         ;change eax , edx
			 ;string is end with (0x0D,)0x00
_DISPSTRING:
	push eax
	push edx
	
	xor edx , edx

;	xor ax , ax
;	mov ax , selector_video
;	mov gs , ax

.1_str:
	lodsb
	cmp al , 0x0D
	je  .2_str
	cmp al , 0x00
	je  .3_str
	mov ah , 00001011b
	mov [gs:edi] , ax
	add edi , 2
	jmp .1_str
.2_str:			;Wrap	
	xor ax , ax
	mov eax , edi
	mov dl , 160
	div dl   	;ah: column , al: row

	add al , 1	;(row+1) *160	
	mul dl
	movzx edi , ax 
	jmp .1_str
.3_str:
	pop edx
	pop eax
	retf
DISPSTRING	equ _DISPSTRING + base_loaderphy
;-------------DISPINTDW---------------
		;data-eax address-edi
_DISPINTDW:
	xor edx , edx
	mov edx , eax

	shr eax , 24
	call selector_flat_c:DISPINT

	mov eax , edx
	shr eax , 16
	call selector_flat_c:DISPINT

	mov eax , edx
	shr eax , 8
	call selector_flat_c:DISPINT

	mov eax , edx
	call selector_flat_c:DISPINT

	retf
		
DISPINTDW	equ	_DISPINTDW + base_loaderphy
;-------------DISPINTW----------------
		;data-ax address-edi
_DISPINTW:
	xor dx , dx
	mov dx , ax

	shr ax , 8
	call selector_flat_c:DISPINT

	mov ax , dx
	call selector_flat_c:DISPINT

	retf

DISPINTW	equ 	_DISPINTW + base_loaderphy
;-------------DISPINT-----------------

		;data-al  address-edi
		;change edx 
		;warning:last number may be changed!so edi should be added 2
_DISPINT:
	push edx

	mov dl , al
	
;	xor ax , ax
;	mov ax , selector_video
;	mov gs , ax	

	mov al ,dl
	shr al , 4
	and al , 00001111b
	cmp al , 9
	ja .1
	add al , '0'
	jmp .2
.1:	
	sub al , 0x0A
	add al , 'A'
.2:
	mov ah , 00000100b
	mov [gs:edi] , ax
	add edi , 2
		
	mov al , dl
	and al , 00001111b
	cmp al , 9
	ja .3
	add al , '0'
	jmp .4
.3:	
	sub al , 0x0A
	add al , 'A'
.4:
	mov ah , 00000100b
	mov [gs:edi] , ax
	add edi , 2
	mov ah , 00000101b
	mov al , 'h'
	mov [gs:edi] , ax

	pop edx
	retf
DISPINT	equ	_DISPINT + base_loaderphy

;---------MEMCPY-------------
		;source-esi(ds) number-ecx destination-edi(es)
		;change-eax

_MEMCPY:
;	push eax
;	push es

	xor ax , ax
	mov ax , selector_flat_d
	mov es , ax
	xor ax , ax

while_memcpy:

	lodsb
	stosb	
	loop while_memcpy


;	pop es
;	pop eax

	retf
MEMCPY	equ _MEMCPY + base_loaderphy

;------------------------------------------------
_videoaddr:
	dd 0
videoaddr	equ _videoaddr + base_loaderphy

;_ctrl:
;	db 0x5E , 0x0D , 0x00
;ctrl	equ _ctrl + base_loaderphy

_pageinfo:
	db "PAGING SUCCESS!" , 0x0D , 0x00
pageinfo	equ _pageinfo + base_loaderphy

_string1:
	db "-----KERNEL ELF-----" , 0x0D , 0x00
string1	equ _string1 + base_loaderphy

_copyinfo:
	db "COPYED!..." , 0X00 , 0x00
copyinfo	equ _copyinfo + base_loaderphy
;--------entry--
_str_entry:
	db "  ->entry program" , 0x0D, 0x00
str_entry	equ _str_entry + base_loaderphy

_entry:
	dd 0
entry	equ _entry + base_loaderphy

;-------phoff---
_str_phoff:
	db "  ->offset program header Table" , 0x0D , 0x00;=1214B
str_phoff	equ _str_phoff + base_loaderphy

_phoff:
	dd 0
phoff	equ _phoff + base_loaderphy

;------phentsize--
_str_phentsize:
	db "      ->length program header" , 0x0D , 0x00
str_phentsize	equ _str_phentsize + base_loaderphy

_phentsize:
	dw 0
phentsize	equ _phentsize + base_loaderphy

;------phnum-----
_str_phnum:
	db "      ->number program header" , 0x0D , 0x00
str_phnum	equ _str_phnum + base_loaderphy

_phnum:
	dw 0
phnum	equ _phnum + base_loaderphy

;------offset------
_str_offset:
	db "  ->offset program section" , 0x0D , 0x00
str_offset	equ _str_offset + base_loaderphy

_offset:
	dd 0
offset	equ _offset + base_loaderphy

;------vaddr-------
_str_vaddr:
	db "  ->vaddr program section" , 0x0D , 0x00
str_vaddr	equ _str_vaddr + base_loaderphy

_vaddr:
	dd 0
vaddr	equ _vaddr + base_loaderphy

;------filesz-------
_str_filesz:
	db "  ->size program section" ,0x0D ,  0x00
str_filesz	equ _str_filesz + base_loaderphy

_filesz:
	dd 0
filesz	equ _filesz + base_loaderphy










