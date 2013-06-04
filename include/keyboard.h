/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: keyboard.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H


#define NUM_FORMS 3 	//Each base scan code may refer to three different forms

#define CL_SHIFT	0
#define ST_SHIFT	1
#define ST_AHEAD	2

#define FLAG_EXIT	0x80
#define FLAG_AHEAD	0xE0


#define NUM_SCAN_CODES 0x80	//There are 0x80 base scan codes



#define KB_IN_BYTES 32	//Buff contents 32 bytes
//队列，头进尾出
typedef struct stc_kb{

	int head;
	int tail;
	int counter;
	char buff[KB_IN_BYTES];


}KB_INPUT;



/*
 * function declare
 */
void key_control(TTY *p_tty);
char get_keycode();

void shift_ahead(TTY *p_tty);
void backspace(TTY *p_tty);
void enter(TTY *p_tty);
void alt(TTY *p_tty);
void num_lock(TTY *p_tty);
void caps_lock(TTY *p_tty);
void pad_direct(u32 scan_code , TTY *p_tty);







/*
 * Base scan code of Special key
*/
#define ESC 		0x01
#define BACKSPACE	0x0E
#define TAB			0x0F

#define ENTER		0X1C
#define PAD_ENTER	0x1C	//They share same base scan code,PAD_ENTER has 0xEO ahead

#define CTRL_L		0x1D
#define CTRL_R		0x1D	//(0xE0)

#define SHIFT_L	0x2A
#define PAD_SLASH 0x35
#define SHIFT_R	0x36

#define ALT_L		0x38
#define ALT_R		0x38

#define CAPS_LOCK	0x3A

#define F1			0x3B
#define F2			0x3C
#define F3			0x3D
#define F4			0x3E
#define F5			0x3F
#define F6			0x40
#define F7			0x41
#define F8			0x42
#define F9			0x43
#define F10			0x44

#define NUM_LOCK	0x45

#define SCROLL_LOCK	0x46

#define PAD_HOME	0x47
#define HOME		0x47

#define PAD_UP		0x48
#define UP			0x48

#define PAD_PAGEUP	0x49
#define PAGEUP		0x49

#define PAD_MINUS	0x4A

#define PAD_LEFT	0x4B
#define LEFT		0x4B

#define PAD_MID  	0x4C

#define PAD_RIGHT	0x4D
#define RIGHT		0x4D

#define PAD_PLUS	0x4E

#define PAD_END	0x4F
#define END			0x4F

#define PAD_DOWN	0x50
#define DOWN		0x50

#define PAD_PAGEDOWN	0x51
#define PAGEDOWN	0x51

#define PAD_INS	0x52
#define INSERT		0x52

#define PAD_DOT	0x53
#define DELETE		0x53

#define F11			0x57
#define F12			0x58

#define GUI_L		0x5B
#define GUI_R		0x5C

#define APPS		0x5D


#define TTY1_CHOSEN	0x02
#define TTY2_CHOSEN	0x03
#define TTY3_CHOSEN	0x04



#endif



































