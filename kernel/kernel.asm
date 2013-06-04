SELECTOR_KERNEL_CS		equ 8

global _start

;-------int handle function
global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error

;------irq handle function
global hwint00
global hwint01
global hwint02
global hwint03
global hwint04
global hwint05
global hwint06
global hwint07
global hwint08
global hwint09
global hwint10
global hwint11
global hwint12
global hwint13
global hwint14
global hwint15

;-----sys_call---------
global sys_call
;---process restart----
global start

;from kernel
extern  global_define ,  cstart
extern  base_selec  , init_desc
extern  exception_handler , spurious_irq , set_irq_table  , enable_irq
extern  clock_handler , keyboard_handler , hd_handler
extern  kernel_main , disphexdw , dispstr

;from C
extern  proc_table

%include "global.inc"

;---inner MACRO FUNCTIONS-----------
;-----includes M_hwint_master and M_hwint_slave--

;M_hwint_master
%macro M_hwint_master 1
;before int program is running in ring1 and esp -> task_stack + STACK_SIZE_TOTAL
;after int return ring 0 and should use stack in ring0 but esp not point to stack_top of ring0
;esp -> sel_ldt of pcb then esp -> eip (eip , cs , eflags , esp , ss They are pushed by CPU)

	call save

;	M_disphexd [esi + PCB_CS] , 0
;屏蔽当前中断
	in al , INT_M_CTLMASK ; 获得8259主片0x21端口的中断信息(因为在设定时最后输入的是OCW1，就是开/闭中断号的向量--)
	or al , (1 << %1) ;屏蔽中断是将对应位置设1.而IRQ号和位置号是对应的，比如IRQ_KEYBORD=1 而在向量就是第一位(从零开始)
	out INT_M_CTLMASK , al


;EOI
	mov al , EOI;输入EOI，每次中断处理结束后需要发送EOI才能再次接受新的中断
	out INT_M_CTL , al

sti;CPU在响应中断的过程中会自动关闭中断，这句之后就允许相应新的中断
;-------Inner logic----
	push dword %1
	call [irq_table + 4 * %1]
	pop ecx ;等效于add esp , 4 同时将IRQ号放入ecx
cli

;打开当前中断
	in al , INT_M_CTLMASK
	and al , ~(1 << %1) ;打开中断使相应位置为0
	out INT_M_CTLMASK , al

	ret	;return to restart or restart_reenter

%endmacro


;M_hwint_slave
%macro M_hwint_slave 1
;before int program is running in ring1 and esp -> task_stack + STACK_SIZE_TOTAL
;after int return ring 0 and should use stack in ring0 but esp not point to stack_top of ring0
;esp -> sel_ldt of pcb then esp -> eip (eip , cs , eflags , esp , ss They are pushed by CPU)

	call save


;屏蔽当前中断
	in al , INT_S_CTLMASK ; 获得8259从片0xA1端口的中断信息(因为在设定时最后输入的是OCW1，就是开/闭中断号的向量--)
	or al , (1 << %1 - 8) ;屏蔽中断是将对应位置设1.而IRQ号和位置号是对应的，比如IRQ_KEYBORD=1 而在向量就是第一位(从零开始)
	out INT_S_CTLMASK , al


;EOI
	mov al , EOI;输入EOI，每次中断处理结束后需要发送EOI才能再次接受新的中断
	out INT_M_CTL , al
	nop
	out INT_S_CTL , al	;注意从主片都要接收EOI

sti;CPU在响应中断的过程中会自动关闭中断，这句之后就允许相应新的中断
;-------Inner logic----
	push dword %1
	call [irq_table + 4 * %1]
	pop ecx ;等效于add esp , 4 同时将IRQ号放入ecx
cli

;打开当前中断
	in al , INT_S_CTLMASK
	and al , ~(1 << %1 - 8) ;打开中断使相应位置为0
	out INT_S_CTLMASK , al

	ret	;return to restart or restart_reenter

%endmacro






[section .data]

title:
	db "-------CSTART------" , 0x00 , 0x00

signal:
	db "++++++" , 0x00

;videoaddr:	dd 0

debug:
	db "$" , 0x00

[SECTION .bss]

stackspace:	resb 3 * 1024
stacktop	equ	$ - stackspace - 1;

[SECTION .text]	; 代码在此
ALIGN 4

;ALIGN 32



_start:	

