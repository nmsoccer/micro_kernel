/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: string.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef STRING_H
#define STRING_H

/*
 * -----------------------STRING.H------------------
 * ---------------------STRING.C 的头文件----------------
 */


/*
 * int strncpy(char *dest , char *src , int number);
 * 成功返回拷贝的字符个数 ， 错误返回1
 * number = 0 表示复制src直到遇见0x00(不会拷贝0x00)
 * number > 0 表示复制src一共number个字符到dest之中
 *
 * 注意dest要有足够的空间。如果number大于src的长度可能会出现错误
 *
 */
int strncpy(char *dest , const char *src , int number);


/*
 * int strlen(char *str);
 * 返回以str开头的字符串的个数，以0x00结束。
 * 但是不包括0x00
 */
int strlen(char *str);


/*
 * int strcmp(const char *str1 , const char *str2);
 *
 * @param0:参与比较的字符串1
 * @param1:参与比较的字符串2
 *
 * 返回值：
 * -1:第一个字符串小于第二个字符串
 * =0:第一个字符串等于第二个字符串
 * =1:第一个字符串大于第二个字符串
 */
int strcmp(const char *str1 , const char *str2);


/*
 * char *strcat(char *dest , char *src);
 * 连结两个字符串，将第二个字符串连在第一个字符串末尾。注意将要剔出第一个字符串的末尾0x00(如果有)
 * 第一个字符串要有足够的空间存储复制的字符串
 *
 * @param0:
 * @param1:
 *
 * 返回值：
 * 成功返回第一个字符串的指针，失败返回NULL
 */
char *strcat(char *dest , char *src);

/*
 * char *strchr(char *s , int c);
 * 查找字符串s中第一个出现目标字符c的位置，返回其地址
 *
 * @param0:字符串
 * @param1:目标字符
 *
 * 返回值:
 * 成功返回地址。失败返回NULL
 *
 */
char *strchr(char *s , int c);



/*
 * char *strrchr(char *s , int c);
 *
  * 查找字符串s中最后一个出现目标字符c的位置，返回其地址
 *
 * @param0:字符串
 * @param1:目标字符
 *
 * 返回值:
 * 成功返回地址。失败返回NULL
 *
 */
char *strrchr(char *s , int c);






#endif
