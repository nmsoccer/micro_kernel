/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: hd.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef HD_H
#define HD_H


/*
 ********************************HD.H**********************
 ************************************硬盘驱动头文件**************************
 */
#include "const.h"




/* Hard Drive */
#define SEC_SIZE	512	//一个扇区的大小。也是REG_DATA的大小
#define SEC_BITS	(SECTOR_SIZE * 8)
#define SEC_SIZE_SHIFT	9

/*
 * 分区信息表与分区表项的信息
 * 注意，是相对于该扇区首字节的偏移
 */
#define PARTION_TABLE_START 0x1BE	//分区信息表在扇区里的起始位置
#define PARTION_ITEM_SIZE 16	//每个分区信息结构体的长度
#define PARTION_STATUS 0	//状态。如果是0x80可以引导。0x00不可引导。其他不合法
#define PARTION_SYSTEM_ID 4	//分区的类型。
#define PARTION_BASE_SEC 8	//相对该分区所在扇区的起始扇区
#define PARTION_NUM_SEC 12	//分区占有扇区的个数

//分区类型
#define PARTION_EMPTY	0
#define PARTION_EXTEND	0x05
/*
 * 分区的信息
 */
#define MAX_PRIME_PARTION	4	//最多的主要分区个数
#define LOGIC_PER_PRIME		16	//每个主要分区所拥有的最多逻辑分区个数
#define MAX_LOGIC_PARTION	MAX_PRIME_PARTION * LOGIC_PER_PRIME	//硬盘上最多逻辑分区个数


/*
 * 相关寄存器端口。这里只使用IDE0的Primary
 */

//COMMAND BLOCK REG
#define REG_DATA			0x1F0	//	I/O	读写的都是数据

#define REG_ERROR 		0x1F1	// I		读时为error
#define REG_FEATURE		0x1F1	//	O		写时为特征量

#define REG_SEC_COUNTER	0x1F2	//	I/O
#define REG_LBA_LOW		0x1F3	// I/O
#define REG_LBA_MID		0x1F4	// I/O
#define REG_LBA_HIG		0x1F5	// I/O

#define REG_DEVICE		0x1F6	// I/O
	/*
	 * REG_DEVICE 各位的含义
	 * 	7		6		5		4		3		2		1		0
	 * 1     L     1    DRV   HS3   HS2   HS1   HS0
	 * L -- LBA mode.选择操作的模式，为1则表示采用LBA模式
	 * DRV -- Drive. =0 drive0(master)selected, =1 drive1(slave)selected
	 * 最后四位。如果L=0,select the head number
	 *				  L=1,LBA的最高四位。24～27
	 */
	#define SET_REG_DEVICE(LBA , DRV , HIGHEST_LBA)( 0xA0 | (LBA << 6) | (DRV << 4) | (HIGHEST_LBA & 0xF))//设定REG_DEVICE的宏


#define REG_STATUS		0x1F7	// I		读时为状态
	/*
	 * REG_STATUS 各位的含义
	 *  7		6		5		4		3		2		1		0
	 *BSY  DRDY  DF/SE   #    DRQ	   -     -    ERR
	 *BSY -- BUSY。=1 处于忙的状态。 =0 可以使用
	 *DRDY -- DRIVE READY
	 *DF/SE -- DEVICE FAULT/STREAM ERROR
	 * # -- Command Dependent
	 * - Obsolete
	 * DRQ - Data Request(Ready to transfer data)
	 *ERR -- Error
	 */
	//几种状态
	#define	STATUS_BSY	0x80
	#define	STATUS_DRDY	0x40
	#define	STATUS_DFSE	0x20
	#define	STATUS_DSC	0x10
	#define	STATUS_DRQ	0x88
	#define	STATUS_CORR	0x04
	#define	STATUS_IDX	0x02
	#define	STATUS_ERR	0x01

#define REG_COMMAND		0X1F7	// O		写时为命令
	/*
	 * 几种命令
	 */
	#define ATA_IDENTIFY		0xEC
	#define ATA_READ		0x20
	#define ATA_WRITE		0x30

//CONTROL BLOCK REG
#define REG_ALT_STATUS	0x3F6	// I		读时为改变状态
#define REG_DEV_CONTROL	0x3F6	//	O		写时为装置控制

	/*
	 * REG_DEV_CONTROL 各位的含义
	 * 	7		6		5		4		3		2		1		0
	 * HOB   -     -     -    -     SRST  IEN    0
	 * HOB -- High Order Byte
	 * SRST -- Software Rest
	 * IEN -- Interrupt Enable
	 */





/*
 * 相关结构体。
 */
typedef struct stc_cmd{	//发送给硬盘的命令。一共写从REG_FEATURE到REG_COMMAND七个寄存器。当写入REG_COMMAND成功后，硬盘产生中断.每个成员8bits

	unsigned char feature;	//放入REG_FEATURE
	unsigned char sec_counter;	//放入REG_SEC_COUNTER
	unsigned char lba_low;	//放入REG_LBA_LOW
	unsigned char lba_mid;	//放入REG_LBA_MID
	unsigned char lba_hig;	//放入REG_LBA_HIG
	unsigned char device;	//放入REG_DEVICE
	unsigned char command;	//放入REG_COMMAND

}HD_CMD;


