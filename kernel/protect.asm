global exception_handler  , base_selec , init_desc

;extern function
extern dispstr , disphexdw
;extern global varibales

%include "global.inc"





space:
	db " " , 0x00

;check:
;	db "000000000000" , 0x00

title:
	db "Exception!   -->  " ,  0x00

err_title:
	db "Error code --> " ,  0x00

err_msg0:
	db "#DE Divide Error" , 0X00
err_msg1:
	db "#DB RESERVED" , 0x00
err_msg2:
	db "-- NMI Interrupt" , 0x00
err_msg3:
	db "#BP Breakpoint" , 0x00
err_msg4:
	db "#OF Overflow" , 0x00
err_msg5:
	db "#BR BOND Range Exceeded" , 0x00
err_msg6:
	db "#UD Invalid Opcode " , 0x00
err_msg7:
	db "#NM Device Not Availabel" , 0x00
err_msg8:
	db "#DF Double Fault" , 0x00
err_msg9:
	db "Coprocssor Segment Overrun" , 0x00
err_msg10:
	db "#TS Invalid" , 0x00
err_msg11:
	db "#NP Segment Not Present" , 0x00
err_msg12:
	db "#SS Stack-Segment Fault" , 0x00
err_msg13:
	db "#GP General Protection" , 0x00
err_msg14:
	db "#PF Page Fault" , 0x00
err_msg15:
	db "--(Intel reserved)" , 0x00
err_msg16:
	db "MF X87 FPU Floating-Point Error" , 0x00
err_msg17:
	db "AC Alignment Check" , 0x00
err_msg18:
	db "MC Machine Check" , 0x00
err_msg19:
	db "XF SIMD Floating-Point Exception" , 0x00

;--------base_selec------------
;int base_selec(int selector , int * table);
;第一个参数是选择子，传入双字只使用后16位。第二个参数是表的地址。(GDT或者LDT)
;exam1: base_selec(selector_in_gdt , gdt); -> return selector_in_gdt-> desc -> base
;exam2: base_selec(selector_ldt , gdt); -> return address of ldt (ldt) then base_selec(selector_in_ldt , ldt) -> return ...
base_selec:
	push ebp
	mov ebp , esp
	push ebx

	xor ebx , ebx
	mov ebx , [ebp + 12];address of table -> ebx

	xor eax , eax
	mov eax , [ebp + 8];selector -> eax ; only use low 16bits (ax)

	and al , 11111000b;只需要注意到，将选择子的最低三位清零就是该选择子对应描述符在表中的偏移

	add ebx , eax;描述符的绝对地址(gdt/ldt + 表内偏移) -> ebx


	push ebx
	call base_desc
	add esp , 4

;	return eax -> base

	pop ebx
	mov esp , ebp
	pop ebp
	ret

;---------base_desc-----------
;int base_desc(int * descriptor);
;get base address from descriptor
;input -> offset descriptor(pointer); output -> base(eax) ; DESCRIPTOR_LEN = 8B
;第一个实参是描述符的绝对地址。GDT：gdt + 表内偏移;LDT：ldt + 表内偏移.
base_desc:
	push ebp
	mov ebp , esp
	push ebx

	xor ebx , ebx
	mov ebx , [ebp + 8];address of desc -> eax

	xor eax , eax
	mov byte ah , [ebx + 7];high 1B of base -> ah
	mov byte al , [ebx + 4];mid  1B of base -> al

	shl eax , 16 ;give space to low 2B of base

	mov word ax , [ebx + 2];low 2B -> ax

	;now base are all put in eax
	;return eax

	pop ebx
	mov esp , ebp
	pop ebp

	ret

;-----init_desc-------
;void init_desc(int selector , int base , int limit , int attr , int *table);
;as for selector we use 2B (16bits)
;as for limit we use 2.5B (20bits)
;as for attr we really use 1.5B (12bits);传入的属性我们注意使用低字,这是与描述符安排保持一致
;last param is address of gdt or ldt

init_desc:
	push ebp
	mov ebp , esp
	push ebx

;get addr of desc
	xor ebx , ebx
	mov dword ebx , [ebp + 8];selector -> ebx
	and bl , 11111000b	; offset of descripor in table
	add dword ebx , [ebp + 24] ; address of descriptor -> ebx


;set base
	xor eax , eax
	mov dword eax , [ebp + 12] ; base -> eax

	mov word [ebx + 2] , ax ;set low 2B
	shr eax , 16
	mov byte [ebx + 4] , al ;set mid 1B
	mov byte [ebx + 7] , ah ;set high 1B

