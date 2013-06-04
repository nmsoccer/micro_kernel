/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: stdlib.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */


#ifndef STDLIB_H
#define STDLIB_H

/*
 * -------------------STDLIB.H--------------------
 * -------主要记录了stdlib.c文件中函数的声明-----------------
 *
 */

/*
 * FUNCTION DECLARE
 */

//int itoa(int i , char *buff);
//将整数i转化为字符串放入buff开始的字符串中
//返回转化成字符的个数
int itoa(int data , char *buff);

#endif
