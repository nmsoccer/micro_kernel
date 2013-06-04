//#define I_DEFINE
//#include "global.h"
#include "const.h"
#include "console.h"
#include "process.h"
#include "tty.h"
#include "keymap.h"
#include "message.h"
#include "assert.h"

/*
 * LOCAL CONST
 */
static KB_INPUT  keybuff_in;
static u8 NUM_LOCK_ON = 1;
static u8 CAPS_LOCK_ON = 1;
/*
 * outside link
 */
extern int TTY_ENABLE;
/*
 * realization
 */

void keyboard_handler(){
	char * full_msg = {"buffer is full!"};

	u8 code;
	u32 port_in = 0x60;

	KB_INPUT * p = &keybuff_in;

	code = in_byte(port_in);

	if(p -> counter == KB_IN_BYTES){
		//if counter == KB_IN_BYTES , display full_msg
//		cvideoaddr = dispstr(full_msg , cvideoaddr);


	}else{
		//if counter < KB_IN_BYTES

		p -> buff[p -> head] = (char)code;
		p -> head = (p -> head + 1) % KB_IN_BYTES;
		p -> counter++;

	}


}

/*
 * void key_control(TTY *p_tty);
 * deal with scan code from keyboard
 * and mainly control tty
*/
void key_control(TTY * p_tty){
	char *signal = {"^@^"};
	char *output = {"1"};

//	tty_write_str(signal , p_tty);

	u8 code;

	set_start_vmem((p_tty -> p_console) -> start_vmem);

	set_buff_tty_normal(p_tty);	//set buff of tty from buff of key

	code = (u8)read_buff_tty(p_tty); //get scan code from buff of tty

	if(code == NONE){

	}else	if(code >= FLAG_EXIT){

	}else{

		if((code == SHIFT_L) || (code == SHIFT_R)){

			set_buff_tty(p_tty);			//当shift出现时一般是组合键，因此需要从键盘缓冲区中再次读取扫描码到tty缓冲区中，直到出现非0的有效值
			code = (u8)read_buff_tty(p_tty);

			if(code >= FLAG_EXIT){

				if(code == FLAG_AHEAD){
					//shift + special key
					shift_ahead(p_tty);

				}

			}else{

				code = keymap[(code * NUM_FORMS) + ST_SHIFT];

				switch(code){
				/*
				* Special function code
				*/
					case ESC:
						break;
					case BACKSPACE:
						backspace(p_tty);
						break;
					case TAB:
						break;
					case ENTER:
						enter(p_tty);
						break;
					case CTRL_L:
						break;
					case ALT_L:
						alt(p_tty);
						break;
					case CAPS_LOCK:
						caps_lock(p_tty);
						break;
					case F1:
					case F2:
					case F3:
					case F4:
					case F5:
					case F6:
					case F7:
					case F8:
					case F9:
					case F10:
						break;
					case NUM_LOCK:
						num_lock(p_tty);
						break;
					case SCROLL_LOCK:
					case F11:
					case F12:
						break;
					/*
					 * normal display code
					 */
					default:
						if(!set_input_tty((u32)code , p_tty)){	// if input_buff of tty is full , will not display any more
							tty_write_char((u32)code , p_tty);
						}

						break;
					}
				}



		}else if(code == FLAG_AHEAD){
			set_buff_tty(p_tty);		//是双字节扫描码，所以还要读取下一个扫描码
			code = (u8)read_buff_tty(p_tty);
			code = keymap[(code * NUM_FORMS) + ST_AHEAD];
			switch(code){
				/*
				* Special function code
				*/
				case PAD_ENTER:
					enter(p_tty);
					break;
				case CTRL_R:
				case PAD_SLASH:
					break;
				case ALT_R:
					alt(p_tty);
					break;
/*
 * USB键盘上无对应的扫描码，相应的扫描码与PAD_?一致
				case HOME:
				case UP:
				case LEFT:
				case RIGHT:
				case END:
				case DOWN:
*/
				case PAGEUP:
				case PAGEDOWN:
				case INSERT:
				case DELETE:
				case GUI_L:
				case GUI_R:
				case APPS:
					break;
				default:
					break;
			}


		}else{

			code = keymap[(code * NUM_FORMS) + CL_SHIFT];

			switch(code){
			/*
			 * Special function code
			 */
				case ESC:
					break;
				case BACKSPACE:
					backspace(p_tty);
					break;
				case TAB:
					break;
				case ENTER:
					enter(p_tty);
					break;
				case CTRL_L:
					break;
				case ALT_L:
					alt(p_tty);
					break;
				case CAPS_LOCK:
					caps_lock(p_tty);
					break;
				case F1:
				case F2:
				case F3:
				case F4:
				case F5:
				case F6:
				case F7:
				case F8:
				case F9:
				case F10:
					break;
				case NUM_LOCK:
					num_lock(p_tty);
					break;
				case SCROLL_LOCK:
					break;
				case PAD_HOME:		//HOME
				case PAD_END:		//END
				case PAD_UP:		//UP
				case PAD_LEFT:		//LEFT
				case PAD_RIGHT:	//RIGHT
				case PAD_DOWN:		//DOWN
					if(NUM_LOCK_ON == 1){
						pad_direct((u32)code , p_tty);
					}else{	//NUM_LOCK_ON == 0
						code = keymap[(code * NUM_FORMS) + ST_SHIFT];

						if(!set_input_tty((u32)code , p_tty)){	// if input_buff of tty is full , will not display any more
							tty_write_char((u32)code , p_tty);
						}

					}

					break;
				case PAD_PAGEUP:
				case PAD_MINUS:
				case PAD_MID:
				case PAD_PLUS:
				case PAD_PAGEDOWN:
				case PAD_INS:
				case PAD_DOT:
					if(NUM_LOCK_ON == 1){

					}else{	//NUM_LOCK_ON == 0 , display number
						code = keymap[(code * NUM_FORMS) + ST_SHIFT];

						if(!set_input_tty((u32)code , p_tty)){	// if input_buff of tty is full , will not display any more
							tty_write_char((u32)code , p_tty);
							}
					}
					break;
				case F11:
				case F12:
					break;
				/*
				 * normal display code
				 */
				default:
					if((code >= 0x61) && (code <= 0x7A)){
						if(CAPS_LOCK_ON == 0){	// change into capital letter
							code -= 0x20;
						}
					}


					if(!set_input_tty((u32)code , p_tty)){	// if input_buff of tty is full , will not display any more
						tty_write_char((u32)code , p_tty);

					}

					break;

			}


		}

	}

}







