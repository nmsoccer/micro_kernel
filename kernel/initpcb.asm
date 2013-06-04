global set8253
global _init_usr_pcb , _init_sys_pcb
;增加进程需要在其他文件中增加：进程体\SELECTOR_LDT_*\ID_PROCESS*\(proctec.inc , process.inc)在本文件中参照PCB1带;$的行 需要改变


extern memset , memcpy , base_selec  , init_desc

%define USE_STC_PCB

%include "global.inc"


;-------SET 8253----------
set8253:

	;reset clock T=50ms(8253)

	M_out_b TIMER_MODE , RATE_GENERATOR ; mode control reg is 8bits

	M_out_b TIMER0 , TIMER_FREQ/HZ ; for counter0 is 16bits
	M_out_b TIMER0 , (TIMER_FREQ/HZ >> 8)


	ret

;;---------------------------------------------------------------------------------------------------------------------------------------------

;void _init_usr_pcb(PCB *p_pcb , u32 sel_ldt ,  int * function , STACK_PROC *p_stack_proc , int pid , int parent_pid , char *p_name);
_init_usr_pcb:
	push ebp
	mov ebp , esp

	mov esi, [ebp + 8]	;p_pcb -> esi
;	mov pcb1 , esi
;----------------pcb1---------------------
;-----set pcb1 default---

;	M_memset pcb1 , 0 , PCB_LEN , 0										;$
	M_memset esi , 0 , PCB_LEN , 0

;----register pcb0 to proc_table---

;	M_m2md [proc_table + ID_PROC1 * 4] , pcb							;$


;----intialize pcb1-----
;----set sel_ldt--

;	M_m2md	[pcb1 + stc_pcb.sel_ldt] , [ebp + 12]					;$
	M_m2md   [esi + stc_pcb.sel_ldt] , [ebp + 12]

;----set des in ldt-
;----des0-
;copy descriptor_flat_code in gdt to des0 in ldt

	xor ebx , ebx
	mov dword ebx , gdt
	M_multib INDEX_FLAT_CODE , DESCRIPTOR_LEN
	add ebx , eax
	;&gdt[index]

	push esi


;	M_memcpy (pcb + stc_pcb.ldt) , ebx , DESCRIPTOR_LEN			;$
	lea ecx , [esi + stc_pcb.ldt]
	M_memcpy ecx , ebx , DESCRIPTOR_LEN


	pop esi

;change DPL
;	mov byte al , [pcb1 + stc_pcb.ldt + 5]								;$
	mov byte al , [esi + stc_pcb.ldt + 5]
	mov bl , PRIVILEGE_USER
	shl bl , 5
	or al , bl ; bite 5,6 is DPL
	mov byte [esi + stc_pcb.ldt + 5] , al								;$

;---des1--

	xor ebx , ebx
	mov dword ebx , gdt
	M_multib INDEX_FLAT_DATA , DESCRIPTOR_LEN
	add ebx , eax
	;&gdt[index]

	push esi

	lea ecx , [esi + stc_pcb.ldt + DESCRIPTOR_LEN]
	M_memcpy ecx , ebx , DESCRIPTOR_LEN
;	M_memcpy (pcb + stc_pcb.ldt + DESCRIPTOR_LEN) , ebx , DESCRIPTOR_LEN	;$
	pop esi
;change DPL

	mov byte al , [esi + stc_pcb.ldt + DESCRIPTOR_LEN + 5]		;$
	mov bl , PRIVILEGE_USER
	shl bl , 5
	or al , bl ; bite 5,6 is DPL
	mov byte [esi + stc_pcb.ldt + DESCRIPTOR_LEN + 5] , al		;$

;-----set register-------
;this at some point in fact sets selector fro ldt[0] and ldt[1] sperately
;remember: offset + TI + RPL

;set gs 0x18+1
	xor eax , eax

	mov eax , SELECTOR_KERNEL_GS ;0x18 + 3;DPL = 3

	and eax , SA_RPL_MASK ; set RPL (=3)
	or  eax , SA_RPL3

	mov dword [esi + stc_pcb.gs] , eax									;$

;set selector_code in fs , es , ds , ss 0xD
	xor eax , eax

	mov eax , DESCRIPTOR_LEN ;offset of des1 in ldt is DESCRIPTOR_LEN (8)

	and eax , SA_RPL_MASK ; set RPL (=3)
	or  eax , SA_RPL3

	and eax , SA_TI_MASK  ; set TI=1
	or  eax , SA_TI_TIL

	mov dword [esi + stc_pcb.fs] , eax ;fs							;$
	mov dword [esi + stc_pcb.es] , eax ;es							;$
	mov dword [esi + stc_pcb.ds] , eax ;ds							;$
	mov dword [esi + stc_pcb.ss] , eax ;ss							;$

