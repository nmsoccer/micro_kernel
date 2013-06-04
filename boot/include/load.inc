base_loaderphy	equ	base_loader * 0x10 ;the physical address of Loader,bin
					   ;base_loader is defined in fat12hdr.inc	
;---------LOADER----------------------------

base_kernel	equ 0x5000	;absolutely 0x50000 ~ 0x8FFFF total 256K
offset_kernel	equ 0x0


;--------KERNEL-----------------------------
base_kernelphy		equ	base_kernel * 0x10
;entry_kernelphy		equ  	0x30400 
entry_kernelphy	equ 	0x1400	;absolutely 0x1000 ~ 0x4FFFF total 316K 