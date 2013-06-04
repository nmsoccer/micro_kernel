#include "const.h"
#include "sys/types.h"
#include "process.h"
#include "message.h"
#include "signal.h"
#include "assert.h"
#include "sys/stat.h"
#include "fcntl.h"

//////////函数声明//////////


/*
 *extern int creat(char *path , mode amode);
 * 创建文件。
 *
 * @param0:创建的文件路径。
 * @param1:模式。S_IWIRTE只写，S_IREAD只读，S_IWRITE | S_IREAD读写。
 *
 *注意创建文件的时候读写权限都是只针对所有者。
 *组和其他用户都是默认无读写权限，只有所有者更改权限才可以获得
 *
 *
 *返回值：
 *成功返回0。失败返回-1
 */
int creat(char *path , mode_t amode){
	MSG *p_msg;
	int i;

	p_msg = get_msg();

	p_msg->type = MSG_CREATE;	//消息类型为创建文件
	p_msg->recv_pid = TASK_FS;
	p_msg->int_info[0] = amode;



	i = strncpy(p_msg->str_info , path , 0);	//返回拷贝的字符个数(不包括0x00)

	p_msg->str_info[i] = 0x00;	//用于标明字符串结束

	assert(send_msg(p_msg) == 0);

	if(pause() == SIG_SUCCESS){	//发送消息后会挂起。直到fs发送的信号将其唤醒
		return 0;
	}else{
		return -1;
	}

}	//end f





/*
 * extern int remove(char *path);
 * 删除文件。
 * 不能删除只读文件。无法删除其他用户的文件
 *
 * @param:文件的路径。
 *
 *注意删除文件的时候如果其他用户获得写权限可以删除
 *
 * 返回值:
 * 成功返回0.失败返回-1
 */
int remove(char *path){
	MSG *p_msg;
	int i;

	p_msg = get_msg();

	p_msg->type = MSG_REMOVE;	//消息类型为删除文件
	p_msg->recv_pid = TASK_FS;

	i = strncpy(p_msg->str_info , path , 0);	//返回拷贝的字符个数(不包括0x00)

	p_msg->str_info[i] = 0x00;	//用于标明字符串结束

	assert(send_msg(p_msg) == 0);

	if(pause() == SIG_SUCCESS){	//发送消息后会挂起。直到fs发送的信号将其唤醒
		return 0;
	}else{
		return -1;
	}

}	//end f



/*
 * extern int open(char *abs_path , int flags , mode_t mode);
 * 打开文件。
 *
 * @param0:文件路径。
 * @param1:打开标志
 * @param2:打开的权限
 *
 * 返回值：
 * 成功返回打开的fd号，失败返回-1
 *
 */
int open(char *abs_path , int flags , mode_t mode){
	MSG *p_msg;
	int i;

	//如果这是建立文件的，处理过程和其他情况不同
	if((flags & O_CREAT_MASK) == O_CREAT){
		//创建文件
//		printf("Creat!");
		creat(abs_path , mode);

	}

	p_msg = get_msg();

	p_msg->type = MSG_OPEN;	//消息类型为打开文件
	p_msg->recv_pid = TASK_FS;

	i = strncpy(p_msg->str_info , abs_path , 0);	//返回拷贝的字符个数(不包括0x00)
	p_msg->str_info[i] = 0x00;	//用于标明字符串结束

	p_msg->int_info[0] = flags;

	assert(send_msg(p_msg) == 0);
	return pause();


}	//end f







