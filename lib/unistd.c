#include "const.h"
#include "process.h"
#include "message.h"
#include "assert.h"
#include "unistd.h"


/*
 * int close(int fd);
 *
 * 关闭文件。
 * 如果关闭文件之后还有进程处于打开队列之中将会唤醒下一个进程
 * 如果没有进程了将撤销该fd
 *
 *@param:通过open返回的文件描述符
 *
 * 返回值：
 * 成功返回0失败返回－1
 */
int close(int fd){
	MSG *p_msg;

	p_msg = get_msg();

	p_msg->type = MSG_CLOSE;
	p_msg->recv_pid = TASK_FS;
	p_msg->int_info[0] = fd;

	assert(send_msg(p_msg) == 0);

	return pause();
}



/*
 * ssize_t read(int fd , char *buff , size_t count);
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
ssize_t read(int fd , char *buff , size_t count){
	if(count == 0){
		return 0;
	}

	MSG *p_msg;

	p_msg = get_msg();
	p_msg->type = MSG_READ;
	p_msg->recv_pid = TASK_FS;

	p_msg->int_info[0] = fd;
	p_msg->int_info[1] = (int)buff;
	p_msg->int_info[2] = (int)count;

	assert(send_msg(p_msg) == 0);

	return pause();
}	//end f




/*
 * ssize_t write(int fd , const char *buff , size_t count);
 *
 * 往打开的文件里写入buff里的count个字节
 * count为0则该函数不起作用
 *
 * @param0:打开的文件描述符
 * @param1:字符缓冲区buff
 * @param2:试图写入的字节数
 *
 * 返回值：
 * 成功返回写入的字节数。失败返回-1
 */
ssize_t write(int fd , const char *buff , size_t count){
	if(count == 0){
		return 0;
	}
	MSG *p_msg;

	p_msg = get_msg();
	p_msg->type = MSG_WRITE;
	p_msg->recv_pid = TASK_FS;

	p_msg->int_info[0] = fd;

//	strncpy(p_msg->str_info , buff , count);	//不能超过MAX_STR_INFO
// 并没有将buff的字节复制到消息里，而是复制了buff指针
	p_msg->int_info[1] = (int)buff;
	p_msg->int_info[2] = (int)count;

	assert(send_msg(p_msg) == 0);

	return pause();
}




/*
 * int pause();
 * 使自身进程处于挂起状态
 *其他进程向其发送信号将使它激活
 *
 * 成功返回信号类型。失败返回-1
 */
int pause(){
	MSG *p_msg;
	int signal = -1;

	p_msg = get_msg();

	p_msg->type = MSG_PAUSE;



	send_msg(p_msg);


	/*
	 * 这里使借鉴了wait(signal)的设计。因为发送包到处理会有一段时间，因此调用程序完全可能跨越pause()而执行之后的部分程序
	 * 这是不被允许的。因此使用此循环，因为pause()的进程被激活是有进程向其发送包，所以其接收队列中一定有信号包。
	 */


	while(1){
		p_msg = recv_msg();

		if(p_msg -> type == MSG_SIGNAL){
			signal = p_msg->signal;
			del_msg(p_msg);
			return signal;	//返回信号的类型
		}

		if(p_msg != (MSG *)NULL)
			reload_msg(p_msg);	//如果出现了自己不需要的消息，将会把该消息重新放入自己的收件箱

	}	//end while


}