/*
 * void init_keyboard();
*/
void init_keyboard(){
	int i = 0;

	keybuff_in.head = 0;
	keybuff_in.tail = 0;
	keybuff_in.counter = 0;
	for(;i < KB_IN_BYTES;i++){
		keybuff_in.buff[0] = 0;
	}

}



/*
 * get scan code from buff
*/

char get_keycode(){
	char * empty_msg = {"buffer is empty"};
	KB_INPUT *p = &keybuff_in;

	char code;

	if(p -> counter == 0){

		return NONE;

	}else{
		disable_int();

		code = p -> buff[p -> tail];
		p -> tail = (p -> tail + 1) % KB_IN_BYTES;
		p -> counter--;

		enable_int();

		return code;
	}


}


/*
 * 专为key_contorl()服务的专门函数
 */




void shift_ahead(TTY *p_tty){ //0x2A(0x36) + 0xE0 + ?


	u8 scan_code;
	CONSOLE *p_console;

	set_buff_tty(p_tty);	// get ? from key buff
	scan_code = (u8)read_buff_tty(p_tty); //get ? from tty buff;
	p_console = p_tty -> p_console;		//get console

	switch(scan_code){
		case UP:

			if(p_console -> start_vmem <= p_console -> base_vmem){
			}else{
				p_console -> start_vmem -= LINE_SCROLL;
				set_start_vmem(p_console -> start_vmem);

				p_console -> current_videoaddr = ((p_console -> current_videoaddr) / 160) * 80 * 2;
															//here delete column only row left

				set_cursor_console(p_console);
			}

				break;

		case DOWN:
			p_console -> start_vmem += LINE_SCROLL;

			if((p_console -> start_vmem + SCREEN_SCROLL) > (p_console -> base_vmem + SCREEN_SCROLL * 3)){
				p_console -> start_vmem -= LINE_SCROLL;
			}else{
				set_start_vmem(p_console -> start_vmem);
				p_console -> current_videoaddr = ((p_console -> current_videoaddr) / 160) * 80 * 2;
															//here delete column only row left

				set_cursor_console(p_console);
			}

				break;

		case LEFT:
			if(p_console -> start_vmem < (p_console -> base_vmem + SCREEN_SCROLL)){
				//if can not scroll screen , return to the start
				p_console -> start_vmem = p_console -> base_vmem;
				set_start_vmem(p_console -> start_vmem);

				p_console -> current_videoaddr = 0;

				set_cursor_console(p_console);
			}else{
				p_console -> start_vmem -= SCREEN_SCROLL;
				set_start_vmem(p_console -> start_vmem);

				p_console -> current_videoaddr = 0;

				set_cursor_console(p_console);
			}
				break;

		case RIGHT:
			p_console -> start_vmem += SCREEN_SCROLL;

			if((p_console -> start_vmem + SCREEN_SCROLL) > (p_console -> base_vmem + SCREEN_SCROLL * 3)){
				p_console -> start_vmem -= SCREEN_SCROLL;
			}else{
				set_start_vmem(p_console -> start_vmem);

				p_console -> current_videoaddr = 0;

				set_cursor_console(p_console);
			}

				break;



	}

}



