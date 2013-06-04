/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: fs.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef FS_H
#define FS_H


#include "hd.h"
#include "types.h"
/////////////////////////////文件系统//////////////////////////////////////

/*
 * 说明
 * 在下面数据结构中所有定义的扇区号除了base_sec_fs其他都是相对于文件系统的编号
 * 一个块大小等于一个扇区。块用来标示FAT32等表的每一个512字节
 *
 * #define定义的是常量可以迭代使用。比如#define a 10  #define b a + 2正确
 * const定义的是只读变量。不能迭代使用。比如const int a = 10; const int b = a + 2; 出错
 * 注意define定义的常量在参加运算时应加上括号(XX)
 */

#define MAX_SEC_FS 1024 * 256	//文件系统支持最多的扇区个数暂时定为1024 * 256就是最大硬盘寻址128M
#define MAX_SEC_FAT32 (MAX_SEC_FS * 4) / SEC_SIZE	//那么FAT32表最多占用2048个扇区(1M) 一共有2048块
#define ITEM_FAT32_PER_SEC  SEC_SIZE / 4	//每一个扇区所拥有的FAT32表项。因为每个表项4字节，所以应该为128项


#define MAX_INODE_FS 1024 * 4		//支持最多文件个数为4096个
#define MAX_SEC_INODE_TABLE (MAX_INODE_FS * INODE_SIZE) / SEC_SIZE	//inode_array 最多占用128个扇区(64K) 一共有128块
#define INODE_PER_SEC SEC_SIZE / (INODE_SIZE)	//每一个扇区所能拥有的INODE数量。这里应该为512/16 32个


#define MAX_DEPTH_PATH  8			//文件的路径的最大嵌套深度。包括目录加上文件名。/dir1/dir2/dir3/.../fle.出现一次‘/’表明嵌套一次
#define MAX_FILE_NAME	28			//文件名最长28字节
#define MAX_LEN_PATH		MAX_DEPTH_PATH * MAX_FILE_NAME	//路径名最大的长度




/*
 * FAT32
 * 文件分配表。每个文件占用那些扇区。数组［扇区号］ = 下一个所占扇区号。如果遇到数组［扇区号］ = 该扇区号则表明结束。
 * FAT[index] == 0表示对应的扇区还未被使用，可以利用。注意FAT32和INODE_TABLE所占的扇区全非0 不能被文件使用
 *
 * 因为文件分配表在内存中只有一个扇区的缓冲区。因此，在record_in_mem之中确定了0～MAX_SEC_FAT32-1哪一块被放进了内存
 *
 * 1.已知扇区号n_sec求对应FAT32[index]的index。
 *加载FAT32表第n_sec / ITEM_FAT32_PER_SEC块入内存。那么index = n_sec % ITEM_FAT32_PER_SEC
 *
 * 2.已知FAT32[index]的index求对应的扇区号n_sec。n_sec = record_in_mem.record_fat32 * ITEM_FAT32_PER_SEC + index
 *
 * 将会有两个函数来进行专门操作
 */




/*
 * INODE_TABLE
 * inode_table。存储inode的数组。通过下标索引可以得到文件对应的inode
 * 同理，因为在内存只有一个扇区的缓冲区。所以record_in_mem之中确定了0～MAX_SEC_INODE_TABLE哪一块放入了内存
 *
 * 1.inode号对应的下标。 已知n_inode 求 inode_table[index]的index
 * 加载inode_table的第n_inode / INODE_PER_SEC块入内存。那么index = n_inode % INDOE_PER_SEC
 *
 * 2.已知inode_table[index]的index,求对应的inode号n_inode
 * n_inode = record_in_mem.record_inode_table * INODE_PER_SEC + index
 *
 * 将会有两个函数来进行专门操作
 */



/*
 * INODE MODE
 */
#define I_NO_EXIST	0x0000		//不存在。
#define I_SPECIAL		0x0001		//特殊结点



/*
 * 下面定义了文件与目录的权限。注意如果权限是用户
 */




//文件权限宏定义
//所有者                                                  				对应的二进制位
#define I_F_RD_ONLY	0x1100	//用户只读文件  					0001
#define I_F_WT_ONLY	0x1200	//用户只写文件						0010
#define I_F_XT_ONLY	0x1400	//用户只执行文件					0100
#define I_F_RDWT (I_F_RD_ONLY | I_F_WT_ONLY)

