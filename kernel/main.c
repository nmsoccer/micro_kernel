//#include "global.h"
#include "const.h"
#include "process.h"
#include "klibc.h"


//extern int dispstr(char *str , int loacation);
extern void init_pcb();
extern void init_sys_call_table();
extern void set8253();
extern void start();

extern void init_keyboard();

extern void task_tty();
extern void process1();
extern void process2();
extern void process3();

void kernel_main();


void kernel_main(){
	char * title = {"-----kernel_main-----"};
	char * space = {" "};

//	cvideoaddr = dispstr(title , cvideoaddr);

	init_proc_table();
	init_sel_ldt_table();
//	init_stack_proc_table();
	init_pcb_table();

//	init_pcb();
	init_sys_call_table();
	set8253();


	init_keyboard();

	init_msg_table();	//初始化消息表

	init_mail_center();


	//创建init进程，由这个进程来创建其他的进程
	create_sys_process((int *)task_init , 0 , 1 , " ");


	start();
}
