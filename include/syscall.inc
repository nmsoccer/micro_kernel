;system call function code

;------number of sys_calls---------
%define NUM_SYS_CALL 3

;------return value----------------
%define SUCCESS 0
%define FAULT   1

;------function code---------------

%define _FUC_GET_TICKS 	0x00

%define _FUC_SEND_MSG	0x01
%define _FUC_RECV_MSG	0x02	