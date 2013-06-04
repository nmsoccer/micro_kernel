
;------------DESCRIPTOR-----------
%macro DESCRIPTOR 0
	;arg1 -> limit 2.5B %1	input dword high 1.5B is not used
	;arg2 -> base  4B   %2	input dword
	;arg3 -> attr  1.5B %3	input word  high 0.5B is not used

;	dw 0xFFFF & %1	;limit low 2B

;	dw 0xFFFF & %2	;base low 2B
;	db 0xFF & (%2 >> 16)	;base mid 1B

;	db 0xFF & %3	;attr low 1B
;	db ((0x0F & (%3 >> 8)) << 4) | (%1 >> 16)
	;attr high 0.5B in high 0.5B of this Byte	
	;limit high 0.5B in low 0.5B of this Byte

;	db 0xFF & (%2 >> 24)	; base high 1B

	dd 0
	dd 0


%endmacro

%define DESCRIPTOR_LEN 8

;---------INT_GATE----------------
;total 8B

%macro GATE 0
	dw 0 ; offst_low

	dw 0 ; selector
	db 0 ; dcount only useful in call gate

	db 0 ; attr
	dw 0 ; offset_high


%endmacro

;-----------define DPL------
;all only refer to DPL 2byte
%define PRIVILEGE_KRNL	0
%define PRIVILEGE_TASK	1
%define PRIVILEGE_USER	3

%define RPL_KRNL PRIVILEGE_KRNL
%define RPL_TASK PRIVILEGE_TASK
%define RPL_USER PRIVILEGE_USER



;------INDEX-------------------
;index实际上是selector右移三位(除以8)取整所得
%define INDEX_DUMMY 0
%define INDEX_FLAT_CODE 1
%define INDEX_FLAT_DATA 2
%define INDEX_VIDEO 3
%define INDEX_TSS 4
%define INDEX_LDT_FIRST 5 ;the first ldt(in process1)


;-------SELECTOR----------------
;selector 实际上就是描述符在GDT中的相对位置，因为最多有128个描述符 每个8字节 因此selector最多127 * 8

%define SELECTOR_DUMMY	0
%define SELECTOR_FLAT_CODE	0x8
%define SELECTOR_FLAT_DATA	0x10
%define SELECTOR_VIDEO 0x18 + 3;DPL = 3
;They are all defined in Loader; 

%define SELECTOR_TSS 0x20

%define SELECTOR_LDT_FIRST 0x28
%define SELECTOR_LDT_SECOND 0x30

%define SELECTOR_LDT_03	0X38
%define SELECTOR_LDT_04 0x40


%define SELECTOR_KERNEL_CS SELECTOR_FLAT_CODE
%define SELECTOR_KERNEL_DS SELECTOR_FLAT_DATA
%define SELECTOR_KERNEL_GS SELECTOR_VIDEO




;--------------------ATTR------------
;------------------------------------------------
;7	6	5	4	3	2	  1	   0
;P	 DPL	S	E ED/C W/R A

;这是属性段的低八位
;A是访问标志，有CPU在第一次访问时设置。一般置0
;R/W设置为1使段可读；ED/C表示顺从性，如果置1，表示低优先级代码段可以跳转或调用该段，一般置0.
;E置为1，表示描述符描述的是代码段；
;S表示该段是代码段或数据段，置为1 or system
;descriptor；DPL表示优先级
;
;而还有半个字节的位于BIT6的属性如下：
;7    6       5      4    |  3    2    1    0
;G   D/B   0   AVL |        段界限
;4位偏移量，设置为0Fh；
;AVL为表示Available，AV=1表示segment is available，此处忽略该位，设为0；
;Intel保留了一位必须设为0；
;D表示大小位，置为1，它告诉CPU使用32位代码而不是16位代码；
;G表示粒度，如果G=0，则Limit所表示的段偏移是00000H-FFFFFH，如果G=1，
;则Limit表示的段偏移是00000;XXXH-FFFFFXXXH，即Limit所表示的段偏移实际上是它的值再乘以4K。此处设置G=1。
;



;-----------define SELECTOR ATTR---------
%define SA_RPL_MASK 0xFFFC  ;1111 1111 1111 1100 last 2bits refer to RPL ;将RPL所在的两位清零

%define SA_RPL0 0
%define SA_RPL1 1
%define SA_RPL2 2
%define SA_RPL3 3


%define SA_TI_MASK 0xFFFB	 ;1111 1111 1111 1011 the 3rd bit refers to TI; 将TI所在的一位清零

%define SA_TI_TIG 0 ;0000 TI=0
%define SA_TI_TIL 4 ;0100 TI=1

;----------define NORMAL DESCRIPTOR ATTR---------
;对属性的使用规则是，只给出属性值，至于放置在哪里做什么组合由用户自己决定