void backspace(TTY *p_tty){
	char *space = {" "};

	CONSOLE *p_console = p_tty -> p_console;

	if(p_tty -> index > 0){
		p_tty -> index--;
		p_tty -> input_buff[p_tty -> index] = 0;
		if(p_console -> current_videoaddr > 0 ){
			p_console -> current_videoaddr = p_console -> current_videoaddr - 2; //跳到前面一个显示位置，显示空格(将字符掩盖掉)
			tty_write_str(space , p_tty);

			p_console -> current_videoaddr = p_console -> current_videoaddr - 2;//因为刚才打印了一个空格，现在需要跳回到上一个位置

			set_cursor_console(p_console);

		}

	}

}

void enter(TTY *p_tty){
	MSG *p_msg;

	p_msg = get_msg();
	p_msg ->type = MSG_ENTER_KEY;
	p_msg ->recv_pid = USR_MAN;

	CONSOLE *p_console = p_tty -> p_console;

	if(p_tty -> index == TTY_IN_BYTES){
		p_tty -> index--;
	}

	p_tty -> input_buff[p_tty -> index] = (char)0x0D;

	//当用户输入回车之后，将input_buff之中的字符串复制到消息包中。并发送给当前用户进程
	strncpy(p_msg->str_info , p_tty->input_buff , p_tty->index);
	p_msg->int_info[0] = p_tty->index;

	assert(send_msg(p_msg) == 0);

	//发送完之后将标志清零，从开始接收字符
	p_tty -> index = 0;

	p_console -> current_videoaddr = ((p_console -> current_videoaddr) / 160) * 80 * 2 + 80 * 2; //下一行的起始位置

	set_cursor_console(p_console);

}

void alt(TTY *p_tty){	//ALT + ?()
	u8 scan_code;


	set_buff_tty(p_tty); 	//ALT + ?. set ? in buff of tty
	scan_code = (u8)read_buff_tty(p_tty);

	switch(scan_code){
		case TTY1_CHOSEN:
			TTY_ENABLE = 1;
			break;
		case TTY2_CHOSEN:
			TTY_ENABLE = 2;
			break;
		case TTY3_CHOSEN:
			TTY_ENABLE = 3;
			break;
		case F4:
			break;
		default:
			break;
	}

}

void num_lock(TTY *p_tty){
	if(NUM_LOCK_ON  == 1){
		NUM_LOCK_ON = 0;
	}else{
		NUM_LOCK_ON = 1;
	}


}


void caps_lock(TTY *p_tty){
	if(CAPS_LOCK_ON == 1){
		CAPS_LOCK_ON = 0;
	}else{
		CAPS_LOCK_ON = 1;
	}

//	tty_write_int((int)CAPS_LOCK_ON , p_tty);

}



void pad_direct(u32 scan_code , TTY *p_tty){
	int row = 0;
	int col = 0;
	CONSOLE *p_console = p_tty -> p_console;
//	tty_write_int(78 , p_tty);

	switch((u8)scan_code){

		case PAD_HOME:
			p_console -> current_videoaddr = ((p_console -> current_videoaddr) / 160) * 80 * 2;
			set_cursor_console(p_console);
			break;

		case PAD_END:
			break;

		case PAD_UP:
			col = (p_console -> current_videoaddr / 2) % 80;
			row = (p_console -> current_videoaddr) / 160;
			if(row > 0){
				row--;
			}
			p_console -> current_videoaddr = (80 * row + col) * 2;
			set_cursor_console(p_console);

			break;

		case PAD_DOWN:
			col = (p_console -> current_videoaddr / 2) % 80;
			row = (p_console -> current_videoaddr) / 160;
				if(row < 24){
					row++;
				}
			p_console -> current_videoaddr = (80 * row + col) * 2;
			set_cursor_console(p_console);

			break;

		case PAD_LEFT:
			col = (p_console -> current_videoaddr / 2) % 80;
			row = (p_console -> current_videoaddr) / 160;
				if(col > 0){
					col--;
				}
			p_console -> current_videoaddr = (80 * row + col) * 2;
			set_cursor_console(p_console);

			break;

		case PAD_RIGHT:
			col = (p_console -> current_videoaddr / 2) % 80;
			row = (p_console -> current_videoaddr) / 160;
				if(col < 79){
					col++;
				}
			p_console -> current_videoaddr = (80 * row + col) * 2;
			set_cursor_console(p_console);

			break;
		default:
			break;

	}



}























