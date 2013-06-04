;%include "const.inc"
;%include "protect.inc"

;----------common struct--------
%ifdef USE_STC_MYTYPE

STRUC stc_mytype	

.name:
	 	resb 12	;没有在start.asm中声明的时候1,2,3,4都可以但是如果在其中定义则1,3,5均不行原因可能在于不同文件中的数据对齐出现了问题

.p_name:
		resd 1 ; pointer -> string
.age:
		resd 1


ENDSTRUC

%endif
%define USE_STC_MYTYPE
;----------------------------------------------

%include "process.inc"

%include "macrofunc.inc"
%include "syscall.inc"

;procss.inc includes const.inc and protect.inc
;注意全局变量只在某处定义一次 其余的引用（声明）即可。这和结构体不同，在需要用结构体的地方都必须有定义


;----------define global varibales here---------

%ifdef GLOBAL_VAR_HERE

global gdt , gdt_len , gdtptr
global idt , idt_len , idtptr
global tss , tss_len
global irq_table , sys_call_table
global k_reenter , ticks
global videoaddr
;global me


;------gdt--------------
gdt:
%rep GDT_SIZE
	DESCRIPTOR
%endrep

gdt_len	equ ($ - gdt) -1
gdtptr:
	dw 0 ; limit of gdt
	dd 0 ; base of gdt

;-------idt-------------
idt:
%rep IDT_SIZE
	GATE
%endrep

idt_len	equ ($ - idt) - 1
idtptr:
	dw 0 ; limit of idt
	dd 0 ; base of idt
	
	
;-----tss---------
tss:
%rep 25	
	dd 0
%endrep

	dw 0
	dw 0
	
tss_len	equ ($ - tss) - 1	


;----irq_table----------
;This table includes pointers of NUM_IRQ hardware ints

irq_table:
%rep NUM_IRQ
	dd 0
%endrep

;----sys_call_table-----
;This table includes pointers of NUM_SYS_CALL functions

sys_call_table:
%rep NUM_SYS_CALL
	dd 0
%endrep	
;---mutex singal-------------
;clock reenter 
k_reenter:
	dd 1
	
;---ticks--------------------
;record click ticks
ticks:
	dd 0	

;-----display location-------
 videoaddr:
	 dd 0

;---------------------------------
;me: ISTRUC stc_mytype
;	IEND
;---------------------------------

%endif


;-----------just declare outside link--------
;-----------------------------------------------------------
%ifndef GLOBAL_VAR_HERE
extern gdt , gdt_len , gdtptr
extern idt , idt_len , idtptr
extern tss , tss_len
extern irq_table , sys_call_table
extern k_reenter , ticks
extern videoaddr
;extern me

%endif 