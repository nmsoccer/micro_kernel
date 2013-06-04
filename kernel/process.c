
#include "global.h"
#include "klibc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "hd.h"
#include "sys/fs.h"
#include "process.h"
#include "unistd.h"

#include "sys/stat.h"
#include "fcntl.h"
#include "command.h"

//extern void _init_usr_pcb(PCB *p_pcb , u32 sel_ldt ,  int * function , STACK_PROC *p_stack_proc , int pid , char *p_name);
extern void _init_usr_pcb(PCB *p_pcb , u32 sel_ldt , int *function , STACK_PROC *p_stack_proc , int pid , int parent_pid
		, int p_tty);
extern void _init_sys_pcb(PCB *p_pcb , u32 sel_ldt , int *function , STACK_PROC *p_stack_proc , int pid , int parent_pid
		, int p_tty);

extern void task_tty();


//-------------------SYS PROCESS----------------------------------
//所有进程的父进程，由内核初始化完成调用开始，用于初始化系统进程，以及为应用进程创建子进程
void task_init(){
//	init_msg_table();	//初始化消息表
	MSG *p_msg;

	create_sys_process((int *)task_tty , 0 , 1 , " ");
	create_sys_process((int *)task_hd , 0 , 1 , " ");
	create_sys_process((int *)task_fs , 0 , 1 , " ");
	create_usr_process((int *)usr_man , 0 , 1 , "");
//	create_usr_process((int *)process1 , 0 , 1 , " ");
//	create_usr_process((int *)process2 , 0 , 1 , " ");
//	create_usr_process((int *)process3 , 0 , 1 , " ");

//	suspend_proc(10);


	while(1){
		p_msg = recv_msg();

		if(p_msg != (MSG *)NULL){
			switch(p_msg -> type){
				case MSG_CREATE_PROC:
					del_msg(p_msg);
					break;
				case MSG_DELETE_PROC:
					del_msg(p_msg);
					break;
				case MSG_WAIT:	//当出现WAIT时，先将目标进程挂起。延迟一段时间，再激活。而目标进程是一个循环查找。
					suspend_proc(p_msg -> send_pid);
					delay(1000);	//本意是在此处循环查找。但是如果没有消息出现会使进程0陷入死循环，因此选择延迟
					awake_proc(p_msg -> send_pid);
					del_msg(p_msg);
					break;
				case MSG_SLEEP:
					suspend_proc(p_msg -> send_pid);
					delay(p_msg -> int_info[0] * 100);
					awake_proc(p_msg -> send_pid);
					del_msg(p_msg);
					break;
				default:
					del_msg(p_msg);
					break;
			}
		}

	}

}

