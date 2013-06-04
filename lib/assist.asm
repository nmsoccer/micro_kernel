global  delay , out_byte , in_byte , port_read , port_write

extern get_ticks

%include "global.inc"
;-----delay---------
;void delay(int time); 时间单位10ms
delay:
	push ebp
	mov ebp , esp
	push ebx

	mov dword [esp - 4] , 0
	mov dword [esp - 8] , 0

	mov ebx , [ebp + 8]
	M_multiw bx , (TIMER_FREQ/HZ)
	mov [esp - 4] , eax ;(time * 10)ms -> [esp - 4]

	call get_ticks
	mov [esp - 8] , eax ; base ticks -> [esp - 8]

.1:
	call get_ticks
	sub eax , [esp - 8]
	mov ebx , eax
	M_multiw bx , (TIMER_FREQ/HZ)
	mov ebx , eax	; (now - base) * 10ms

	cmp ebx , [esp - 4]
	jae .2

	jmp .1

.2:

	pop ebx
	mov esp , ebp
	pop ebp
	ret

;--------out_byte---------
;void out_byte(u32 data , u32 port); use byte , word
out_byte:
	push ebp
	mov ebp , esp

	mov eax , [ebp + 8] ; data -> al
	mov edx , [ebp + 12] ; port -> dl

	out dx , al

	mov esp , ebp
	pop ebp
	ret

;-------in_byte------------
;u8 in_byte(u32 port) ;only use word ; return in al
in_byte:
	push ebp
	mov ebp , esp

	xor eax , eax

	mov edx , [ebp + 8]

	in al , dx
	;return in al
	;accept eax
	mov esp , ebp
	pop ebp
	ret


;-------port_read----------
;void port_read(char *buff , int number , u32 port) ;从port端口读入number个双字到buff之中
port_read:
	push ebp
	mov ebp , esp
	push edi

	mov edx , [ebp + 16] ;port -> edx
	mov ecx , [ebp + 12] ;number -> ecx
	mov edi , [ebp + 8]  ;buff -> edi

	rep insd

	pop edi
	mov esp , ebp
	pop ebp
	ret


;------port_write----------
;void port_write(char *buff , int number , u32 port) ;将buff之中number个字节放入port端口中
port_write:
	push ebp
	mov ebp , esp
	push esi

	mov edx , [ebp + 16] ;port -> edx
	mov ecx , [ebp + 12] ;number -> ecx
	mov esi , [ebp + 8]  ;buff -> edi

	rep outsd

	pop esi
	mov esp , ebp
	pop ebp
	ret

;---delay0------	------
;void delay0(dword time);

delay0:
	push ebp
	mov ebp , esp

	xor ecx , ecx
	mov ecx , [ebp + 8]


loop1:
	push ecx


	mov ecx , 10
loop2:
	push ecx


	mov ecx , 10000
loop3:
	loop loop3

	pop ecx
	loop loop2

	pop ecx
	loop loop1



	mov esp , ebp
	pop ebp
	ret