//组用户
#define I_F_RD_ONLY_G	0x1010	//组只读文件					0001
#define I_F_WT_ONLY_G	0x1020	//组只写文件					0010
#define I_F_XT_ONLY_G	0x1040	//组只执行文件					0100
#define I_F_RDWT_G (I_F_RD_ONLY_G | I_F_WT_ONLY_G)

//#define I_F_RDWT_G		0x1030	//组读写文件					0011
//其他用户
#define I_F_RD_ONLY_O	0x1001	//其他用户只读文件			0001
#define I_F_WT_ONLY_O	0x1002	//其他用户只写文件			0010
#define I_F_XT_ONLY_O	0x1004	//其他用户只执行文件			0100
#define I_F_RDWT_O (I_F_RD_ONLY_O | I_F_WT_ONLY_O)

//#define I_F_RDWT_O		0x1003	//其他用户读写文件			0011

//目录权限宏定义
//所有者
#define I_D_RD_ONLY	0x2100	//用户只读目录
#define I_D_WT_ONLY	0x2200	//用户只写目录
#define I_D_RDWT		0x2300	//用户读写目录
//组用户
#define I_D_RD_ONLY_G	0x2010	//组只读目录
#define I_D_WT_ONLY_G	0x2020	//组只写目录
#define I_D_RDWT_G		0x2030	//组读写目录
//其他用户
#define I_D_RD_ONLY_O	0x2001
#define I_D_WT_ONLY_O	0x2002
#define I_D_RDWT_O	0x2003


//与上面定义对应 不能改变
#define I_FLAG_RO		0x0100		//用户只读标志
#define I_FLAG_WO		0x0200		//用户只写标志
#define I_FLAG_RW		(I_FLAG_RO | I_FLAG_WO)

#define I_FLAG_XO		0x0400		//用户只执行标志


#define I_FLAG_RO_G		0x0010		//组用户只读标志
#define I_FLAG_WO_G	0x0020		//组用户只写标志
#define I_FLAG_RW_G 	(I_FLAG_RO_G | I_FLAG_WO_G)

#define I_FLAG_XO_G	0X0040		//组用户只执行标志


#define I_FLAG_RO_O		0x0001		//其他用户只读标志
#define I_FLAG_WO_O	0x0002		//其他用户只写标志
#define I_FLAG_RW_O 	(I_FLAG_RO_O | I_FLAG_WO_O)

#define I_FLAG_XO_O  0X0004		//其他用户只执行标志


#define I_FLAG_S		0		//特殊文件标志
#define I_FLAG_F		1		//文件标志
#define I_FLAG_D		2		//目录标志


#define I_MASK_FD		0xF000	//文件目录掩码

#define I_MASK_USR	0x0F00	//用户的掩码
#define I_MASK_GRP	0x00F0	//组用户的掩码
#define I_MASK_OTH	0x000F	//其他用户的掩码
//删除选项
#define RM_F			0		//删除文件
#define RM_D			1		//删除目录
/*
 * 文件核心数据结构inode.对应一个文件
 */
typedef struct stc_inode{
	u32 i_mode;		//该结点的模式 RO?RW?R/W?X
	u32 n_sec_i; 	//结点所对应文件的起始扇区（相对于base_sec_fs）
	u32 usr_pid;	//创建结点的用户pid
	u32 i_size; 	//结点对应文件的大小(B)

}INODE;

#define INODE_SIZE	4 * 4	//(B)


/*
 * 目录的核心数据结构。一个目录项对应一个文件(inode)
 */
typedef struct stc_dir_entry{
	u32 n_inode;
	u8 f_name[MAX_FILE_NAME];
}DIR_ENTRY;

#define DIR_ENTRY_SIZE	4 + MAX_FILE_NAME	//最好长度为2的n次方+。这样所有项目都可以在同一个扇区.即不会出现某ENTRY跨扇区的情况出现(32B)
														//一个扇区最多目录项为512/32 = 16条

#define NR_ENTRY_PER_SEC (SEC_SIZE) / (DIR_ENTRY_SIZE)	//每一个扇区拥有的目录项个数
/*
 * SUPER_BLOCK超级块
 */
