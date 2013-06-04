#include "const.h"
#include "process.h"
#include "message.h"
#include "signal.h"

/*
 * void wait(int signal);
 * 等待目标的信号量出现
 * 等待过程中进程会被进程0暂停。直到进程的收件箱出现携带目标信号量的消息包。目标消息包随后被销毁
 *
 * @param:信号
 */
void wait(int signal){
	MSG *p_msg;

	p_msg = get_msg();
	p_msg -> type = MSG_WAIT;
	p_msg -> recv_pid = TASK_INIT;
	p_msg -> int_info[0] = signal;
	send_msg(p_msg);

	while(1){
		p_msg = recv_msg();

		if(p_msg -> signal == signal){
			del_msg(p_msg);
			break;
		}

		if(p_msg != (MSG *)NULL)
			reload_msg(p_msg);	//如果出现了自己不需要的消息，将会把该消息重新放入自己的收件箱

	}	//end while
}	//end f

/*
 * int kill(int pid , int signal);
 * 发送信号到指定的进程
 *
 * @param0:目标进程
 * @param1:信号
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
int kill(int pid , int signal){
	MSG *p_msg;

	awake_proc(pid);	//首先使目标进程运行

	p_msg = get_msg();
	p_msg -> type = MSG_SIGNAL;
	p_msg -> recv_pid = pid;
	p_msg -> signal = signal;

	return send_msg(p_msg);



}


/*
 * 使目标进程处于睡眠状态指定时间
 */
unsigned int sleep(unsigned int seconds){

	MSG *p_msg;

	p_msg = get_msg();
	p_msg -> type = MSG_SLEEP;
	p_msg -> recv_pid = TASK_INIT;
	p_msg -> int_info[0] = seconds;

	return send_msg(p_msg);


}

