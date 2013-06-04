/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: asm_function.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef ASM_FUNCTION_H
#define ASM_FUNCTION_H


#include "const.h"
//这里用于大部分经常利用到的ASM函数的声明
//注意C函数与汇编函数的区别。
//1.变量类型：汇编函数都是无符号，所以一般使用unsigned char , short , int
//2.返回值：因为返回值都在eax之中，最终解析成什么编码由在C中声明的变量类型决定，所以C之中若声明成int，则会出现带符号的情况.
//因为汇编函数的返回值非常灵活，这里只是提供一些常见形式，如果没有预定义I_DEFINE，则使用，如果有I_DEFINE可以在本地文件声明不同的形式

#ifndef I_DEFINE

extern void delay(u32 time);
extern u32 dispstr(u8 * str  , u32 location);
extern u32 disphexdw(u32 data , u32 location);
extern u32 disphexb(u32 data , u32 location);

extern void disable_int();
extern void enable_int();

//system calls
extern u32 get_ticks();


extern u8 in_byte(u32 port);
extern void out_byte(u32 data , u32 port);

extern void port_read(char *buff , int number , u32 port); //从port端口读入number个字到buff之中
extern void port_write(char *buff , int number , u32 port); //将buff之中number个字放入port端口中

extern void memcpy(void *dest , const void *src , int number);	//从src所指内存复制number个字节到dest所指内存


/*
 * 设置dest所指内存到dest+number为数据data.
 * opt=0时选择data的最低字节。	dest ~ dest + number (B) 		memset(char *dest , (int)char , int number , 0);
 * opt=1时选择data的低字	  	dest ~ dest + number * 2 (B)	memset(char *dest , (int)short , int number , 1);
 * opt=2时选择data双字			dest ~ dest + number * 4 (B)	memset(char *dest , int , int number , 2);
 */
extern void memset(void * dest , int data , int number , int opt);

#endif
#endif