//-------------------USR PROCESS--------------------------------------
void usr_man(){
	MSG *p_msg;	//消息包
	PASSWD_ITEM *p_item;	//密码项目指针

	char buff[50];	//缓冲区为100字节
	char path[50];	//当前的目录路径。用于程序的相对路径寻址
	char backup[50];	//后备缓冲存储备份
	char usr_name[12];	//和passwd_item保持一致
	char *s;	//字符(串)指针

	int fd;	//文件描述符
	int fd1;
	int i;

	u8 opt;	//是否存在选项的标志
	u8 param;	//是否存在参数的标志
//	pause();
//	list("/" , 0);
//	printf("AFTER");
//	chmod("/etc/passwd" , S_IRGRP | S_IROTH);


//	i = check_file_dir("/home/leiming" , 0);
//	printf("I:%d" , i);


	//检验是否有用户存在,如果没有则创建名－密码项目，同时在/home中创建用户名开头的文件夹
	fd = open("/etc/passwd" , O_RDWR | O_APPEND , 0);

	if(fd == -1){
		printf("Can not open passwd");
		pause();
	}


	read(fd , buff , PASSWD_ITEM_LEN);	//读入第一个表项目。这里应该是超级用户项
	p_item = (PASSWD_ITEM *)buff;

	if(p_item->uid == 0){	//表示超级用户密码没有创建。默认创建超级用户名和密码。以后可以修改.这个部分只进行一次
		//创建超级用户及密码
		p_item->uid = 1;
		strncpy(p_item->usr_name , "root" , 0);
		strncpy(p_item->password , "123" , 0);
		write(fd , buff , PASSWD_ITEM_LEN);

		//创建一个普通用户名及密码
		memset(buff , 0x00 , 50 , 0);
		memset(path , 0x00 , 50 , 0);
		//输入用户名
		printf("We will create a new user! Your name: ");
		p_msg = wait_msg(MSG_ENTER_KEY);
		p_msg->str_info[p_msg->int_info[0]] = 0x00;

		//创建/home/xx文件夹
		i = strlen("/home/");
		strncpy(buff , "/home/" , 0);
		strncpy(&buff[i] , p_msg->str_info , 0);
		mkdir(buff , S_IWRITE | S_IREAD);
		delay(1000);
		chmod(buff , S_IRWGRP | S_IRWOTH);

		//创建/home/xx/.profile
		strncpy(path , buff , 0);
		i = strlen(path);
		strcat(path , "/.profile");
		creat(path , S_IWRITE | S_IREAD);
		delay(1000);
		chmod(path , S_IRWGRP | S_IRWOTH);
		//写入/home/xx/.profile
		i = strncpy(buff , "PATH=" , 0);
		buff[i] = (char)0x00;
		fd1 = open(path , O_RDWR | O_APPEND , 0);
		write(fd1 , buff , i);
		close(fd1);

		memset(buff , 0x00 , 50 , 0);
		p_item->uid = 2;
		strncpy(p_item->usr_name , p_msg->str_info , p_msg->int_info[0]);
		del_msg(p_msg);
		//输入密码
		printf("Your password: ");
		p_msg = wait_msg(MSG_ENTER_KEY);
		strncpy(p_item->password , p_msg->str_info , p_msg->int_info[0]);
		del_msg(p_msg);

		write(fd , buff , PASSWD_ITEM_LEN);
		close(fd);
		//buff已经设置好了直接到下步

	}else{	//已经有了root和一个普通用户
		//读取下一项
		read(fd , buff , PASSWD_ITEM_LEN);
		close(fd);
	}

	//验证用户名
	while(1){
		printf("user name:");
		p_msg = wait_msg(MSG_ENTER_KEY);

		p_msg->str_info[p_msg->int_info[0]] = 0x00;
		if(strcmp(p_item->usr_name , p_msg->str_info) != 0){
			printf("%s is not existed!/n" , p_msg->str_info);
			del_msg(p_msg);
			continue;
		}
		del_msg(p_msg);
		break;
	}	//end while

	//验证密码
	while(1){
		printf("password:");
		p_msg = wait_msg(MSG_ENTER_KEY);
		p_msg->str_info[p_msg->int_info[0]] = 0x00;

		if(strcmp(p_item->password , p_msg->str_info) != 0){
			printf("password is not correct! /n" , p_msg->str_info);
			del_msg(p_msg);
			continue;
		}

		del_msg(p_msg);
		break;
	}	//end while

	//目前的激活的用户uid
	USR_PID = p_item->uid;
	//设置当前用户所在目录
	memset(path , (char)0x00 , 50 , 0);
	strncpy(path , "/home/" , 0);
	strcat(path , p_item->usr_name);

	i = strncpy(usr_name , p_item->usr_name , 0);
	usr_name[i] = (char)0x00;

	//主体处理程序
	//若是MSG_ENTER_KEY类型的消息将返回循环首，若是其他类型的消息则返回inner_circle不用显示指示符
	while(1){
		printf("[%s@%s]>" , usr_name , path);

inner_circle:
		p_msg = recv_msg();

		switch(p_msg->type){
			//主要处理的消息。用于与用户交互
			case MSG_ENTER_KEY:
//				memset(buff , (char)0x00 , 50 , 0);
				i = strncpy(buff , p_msg->str_info , p_msg->int_info[0]);
				buff[i] = (char)0x00;
				del_msg(p_msg);

				//分析命令行
				//形式为<命令> [-opt...] [param...]

				//找出命令
				s = strchr(buff , ' ');	//第一个空格之前为命令
				if(s == (char *)NULL){
					//表示只有命令而没有参数
					param = 0;
				}else{
					//不一定存在参数
					*s = (char)0x00;
					s++;

					while(1){	//主要用于查找是否有参数存在

						if(*s == (char)0x00){	//到了末尾还没有非空格字符。表示没有参数
							param = 0;
							break;
						}
						if(*s != ' '){		//出现了非空格而且非0x00那么一定为参数
							param = 1;
							break;
						}
						s++;
					}	//end while

				}	//end if-else

				//比较是否为ls命令
				i = strcmp("ls" , buff);
				if(i == 0){
					if(param == 0){	//没有参数 表示显示当前目录。比如ls
						list(path , 0);
					}else{
						//带有参数。此时s指向param首字符
						if(*s == '/'){	//表示显示绝对路径的目录形如ls /mnt/hgfs
							list(s , 0);
						}else{	//表示显示的是相对路径形如ls xx
								memset(backup , (char)0x00 , 50 , 0);
								//形成绝对路径。存放入backup
								i = strncpy(backup , path , 0);
								backup[i] = (char)0x00;
								strcat(backup , "/");
								strcat(backup , s);

								list(backup , 0);
						}

					}	//if-else

					break;
				}


				//比较是否为cd命令
				i = strcmp("cd" , buff);
				if(i == 0){
					if(param == 0){	//没有带参数.比如cd。那么返回用户主目录

						memset(path , (char)0x00 , 50 , 0);
						strncpy(path , "/home/" , 0);
						strcat(path , usr_name);

					}else{	//带有参数
						switch(*s){
							case '.':
								//读取下一个字符检测是cd . 还是cd ..如果是前者不作处理
								s++;
								if(*s == '.'){	//是cd ..
									if(strncpy(path , "/" , 0) != 0){	//如果当前已经是根目录了 不做处理。否则回到上层目录
										s = strrchr(path , '/');	//找到最后一个/

										if(path == s){	//只有一层嵌套如/home
											s++;
											*s = (char)0x00;
										}else{	//多层嵌套如/home/leiming
											*s = (char)0x00;
										}

									}	//end if

								}	//end if

								break;

							case '/':
								//参数为绝对路径.比如cd /home/xx
								i = check_file_dir(s , 1);	//检查目标是否是目录或者是否存在

								if(i == -1){	//目标不是目录或者不存在
									printf("error!");
									break;
								}

								//修改当前路径
								i = strncpy(path , s , 0);
								path[i] = (char)0x00;
								break;
							default:
								//参数为相对路径.比如cd dir
								memset(backup , (char)0x00 , 50 , 0);
								strncpy(backup , path , 0);

								if(strcmp(backup , "/") != 0){	//不是根目录处理。如果已经是根目录不需要处理
									strcat(backup , "/");
								}
								strcat(backup , s);

								i = check_file_dir(backup , 1);

								if(i == -1){	//目标不是目录或者不存在
									printf("error!");
									break;
								}

								//修改当前路径
								i = strncpy(path , backup , 0);
								path[i] = (char)0x00;
								break;

						}	//end switch

					}	//end if-else
					break;
				}	//end if


				//比较是否为mkdir命令
				i = strcmp("mkdir" , buff);
				if(i == 0){
					if(param == 0){	//必须带有目录名
						printf("You must input the name of directory!/n");
					}else{	//创建用户可以读写的目录.目录名在s之中
						memset(backup , (char)0x00 , 50 , 0);
						strncpy(backup , path , 0);
						strcat(backup , "/");
						strcat(backup , s);
						i = mkdir(backup , S_IWRITE | S_IREAD);
						if(i == -1){
							printf("Create Directory Failed!/n");
						}
					}

					break;
				}	//end if

				//比较是否为rmdir命令
				i = strcmp("rmdir" , buff);
				if(i == 0){
					if(param == 0){	//必须带有目录名
						printf("You must input the name of directory!/n");
					}else{	//创建用户可以读写的目录.目录名在s之中
						memset(backup , (char)0x00 , 50 , 0);
						strncpy(backup , path , 0);
						strcat(backup , "/");
						strcat(backup , s);
						i = mkdir(backup , S_IWRITE | S_IREAD);
						i = rmdir(backup);
						if(i == -1){
							printf("Delete Directory Failed!/n");
						}
					}

					break;
				}	//end if

				break;
			default:
				goto inner_circle;
				del_msg(p_msg);
				break;
		}

	}	//end while





}