/*
 * 任一分区的信息
 */
typedef struct stc_partion_info{
	unsigned char bootable;				//是否可以引导
	unsigned int  base_sec;	//该分区的起始扇区
	unsigned int  num_sec;	//该分区所占扇区个数

}PARTION_INFO;

/*
 * 硬盘总的分区信息
 */
typedef struct stc_hd_info{

	PARTION_INFO	prime_partion[MAX_PRIME_PARTION];
	PARTION_INFO   logic_partion[MAX_LOGIC_PARTION];

	//因为有四个主要分区。下面是分别指向四个主要分区所拥有的逻辑分区。每个的范围从0～15
	unsigned char index_sub[MAX_PRIME_PARTION];

}HD_INFO;


////////////////////////////////////全局变量///////////////////////////////////
/*
 * HD.C之中的全局变量
 */

extern HD_INFO hd_info;
//extern u8 hd_buff[SEC_SIZE];






//////////////////////////////////////////////////////////////////////////////
////////////////////////////硬盘系统进程////////////////////////////////////
/*
 * 进程 TASK_HD。是硬盘驱动程序。用于处理关于硬盘的消息和初始化一些硬盘信息
 */
void task_hd();




/////////封装硬盘消息的函数&硬盘驱动接口/////////////////////////////////



/*
 * int hd_open();
 * 打开硬盘驱动。获取硬盘信息
 * 成功返回0否则返回-1
 */
int hd_open();


/*
 * int do_read_hd(u8 *hd_buff , int driver , u32 abs_sec);
 * 用于向磁盘驱动程序hd.c发送消息。
 * 功能是读取磁盘driver某绝对扇区abs_sec的内容到文件缓冲区hd_buff中。
 * 一次读取一个扇区。
 * 该函数和read_hd_sec几乎一样。但这是为其他进程提供的接口。因为磁盘中断总是向磁盘驱动程序发送信号包。
 * 如果其他进程直接使用read_hd_sec的话将陷入永久循环。
 *同理write也是一样的。所以对磁盘的操纵只能由hd进程完成
 *
 *@param0:磁盘缓冲区
 *@param1:磁盘主设备号
 *@param2:磁盘绝对扇区号
 *成功返回0 失败返回-1
 */
int do_read_hd(void *hd_buff , int driver , u32 abs_sec);



/*
 * int do_write_hd(u8 *hd_buff , int driver , u32 abs_sec);
 * 用于向磁盘驱动程序hd.c发送消息。
 * 功能是写hd_buff到磁盘driver的某绝对扇区abs_sec中。
 * 一次写一个扇区。
 *
 *@param0:磁盘缓冲区
 *@param1:磁盘主设备号
 *@param2:磁盘绝对扇区号
 *成功返回0 失败返回-1
 */
int do_write_hd(void *hd_buff , int driver , u32 abs_sec);





////////////////////////////内部函数//////////////////////////////////////
/*
 * FUNCTIONS DECLARE
 */

/*
 * static void init_hd_info(HD_INFO *p_hd_info);
 * 用于初始化硬盘分区信息表
 */
static void init_hd_info(HD_INFO *p_hd_info);

/*
 * void get_hd_info(int driver);
 */

/*
 * static int get_hd_partion(int driver , unsigned int abs_sec , int prime_partion);
 * 获得硬盘的分区信息。
 * prime_partion表示该分区所属的主分区。主要针对逻辑分区使用
 */
static int get_hd_partion(unsigned char *hd_buff , int driver , unsigned int abs_sec , int prime_partion);



/*
 *static int read_hd_sector(void *hd_buff , int driver , u32 abs_sec);
 * 读硬盘一个扇区的信息。driver表示选中的硬盘。sec表示扇区号(绝对扇区号)
 * 成功返回0否则返回-1
 */
static int read_hd_sector(void *hd_buff , int driver , unsigned int abs_sec);

/*
 *static int write_hd_sector(void *hd_buff , int driver , u32 abs_sec);
 *@param0 缓冲区
 *@param1 主设备号
 *@param2 绝对扇区号
 *成功返回0失败返回-1
 */
static int write_hd_sector(void *hd_buff , int driver , u32 abs_sec);


/*
 * int hd_identify(int driver);
 * 获取硬盘的信息。硬盘号由driver给出。
 */
static int hd_identify(int driver);


/*
 * static void send_hd_cmd(HD_CMD *cmd);
 * 通过传入命令指针，将相应的寄存器设置为HD_CMD的成员。
 * 当设置command之后硬盘发生中断
 */
static int send_hd_cmd(HD_CMD *cmd);


/*
 * static void print_identify_info(unsigned short* hdinfo);
 * 该函数用于显示硬盘的某些信息
 */
static void print_identify_info(unsigned short* hdinfo);

/*
 * static void print_hd_info(HD_INFO *p_hd_info);
 * 用于打印硬盘分区信息
 */
static void print_hd_info(HD_INFO *p_hd_info);



#endif