typedef struct stc_sup_block{

	u32 magic;				//魔数，标示一个文件系统
	u32 base_sec_fs;	//文件系统所在的绝对起始扇区号。因为其他扇区均是相对于该起始扇区号计算的
	u32 nr_sec_fs;				//文件系统占用的扇区个数

	u32 n_sec_sup_block;	//超级块在文件系统中的相对扇区

	u32 n_sec_fat32;		//文件分配表(扇区)的起始扇区.FAT32表示一个表项目为32B
	u32 nr_sec_fat32;		//文件分配表所占扇区个数

	u32 nr_inode;			//文件的个数(inode)
	u32 inode_size;		//inode的大小(B)
	u32 n_sec_inode_table;		//存储inode_array的起始扇区号
	u32 nr_sec_inode_table;		//存储inode_array的扇区个数

	u32 n_inode_root;		//第一个(根文件＆目录)的inode号

	u32 n_1st_dsec;		//第一个存储数据的扇区号

	u32 dir_entry_size;	//目录中一个ENTRY的大小

}SUP_BLOCK;


/*
 * 因为FAT32和INODE_TABLE不止一个扇区
 * 该数据结构主要记录放入内存的各个表的块的编号。注意从0开始。最大值为该表所占最大扇区数目
 * 表示第record + 1块
 * 也表示从表起始到该块起始之间距离record块
 */
typedef struct stc_record_in_mem{
//	u8  spin_fat32;			//因为可能有多个进程同时读/写。用于控制该临界区。凡是有进程读写时为1.其余不能修改。为0时可以使用
//	u8  spin_inode_table;

	u32 record_fat32;
	u32 record_inode_table;

}RECORD_IN_MEM;


/*
 * FD_PROC_LINE
 * 打开同一文件的进程队列
 * 这里的机制是:除了管道文件之外，其他进程互斥打开同一文件。当打开一个已经打开的文件时该进程挂起进入队列。
 * 有进程关闭文件将激活挂起的下一个进程
 */
#define FD_WAIT_PID	12	//一个文件最多能被12个进程打开

typedef struct stc_item_fd{	//fd之中一个项。包括等待进程号和该进程试图打开文件的标志

	int pid;	//进程号
	int flags;	//打开文件的标志

}ITEM_FD;


typedef struct stc_fd_proc_line{
	u8	head;	//队列头
	u8 tail;	//队列尾
	u16 counter;	//队列中的进程
	ITEM_FD item_fd_line[FD_WAIT_PID];		//最多容纳等待进程
}FD_PROC_LINE;

//sizeof(FD_PROC_LINE) == 24B

/*
 * FILE_DESCRIPTOR
 * 文件描述符。内存中关于文件的一个重要数据结构
 * 打开文件将返回FD号
 */
typedef struct stc_file_descriptor{
	u32 n_inode;	//文件n_inode号
//	mode_t mode;	//权限
	int	flags;	//打开标志
	u32 f_pos;	//写文件内容的位置
	u32 r_pos;	//读文件内容的位置
	FD_PROC_LINE fd_proc_line;
}FILE_DESCRIPTOR , FD;

//sizeof(FD) == 64B


/*
 * PASSWD_ITEM
 * 密码表项目。包括用户名及其密码
 * 长度为32字节
 */
typedef struct stc_passwd_item{
	u32 uid;
	u8  usr_name[12];
	u8 password[16];
}PASSWD_ITEM;

#define PASSWD_ITEM_LEN 32
#define NR_PASSWD_ITEM	 (SEC_SIZE)/(PASSWD_ITM_LEN);	//最多十六项

/*
 * FD_TABLE
 * 放入内存某线性地址之中
 */
#define MAX_NR_FD	10	//内存最多打开的文件数目



//用于读写较大文件的缓冲区
//typedef struct



//////////////////////////////全局变量//////////////////////////////////

//const 修饰的是指针地址不是地址指向的内容

extern SUP_BLOCK * const p_sup_block;	//superblock 被加载进内存的位置
extern u32 * const fat32;						//fat表被加载进内存的位置。可能不能被完全加载。一次加载一个扇区
extern INODE * const inode_table;	//inode_array被加载进内存的位置。也可能不被完全加载。一次加载一个扇区
extern u8 * const fs_buff;					//文件系统的缓冲区。用于查找搜索放置数据。不能用于文件数据缓冲。
extern u8 * const file_buff_in;				//文件输入缓冲区
extern u8 * const file_buff_out;				//文件输出缓冲区

