/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: command.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.8.
 */

#ifndef COMMAND_H
#define COMMAND_H


/*
 * int list(char *dir_path , int opt);
 * 列举目录内容
 *
 * @param0:显示的目录
 * @param1:选择项
 *
 * 返回值：
 * 成功返回0。失败返回-1
 */
int list(char *dir_path , int opt);


/*
 * static int check_file_dir(char *abs_path , int opt);
 *
 * 检测目标是否是文件或者目录
 *
 * @param0:文件(目录)的路径
 * @param1:选项。0为测试文件；1为测试目录
 *
 * 返回值:
 * 成功返回0.失败返回-1
 *
 */
int check_file_dir(char *abs_path , int opt);





#endif
