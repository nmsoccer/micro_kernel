#include "global.h"
#include "klibc.h"
#include "string.h"
#include "tty.h"
/*
 * LOCAL CONST
 */
int TTY_ENABLE = 1;


/*
 * outside link
 */

extern void key_control(TTY *p_tty);
extern void set_cursor(u32 cursor);
extern char get_keycode();

extern int videoaddr;

/*
 * definition
 */
void task_tty(){
	int i;
	MSG *p_msg;



/*
 * Can not change
 */
	init_console();

	init_tty();

//	tty_write_str("LeiMing" , (TTY *)&tty_table[TTY_ENABLE]);
//	tty_write_ctrl((TTY *)&tty_table[TTY_ENABLE]);
//	tty_write_str("is Best!" , (TTY *)&tty_table[TTY_ENABLE]);

/*
 * Test

	msg = get_msg();
	msg -> type = 8;

	tty_write_int((u32)msg -> type , (TTY *)&tty_table[TTY_ENABLE]);

	del_msg(msg);

	msg = get_msg();
	tty_write_int((u32)msg -> type , (TTY *)&tty_table[TTY_ENABLE]);
*/

//	int i;
//	char *src = {"Lei Ming is best hahaha"};
//	char dest[20];

//	for(i = 0; i < 20; i++){
//		dest[i] = '0';
//	}


//	strcpy(dest , src , sizeof(dest) - 1);
//	tty_write_str(dest , (TTY *)&tty_table[TTY_ENABLE]);


//	disp_int(i);

//	set_start_vmem((u32)(80 * 1));
	while(1){



		p_msg = recv_msg();

		if(p_msg == (MSG *)NULL){

		}else{

			switch(p_msg -> type){	//根据消息的不同类型来不同地处理
//				case MSG_WRITE_ENTER:

				case MSG_WRITE_CHAR:
					tty_write_char(p_msg -> int_info[0] , (TTY *)&tty_table[p_msg -> int_proc_info[1]]);
					del_msg(p_msg);

					break;
				case MSG_WRITE_INT:
					tty_write_int(p_msg -> int_info[0] , (TTY *)&tty_table[p_msg -> int_proc_info[1]]);
					del_msg(p_msg);

					break;
				case MSG_WRITE_STR:
					tty_write_str(p_msg -> str_info , (TTY *)&tty_table[p_msg -> int_proc_info[1]]);	//第二个参数是进程所在的p_tty
					del_msg(p_msg);

					break;

				case MSG_PRINT_CTRL:
					tty_print_ctrl((TTY *)&tty_table[p_msg -> int_proc_info[1]]);
					del_msg(p_msg);
					break;
				default:
					break;
			}
		}


		key_control((TTY *)&tty_table[TTY_ENABLE]);

	}

}




/*
 * 可以为其他函数调用的通用函数
 */





void init_tty(){
	TTY * p_tty;
	int i;
	int j;

	for(i = 1; i <= NUM_TTY ; i++){
		p_tty =(TTY *) &tty_table[i];

		p_tty -> head = 0;
		p_tty -> tail = 0;
		p_tty -> counter = 0;

		p_tty -> index = 0;

		for(j = 0; j < TTY_IN_BYTES; j++){
			p_tty -> buff[j] = 0;
			p_tty -> input_buff[j] = 0;
		}

		p_tty -> p_console = (CONSOLE *)&console_table[i];
	}

}

/*
 * 针对tty的键盘扫描码缓冲区buff操作函数，一般由键盘处理函数调用
 */

//不会持续扫描键盘缓冲区
void set_buff_tty_normal(TTY *p_tty){
	u8 code;

	code = (u8)get_keycode();

	if(code != NONE){
		/*
		 * 要求缓冲区没有满.
		 */
		if((p_tty -> counter <= TTY_IN_BYTES)){
			p_tty -> buff[p_tty -> head] = (char)code;
			p_tty -> head = (p_tty -> head + 1) % TTY_IN_BYTES;
			p_tty -> counter++;
		}
	}


}



