#include "global.h"

void init_console(){
	CONSOLE *p;
	int i;

	for(i = 1 ; i <= NUM_CONSOLE ; i++){
		p = (CONSOLE *) &console_table[i];

		switch(i){	//initialize base_vmem
			case 1:
				p -> base_vmem = (u32)BASE1_VMEM;
				break;
			case 2:
				p -> base_vmem = (u32)BASE2_VMEM;
				break;
			case 3:
				p -> base_vmem = (u32)BASE3_VMEM;
				break;
		}

		p -> start_vmem = p -> base_vmem;	//They are equal at the beginning
		p -> size_vmem	= (u32)SIZE_VMEM;
//		p -> current_videoaddr = (u32) (80 * 16 + 0) * 2;
		p -> current_videoaddr = 0;
		p -> cursor = 0;

	}

}

//package set_cursor and some commonds
void set_cursor_console(CONSOLE *p_console){

	p_console -> cursor = p_console -> current_videoaddr / 2 + p_console -> start_vmem;
									 //current_videoaddr / 2 => line
									 //whenever cursor points absolutely (offset B8000)

	set_cursor(p_console -> cursor);

}






void set_cursor(u32 cursor){
	// CRT_DATA_REG only 16bits

	disable_int();
	out_byte((u32)INDEX_CURSOR_H , (u32)CRT_ADDR_REG);
	out_byte(cursor >> 8 , (u32)CRT_DATA_REG);

	out_byte((u32)INDEX_CURSOR_L , (u32)CRT_ADDR_REG);
	out_byte(cursor , (u32)CRT_DATA_REG);
	enable_int();
}

void set_start_vmem(u32 start_vmem){

	disable_int();
	out_byte((u32)INDEX_ST_VMEM_H , (u32)CRT_ADDR_REG);
	out_byte(start_vmem >> 8 , (u32)CRT_DATA_REG);

	out_byte((u32)INDEX_ST_VMEM_L , (u32)CRT_ADDR_REG);
	out_byte(start_vmem , (u32)CRT_DATA_REG);
	enable_int();

}