extern FD * const fd_table;				//放置FD_TABLE的内存位置。预计占据sizeof(FD) * MAX_NR_FD
////////////////////////文件系统进程//////////////////////////////////
/*
 * 进程TASK_FS。是文件系统管理进程。
 */
void task_fs();





///////////////////文件系统内部专有函数//////////////////////////////
/*
 * void search_partion(void);
 */

static void test(void);


/*
 *static int mkfs(void);
 * 产生文件系统。
 * 成功返回0失败返回-1
 */

static int mkfs(void);


/*
 *static void set_sup_block(void)
 * 初始化SUPERBLOCK
 */
static void init_sup_block(void);

/*
 * static void init_fd_table(void)
 * 初始化fd_table。文件描述符列表
 */
static void init_fd_table(void);

/*
 *static u32 gain_empty_dsec();
 * 通过扫描FAT32获得一块空闲的数据扇区。
 *
 * 返回值：
 * 成功返回数据扇区号。失败返回0
 */
static u32 gain_empty_dsec();


/*
 *static int free_dsec(u32 n_dsec);
 * 清空一块磁盘扇区。即回收此扇区。慎用。因为磁盘扇区的数据信息会被清空
 *
 * @param:扇区号
 *
 * 返回值：
 * 成功放回0，失败返回-1
 *
 */
static int free_dsec(u32 n_dsec);


/*
 *static int get_index_fat32(u32 n_sec);
 * 获得扇区在FAT32表对应的下标号.
 * @param0:扇区号。注意，是相对于文件系统而言。而不是绝对扇区号
 * 成功返回对应的下标号0~ITEM_FAT32_PER_SEC。失败返回-1
 */
static int get_index_fat32(u32 n_sec);


/*
 *static u32 get_n_sec(u32 index_fat32);
 *获得FAT32下标所指的扇区。注意FAT[INDEX]表示与INDEX所指的扇区有逻辑关系的下一个扇区。区分INDEX所指的扇区与FAT[INDEX]的区别
 *@param0:FAT32的下标
 *返回该下标对应的扇区。
 */
static u32 get_n_sec(u32 index_fat32);


/*
 *static int syn_fat32();
 * 将放入内存的FAT32表某块写入对应的磁盘扇区(同步)
 * 成功返回0.失败返回-1
 */
static int syn_fat32();


/*
 *static u32 gain_empty_inode();
 * 获取一个空inode以供使用。并返回对应的n_inode
 *
 * 返回值：
 * 成功返回一个大于0的的n_inode号。
 * 失败返回0
 */
static u32 gain_empty_inode();


/*
 *static int del_inode(u32 n_inode);
 * 删除一个inode
 *
 * @param:欲删除的inode的n_inode号
 *
 * 返回值：
 * 成功返回0。失败返回-1
 */
static int del_inode(u32 n_inode);




/*
 *static int get_index_inode_table(u32 n_inode);
 * 获得inode在inode_table表对应的下标号.
 * @param0:inode序号。
 * 成功返回对应的下标号0~INODE_PER_SEC。失败返回-1
 */
static int get_index_inode_table(u32 n_inode);


/*
 *static u32 get_n_inode(u32 index_inode_table);
 *获得对应index_inode_table的n_inode。
 *@param0:inode_table的下标
 *返回该下标对应的n_inode。
 */
static u32 get_n_inode(u32 index_inode_table);

/*
 *static int syn_inode_table();
 * 将放入内存的INODE_TABLE表某块写入对应的磁盘扇区(同步)
 * 成功返回0.失败返回-1
 */
static int syn_inode_table();


/*
 *static int read_dsec(u32 dsec);
 *
 * 将数据扇区的内容读入内存的磁盘缓冲区file_buff_in中
 *
 * @param0:读取的扇区。注意是相对扇区不是绝对扇区
 *
 * 返回值：
 * 成功返回0
 * 失败返回-1
 *
 */

static int read_dsec(u32 dsec);

