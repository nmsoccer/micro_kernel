global get_ticks , init_sys_call_table

global send_msg , recv_msg

extern dispstr , disphexdw

extern sys_send_msg , sys_recv_msg

%include "global.inc"

[SECTION .data]

[SECTION .text]
;-------register in sys_call_table---

init_sys_call_table:

	M_m2md [sys_call_table + _FUC_GET_TICKS * 4] , sys_get_ticks
	M_m2md [sys_call_table + _FUC_SEND_MSG * 4] , sys_send_msg
	M_m2md [sys_call_table + _FUC_RECV_MSG * 4] , sys_recv_msg


	ret

;-------register ends----------------






;--------FUNCTIONS Realization-------
;Package NUM_SYS_CALL functions


;-------FUN CODE =0---------------------
;return in eax -> ticks
;int get_ticks();
;user level
get_ticks:
	mov dword eax , _FUC_GET_TICKS
	int INT_VECTOR_SYS_CALL
	ret

;kernel level
sys_get_ticks:

	mov dword eax , [ticks]

	ret


;------FUNCTION CODE = 1----------------
;return in eax -> success or not
;int send_msg(MSG *p_msg);
send_msg:
	push ebp
	mov ebp , esp
	push ebx

	mov dword eax , _FUC_SEND_MSG
	mov dword ebx , [ebp + 8]
	int INT_VECTOR_SYS_CALL



	pop ebx
	mov esp , ebp
	pop ebp
	ret


;sys_send_msg locates in message.c


;------FUNCTION CODE = 2-----------------
;return in eax -> MSG* or NULL
;MSG* recv_msg();
recv_msg:

	mov dword eax , _FUC_RECV_MSG
	int INT_VECTOR_SYS_CALL

	ret

;sys_recv_msg locates in message.c