;	xor ax , ax
;	mov ax , ds
;	mov gs , ax

	mov esp , stacktop

	call global_define

	mov dword eax , [videoaddr]

	sgdt [gdtptr] ; get information of gdt
	call cstart
	lgdt [gdtptr]



	lidt [idtptr]


	
	jmp SELECTOR_KERNEL_CS:csinit

csinit:

	xor ax , ax
	mov ax , ds
	mov fs , ax;loader set gs -> 0x2000 ; now reset gs = ds
	mov ss , ax;loader set ss -> 0x9000 ; now reset ss = ds


;----init TSS--------
	mov dword [tss + 8] , SELECTOR_KERNEL_DS;tss.ss0 = SELECTOR_KERNEL_DS


;set descriptor_tss
;get base of desc
	M_base_selec ds , gdt
	mov ebx , eax
	M_vir2phy ebx , tss
;set desc
	M_init_desc SELECTOR_TSS , eax , tss_len , DA_386TSS , gdt

	mov word [tss + 102] , (tss_len + 1);tss.iobase = sizeof(tss) NO I/O bitmap
	;当I/O位图基址大于或者等于TSS的段界限表示没有I/O位图

;load tss

	xor eax , eax
	mov ax , SELECTOR_TSS
	ltr ax


;-----register irq_table------------------------
	M_set_irq_table IRQ_CLOCK , clock_handler
	M_enable_irq IRQ_CLOCK

	M_set_irq_table IRQ_KEYBOARD , keyboard_handler
	M_enable_irq IRQ_KEYBOARD

	M_set_irq_table IRQ_CASCADE , spurious_irq
	M_enable_irq IRQ_CASCADE

	M_set_irq_table IRQ_ETHER , spurious_irq
;	M_enable_irq IRQ_ETHER

	M_set_irq_table IRQ_SECONDARY , spurious_irq
;	M_enable_irq IRQ_SECONDARY

	M_set_irq_table IRQ_RS232 , spurious_irq
;	M_enable_irq IRQ_RS232

	M_set_irq_table IRQ_XT_WINI , spurious_irq
;	M_enable_irq IRQ_XT_WINI

	M_set_irq_table IRQ_FLOPPY , spurious_irq
;	M_enable_irq IRQ_FLOPPY

	M_set_irq_table IRQ_PRINTER , spurious_irq
;	M_enable_irq IRQ_PRINTER

	M_set_irq_table IRQ_AT_WINI , hd_handler
	M_enable_irq IRQ_AT_WINI


	jmp kernel_main


;-----------int handle function---------------------------------
;if no error code push 0xFFFFFFFF first and push vector
;else push error code first and push vector
divide_error:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_DIVIDE			;vector_number = 0
	jmp  exception

single_step_exception:
	push 0xFFFFFFFF ;no err_code
	push dword INT_VECTOR_DEBUG			;vector_number = 1
	jmp exception

nmi:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_NMI			;vector_number = 2
	jmp exception

breakpoint_exception:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_BREAKPOINT			;vector_number = 3
	jmp exception

overflow:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_OVERFLOW			;vector_number = 4
	jmp exception

bounds_check:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_BOUNDS			;vector_number = 5
	jmp exception

inval_opcode:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_INVAL_OP			;vector_number = 6
	jmp exception

copr_not_available:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_COPROC_NOT			;vector_numbr = 7
	jmp exception


double_fault:
	push dword INT_VECTOR_DOUBLE_FAULT			;vector_number = 8
	jmp exception

copr_seg_overrun:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_COPROC_SEG			;vector_number = 9
	jmp exception

inval_tss:
	push dword INT_VECTOR_INVAL_TSS		;vector_number = 10
	jmp exception	

segment_not_present:
	push dword INT_VECTOR_SEG_NOT		;vector_number = 11
	jmp exception

stack_exception:
	push dword INT_VECTOR_STACK_FAULT		;vector_number = 12
	jmp exception

general_protection:
	push dword INT_VECTOR_PROTECTION		;vector_number = 13
	jmp exception

page_fault:
	push dword INT_VECTOR_PAGE_FAULT 		;vector_number =14
	jmp exception

copr_error:
	push 0xFFFFFFFF	;no err_code
	push dword INT_VECTOR_COPROC_ERR		;vector_number = 16
	jmp exception

exception:
	call exception_handler
	add esp , 8
	hlt

;---------irq handle function--------
;------master----
;%macro hwint_master 1

