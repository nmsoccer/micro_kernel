/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: tty.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef TTY_H
#define TTY_H


#include "console.h"



/*
 * NUMBER OF TTYS
 */
#define NUM_TTY 3

#define TTY1	1
#define TTY2	2
#define TTY3 	3


/*
 * SIZE OF BUFF
 */
#define TTY_IN_BYTES 128

typedef struct stc_tty{
	int head;
	int tail;
	int counter;							//pointer to buff

	int index;								//pointer to input_buff;

	char buff[TTY_IN_BYTES];			//store scan code
	char input_buff[TTY_IN_BYTES];	//存储来自键盘扫描码所对应的，可以显示的字符，由键盘处理程序操作，不能由其他程序使用

	CONSOLE *p_console;

}TTY;


/*
 * function declare
 */
void init_tty();


void set_buff_tty_normal(TTY *p_tty);	//不会持续扫描键盘缓冲区
void set_buff_tty(TTY *p_tty);	//会持续扫描键盘缓冲区

char read_buff_tty(TTY *p_tty);
char set_input_tty(u32 c , TTY *p_tty);


void tty_print_ctrl(TTY *p_tty);

void tty_write_char(u32 char_code , TTY *p_tty);
void tty_write_str(char *str , TTY *p_tty);
void tty_write_hexb(u32 byte , TTY *p_tty);
void tty_write_hexw(u32 word , TTY *p_tty);
void tty_write_hexd(u32 dword , TTY *p_tty);
void tty_write_int(int data , TTY *p_tty);

#endif
