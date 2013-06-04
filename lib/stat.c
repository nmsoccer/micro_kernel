#include "const.h"
#include "assert.h"
#include "process.h"
#include "message.h"
#include "signal.h"
#include "unistd.h"
#include "sys/fs.h"
#include "sys/stat.h"

/*
 *int mkdir(char *path , mode_t mode);
 *
 * @param0:创建的目录路径。
 * @param1:模式。S_IWIRTE只写，S_IREAD只读，S_IWRITE | S_IREAD读写。
 *
 *注意创建目录的时候读写权限都是只针对所有者。
 *组和其他用户都是默认无读写权限，只有所有者更改权限才可以获得
 *
 *返回值：
 *成功返回0。失败返回-1
 */
int mkdir(char *path , mode_t mode){
	MSG *p_msg;
	int i;

	p_msg = get_msg();

	p_msg->type = MSG_MKDIR;	//消息类型为创建文件
	p_msg->recv_pid = TASK_FS;
	p_msg->int_info[0] = mode;



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
 *int rmdir(char *path);
 * 删除目录。
 *
 *@param:目录路径
 *
 *注意删除目录的时候如果其他用户获得写权限可以删除
 *
 *返回值：
 *成功返回0.失败返回-1
 */
int rmdir(char *path){
	MSG *p_msg;
	int i;

	p_msg = get_msg();

	p_msg->type = MSG_RMDIR;	//消息类型为删除文件
	p_msg->recv_pid = TASK_FS;

	i = strncpy(p_msg->str_info , path , 0);	//返回拷贝的字符个数(不包括0x00)

	p_msg->str_info[i] = 0x00;	//用于标明字符串结束

	assert(send_msg(p_msg) == 0);

	if(pause() == SIG_SUCCESS){	//发送消息后会挂起。直到fs发送的信号将其唤醒
		return 0;
	}else{
		return -1;
	}	//end if-else


}	//end f



/*
 * int chmod(char *path , mode_t mode);
 * 改变文件（目录）的权限。
 * 只能文件（目录）的创建者可以改变
 *
 * @param0:文件（目录）路径
 * @param1:修改后的权限
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
int chmod(char *path , mode_t mode){
	MSG *p_msg;
	int i;

	p_msg = get_msg();

	p_msg->type = MSG_CHMOD;	//
	p_msg->recv_pid = TASK_FS;
	p_msg->int_info[0] = mode;

	i = strncpy(p_msg->str_info , path , 0);	//返回拷贝的字符个数(不包括0x00)

	p_msg->str_info[i] = 0x00;	//用于标明字符串结束

	assert(send_msg(p_msg) == 0);

	if(pause() == SIG_SUCCESS){	//发送消息后会挂起。直到fs发送的信号将其唤醒
		return 0;
	}else{
		return -1;
	}

}	//end f

















