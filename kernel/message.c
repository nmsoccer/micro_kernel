#include "global.h"
#include "klibc.h"
#include "message.h"
/*
 * void init_msg_table()
 */

void init_msg_table(){
	MSG_TABLE *p = &msg_table;
	int i;
	int j;
	int k;


	p -> spin_lock = 0;	//获取锁上锁时其他进程不能获取消息，但可以删除消息。而删除消息的进程不需要获得该锁


	p -> index_ready = 0;

	for(i = 0; i < MAX_MSG; i++){
		p -> msg_table[i].type = 0;	//用于鉴定该消息是否被使用，如果等于1表示没有.大于0则表示已经使用

		p -> msg_table[i].send_pid = 0;
		p -> msg_table[i].recv_pid = 0;

		p -> msg_table[i].signal = 0;
//		p -> msg_table[i].int_info[0] = 0;

/*
 * 		信息内容需要时再赋值，初始化不用
		for(j = 0; j < MAX_INT_INFO; j++){
			p -> msg_table[i].int_info[j] = 0;
		}
		for(k = 0; j < MAX_STR_INFO; k++){
			p -> msg_table[i].str_info[k] = 0;
		}
*/
	}

}

/*
 *msg* get_msg();
 *获得一个消息(指针)
 */

MSG* get_msg(){
	int i;
	MSG *p_msg;
	//会不断的查询，直到获得一个消息包为止
	while(1){
		if(msg_table.spin_lock == 1){	//已经上锁
			continue;
		}
		if(msg_table.spin_lock == 0){	//没有上锁
			msg_table.spin_lock = 1;	//上锁

			while(1){
				for(i = 0; i < MAX_MSG; i++){
					if(msg_table.msg_table[i].type == 0){
						p_msg = &msg_table.msg_table[i];
						break;
					}
				}

				break;
			}

			msg_table.spin_lock = 0;	//解锁
		}

		break;
	}

	return p_msg;

}


/*
 * void del_msg(MSG *p_msg)
 * 消除一个消息。即使得该消息可以被其他的进程使用
 */

void del_msg(MSG *p_msg){

	p_msg -> type = 0;

	p_msg -> send_pid = 0;
	p_msg -> recv_pid = 0;
}


/*
 * void init_mail_center()
 * 初始化通信中心
 */
void init_mail_center(){
	int i;
	int j;
	MAIL_CENTER *p;

	p = &mail_center;

	for(i = 0; i < MAX_PROCESS; i++){
		p -> mail_box_row[i].head = 0;
		p -> mail_box_row[i].tail = 0;
		p -> mail_box_row[i].counter = 0;
		p -> mail_box_row[i].spin_lock = 0;	//初始化为0 表示无写者

		for(j = 0; j < MSG_PER_PROC; j++){
			p -> mail_box_row[i].msg_row[j] = 0;
		}
	}

}


/*
 * SYSTEM CALL
 */
int sys_send_msg(MSG *p_msg , int send_pid , int send_pid_parent , int p_tty , int p_flag){
//	char *str = {"Victory!"};
//	MAIL_BOX *p_mail_box;

	if(p_msg -> type < MSG_SPECIAL_TYPE){	//如果不是特殊消息包，将消息包放入目标进程的邮箱。否则函数自己处理

		//获得目的进程的邮箱指针
//		p_mail_box = &(mail_center.mail_box_row[p_msg -> recv_pid]);

		//封装填充消息的部分内容
		p_msg -> send_pid = send_pid;
		p_msg -> int_proc_info[0] = send_pid_parent;//依照在PCB中的顺序填入
		p_msg -> int_proc_info[1] = p_tty;
		p_msg -> int_proc_info[2] = p_flag;

		return post_msg(p_msg);

	}else{	//处理特殊消息包

		switch(p_msg -> type){
			case MSG_SUSPEND_PROC:

				if(send_pid == 0){	//必须0进程才能暂停某进程
					proc_table.p_pcb[p_msg -> recv_pid] -> p_flag = P_FLAG_SUS;	//根据接收进程的pid设置其p_flag为挂起状态
				}

				del_msg(p_msg);

				break;
			case MSG_AWAKE_PROC:

				proc_table.p_pcb[p_msg -> recv_pid] -> p_flag = P_FLAG_RUN;	//根据接收进程的pid设置其p_flag为激活状态

				del_msg(p_msg);
				break;
			case MSG_RELOAD:	//重新装填消息到自身收件队列中
				post_msg((MSG *)p_msg -> int_info[0]);	//因为p_msg -> int_info[0]存储的是目标消息指针

				del_msg(p_msg);

				break;
			case MSG_PAUSE:
				proc_table.p_pcb[send_pid] -> p_flag = P_FLAG_SUS;	//将发送者的进程挂起。因为这是其自己提出的

				del_msg(p_msg);
				break;
			default:
				del_msg(p_msg);
				break;
		}
		return 0;
	}

}