/*
 *static int write_dsec(u32 dsec);
 *
 * 将放入内存的数据扇区内容写回磁盘
 * 凡是写回的内容都放在fille_buff_out之中
 *
 *@param0:写入的扇区号。注意是相对扇区不是绝对扇区
 *
 * 返回值：
 * 成功返回0
 * 失败返回-1
 *
 */
static int write_dsec(u32 dsec);

////////////
/*
 *static void init_fs(void);
 * 初始化文件系统。
 * 设置相应内存
 * 加载特殊块进入内存。主要是加载超级块。FAT32数据扇区起始块。INODE_TABLE的起始块等
 */
static void init_fs(void);


/*
 *static void sep_path(char *path , char **seg_path , char delim);
 * 路径始终是以/dir/dir/.../file的形式出现的.
 * 解析文件路径。将出现的delim置0x00。并将分割的字符串首地址存入seg_path之中
 * 注意，第一个'/'代表根目录，而其他的'/'代表分隔符。所以第一个'/'将不会被设置为0。系统默认从根目录开始查找
 *
 * @param0:路径
 * @param1:放置路径子字符串首地址的结构
 * @param2:分隔符
 *
 * 返回值为子字符串的个数
 */
static int sep_path(char *path , char **seg_path , char delim);


/*
 *static u32 search_f_in_dir(const char *f_name , const u32 n_inode_dir);
 * 在指定的目录内寻找文件名为f_name的文件(目录)。
 *
 * @param0:查找的文件名。
 * @param1:目录的n_inode号。
 * *
 * 返回值：
 * 成功返回文件名对应的n_inode号
 * 失败返回0
 */
static u32 search_f_in_dir(const char *f_name , const u32 n_inode_dir);


/*
 *static int reg_f_in_dir(const u32 n_inode_f , const char *f_name , const u32 n_inode_dir);
 * 在指定的目录内创建一个指向文件的目录项。
 *
 * @param0:创建文件的n_inode号
 * @param1:创建文件的文件名
 * @param2:目录的n_inode号
 *
 * 返回值：
 * 创建成功返回0 失败返回-1
 */
static int reg_f_in_dir(const u32 n_inode_f , const char *f_name , const u32 n_inode_dir);



/*
 *static u32 del_f_in_dir(const char *f_name , const u32 n_inode_dir);
 * 在指定的目录内删除一个文件的目录项。
 *
 * @param0:欲文件的文件名
 * @param1:目录的n_inode号
 *
 * 返回值：
 * 删除成功返回文件的n_inode号。 失败返回0
 */
static u32 del_f_in_dir(const char *f_name , const u32 n_inode_dir);





/*
 *static u32 create_file(char *abs_path , u32 mode , int usr_pid)
 * 创建文件。
 *
 * @param0:绝对路径。已经包含了文件名。
 * @param1:模式。比如设定是只读文件或者目录等。I_F_* I_D_*
 * @param2:创建者的进程号
 *
 * 返回值：
 * 成功返回创建文件的n_inode，失败返回0
 */
static u32 create_file(char *abs_path , u32 mode , int usr_pid);

/*
 *static u32 search_file(char *abs_path)
 *根据文件名搜寻文件（目录）
 *
 *@param:文件（目录）绝对路径
 *
 *返回值：
 *成功返回目标文件（目录）的INODE号。失败返回0
 */
static u32 search_file(char *abs_path);


/*
 * static int delete_file(const char *f_name , u32 n_inode_parent , int usr_pid)
 * 删除文件。
 *
 * @param0:文件名。
 * @param1:文件父目录n_inode号
 * @param2:想要删除该文件的用户PID
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
static int delete_file(const char *f_name , u32 n_inode_parent , int usr_pid);


/*
 * static int delete_dir(const char *dir_name , u32 n_inode_parent , int usr_pid)
 * 删除一个目录。
 * 注意目录必须为空。
 *
 * @param0:目录名。
 * @param1:目录父目录n_inode号
 *	@param2:删除目录的用户PID
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
static int delete_dir(const char *dir_name , u32 n_inode_parent , int usr_pid);


/*
 * static int rm_file(char *abs_path , int usr_pid , int opt);
 * 删除文件。
 *
 * @param0:文件的绝对路径
 * @param1:删除文件的用户ID
 *	@param2:选项。0删除文件。1删除目录
 *
 * 返回值：
 * 成功返回0.
 * 失败返回-1
 */
