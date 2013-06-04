/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: message.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

/*
 * MACRO DEFINE
 */

#define MAX_MSG	20				// TOTAL MSG


#define MSG_PER_PROC	10	// MSG PER PROCESS

#define MAX_INT_INFO	3
#define MAX_STR_INFO	64

/*
 * 	消息包的类型。一共有10个系统进程，每个进程有256个消息可以用除了进程0(只有255个)
 *
 */

//进程0 消息从0x001 ~ 0x0FF 一共255个。主要处理进程的产生和终止. TASK_INIT
#define MSG_CREATE_PROC	0x001
#define MSG_DELETE_PROC	0x002
#define MSG_WAIT		0x003			//用于等待信号量
#define MSG_SLEEP		0x004			//用于阻塞自身进程 直到指定时间到
//#define MSG_PAUSE	0x005			//挂起自身进程.如果有进程向其发送信号会被激活

//进程1 消息从0x100 ~ 0x1FF 一共256个。主要处理显示与键盘。 TASK_TTY
#define MSG_WRITE_CHAR	0x100
#define MSG_WRITE_INT	0x101
#define MSG_WRITE_STR	0x102

#define MSG_PRINT_CTRL	0x103

#define MSG_ENTER_KEY	0x104
//进程2 消息从0x200 ~ 0x2FF 一共256个。主要处理硬盘驱动。TASK_HD
#define MSG_HD_OPEN		0x200		//打开硬盘驱动。获取硬盘信息
#define MSG_READ_HD		0x201		//读硬盘
#define MSG_WRITE_HD		0x202		//写硬盘

//进程3 消息从0x300 ~ 0x3FF 一共266个。主要处理文件管理。TASK_FS
#define MSG_CREATE	0x300		//创建文件
#define MSG_REMOVE	0x301		//删除文件

#define MSG_MKDIR		0x302		//创建目录
#define MSG_RMDIR		0x303		//删除目录

#define MSG_CHMOD		0x304		//修改文件（目录）权限

#define MSG_OPEN		0x305		//打开文件
#define MSG_CLOSE		0x306		//关闭文件

#define MSG_READ		0x307		//读取文件
#define MSG_WRITE	0x308		//写入文件

#define MSG_LIST		0x309		//列出目录内容
#define MSG_CHECK	0x30A		//检测目标是文件还是目录
/*
 * 每个进程都使用的消息。即共用消息类型。比如SIGNAL类消息。从0xA00开始。
 * 0xA0~0xAF
 */
#define MSG_SIGNAL			0xA00

/*
 * 特殊的消息。这样的消息发送对象不是服务进程，而是系统调用程序。用来修改进程本身的一些信息
 * 从0xB00开始编号
 */
#define MSG_SPECIAL_TYPE	0xB00
#define MSG_SUSPEND_PROC	MSG_SPECIAL_TYPE	//挂起(暂停)一个进程。消息的发送者必须是目标进程的父进程，或者系统0进程
#define MSG_AWAKE_PROC	0xB01					//激活一个已经挂起的进程。只能有父进程或者0进程可以激活
#define MSG_RELOAD			0xB02					//将消息重新放入自己的收件队列中。比如在等待目标消息时收到非目标消息，暂时不做处理，放入自己的队列中
#define MSG_PAUSE			0xB03

/*
 * SOME DATA STRUCTURE
 */

typedef struct stc_msg{
	unsigned int type;

	unsigned int send_pid;	//发送消息包的进程
	unsigned int recv_pid;	//接受消息包的进程

	int signal;					//signal 主要用于信号量传递。是基于消息之上的整形IPC机制

	int int_info[MAX_INT_INFO];	//消息携带的整形信息
	int int_proc_info[INT_PROC_INFO];				 //携带的进程的相关信息
	char str_info[MAX_STR_INFO];//消息携带的字符信息

}MSG;


typedef struct stc_msg_table{

	unsigned int spin_lock;	//获取锁.

	unsigned int index_ready;	//指向空闲的消息

	MSG	msg_table[25];		//存有消息的个数

}MSG_TABLE;


typedef struct stc_mail_box{	//每个进程的邮箱
	int head;
	int tail;
	int counter;
	int spin_lock;					//一个读者 多个写者。该锁用于多写者互斥写邮箱。当为0时可写，当为1时不可写
	MSG *msg_row[MSG_PER_PROC];

}MAIL_BOX;

typedef struct stc_mail_center{	//汇集所有进程邮箱的投递中心

	MAIL_BOX mail_box_row[MAX_PROCESS];

}MAIL_CENTER;



/*
 * DELCARE FUNCTIONS IN msg.c
 */

void init_msg_table();

MSG* get_msg();

void del_msg(MSG *p_msg);

//////////////////
void init_mail_center();

/*
 * SYSTEM CALL
 */
int sys_send_msg(MSG *p_msg , int send_pid , int send_pid_parent , int p_tty , int p_flag);
MSG *sys_recv_msg(int virtual_param , int pid);


/*
 *
 * int send_msg(MSG *p_msg); success return 0 or return 1
 * MSG *p_msg recv_msg();	  success return MSG* or return null
 */


int send_msg(MSG *p_msg); //success return 0 or return 1
MSG *recv_msg();	 // success return MSG* or return null

////////////////////////////////////////////////////////////////////////////////////////////////
//////////////SOME COMMON FUNCTIONS CONCERING MESSAGE///////////////////



/*
 * int reload_msg(MSG *msg);
 * 重新装填信息包到本进程的收件箱中
 */
int reload_msg(MSG *msg);

/*
 * int posg_msg(MSG *msg);
 *投递消息到目标进程的收件队列
 *成功返回0 失败返回-1
 */
int post_msg(MSG *p_msg);




/*
 * MSG *wait_msg(int type);
 * 使自身进程处于无穷迭代状态
 * 直到收到目标类型的信息包
 *
 * 成功返回消息包指针。失败返回NULL
 */
extern MSG *wait_msg(int type);



#endif