;set limit
	xor eax , eax
	mov dword eax , [ebp + 16] ; limit -> eax

	mov word [ebx] , ax ;set low 2B
	shr eax , 16
	mov byte [ebx + 6] , al ; set low 4bits

;set attr
	xor eax , eax
	mov dword eax , [ebp + 20] ; attr -> eax

	mov byte [ebx + 5] , al ; set low 1B
	or  byte [ebx + 6] , ah ; set high 4bits;because low 4bits of ah is always 0


	pop ebx
	mov esp , ebp
	pop ebp
	ret
;------exception_handler------------
;void exception_handler(dword vec_no , dword err_code , dword eip , dword cs , dword eflags);
;eflags , cs , eip is pushed into stack by cpu

exception_handler:
	push ebp
	mov ebp , esp

	mov dword [videoaddr] , 0
	xor ecx , ecx
	mov ecx , 80 * 2
;---------clear first two lines----------	
clear:
	push dword [videoaddr]
	push dword space
	call dispstr
	mov [videoaddr] , eax
	loop clear

;--------display information-----------
	mov dword [videoaddr] , 0

	push dword [videoaddr]
	push dword title
	call dispstr
	mov [videoaddr] , eax

	xor eax , eax
	mov dword eax , [ebp + 8]

.0:
	cmp eax , 0
	jnz .1
	push dword [videoaddr]
	push dword err_msg0
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.1:
	cmp eax , 1
	jnz .2
	push dword [videoaddr]
	push dword err_msg1
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.2:
	cmp eax , 2
	jnz .3
	push dword [videoaddr]
	push dword err_msg2
	call dispstr
	mov [videoaddr] , eax
	jmp end
.3:
	cmp eax , 3
	jnz .4
	push dword [videoaddr]
	push dword err_msg3
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end

.4:
	cmp eax , 4
	jnz .5
	push dword [videoaddr]
	push dword err_msg4
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.5:
	cmp eax , 5
	jnz .6
	push dword [videoaddr]
	push dword err_msg5
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.6:
	cmp eax , 6
	jnz .7
	push dword [videoaddr]
	push dword err_msg6
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.7:
	cmp eax , 7
	jnz .8
	push dword [videoaddr]
	push dword err_msg7
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.8:
	cmp eax , 8
	jnz .9
	push dword [videoaddr]
	push dword err_msg8
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.9:
	cmp eax , 9
	jnz .10
	push dword [videoaddr]
	push dword err_msg9
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.10:
	cmp eax , 10
	jnz .11
	push dword [videoaddr]
	push dword err_msg10
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.11:
	cmp eax , 11
	jnz .12
	push dword [videoaddr]
	push dword err_msg11
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.12:
	cmp eax , 12
	jnz .13
	push dword [videoaddr]
	push dword err_msg12
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.13:
	cmp eax , 13
	jnz .14
	push dword [videoaddr]
	push dword err_msg13
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.14:
	cmp eax , 14
	jnz .15
	push dword [videoaddr]
	push dword err_msg14
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.15:
	cmp eax , 15
	jnz .16
	push dword [videoaddr]
	push dword err_msg15
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.16:
	cmp eax , 16
	jnz .17
	push dword [videoaddr]
	push dword err_msg16
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.17:
	cmp eax , 17
	jnz .18
	push dword [videoaddr]
	push dword err_msg17
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.18:
	cmp eax , 18
	jnz .19
	push dword [videoaddr]
	push dword err_msg18
	call dispstr
	add esp , 8
	mov [videoaddr] , eax
	jmp end
.19:
	cmp eax , 19
	jnz end
	push dword [videoaddr]
	push dword err_msg19
	call dispstr
	add esp , 8
	mov [videoaddr] , eax

	jmp end


	


end:
	mov dword [videoaddr] , (80 * 1 + 0) * 2

	push dword [videoaddr]
	push dword err_title
	call dispstr
	add esp , 8
	mov [videoaddr] , eax

	xor eax , eax
	mov dword eax , [ebp + 12]
	push dword [videoaddr]
	push eax
	call disphexdw
	add esp , 8
	mov [videoaddr] , eax
;--------------------------
;	xor eax , eax
;	mov eax , 3
;	add byte  [check + eax] , 0x1

;	push dword [videoaddr]
;	push check
;	call dispstr
;	add esp , 8
;	mov [videoaddr] , eax	


	mov esp , ebp
	pop ebp

	ret





