MSG *sys_recv_msg(int virtual_param , int pid){	//因为中断发生时会依次传入ebx,pid,parent_pid,p_tty,p_flag等参数。我们只使用pid。所以第一个
	MSG *p_msg;
	MAIL_BOX *p_mail_box;															//当作空参数

	p_mail_box = &(mail_center.mail_box_row[pid]);


	if(p_mail_box -> counter == 0){
		return (MSG *)NULL;
	}else{
		p_msg = p_mail_box -> msg_row[p_mail_box -> tail];
		p_mail_box -> tail = (p_mail_box -> tail + 1) % MSG_PER_PROC;
		p_mail_box -> counter--;


		return p_msg;
	}


}

//////////////////////////////////////////////////////////////////////
/*
 * SOME FUNCTONS CONCERING MESSAGE
 */

/*
 * 重新装填信息包到本进程的收件箱中
 */
int reload_msg(MSG *msg){
	MSG *p_msg;

	p_msg = get_msg();

	p_msg -> type = MSG_RELOAD;
	p_msg -> int_info[0] = (int)msg;

	return send_msg(p_msg);
}

/*
 * int posg_msg(MSG *msg);
 *投递消息到目标进程的收件队列
 *成功返回0 失败返回-1
 */
int post_msg(MSG *p_msg){
	MAIL_BOX *p_mail_box;

	//获得目的进程的邮箱指针
	p_mail_box = &(mail_center.mail_box_row[p_msg -> recv_pid]);
	//根据接收进程号将包放入接收进程的消息队列里
	if(p_mail_box -> counter == MSG_PER_PROC){	//如果邮箱已经满，返回。不再尝试投放
		return -1;
	}else{	//如果邮箱未满，需要锁已经打开

		while(1){
			if(p_mail_box -> spin_lock == 0){	//没有上锁 可以投递
				p_mail_box -> spin_lock = 1;		//上锁 准备投放消息


				p_mail_box -> msg_row[p_mail_box -> head] = p_msg;
				p_mail_box -> head = (p_mail_box -> head + 1) % MSG_PER_PROC;
				p_mail_box -> counter++;


				p_mail_box -> spin_lock = 0;		//投放完毕 解锁
				break;
			}

			//如果已经上锁，则需要等待。不如等待一段时间再投放

		}	//end while

		return 0;

	}	//end if-else

}	//end function


/*
 * MSG *wait_msg(int type);
 * 使自身进程处于无穷迭代状态
 * 直到收到目标类型的信息包
 *
 * 成功返回消息包指针。失败返回NULL
 */
MSG *wait_msg(int type){
	MSG *p_msg;

	while(1){
		p_msg = recv_msg();

		if(p_msg -> type == type){	//如果正是目标类型的消息则返回消息指针
			return p_msg;
		}

		if(p_msg != (MSG *)NULL){
			reload_msg(p_msg);	//如果出现了自己不需要的消息，将会把该消息重新放入自己的收件箱
			continue;
		}

//		return (MSG *)NULL;
	}	//end while

}	//end f



