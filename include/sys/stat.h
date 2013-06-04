/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: stat.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 * Modiy:2010.6.
 */

#ifndef _STAT_H
#define _STAT_H


#include "types.h"
//#include "sys/fs.h"
//////////常数声明//////////
/*
 * 文件权限
 */
//它们与fs.h之中的定义保持一致
//用于creat/remove mkdir/rmdir (用于目录是需要做一定的更改。文件不需要改变)
#define S_IREAD		0x1100
#define S_IWRITE	0x1200

//用于chmod
#define S_IRUSR		S_IREAD	//用户只读
#define S_IWUSR	S_IWRITE	//用户只写
#define S_IRWUSR	(S_IRUSR | S_IWUSR)	//用户读写

#define S_IRGRP		0x1010	//组只读
#define S_IWGRP	0x1020	//组只写
#define S_IRWGRP (S_IRGRP | S_IWGRP)	//组读写

#define S_IROTH		0x1001 	//其他只读
#define S_IWOTH	0x1002	//其他只写
#define S_IRWOTH (S_IROTH | S_IWOTH)	//其他读写
//////////函数声明//////////


/*
 *int mkdir(char *path , mode_t mode);
 *
 * @param0:创建的目录路径。
 * @param1:模式。S_IWRITE只写，S_IREAD只读，S_IWRITE | S_IREAD读写。
 *
 *注意创建目录的时候读写权限都是只针对所有者。
 *组和其他用户都是默认无读写权限，只有所有者更改权限才可以获得
 *
 *返回值：
 *成功返回0。失败返回-1
 */
extern int mkdir(char *path , mode_t mode);



/*
 *int rmdir(char *path);
 * 删除目录。
 *
 *@param:目录路径
 *
 *注意删除目录的时候如果其他用户获得写权限可以删除
 *
 *
 *返回值：
 *成功返回0.失败返回-1
 */
extern int rmdir(char *path);

/*
 * int chmod(char *path , mode_t mode);
 * 改变文件（目录）的权限。
 * 只能文件（目录）的创建者可以改变
 *
 * @param0:文件（目录）路径
 * @param1:修改后的权限
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
extern int chmod(char *path , mode_t mode);









#endif