;描述符类型值说明

;only use high 4bits of the word
%define	DA_32			0x4000	 ;32 位段 0100 0000 0000 0000 ;
%define	DA_LIMIT_4K		0x8000 ;段界限粒度为 4K 字节 1000 0000 0000 0000

;use low 1B of a word
%define	DA_DPL0			0x00	 ;DPL = 0	0000 0000			
%define	DA_DPL1			0x20	 ;DPL = 1	0010 0000			
%define	DA_DPL2			0x40	 ;DPL = 2	0100 0000			
%define	DA_DPL3			0x60	 ;DPL = 3	0110 0000	
		
;存储段描述符类型值说明
%define	DA_DR			0x90	;存在的只读数据段类型值		1001 0000
%define	DA_DRW			0x92	;存在的可读写数据段属性值		1001 0010
%define	DA_DRWA			0x93	;存在的已访问可读写数据段类型值	1001 0011
%define	DA_C			0x98	;存在的只执行代码段属性值		1001 1000
%define	DA_CR			0x9A	;存在的可执行可读代码段属性值		1001 1010
%define	DA_CCO			0x9C	;存在的只执行一致代码段属性值		1001 1100
%define	DA_CCOR			0x9E	;存在的可执行可读一致代码段属性值	1001 1110

;系统段描述符类型值说明
%define	DA_LDT			0x82	;局部描述符表段类型值		1000 0010	
%define	DA_386TSS		0x89	;可用 386 任务状态段类型值		1000 1001


;---------define GATE ATTR--------

;因为门的描述符结构与普通描述符不一样 所以单列
;注意 门的属性虽然分配两个字节，但是只用高字节。低字节在调用门中可以传参，中断门则是保留。
%define	DA_TASKGATE		0x85	;任务门类型值				1000 0101
%define	DA_386CGATE		0x8C	;386 调用门类型值			1000 1100
%define	DA_386IGATE		0x8E	;386 中断门类型值			1000 1110
%define	DA_386TGATE		0x8F	;386 陷阱门类型值			1000 1111


;-----INT TYPE-------------
%define INT_VECTOR_DIVIDE 				0x0
%define INT_VECTOR_DEBUG  				0x1	
%define INT_VECTOR_NMI       			0x2
%define INT_VECTOR_BREAKPOINT       0x3
%define INT_VECTOR_OVERFLOW       	0x4
%define INT_VECTOR_BOUNDS       		0x5
%define INT_VECTOR_INVAL_OP       	0x6
%define INT_VECTOR_COPROC_NOT       0x7
%define INT_VECTOR_DOUBLE_FAULT     0x8
%define INT_VECTOR_COPROC_SEG       0x9
%define INT_VECTOR_INVAL_TSS     	0xA
%define INT_VECTOR_SEG_NOT    	   0xB
%define INT_VECTOR_STACK_FAULT      0xC 
%define INT_VECTOR_PROTECTION       0xD
%define INT_VECTOR_PAGE_FAULT       0xE
%define INT_VECTOR_COPROC_ERR       0x10 ; not 0xF

%define INT_VECTOR_SYS_CALL			0x90 ;linux = 0x80

;----IRQ TYPE---------------
;Hardware interrupts

%define	IRQ_CLOCK	0		;clock
%define	IRQ_KEYBOARD	1	;keyboard
%define	IRQ_CASCADE	2		; cascade enable for 2nd AT controller 
%define	IRQ_ETHER	3		;default ethernet interrupt vector 
%define	IRQ_SECONDARY	3	; RS232 interrupt vector for port 2 
%define	IRQ_RS232	4		; RS232 interrupt vector for port 1 
%define	IRQ_XT_WINI	5		;xt winchester 
%define	IRQ_FLOPPY	6		;loppy disk 
%define	IRQ_PRINTER	7
%define	IRQ_AT_WINI	14		; at winchester 



;---------DEFINE STRUCT------------
;---------------------------------------------
;use predefine in order to save space


;---example----

;---declare---

;---use----
;%define USE_S_MYTYPE
;mystruct: ISTRUC mytype
;DO NOT USE AT
;	at mytype.age , dd 23
;	at mytype.name , db 0x3C  ;from 0x3C dump

;			 IEND

;	mov dword [mystruct + mytype.age] , 23

;	xor eax , eax
;	mov eax , testinfo
;	mov [mystruct + mytype.p_name] , eax


;-------end example---

;-----begin-------
%ifdef USE_STC1

STRUC	s_stc1

	.p_stc
		resd 1

ENDSTRUC

%endif

%ifdef USE_STC2

STRUC s_stc2

	.p_str
		resd 1

ENDSTRUC

%endif

































