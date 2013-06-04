/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: fcntl.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 *
 * 文件控制函数。包括文件的创建删除修改等
 */

#ifndef _FCNTL_H
#define _FCNTL_H

#include "sys/types.h"


///////////宏定义/////////////
#define O_RDONLY	0x0001
#define O_WRONLY	0x0002
#define O_RDWR	0x0003
#define O_RDWR_MASK	0x000F

#define O_CREAT	0x0010
#define O_CREAT_MASK	0x00F0

#define O_APPEND	0x0100
#define O_TRUNC	0x0200
#define O_EXCL	0x0300
#define O_POS_MASK	0x0F00

#define O_BINARY	0x1000
#define O_TEXT	0x2000
#define O_TYPE_MASK	0xF000


//////////函数声明//////////

/*
 *extern int creat(char *path , mode amode);
 * 创建文件。
 *
 * @param0:创建的文件路径。
 * @param1:模式。S_IWIRTE只写，S_IREAD只读，S_IWRITE | S_IREAD读写。
 *
 *注意创建文件的时候读写权限都是只针对所有者。
 *组和其他用户都是默认无读写权限，只有所有者更改权限才可以获得
 *
 *
 *返回值：
 *成功返回0。失败返回-1
 */
extern int creat(char *path , mode_t amode);


/*
 * extern int remove(char *path);
 * 删除文件。
 * 不能删除只读文件。无法删除其他用户的文件
 *
 * @param:文件的路径。
 *
 *注意删除文件的时候如果其他用户获得写权限可以删除
 *
 * 返回值:
 * 成功返回0.失败返回-1
 */
extern int remove(char *path);


/*
 * extern int open(char *abs_path , int flags , mode_t mode);
 * 打开文件。
 *
 * @param0:文件路径。
 * @param1:打开标志
 * @param2:打开的权限
 *
 * 返回值：
 * 成功返回打开的fd号，失败返回-1
 *
 */
extern int open(char *abs_path , int flags , mode_t mode);





#endif
