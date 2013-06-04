/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: process.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */

#ifndef PROCESS_H
#define PROCESS_H


#include "protect.h"

/*
 * ---------------------Process.h---------------------
 * 对应的汇编头文件为process.inc，分别为C与汇编程序所使用的预定义常量，一些常量是保持一致的
 */

/*
 * Number of processes
 */

#define MAX_SYS_PROCESS	10	//该OS设置最多系统进程为10个
#define MAX_USR_PROCESS	10 //用户应用进程暂时定为10个

#define MAX_PROCESS	MAX_SYS_PROCESS + MAX_USR_PROCESS

/*
 * LDT_SIZE of process
 */
#define LDT_SIZE	2

/*
 * STACK_PROC_SIZE
 */
#define STACK_PROC_SIZE	512 * 6	//3KB per stack of process

/*
 * INT_PROC_INFO
 */
#define INT_PROC_INFO	3//进程内的整形信息的个数。从PARENT_PID 到 P_NAME之前结束

/*
 * PROC_FLAG
 * 进程的状态，有运行挂起死亡三种
 */
#define P_FLAG_RUN	0
#define P_FLAG_SUS	1
#define P_FLAG_DEAD	2


/*
 * 系统进程的ID
 */
#define TASK_INIT	0
#define TASK_TTY	1
#define TASK_HD	2
#define TASK_FS	3


/*
 * 用户进程的PID
 */
#define USR_MAN	10
//#define USR_PERSON2	11
//#define USR_PERSON3	12

/*
 * PCB
 */
typedef struct stc_stack_frame{
	u32 gs;
	u32 fs;
	u32 es;
	u32 ds;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 kernel_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 retaddr;
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;

}STACK_FRAME;

typedef struct stc_pcb{
	STACK_FRAME stack_frame;

	u32 sel_ldt;
	DESCRIPTOR ldt[LDT_SIZE];

	u32 slices;
	u32 priority;

	u32 pid;
	u32 parent_pid;
	u32 p_tty;	//进程所在tty
	u32 p_flag;	//进程的状态. RUN, SUS, DEAD.
	u8  p_name[16];
}PCB;


//PCB_TABLE

typedef struct stc_pcb_table{			//r1,r3进程的PCB都放在这里，而对应的指针放在相应的proc_r1_table与proc_r3_table之中
	PCB pcb_table[MAX_PROCESS];	//按照惯例，数组元素0不使用
	int index_end;
}PCB_TABLE;



/*
 * proc_table
 * 存储的都是PCB指针，其中proc_table存储所有进程pcb指针，proc_r1_table存储所有运行在ring1进程pcb指针；proc_r3_table存储所有运行在ring3用户级别的
 * 进程pcb指针
 */

typedef struct stc_proc_table{
	int index_ready;
	PCB* p_pcb[MAX_PROCESS];
//	int index_end;
	int index_sys;	//在系统进程中移动
	int index_usr; //在用户进程中移动
	int sum;

}PROC_TABLE;



/*
 * SELECTOR_LDT_TABLE
 * 用在进程产生之中
 */
typedef struct stc_sel_ldt_table{	//因为GDT_SIZE=128，所以一个字节足以表示
	u8	sel_ldt[MAX_PROCESS];
	u8	index_end;

}SEL_LDT_TABLE;

/*
 * STACK_PROC_TABLE
 * R1与R3进程需要的堆栈
 */
typedef struct stc_stack_proc{

	u8 stack[STACK_PROC_SIZE];

}STACK_PROC;

typedef struct stc_stack_proc_table{

	STACK_PROC	stack_table[MAX_PROCESS];

}STACK_PROC_TABLE;

//////////////////////////////////////////////////////////////////////////
///////////////////FUNCTIONS DECLARE///////////////////

//////////////////KEY FUNCTIONS////////////////////////
void create_sys_process(int * function , int parent_pid , int p_tty , char *p_name);
void create_usr_process(int * function , int parent_pid , int p_tty , char *p_name);

/////////////////TOOL FUNCTIONS////////////////////////

/*
 * init functons
 */
void init_proc_table();

void init_sel_ldt_table();

void init_stack_proc_table();

void init_pcb_table();




/*
 * int suspend_proc(int pid);
 * 暂停目标进程。只能由0进程可以
 * 成功返回0。失败返回-1
 */

int suspend_proc(int pid);

/*
 * int awake_proc(int pid);
 * 激活已经暂停的目标进程。任何进程都可以
 *
 * 返回值：
 * 成功返回0。失败返回-1
 */
int awake_proc(int pid);


///////////////////////////PROCESS//////////////////////
/*
 * task_init
 * 用于创建其他的进程，是所有进程的父进程
 */
void task_init();



void usr_man();
//------------------
	/*
	 * 用于测试的应用进程
	 */
void process1();
void process2();
void process3();


#endif
