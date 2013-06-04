/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: global.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */
#ifndef GLOBAL_H
#define GLOBAL_H


#include "asm_function.h"
#include "tty.h" //include "console.h
#include "process.h"
#include "message.h"


#ifdef DEF_HERE

/*
 * because array[0] always leads to some errors , we start from array[1];
 */

/*
 * TTY
 */
TTY tty_table[NUM_TTY + 1];
CONSOLE console_table[NUM_CONSOLE + 1];


/*
 * PCB_TABLE
 */
PCB_TABLE pcb_table;
/*
 * PROCESS_TABLE
 */
PROC_TABLE 	  proc_table;
//PROC_R1_TABLE proc_r1_table;
//PROC_R3_TABLE proc_r3_table;

/*
 * SEL_LDT_TABLE
 */
SEL_LDT_TABLE	sel_ldt_table;

/*
 * STACK_PROC_TABLE
 */
STACK_PROC_TABLE stack_proc_table;


/*
 * MSG_TABLE
 */
MSG_TABLE msg_table;

/*
 * MAIL_CENTER
 */
MAIL_CENTER mail_center;

/*
 * cvideoaddr
 */
int cvideoaddr = (80 * 15 + 0) * 2;

/*
 * USR_PID	user personal identification
 * 用户ID
 */
u32	USR_PID = 1;	//初始为超级用户PID ＝ 1

#else

extern  int cvideoaddr;
extern TTY tty_table[NUM_TTY + 1];
extern CONSOLE console_table[NUM_CONSOLE + 1];

extern PROC_TABLE proc_table;
//extern PROC_R1_TABLE proc_r1_table;
//extern PROC_R3_TABLE proc_r3_table;
extern PCB_TABLE		pcb_table;
extern SEL_LDT_TABLE sel_ldt_table;
extern STACK_PROC_TABLE	stack_proc_table;
extern MSG_TABLE	msg_table;
extern MAIL_CENTER mail_center;
extern u32 USR_PID;
#endif

#endif