;	push dword %1
;	call spurious_irq
;	add esp , 4
;	hlt
;%endmacro

hwint00:		; Interrupt routine for irq 00	(the clock).

	M_hwint_master IRQ_CLOCK


hwint01:		; Interrupt routine for irq 01	(keyboard)
	M_hwint_master IRQ_KEYBOARD
hwint02:		; Interrupt routine for irq 02	(cascade!)
	M_hwint_master IRQ_CASCADE
hwint03:		; Interrupt routine for irq 03	(second serial)
	M_hwint_master IRQ_SECONDARY
hwint04:		; Interrupt routine for irq 04	(first serial)
	M_hwint_master IRQ_RS232
hwint05:		; Interrupt routine for irq 05	(XT winchester)
	M_hwint_master IRQ_XT_WINI
hwint06:		; Interrupt routine for irq 06	(floppy)
	M_hwint_master IRQ_FLOPPY
hwint07:		; Interrupt routine for irq 07	(printer)
	M_hwint_master IRQ_PRINTER

;------slave------
;%macro hwint_slave 1

;	push dword %1
;	call spurious_irq
;	add esp , 4
;	hlt

;%endmacro

hwint08:		; Interrupt routine for irq 08	(realtime clock)
	M_hwint_slave 8
hwint09:		; Interrupt routine for irq 09 	(irq 2 redirected)
	M_hwint_slave 9
hwint10:		; Interrupt routine for irq 10
	M_hwint_slave 10
hwint11:		; Interrupt routine for irq 11
	M_hwint_slave 11
hwint12:		; Interrupt routine for irq 12
	M_hwint_slave 12
hwint13:		; Interrupt routine for irq 13	(FPU exception)
	M_hwint_slave 13
hwint14:		; Interrupt routine for irq 14	(AT winchester)
	M_hwint_slave IRQ_AT_WINI
hwint15:		; Interrupt routine for irq 15
	M_hwint_slave 15

;--------assist functions-----------------------------

sys_call:

;int and irq int push stack are not the same

;function code in eax

	call save


	sti

	push dword [ebx + PCB_P_FLAG]
	push dword [ebx + PCB_P_TTY]
	push dword [ebx + PCB_PARENT_PID]
	push dword [ebx + PCB_PID]
	push dword [ebx + PCB_EBX]	;

	call [sys_call_table + eax * 4] ; eax -> FUN CODE
	mov [ebx + PCB_EAX] , eax ; return value in eax
	add esp , 20

	cli


	ret




;--------Process dealing when int happens----------
;--------------------------------------------------------------------
save:

	pushad

	push  dword ds
	push  dword es
	push  dword fs
	push  dword gs


;	M_disphexd esp , (80 * 4 + 0) * 2


	mov dx , ss
	mov ds , dx
	mov es , dx
	mov fs , dx

	mov ebx , esp;ebx=esp -> start of pcb
	;but after that esp = 0x3309C ebx= 0x330A0 ebx=esp+4
	;中断发生后(无论是IRQ还是INT n) 这句非常重要，不仅保存了esp还自动让esp+4(pcb的起始位置)放入ebx之中.不知道具体机制是怎样的.


;	M_disphexd esp , 0
;	M_disphexd ebx , (80 * 4 + 0) * 2

;	add esp , 8 ;error happened
;	mov ecx , esp
;	M_disphexd ecx , (80 * 4 + 10) * 2

	;test re_enter before change stack
	inc dword [k_reenter]

	cmp dword [k_reenter] , 1
	ja .1

	mov esp , stacktop ; let esp -> stacktop of ring0
	push restart

	jmp [ebx + RETADDR]


.
.1:
	push restart_reenter
	jmp [ebx+ RETADDR]

;------process start----------
;start the initial process
start:
	mov dword [proc_table] , 1

;change process or continue process

restart:
	M_multib [proc_table] , 4	;get offset of pre enable pcb of process
	mov esp , [proc_table + eax];change esp -> head of one pcb in order to enable such process

	;load ldt
	xor eax , eax
	mov dword eax , [esp + PCB_SEL_LDT]
	lldt ax

	xor eax , eax
	lea eax , [esp + PCB_STACKTOP]
	mov [tss + TSS_SP0] , eax


;------------
restart_reenter:

	dec dword [k_reenter]

	pop dword gs
	pop dword fs
	pop dword es
	pop dword ds

	popad

	add esp , 4

	iretd

;--------------dealing end-------------------






