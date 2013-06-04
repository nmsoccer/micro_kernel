#include "const.h"
#include "process.h"
#include "message.h"
#include "string.h"
/*
 *-------------------------libc.c -----------------------------
 *-------------------------LeiMing-----------------------------
 */


/*
 * declare
 */
//void disp_int(int data);


/*
 * realization
 */

int disp_int(int data , int location){
	char str[11] = {0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x00}; //双字的整数最长十位

	int index = 9; //last 0xFF
	char mod = 0;  //除以10的余数

	if(data < 0){	//如果是负数，在最开始加上负号，同时取其绝对值
		data = -data;
		str[0] = '-';
	}
	while(1){		//按整数的位数除以10，每次将所得余数放入数组。从后往前排
		mod = data % 10;
		data = data / 10;

		str[index] = mod + 0x30;
		index--;

		if(data == 0)
			break;

	}

	str[index] = str[0]; //为了显示，中间可能由空白，所以将符号位放到有效值的开始

	if(str[index] != '-'){ //如果是正数，不显示符号
		index++;
	}

	return dispstr(&str[index] , (u32)location);

}


/*
 * 内核输出函数
 */
//只打印字符
//失败返回-1 成功返回输出的字符
int cprintk(char c){

	MSG *p_msg;
	int result;


	p_msg = get_msg();
	p_msg -> type = MSG_WRITE_CHAR;
	p_msg -> recv_pid = TASK_TTY;

	p_msg -> int_info[0] = (int)c;

	/*
	result = send_msg(p_msg);

	if(result == 1){
		return -1;
	}else{
		return (int)c;
	}
*/
	while(send_msg(p_msg) == -1){
			delay(100);
		}

	return (int)c;

}


//只打印整数
//失败返回-1 成功返回输出的整数
int iprintk(int i){

	MSG *p_msg;
	int result;


	p_msg = get_msg();
	p_msg -> type = MSG_WRITE_INT;
	p_msg -> recv_pid = TASK_TTY;

	p_msg -> int_info[0] = i;

	/*
	result = send_msg(p_msg);

	if(result == 1){
		return -1;
	}else{
		return i;
	}
*/
	while(send_msg(p_msg) == -1){
			delay(100);
		}

	return i;

}





//专门打印字符串
//失败返回-1 成功返回输出的字符
int sprintk(char *str){
	int i;

	MSG *p_msg;
	int result;


	p_msg = get_msg();
	p_msg -> type = MSG_WRITE_STR;
	p_msg -> recv_pid = TASK_TTY;

	i = strlen(str);

	if(i < MAX_STR_INFO){

		strncpy(p_msg -> str_info , str , 0);
		p_msg->str_info[i] = 0x00;

	}else{	//如果超过了消息携带的数量将截断

		strncpy(p_msg->str_info , str , MAX_STR_INFO - 1);
		p_msg->str_info[MAX_STR_INFO - 1] = 0X00;
	}


/*
	result = send_msg(p_msg);

	if(result == 1){
		return -1;
	}else{
		return sizeof(p_msg -> str_info);
	}
*/
	while(send_msg(p_msg) == -1){
		delay(100);
	}

	return 5;
}


//回车
//失败返回-1 成功返回0
int print_ctrl(){

	MSG *p_msg;
	int result;


	p_msg = get_msg();
	p_msg -> type = MSG_PRINT_CTRL;
	p_msg -> recv_pid = TASK_TTY;

/*
	result = send_msg(p_msg);

	if(result == 1){
		return -1;
	}else{
		return 0;
	}
*/
	while(send_msg(p_msg) == -1){
		delay(100);
	}
	return 0;

}



















