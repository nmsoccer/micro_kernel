;common variables

global cstart 

;outside reference
;from lib
extern dispstr , memcpy , disphexdw , disphexw , disphex
;from kernel
extern init_protect

;%define USE_STC1
;%define USE_STC2

%include "global.inc"

;[SECTION .data]

;Thease are local variables
startinfo:
	db "-----Cstarting--------" , 0x0D , 0x00  

intinfo:
	db "-----IntGATE SUCCESS!----" , 0x00

testinfo:
	db "GOOD LEIMING" , 0x00

cstart:

	mov dword [videoaddr] , (80 * 2 + 0) * 2

	push dword [videoaddr]
	push dword startinfo	
	call dispstr
	mov [videoaddr] , eax
	add esp , 8
;--------disp old gdt_limit gdt_base---------
	xor eax , eax
	mov word ax , [gdtptr]
	push dword  [videoaddr]
	push eax
	call disphexw
	add esp , 8
	
	mov edi , eax
	xor eax , eax
	mov dword eax , [gdtptr + 2]
	push edi
	push eax
	call disphexdw
	add esp , 8
	mov dword [videoaddr] , eax
;--------change gdt----------
;memcpy(&gdt , gdtptr_base , gdtptr_limit );

	xor eax , eax
	mov word ax , [gdtptr]

	push dword eax	;gdt_old limit
	push dword [gdtptr + 2]	;gdt_old base
	push dword gdt	;gdt_new base
	call memcpy
	add esp , 12

	xor ax , ax
	mov word ax , gdt_len
	mov [gdtptr] , ax ;set new gdt_limit

	xor eax , eax
	mov eax , gdt
	mov [gdtptr + 2] , eax ;set new gdt_base



;-------set idt------------------------------
	xor ax , ax
	mov word ax , idt_len
	mov [idtptr] , ax

	xor eax , eax
	mov eax , idt
	mov [idtptr + 2] , eax

;-------disp new gdt information------
	xor eax , eax
	mov word ax , [gdtptr]
	push dword  [videoaddr]
	push eax
	call disphexw
	add esp , 8
	
	mov edi , eax
	xor eax , eax
	mov dword eax , [gdtptr + 2]
	push edi
	push eax
	call disphexdw
	add esp , 8
	mov dword [videoaddr] , eax

;-------disp idt information------
	xor eax , eax
	mov word ax , [idtptr]
	push dword  [videoaddr]
	push eax
	call disphexw
	add esp , 8
	
	mov edi , eax
	xor eax , eax
	mov dword eax , [idtptr + 2]
	push edi
	push eax
	call disphexdw
	add esp , 8
	mov dword [videoaddr] , eax

;$<
;stc1:
;	ISTRUC s_stc1
;	IEND
;stc2:
;	ISTRUC s_stc2
;	IEND

;	xor eax , eax
;	mov eax , testinfo
;	mov dword [stc2 + s_stc2.p_str] , eax

;	xor eax , eax
;	mov eax , stc2
;	mov dword [stc1 + s_stc1.p_stc] , eax

;	push dword [videoaddr]

;	xor edx , edx
;	mov dword edx , [stc1 + s_stc1.p_stc]
;	mov eax , [edx + s_stc2.p_str]
;	push eax

;	call dispstr
;	add esp , 8
;	mov [videoaddr] , eax


;$>


;------initialize IDT---------------

	call init_protect

	mov dword [videoaddr] , (80 * 14 + 0) * 2
	push dword [videoaddr]
	push dword intinfo
	call dispstr
	add esp , 8
	mov [videoaddr] , eax


	ret
