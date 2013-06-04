global global_define , leiming
;from "global.inc"
;define gdt , gdtptr , videoaddr  , gdt_len'
%define GLOBAL_VAR_HERE

%include "global.inc"

;This function is used to define global variables from global.inc
;can be used once
global_define:


	dd 0

	ret