//不断扫描键盘缓冲区直到有输入为止
void set_buff_tty(TTY *p_tty){
	u8 code;

	while(1){	//must get a scan code

		code = (u8)get_keycode();
		if(code != NONE)
			break;
	}
	/*
	 * 要求缓冲区没有满.
	 */
	if((p_tty -> counter <= TTY_IN_BYTES)){
		p_tty -> buff[p_tty -> head] = (char)code;
		p_tty -> head = (p_tty -> head + 1) % TTY_IN_BYTES;
		p_tty -> counter++;
	}

}




char read_buff_tty(TTY *p_tty){
	char code;

	if(p_tty -> counter == 0){
		return NONE;
	}else{
		code = p_tty -> buff[p_tty -> tail];
		p_tty -> tail = (p_tty -> tail + 1) % TTY_IN_BYTES;
		p_tty -> counter--;

		return code;
	}

}

char set_input_tty(u32 c , TTY *p_tty){
	char *full_msg = {"Full,Press Enter clear!"};

	if(p_tty -> index < TTY_IN_BYTES){	//input common chars;
			p_tty -> input_buff[p_tty -> index] = (char)c;
			(p_tty -> index)++;
			return 0;
		}else{
//			p_tty -> index = 0;
//			tty_write_str(full_msg , p_tty);
			return 1;
		}


}



/*
 * 通用函数，用于基于TTY的显示。以后的显示函数都是通过TTY来进行（封装了dispstr()）
 */
void tty_print_ctrl(TTY *p_tty){
	CONSOLE *p_console = p_tty -> p_console;
	p_console -> current_videoaddr = ((p_console -> current_videoaddr) / 160) * 80 * 2 + 80 * 2; //下一行的起始位置
	set_cursor_console(p_console);


}




void tty_write_char(u32 char_code , TTY *p_tty){
	char *output = {"0"};

	CONSOLE *p_console = p_tty -> p_console;

	*output = (u8)char_code;


	p_console -> current_videoaddr = dispstr(output , (p_console -> start_vmem * 2 + p_console -> current_videoaddr))
													- p_console -> start_vmem * 2;

	set_cursor_console(p_console);


}

void tty_write_str(char *str , TTY *p_tty){
	CONSOLE *p_console = p_tty -> p_console;

	p_console -> current_videoaddr = dispstr(str , (p_console -> start_vmem * 2 + p_console -> current_videoaddr))
																				- p_console -> start_vmem * 2;

	set_cursor_console(p_console);



}

void tty_write_hexb(u32 byte , TTY *p_tty){
	CONSOLE *p_console = p_tty -> p_console;

	p_console -> current_videoaddr = disphexb(byte , (p_console -> start_vmem * 2 + p_console -> current_videoaddr))
														- p_console -> start_vmem * 2;

	set_cursor_console(p_console);

}

void tty_write_hexw(u32 word , TTY *p_tty){
	CONSOLE *p_console = p_tty -> p_console;

	p_console -> current_videoaddr = disphexw(word , (p_console -> start_vmem * 2 + p_console -> current_videoaddr))
														- p_console -> start_vmem * 2;

	set_cursor_console(p_console);

}

void tty_write_hexd(u32 dword , TTY *p_tty){
	CONSOLE *p_console = p_tty -> p_console;

	p_console -> current_videoaddr = disphexdw(dword , (p_console -> start_vmem * 2 + p_console -> current_videoaddr))
														- p_console -> start_vmem * 2;

	set_cursor_console(p_console);

}

void tty_write_int(int data , TTY *p_tty){
	CONSOLE *p_console = p_tty -> p_console;

	p_console -> current_videoaddr = disp_int(data , (p_console -> start_vmem * 2 + p_console -> current_videoaddr))
														- p_console -> start_vmem * 2;

	set_cursor_console(p_console);

}