//-------------------TEST USR PROCESS-----------------------------
void process1(){
	int ticks = 0;
	char * content = "A.";
	char *output = "Nice Girl!";

//	char buff[538] ;
//	memset(buff , 0x00 , 528 , 0);
//	char buff[10];
	int fd;
	int i;
//	spin_me();
//	pause();
//	i = remove("/leiming");
//	i = chmod("/leiming" , S_IRWUSR | S_IROTH );
//	printf("result is:%d" , i);
//	creat("/leiming" , S_IREAD | S_IWRITE);
//	delay(200);

//	printf("A!");
/*
	fd = open("/leiming" , O_RDONLY  , 0);
	if(fd != -1){
//		i = write(fd , buff , 528);
//		i = write(fd , output , strlen(output));
		read(fd , buff , 538);
		buff[538] = (char)0x00;
		printf("%s" , &buff[528]);

//		read(fd , buff , 10);
//		buff[10] = (char)0x00;
//		printf("%s" , buff);

		close(fd);
//		printf("A->%d" , i);
	}
*/
//	printf("A");
//	printf("%s" , buff);

	while(1){
//		ticks = get_ticks();
//		disphexdw(ticks , ((80 * 3 + 0) * 2));

		delay(1);
		cvideoaddr = dispstr(content , cvideoaddr);

	}
}

