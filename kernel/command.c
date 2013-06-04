#include "const.h"
#include "process.h"
#include "message.h"
#include "command.h"
#include "assert.h"

/*
 * int list(char *dir_path , int opt);
 * 列举目录内容
 *
 * @param0:显示的目录
 * @param1:选择项
 *
 * 返回值：
 * 成功返回0。失败返回-1
 */
int list(char *dir_path , int opt){
	MSG *p_msg;
	int i;

	p_msg = get_msg();
	p_msg->type = MSG_LIST;
	p_msg->recv_pid = TASK_FS;

	i = strncpy(p_msg->str_info , dir_path , 0);
	p_msg->str_info[i] = (char)0x00;

	p_msg->int_info[0] = opt;

	assert(send_msg(p_msg) == 0);

	return pause();
}	//end f




/*
 * static int check_file_dir(char *abs_path , int opt);
 *
 * 检测目标是否是文件或者目录
 *
 * @param0:文件(目录)的路径
 * @param1:选项。0为测试文件；1为测试目录
 *
 * 返回值:
 * 成功返回0.失败返回-1
 *
 */
int check_file_dir(char *abs_path , int opt){
	MSG *p_msg;
	int i;

	p_msg = get_msg();
	p_msg->type = MSG_CHECK;
	p_msg->recv_pid = TASK_FS;

	i = strncpy(p_msg->str_info , abs_path , 0);
	p_msg->str_info[i] = (char)0x00;

	p_msg->int_info[0] = opt;

	assert(send_msg(p_msg) == 0);

	return pause();

}	//end f