static int rm_file(char *abs_path , int usr_pid , int opt);

/*
 * static int change_mod(char *abs_path , u32 mode , int usr_pid);
 * 更改文件(目录)权限
 *
 * @param0:文件（目录）的绝对路径
 * @param1:修改成的权限。I_FLAG_RO/I_FLAG_WO/I_FLAG_RW
 * @param2:试图修改权限的用户ID（非属主无法修改）
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
static int change_mod(char *abs_path , u32 mode , int usr_pid);



/*
 * static int gain_empty_fd();
 * 从fd_table获得一个空闲的fd号
 *
 * 返回值：
 * 成功返回fd号，失败返回-1
 */
static int gain_empty_fd();


/*
 * static int gain_fd_by_name(char *abs_path)
 * 根据文件的绝对路径获取文件在fd_table中的fd号。
 *如果文件存在但没有fd号将在fd_table中选择一个号返回。
 *
 * @param:文件绝对路径
 *
 * 返回值：
 * 成功返回文件在fd_table中的fd号。失败返回-1
 */
static int gain_fd_by_name(char *abs_path);

/*
 * static int do_open(char *abs_path , int flags , int pid);
 * 打开文件。
 *
 * @param0:文件路径。
 * @param1:打开标志
 * @param2:打开文件进程号
 *
 * 返回值：
 * 成功返回0，失败返回-1
 */
static int do_open(char *abs_path , int flags , int pid);



/*
 * int do_close(int fd , int pid);
 *
 * 关闭文件。
 * 如果关闭文件之后还有进程处于打开队列之中将会唤醒下一个进程
 * 如果没有进程了将撤销该fd
 * 同时，如果打开标志有写入标志同时要求更新相关文件的inode的f_pos标志
 *
 *@param0:通过open返回的文件描述符
 *@param1:试图关闭文件的进程pid
 *
 * 返回值：
 * 成功返回0失败返回－1
 */
static int do_close(int fd , int pid);



/*
 * static u32 get_n_dsec(u32 n_inode , u32 n_fsec);
 *
 * 根据文件内容的扇区号得到该扇区所在的数据扇区号
 *
 *@param0:文件的inode号
 *@param1:文件内容的扇区号。（文件所占用的扇区数从0开始编号）
 *
 * 返回值：
 * 成功返回数据扇区号，失败返回0
 */
static u32 get_n_dsec(u32 n_inode , u32 n_fsec);


/*
 * static ssize_t do_write(int fd , const char *buff , size_t count);
 *
 * 往打开的文件的末尾里面写入buff之中共count个字节
 * count如果为0则不起作用
 *
 *@param0:已经打开的文件描述符
 *@param1:调用该函数的进程所提供的缓冲区
 *@param2:试图写入的字节数
 *@param3:调用函数的进程PID
 *
 *返回值：
 *成功则返回实际写入的字节数。失败返回－1
 *
 */
static ssize_t do_write(int fd , const char *buff , size_t count);


/*
 * static ssize_t do_read(int fd , char *buff , size_t count);
 *
 * 从打开的文件里读取count个字节放入buff之中
 * 文件的读标志将跟随读取移动，直到文件末尾或者读取结束
 * 如果count为0则读取无用
 *
 * @param0:打开的文件描述符
 * @param1:字节缓冲区buff
 * @param2:欲读取的字节数
 *
 * 返回值：
 * 成功返回实际读入的字节数，失败返回－1
 */
static ssize_t do_read(int fd , char *buff , size_t count);



/*
 * static ssize_t do_get_fsize(int fd);
 * 返回打开的文件大小
 *
 * @param:文件描述符
 *
 * 返回值:
 * 成功返回大小。失败返回-1
 */
static ssize_t do_get_fsize();



/*
 * static int do_list_dir(char *dir_path , int opt);
 * 显示文件夹的内容。
 *
 * @param:文件夹的路径
 *
 * 返回值:
 * 成功返回0。失败返回－1
 */
static int do_list_dir(char *dir_path , int opt);


/*
 * static int is_file_dir(char *abs_path , int opt);
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
static int is_file_dir(char *abs_path , int opt);



#endif