void process2(){
	char *content = {"B."};
	char *output = {"Show Me The Money!"};
	char test[20];
	int i;
	int fd;
//	i = pause();

//	i = creat("/super" , S_IREAD);
//	i = chmod("/home" , S_IWUSR | S_IRGRP | S_IWOTH | S_IROTH);
//	i = mkdir("/home" , S_IREAD | S_IWRITE);
//	i = remove("/super");
//	i = rmdir("/home");
//	printf("result is:%d" , i);
/*
	printf("B!");
	fd = open("/leiming" , O_RDONLY , 0);
	if(fd != -1){
//		i = write(fd , output , strlen(output));
		i = read(fd , test , 10);
		close(fd);
		test[10] = (char)0x00;
		printf("%s" , test);
	}
*/
//	printf("B");

//	delay(2000);

	while(1){
		delay(100);
		delay(1);
//		sleep(10);
		cvideoaddr = dispstr(content , cvideoaddr);
//		printf("B.");
//		kill(10 , SIG_AWAKE);
	}

}

void process3(){
	char * content = {"C."};
	int i;


//	i = remove("/super");
//	i = rmdir("/home");
//	i = chmod("/super" , S_IWGRP);
//	printf("result is %d" , i);

	while(1){
//		kill(11 , SIG_READY);
//		recv_msg();
		delay(1000);
//		awake_proc(11);
		cvideoaddr = dispstr(content , cvideoaddr);
//		printf("C.");
	}

}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////KEY FUNCTIONS/////////////////////////////////


/*
 * void create_usr_process()
 */
void create_usr_process(int * function , int parent_pid , int p_tty , char *p_name){

	int pid;
	u8  sel_ldt;
	PCB *p_pcb;
	STACK_PROC *p_stack_proc;

	/*
	 * params pushed into functions
	 */

//	pid = proc_table.index_end;
	pid = proc_table.index_usr;
	p_pcb = &pcb_table.pcb_table[pid];
	sel_ldt = sel_ldt_table.sel_ldt[pid];

	p_stack_proc = &stack_proc_table.stack_table[pid + 1];


	_init_usr_pcb(p_pcb , (u32) sel_ldt , function , p_stack_proc , pid , parent_pid , p_tty);

	/*
	 * Register
	 */

	proc_table.p_pcb[pid] = p_pcb;
//	proc_table.index_end++;
	proc_table.index_usr++;
	proc_table.sum++;

}