;cs 0x5
	xor eax , eax

	mov eax , 0 ;offset of des0 in ldt is 0

	and eax , SA_RPL_MASK ; set RPL (=3)
	or  eax , SA_RPL3

	and eax , SA_TI_MASK  ; set TI=1
	or  eax , SA_TI_TIL

	mov dword [esi + stc_pcb.cs] , eax									;$


;---set other registers-
;eip -> process1

;	M_m2md [pcb1 + stc_pcb.eip] , [task_table + ID_PROC1 * 4]					;$
	M_m2md [esi + stc_pcb.eip] , [ebp + 16]

;esp -> task_stack + STACK_SIZE_TOTAL

;	M_m2md [pcb1 + stc_pcb.esp] , (task_stack + (STACK_SIZE_TASK * ID_PROC1))	;$
	M_m2md [esi + stc_pcb.esp] , [ebp + 20]

;eflags -> 0x1202 IF=1 IOPL-1 bit 2 is always 1

	M_m2md [esi + stc_pcb.eflags] , 0x202							;$

;----set slices and priority----------
	M_m2md [esi + stc_pcb.slices] , 15									;$
	M_m2md [esi + stc_pcb.priority] , 15								;$ priority = start slices
;----init descriptor of ldt in gdt----
;get base of desc
	M_base_selec ds , gdt
	mov ebx , eax

	lea ecx , [esi + stc_pcb.ldt]
	M_vir2phy ebx , ecx
;	M_vir2phy ebx , (pcb + stc_pcb.ldt)								;$

;	M_init_desc SELECTOR_LDT_FIRST , eax , (DESCRIPTOR_LEN * LDT_SIZE - 1) , DA_LDT , gdt	;$
	M_init_desc [ebp + 12] , eax , (DESCRIPTOR_LEN * LDT_SIZE - 1) , DA_LDT , gdt



;----set pid--------------------------
	M_m2md [esi + stc_pcb.pid] , [ebp + 24]

;----set parent_pid-------------------
	M_m2md [esi + stc_pcb.parent_pid] , [ebp + 28]

;----set p_tty------------------------
	M_m2md [esi + stc_pcb.p_tty] , [ebp + 32]

;----set p_flag-----------------------
	M_m2md [esi + stc_pcb.p_flag] , 0	;RUN = 0 Default

	mov esp , ebp
	pop ebp
	ret

;----------------------------------------
;void _init_sys_pcb(PCB *p_pcb , u32 sel_ldt ,  int * function , STACK_PROC *p_stack_proc , int pid , int parent_pid , char *p_name);
_init_sys_pcb:
	push ebp
	mov ebp , esp

	mov esi, [ebp + 8]	;p_pcb -> esi
;	mov pcb1 , esi
;----------------pcb1---------------------
;-----set pcb1 default---

;	M_memset pcb1 , 0 , PCB_LEN , 0										;$
	M_memset esi , 0 , PCB_LEN , 0

;----register pcb0 to proc_table---

;	M_m2md [proc_table + ID_PROC1 * 4] , pcb							;$


;----intialize pcb1-----
;----set sel_ldt--

;	M_m2md	[pcb1 + stc_pcb.sel_ldt] , [ebp + 12]					;$
	M_m2md   [esi + stc_pcb.sel_ldt] , [ebp + 12]

;----set des in ldt-
;----des0-
;copy descriptor_flat_code in gdt to des0 in ldt

	xor ebx , ebx
	mov dword ebx , gdt
	M_multib INDEX_FLAT_CODE , DESCRIPTOR_LEN
	add ebx , eax
	;&gdt[index]

	push esi


;	M_memcpy (pcb + stc_pcb.ldt) , ebx , DESCRIPTOR_LEN			;$
	lea ecx , [esi + stc_pcb.ldt]
	M_memcpy ecx , ebx , DESCRIPTOR_LEN


	pop esi

;change DPL
;	mov byte al , [pcb1 + stc_pcb.ldt + 5]								;$
	mov byte al , [esi + stc_pcb.ldt + 5]
	mov bl , PRIVILEGE_TASK
	shl bl , 5
	or al , bl ; bite 5,6 is DPL
	mov byte [esi + stc_pcb.ldt + 5] , al								;$

