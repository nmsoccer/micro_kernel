
#include "klibc.h"
#include "stdlib.h"
#include "string.h"



/*
 *格式化输出。只实现了部分功能。返回输出的字符个数。(包括0x00）
 */

int printf(char *fmt , ...){
	char str[50];
	int index_f = 0;	//跟随fmt赋值入str
	int index_s = 0;	//跟随着str的位置标示
	int insert = 0; //当有参数比如整数出现时转化为字符串，并记录插入的字符个数

	int start = 0;

	int *p = &fmt;
	p += 1;

	while(1){
		if(*(fmt + index_f) == 0x00){
			str[index_s] = 0x00;
			sprintk(str + start);
			break;
		}
		if(*(fmt + index_f) == '%'){
			index_f++;

			switch(*(fmt + index_f)){
				case 'd':
					insert = itoa(*p , str + index_s);	//将参数(整形)转化为字符串，并放入以str+index_s开头的串中
					index_s += insert;	//index_s需要加上插入的字符串个数insert

					p += 1;					//p指向下一个参数地址
					index_f++;				//index_f指向fmt的下一个字符
					break;
				case 'c':
					str[index_s] = (char)*p;

					p +=1;
					index_f++;
					index_s++;
					break;
				case 's':
					insert = strncpy(str + index_s , (char *)*p , 0);	//*p为参数字符串的首地址
					index_s += insert;

					p += 1;
					index_f++;
					break;

				default:
					break;
			}




			continue;
		}
		if(*(fmt + index_f) == '/'){

			index_f++;

			switch(*(fmt + index_f)){
				case 'n':

					str[index_s] = 0x00;	//出现转义字符时，输出之前的部分
					sprintk(str + start);

					index_s++;				//重新设置index_s为输出后的部分
					start = index_s;		//start也要做相应设定

					print_ctrl();			//输出回车

					index_f++;
					break;
				case 't':



					break;
				default:
					break;





			}


			continue;
		}

		str[index_s] = *(fmt + index_f);
		index_f++;
		index_s++;


	}

	return index_s;

}



/*
 *
 *该函数与printf的区别在于前者只有出现/n的时候发送信息包。其余都是整体发送。而此函数出现%与/都要发送。显然对资源消耗较大 结果也出现差错



int printf2(char *fmt , ...){


	int start = 0;
	int end = 0;

	int *p = &fmt;
	p += 1;	//p指向第一个参数在栈中的地址.注意指针加减的字节为+- sizeof(type)

	while(1){
		if(*(fmt + end) == 0x00){
			if(start != end){
				sprintk(fmt + start);
			}
			break;
		}
		if((*(fmt + end) != '%') && (*(fmt + end) != '/')){
			end++;
			continue;
		}

		if(*(fmt + end) == '/'){
			*(fmt + end) = 0x00;
			sprintk(fmt + start);
			end++;

			switch(*(fmt + end)){
				case 'n':
					print_ctrl();
					break;
				case 't':
					break;
			}
			end++;
			start = end;
		}

		if(*(fmt + end) == '%'){
			*(fmt + end) = 0x00;
			sprintk(fmt + start);
			end++;

			switch(*(fmt + end)){
				case 'c':
					cprintk(*p);
					p += 1;
					break;

				case 'd':
					iprintk(*p);
					p += 1;
					break;

				case 's':
					sprintk(p);
					p += 1;
					break;

				default:
					break;


			}
			end++;
			start = end;

		}



	}

	delay(100);

}
*/