/*
 * void create_sys_processs(int * function , int parent_pid , char *p_name)
 */

void create_sys_process(int * function , int parent_pid , int p_tty , char *p_name){

	int pid;
	u8  sel_ldt;
	PCB *p_pcb;
	STACK_PROC *p_stack_proc;

	/*
	 * params pushed into functions
	 */

//	pid = proc_table.index_end;
	pid = proc_table.index_sys;
	p_pcb = &pcb_table.pcb_table[pid];
	sel_ldt = sel_ldt_table.sel_ldt[pid];

	p_stack_proc = &stack_proc_table.stack_table[pid + 1];


	_init_sys_pcb(p_pcb , (u32) sel_ldt , function , p_stack_proc , pid , parent_pid , p_tty);

	/*
	 * Register
	 */

	proc_table.p_pcb[pid] = p_pcb;
//	proc_table.index_end++;
	proc_table.index_sys++;
	proc_table.sum++;


}



/////////////////////////////////////////////////////////////////////////////////
//////////////////////TOOL FUNCTIONS////////////////////////

/*
 * void init_proc_x_table();
 */

void init_proc_table(){	//proc_table , proc_r1_table , proc_r3_table
	int i;

	PROC_TABLE * p1;

	/*
	 * initial proc_table
	 *
	 */
	p1 = &proc_table;
	p1 -> index_ready = 0;
//	p1 -> index_end = 0;
	p1 -> index_sys = 0;						// 0 ~ MAX_SYS_PROCESS-1
	p1 -> index_usr = MAX_SYS_PROCESS;  // MAX_SYS_PROCESS ~ MAX_SYS_PROCESS + MAX_USR_PROCESS - 1
	p1 -> sum = 0;
	for(i = 0; i < MAX_PROCESS; i++){
		p1 -> p_pcb[i] = 0;
	}


}
/*
 * void init_sel_ldt_table();
 */
void init_sel_ldt_table(){
	u8 i = 0;
	u16 selector = 0x28;	//in GDT ,first selector of ldt is 0x28
	SEL_LDT_TABLE * p;

	p = &sel_ldt_table;
	for(i = 0; i < MAX_PROCESS; i++)
	{
		p -> sel_ldt[i] = selector;
		selector += DESCRIPTOR_LEN;		  //each length of descriptor
	}
	p -> index_end = 0;
}

/*
 * void init_stack_proc_table()
 */
void init_stack_proc_table(){
	u8 i = 0;
	u8 j = 0;

	STACK_PROC_TABLE *p = &stack_proc_table;

	for(i = 0; i < MAX_PROCESS ; i++){
		for(j = 0; j < STACK_PROC_SIZE; j++){
			p -> stack_table[i].stack[j] = 0;	//p -> stack_table[i] -> STACK_PROC
		}

	}

}

/*
 * void init_pcb_table()
 */
void init_pcb_table(){

	pcb_table.index_end = 0;
}


/*
 * int suspend_proc(int pid);
 * 暂停目标进程。只能由0进程可以
 *
 * 成功返回0。失败返回-1
 */

int suspend_proc(int pid){
	MSG *p_msg;
	p_msg = get_msg();
	p_msg -> type = MSG_SUSPEND_PROC;
	p_msg -> recv_pid = pid;

	return send_msg(p_msg);

}



/*
 * int awake_proc(int pid);
 * 激活已经暂停的目标进程。任何进程都可以
 *
 * 返回值：
 * 成功返回0。失败返回-1
 */
int awake_proc(int pid){
	MSG *p_msg;
	p_msg = get_msg();
	p_msg -> type = MSG_AWAKE_PROC;
	p_msg -> recv_pid = pid;

	return send_msg(p_msg);
}





