;---des1--

	xor ebx , ebx
	mov dword ebx , gdt
	M_multib INDEX_FLAT_DATA , DESCRIPTOR_LEN
	add ebx , eax
	;&gdt[index]

	push esi

	lea ecx , [esi + stc_pcb.ldt + DESCRIPTOR_LEN]
	M_memcpy ecx , ebx , DESCRIPTOR_LEN
;	M_memcpy (pcb + stc_pcb.ldt + DESCRIPTOR_LEN) , ebx , DESCRIPTOR_LEN	;$
	pop esi
;change DPL

	mov byte al , [esi + stc_pcb.ldt + DESCRIPTOR_LEN + 5]		;$
	mov bl , PRIVILEGE_TASK
	shl bl , 5
	or al , bl ; bite 5,6 is DPL
	mov byte [esi + stc_pcb.ldt + DESCRIPTOR_LEN + 5] , al		;$

;-----set register-------
;this at some point in fact sets selector fro ldt[0] and ldt[1] sperately
;remember: offset + TI + RPL

;set gs 0x18+1
	xor eax , eax

	mov eax , SELECTOR_KERNEL_GS ;0x18 + 3;DPL = 3

	and eax , SA_RPL_MASK ; set RPL (=1)
	or  eax , SA_RPL1

	mov dword [esi + stc_pcb.gs] , eax									;$

;set selector_code in fs , es , ds , ss 0xD
	xor eax , eax

	mov eax , DESCRIPTOR_LEN ;offset of des1 in ldt is DESCRIPTOR_LEN (8)

	and eax , SA_RPL_MASK ; set RPL (=1)
	or  eax , SA_RPL1

	and eax , SA_TI_MASK  ; set TI=1
	or  eax , SA_TI_TIL

	mov dword [esi + stc_pcb.fs] , eax ;fs							;$
	mov dword [esi + stc_pcb.es] , eax ;es							;$
	mov dword [esi + stc_pcb.ds] , eax ;ds							;$
	mov dword [esi + stc_pcb.ss] , eax ;ss							;$

;cs 0x5
	xor eax , eax

	mov eax , 0 ;offset of des0 in ldt is 0

	and eax , SA_RPL_MASK ; set RPL (=1)
	or  eax , SA_RPL1

	and eax , SA_TI_MASK  ; set TI=1
	or  eax , SA_TI_TIL

	mov dword [esi + stc_pcb.cs] , eax									;$


;---set other registers-
;eip -> process1

;	M_m2md [pcb1 + stc_pcb.eip] , [task_table + ID_PROC1 * 4]					;$
	M_m2md [esi + stc_pcb.eip] , [ebp + 16]

;esp -> task_stack + STACK_SIZE_TOTAL

;	M_m2md [pcb1 + stc_pcb.esp] , (task_stack + (STACK_SIZE_TASK * ID_PROC1))	;$
	M_m2md [esi + stc_pcb.esp] , [ebp + 20]

;eflags -> 0x1202 IF=1 IOPL-1 bit 2 is always 1

	M_m2md [esi + stc_pcb.eflags] , 0x1202							;$

;----set slices and priority----------
	M_m2md [esi + stc_pcb.slices] , 30									;$
	M_m2md [esi + stc_pcb.priority] , 30								;$ priority = start slices
;----init descriptor of ldt in gdt----
;get base of desc
	M_base_selec ds , gdt
	mov ebx , eax

	lea ecx , [esi + stc_pcb.ldt]
	M_vir2phy ebx , ecx
;	M_vir2phy ebx , (pcb + stc_pcb.ldt)								;$

;	M_init_desc SELECTOR_LDT_FIRST , eax , (DESCRIPTOR_LEN * LDT_SIZE - 1) , DA_LDT , gdt	;$
	M_init_desc [ebp + 12] , eax , (DESCRIPTOR_LEN * LDT_SIZE - 1) , DA_LDT , gdt



;----set p_id--------------------------
	M_m2md [esi + stc_pcb.pid] , [ebp + 24]

;----set parent_pid-------------------
	M_m2md [esi + stc_pcb.parent_pid] , [ebp + 28]

;----set p_tty------------------------
	M_m2md [esi + stc_pcb.p_tty] , [ebp + 32]

;----set p_flag-----------------------
	M_m2md [esi + stc_pcb.p_flag] , 0	;RUN = 0 Default

	mov esp , ebp
	pop ebp
	ret




