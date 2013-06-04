global init_8259A , spurious_irq , set_irq_table
global disable_irq , enable_irq , enable_int , disable_int

extern dispstr , disphexdw

%include "global.inc"

irq_msg:
	db "spurious_irq: " , 0x00


;ALIGN 32

;----------init_8259A------------
;void init_8259A();

init_8259A:
	push ebp
	mov ebp , esp
;--------------------------------
;ICW1 -> 0X20 / 0XA0
;ICW2/ICW3/ICW4 -> 0x21 / 0xA1
;OCW1 -> 0x21 / 0xA1
;OCW2 -> 0x20 / 0xA0

;----------set ICW1-----------

;-------master----
	mov al , 00010001b
	out INT_M_CTL , al
	call io_delay
;-------slave-----
	out INT_S_CTL , al
	call io_delay

;----------set ICW2-----------
;-------master------
	mov al , INT_VECTOR_IRQ0 ;irq0 -> int vector 0x20 
	out INT_M_CTLMASK , al
	call io_delay
;-------slave--------
	mov al , INT_VECTOR_IRQ8 ;irq8 -> int vector 0x28
	out INT_S_CTLMASK , al
	call io_delay

;----------set ICW3-------------
;--------master-------
	mov al , 00000100b
	out INT_M_CTLMASK , al
	call io_delay
;---------slave--------
	mov al , 00000010b
	out INT_S_CTLMASK , al
	call io_delay

;-----------set ICW4--------------
;---------master--------
	mov al , 00000001b
	out INT_M_CTLMASK , al
	call io_delay
;----------slave---------
	out INT_S_CTLMASK , al
	call io_delay

;-----------set OCW1--------------
;open/close outside int 0=open 1=close
;---------master---------
	mov al , 11111111b; IRQ0(TIME CLOCK) -> OPEN
	out INT_M_CTLMASK , al
	call io_delay
;---------slave ---------
	mov al , 11111111b;IRQ8~IRQ15 --> CLOSE
	out INT_S_CTLMASK , al
	call io_delay

;--------set OCW2-----------
;实际上就是输入EOI，每次中断处理结束后需要发送EOI才能持续接受中断

	mov esp , ebp
	pop ebp
	ret


;--------set_irq_table------------------
;void set_irq_table(int IRQ , int *handler);
;注意处理程序在表中的位置与IRQ的值是一样的
set_irq_table:
	push ebp
	mov ebp , esp
	push ebx

	xor ebx , ebx
	mov ebx , [ebp + 8]

;	M_disable_irq ebx ; close IRQ int

	M_multib bl , 4
	mov ebx , eax	;handler offset in table -> eax -> ebx

	M_m2md [irq_table + ebx] , [ebp + 12]

	pop ebx
	mov esp , ebp
	pop ebp
	ret

;-------disable_irq----------------------
;void disable_irq(int irq)
;关闭对应的硬件中断
;if(irq < 8) set INT_M_CTLMASK else set INT_S_CTLMASK
disable_irq:
	push ebp
	mov ebp , esp
	pushf

	xor ecx , ecx
	mov ecx , [ebp + 8];irq -> ecx(cl)

	cmp cl , 8
	jae dis_slave

dis_master:
	in al , INT_M_CTLMASK

	mov ch , 0x01
	shl ch , cl ;(1 << irq -> ch)

	test al , ch
	jnz dis_already	;表示该中断已经被屏蔽(=1)

	or al , ch
	out INT_M_CTLMASK , al
	jmp dis_already

dis_slave:
	in al , INT_S_CTLMASK

	sub cl , 8;beacuse cl >= 8
	mov ch , 0x01
	shl ch , cl

	test al , ch
	jnz dis_already

	or al , ch
	out INT_S_CTLMASK , al

dis_already:

	popf
	mov esp , ebp
	pop ebp
	ret

;---------enable_irq----------------
;void enable_irq(int irq);
;if(irq < 8) set INT_M_CTLMASK els set INT_M_CTLMASK
enable_irq:
	push ebp
	mov ebp , esp
	pushf

	xor ecx , ecx
	mov ecx , [ebp + 8];irq -> ecx

	cmp cl , 8
	jae enable_slave

enable_master:
	in al , INT_M_CTLMASK

	mov ch , 0x01
	shl ch , cl

	test al , ch
	jz enable_already

	not ch
	and al , ch
	out INT_M_CTLMASK , al

	jmp enable_already

enable_slave:
	in al , INT_S_CTLMASK

	sub cl , 8
	mov ch , 0x01
	shl ch , cl

	test al , ch
	jz enable_already

	not ch
	and al , ch
	out INT_S_CTLMASK , al

enable_already:

	popf
	mov esp , ebp
	pop ebp

;---------enable_int------------------
enable_int:
	sti
	ret
;---------disable_int-----------------
disable_int:
	cli
	ret

;---------spurious_irq-----------------
;void spurious_irq(dword IRQ);

spurious_irq:
	push ebp
	mov ebp , esp

	mov dword [videoaddr] , (80 * 2 + 30) * 2

	push dword [videoaddr]
	push dword irq_msg
	call dispstr
	add esp , 8
	mov [videoaddr] , eax


	xor eax , eax
	mov dword eax , [ebp + 8]
	push dword [videoaddr]
	push eax
	call disphexdw
	add esp , 8
	mov [videoaddr] , eax




	mov esp , ebp
	pop ebp
	ret
	

io_delay:
	nop
	nop
	nop
	nop
	ret










