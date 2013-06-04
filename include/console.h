/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: console.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef CONSOLE_H
#define CONSOLE_H


#define NUM_CONSOLE 3

/*
 * SCROLL
 * line
 */
#define SCREEN_SCROLL	80 * 25		//25lines per screen
#define LINE_SCROLL	80 * 1	//one line per scroll

/*
 * SIZE
 */
#define SIZE_VMEM	(SCREEN_SCROLL * 2 * 3)	//每个TTY可以滚动三次满屏

/*
 * BASE_VMEM (offset to (BASE_VMEM)B8000)
 */
#define BASE_VMEM	0XB8000			//start of video memory

#define BASE1_VMEM	80 * 0 				//0
#define BASE2_VMEM 	BASE1_VMEM + SCREEN_SCROLL * 3 	//3 screen
#define BASE3_VMEM	BASE2_VMEM + SCREEN_SCROLL * 3 	//6 screen


typedef struct stc_console{

	unsigned int base_vmem;				//距离显存B8000的相对起始位置，一般设为0K，10K ， 20K（因为只有三个TTY，每个占10K显存）.TTY的起始显存。初始化之后不变
	unsigned int start_vmem; 			//开始与base_vmem相同，但是滚屏之后会改变。是某段显存窗口的起始位置.
	unsigned int size_vmem;				//该TTY使用的显存大小
	unsigned int current_videoaddr; 	//current_videoaddr是相对于start_vmem而言的，而start_vmem是相对于显存B8000而言的.
	/*
	 * 所以调用函数时，注意current_videoaddr = ( dispstr(current_videoaddr + start_vmem) - start_vmem );
	 * 之所以这样调用，是为了与dispstr函数保持一致，那始终是相对B8000（gs）来进行显示。
	*/

	unsigned int cursor;					//cursor是current_videoaddr的1/2
}CONSOLE;

/*
 * function declare
 */
//have relationship with console
void init_console();
void set_cursor_console(CONSOLE *p_console);


//no relationship with console
void set_cursor(u32 cursor);
void set_start_vmem(u32 start_vmem);

#endif
