/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: klibc.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef KLIBC_H
#define KLIBC_H

/*
 * ----------------LIBC.H----------------------
 * ----------------内核需要的一些C库函数---------------------
 */

/*
 * 以十进制打印整数
 */
int disp_int(int data , int location);


/*
 * 内核输出函数
 */

//只打印字符
//失败返回-1 成功返回输出的字符
int cprintk(char c);


//只打印整数
//失败返回-1 成功返回输出的整数
int iprintk(int i);


//专门打印字符串
//失败返回-1 成功返回输出的字符数目
int sprintk(char *str);

//回车
//失败返回-1 成功返回0
int print_ctrl();


#endif
