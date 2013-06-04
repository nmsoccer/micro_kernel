#include "const.h"
#include "process.h"
#include "signal.h"
#include "message.h"
#include "sys/fs.h"
#include "stdio.h"
#include "hd.h"


HD_INFO hd_info;
//u8 hd_buff[SEC_SIZE];

////////////////////////////////////////////////////////////
/////////////////硬盘系统进程/////////////////////////////
void task_hd(){
//	u8 hd_buff[SEC_SIZE];
	MSG *p_msg;
	u8 status;

	u32 *p;



	init_hd_info(&hd_info);

	while(1){
		p_msg = recv_msg();

		if(p_msg != (MSG *)NULL){
			switch(p_msg -> type){

				case MSG_SIGNAL:
					switch(p_msg -> signal){
						case 0:
//							printf("Get SIGNAL 0!");
							break;
						default:
							break;
					}
					del_msg(p_msg);
					break;

				case MSG_HD_OPEN:
					hd_identify(0);

					/*获得硬盘的数量 0x475是BIOS规定的数据点。从这里可以取出硬盘的数量*/
//					u8 *pNrDrives = (u8 *)(0x475);
//					printf("NrDrives:%d./n", *((u8 *)(0x475)));
					get_hd_partion(fs_buff , 0 , 0 , -1);
//					print_hd_info(&hd_info);
					printf("Hard Disk Opened!/n");

					kill(p_msg->send_pid , SIG_READY);	//发送信号。表示分区信息已经获得。

					del_msg(p_msg);
					break;
				case MSG_READ_HD:
					//MSG-> int_info[0]:char *buff int_info[1]:driver int_info[2]:abs_sec
					read_hd_sector((u8 *)p_msg->int_info[0] , p_msg->int_info[1] , p_msg->int_info[2]);

					kill(p_msg->send_pid , SIG_READY);//需要发送信号给目标进程。表示已经成功读入。解除对方的死虚幻

//					printf("Read!");
					del_msg(p_msg);
					break;
				case MSG_WRITE_HD:
					//MSG-> int_info[0]:char *buff int_info[1]:driver int_info[2]:abs_sec
					write_hd_sector((u8 *)p_msg->int_info[0] , p_msg->int_info[1] , p_msg->int_info[2]);

					kill(p_msg->send_pid , SIG_READY);//发送信号给发送进程。表示已经成功写入。解除对方的死循环

//					printf("Write!");
					del_msg(p_msg);
					break;
				default:
					del_msg(p_msg);
					break;
			}
		}

	}
}



//////////////////////////////////////////////////////////////////
//////////////////////硬盘中断程序//////////////////////////////

void hd_handler(){
	MSG *p_msg;
	u8 status;

	status = in_byte((u32) REG_STATUS);	//表示可以再次接收中断

	p_msg = get_msg();						//向硬盘驱动程序发送信号。表示READY
	p_msg -> type = MSG_SIGNAL;
	p_msg -> recv_pid = TASK_HD;
	p_msg -> signal = SIG_READY;

	post_msg(p_msg);

}


/////////封装了硬盘消息的函数&硬盘驱动接口函数///////////////////////////


/*
 * int hd_open();
 * 打开硬盘驱动。获取硬盘信息
 * 成功返回0否则返回-1
 */
int hd_open(){
	MSG *p_msg;

	p_msg = get_msg();

	p_msg -> type = MSG_HD_OPEN;
	p_msg -> recv_pid = TASK_HD;

	return send_msg(p_msg);

}

/*
 * int do_read_hd(u8 *hd_buff , int driver , u32 abs_sec);
 * 用于向磁盘驱动程序hd.c发送消息。
 * 功能是读取磁盘driver某绝对扇区abs_sec的内容到文件缓冲区hd_buff中。
 * 一次读取一个扇区。
 * 该函数和read_hd_sec几乎一样。但这是为其他进程提供的接口。因为磁盘中断总是向磁盘驱动程序发送信号包。
 * 如果其他进程直接使用read_hd_sec的话将陷入永久循环。
 *同理write也是一样的。所以对磁盘的操纵只能由hd进程完成
 *
 *@param0:磁盘缓冲区
 *@param1:磁盘主设备号
 *@param2:磁盘绝对扇区号
 *成功返回0 失败返回-1
 */
int do_read_hd(void *hd_buff , int driver , u32 abs_sec){

	MSG *p_msg;

	p_msg = get_msg();
	//封装消息
	p_msg -> type = MSG_READ_HD;
	p_msg -> recv_pid = TASK_HD;
	p_msg -> int_info[0] = (int)hd_buff;
	p_msg -> int_info[1] = driver;
	p_msg -> int_info[2] = abs_sec;

	if(send_msg(p_msg) == 0){
		wait(SIG_READY);	//之所以这样做是因为调用函数的进程可能马上会用到缓冲区。而此时缓冲区可能还没有被写入。因此需要等待写入后再操作
		return 0;
	}else{
		return -1;
	}

}

/*
 * int do_write_hd(u8 *hd_buff , int driver , u32 abs_sec);
 * 用于向磁盘驱动程序hd.c发送消息。
 * 功能是写hd_buff到磁盘driver的某绝对扇区abs_sec中。
 * 一次写一个扇区。
 *
 *@param0:磁盘缓冲区
 *@param1:磁盘主设备号
 *@param2:磁盘绝对扇区号
 *成功返回0 失败返回-1
 */
int do_write_hd(void *hd_buff , int driver , u32 abs_sec){

	MSG *p_msg;

	p_msg = get_msg();
	//封装消息
	p_msg -> type = MSG_WRITE_HD;
	p_msg -> recv_pid = TASK_HD;
	p_msg -> int_info[0] = (int)hd_buff;
	p_msg -> int_info[1] = driver;
	p_msg -> int_info[2] = abs_sec;

	if(send_msg(p_msg) == 0){	//如果发送消息成功。要等待TASK_HD成功写入才能继续
		wait(SIG_READY);
		return 0;
	}else{
		return -1;
	}
}







///////////////////////内部函数////////////////////////////////////

/*
 * void init_hd_info(HD_INFO *p_hd_info);
 * 用于初始化硬盘分区信息表
 */
static void init_hd_info(HD_INFO *p_hd_info){
	int i;

	for(i = 0; i < MAX_PRIME_PARTION; i++){

		p_hd_info -> prime_partion[i].bootable = 0;
		p_hd_info -> prime_partion[i].base_sec = 0;
		p_hd_info -> prime_partion[i].num_sec = 0;
	}

	for(i = 0; i < MAX_LOGIC_PARTION; i++){
		p_hd_info -> logic_partion[i].bootable = 0;
		p_hd_info -> logic_partion[i].base_sec = 0;
		p_hd_info -> logic_partion[i].num_sec = 0;
	}

	p_hd_info -> index_sub[0] = 0;
	p_hd_info -> index_sub[1] = 16;
	p_hd_info -> index_sub[2] = 32;
	p_hd_info -> index_sub[3] = 48;

}




/*
 * static int send_hd_cmd(HD_CMD *cmd);
 * 通过传入命令指针，将相应的寄存器设置为HD_CMD的成员。
 * 当设置command之后硬盘发生中断
 * 之前需要测试REG_STATUS的BSY是否为0。
 * 之后再设置REG_DEV_CONTROL的IEN为0。表示打开中断
 * 如果成功则返回0 失败返回-1
 */
static int send_hd_cmd(HD_CMD *cmd){
	u8 status;
	int i;

	for(i = 0; i < 1000; i++){

		status = in_byte((u32) REG_STATUS);

		if(status == STATUS_BSY){	//检查STATUS是否忙。如果忙延迟一段时间再检查。当循环结束还没有成功，返回-1否则成功返回0
			delay(i);
			continue;
		}
		if(status != STATUS_BSY){
			out_byte((u32)0 , (u32)REG_DEV_CONTROL);	//设置IEN，表示中断打开
			out_byte((u32)cmd -> feature , (u32)REG_FEATURE);
			out_byte((u32)cmd -> sec_counter , (u32)REG_SEC_COUNTER);
			out_byte((u32)cmd -> lba_low , (u32)REG_LBA_LOW);
			out_byte((u32)cmd -> lba_mid , (u32)REG_LBA_MID);
			out_byte((u32)cmd -> lba_hig , (u32)REG_LBA_HIG);
			out_byte((u32)cmd -> device , (u32)REG_DEVICE);
			out_byte((u32)cmd -> command , (u32)REG_COMMAND);

			return 0;
		}
	}
	return -1;
}


/*
 *static int read_hd_sector(void *hd_buff , int driver , unsigned int abs_sec)
 * 读硬盘一个扇区的信息。driver表示选中的硬盘。sec表示扇区号(绝对扇区号)
 * 成功返回0否则返回-1
 */
static int read_hd_sector(void *hd_buff , int driver , unsigned int abs_sec){
	HD_CMD cmd;
	int result;

	cmd.feature = 0;
	cmd.sec_counter = 1;
	cmd.lba_low = (abs_sec) & 0xFF;	//因为使用的是LBA模式。所以只需要输入绝对扇区号。注意DEVICE最低四位是扇区号最高四位
	cmd.lba_mid = (abs_sec >> 8) & 0xFF;
	cmd.lba_hig = (abs_sec >> 16) & 0xFF;
	cmd.device = SET_REG_DEVICE(1 , driver , (abs_sec) >> 24);	//SET_REG_DEVICE(LBA , DRV , LBA_HIGHEST)
	cmd.command = ATA_READ;

	result = send_hd_cmd(&cmd);


	if(result == 0){
		wait(SIG_READY);
		port_read(hd_buff , SEC_SIZE / 4 , REG_DATA);
		return 0;
	}else{
		printf("Read Error!");	//失败将显示信息。成功则不显示信息
		return -1;
	}

}	//end function


/*
 *static write_hd_sector(u8 *hd_buff , int driver , u32 abs_sec);
 *@param0 缓冲区
 *@param1 主设备号
 *@param2 绝对扇区号
 *成功返回0失败返回-1
 */
static int write_hd_sector(void *hd_buff , int driver , u32 abs_sec){

	HD_CMD cmd;
	u8 status;
	int result;
	int i;

	cmd.feature = 0;
	cmd.sec_counter = 1;
	cmd.lba_low = (abs_sec) & 0xFF;	//因为使用的是LBA模式。所以只需要输入绝对扇区号。注意DEVICE最低四位是扇区号最高四位
	cmd.lba_mid = (abs_sec >> 8) & 0xFF;
	cmd.lba_hig = (abs_sec >> 16) & 0xFF;
	cmd.device = SET_REG_DEVICE(1 , driver , (abs_sec) >> 24);	//SET_REG_DEVICE(LBA , DRV , LBA_HIGHEST)
	cmd.command = ATA_WRITE;

	result = send_hd_cmd(&cmd);


	if(result == 0){
		for(i=0; i<1000; i++){
			status = in_byte(REG_STATUS);
			if(status != (u8)STATUS_BSY){
				port_write(hd_buff , SEC_SIZE / 4 , REG_DATA);
				wait(SIG_READY);
//				printf("Write success!");
				return 0;
			}

		}	//end for
		printf("Write error!");
		return -1;

	}else{
		printf("Write Error!");	//失败返回信息。成功则不返回信息。
		return -1;
	}

}	//end function




/*
 * static int hd_identify(int driver);
 * 获取硬盘的信息。硬盘号由driver给出。
 */
static int hd_identify(int driver){
	HD_CMD cmd;
	u8 status;
	char hdbuf[SEC_SIZE];

	cmd.feature = 0;
	cmd.sec_counter = 0;
	cmd.lba_low = 0;
	cmd.lba_mid = 0;
	cmd.lba_hig = 0;
	cmd.device  = SET_REG_DEVICE(0, driver, 0);
	cmd.command = ATA_IDENTIFY;


	send_hd_cmd(&cmd);


	wait(SIG_READY);

	port_read(hdbuf , SEC_SIZE/4 , (u32)REG_DATA);	//因为port_read一次读入insd四字节

//	print_identify_info((u16*)hdbuf);

	return 0;

}


/*
 * 该函数用于显示硬盘的某些信息
 */

static void print_identify_info(u16* hdinfo){
	int capabilities = hdinfo[49];
	printf("LBA supported: %s /n",
	       (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	printf("LBA48 supported: %s   ",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printf("HD size: %dMB/n", sectors * 512 / 1000000);

}



/*
 *static int get_hd_partion(int driver , unsigned int abs_sec , int prime_partion)
 * 获得硬盘的分区信息。
 */
static int get_hd_partion(u8 *hd_buff , int driver , unsigned int abs_sec , int prime_partion){
	HD_INFO *p_hd_info;
//	u8 hd_buff[SEC_SIZE];
	u32 *p;

	p_hd_info = &hd_info;

	if(read_hd_sector(hd_buff , driver , abs_sec) == -1){
			printf("Error happend!");
			return -1;
	}

	if(prime_partion == -1){	//表示整个硬盘引导扇区。因此是主要分区而不是逻辑分区

		//PRIME_PARTION 0
		if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0 + PARTION_SYSTEM_ID] == PARTION_EMPTY){
			//表示主分区0不存在。那么其他主分区肯定也不存在了。
			return 0;
		}else{
			p_hd_info -> prime_partion[0].bootable = hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0];//主扇区状态

			p = (u32 *)&hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0 + PARTION_BASE_SEC];//主分区起始扇区和长度

			p_hd_info -> prime_partion[0].base_sec = *p;	//主分区0的起始扇区
			p_hd_info -> prime_partion[0].num_sec = *(p + 1);	//主分区0的扇区个数

			if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0 + PARTION_SYSTEM_ID] == PARTION_EXTEND){
				//如果主分区是扩展分区。将要继续递归查找
				get_hd_partion(hd_buff , driver , *p , 0);	//这是递归查找主分区0的逻辑分区
			}

		}

		//PRIME_PARTION 1
		if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 1 + PARTION_SYSTEM_ID] == PARTION_EMPTY){
				//表示主分区0不存在。那么其他主分区肯定也不存在了。
				return 0;
			}else{
				p_hd_info -> prime_partion[0].bootable = hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 1];//主扇区状态

				p = (u32 *)&hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 1 + PARTION_BASE_SEC];//主分区起始扇区和长度

				p_hd_info -> prime_partion[1].base_sec = *p;	//主分区1的起始扇区
				p_hd_info -> prime_partion[1].num_sec = *(p + 1);	//主分区1的扇区个数

				if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 1 + PARTION_SYSTEM_ID] == PARTION_EXTEND){
					//如果主分区是扩展分区。将要继续递归查找
					get_hd_partion(hd_buff , driver , *p , 1);	//这是递归查找主分区1的逻辑分区
				}

			}

		//PRIME_PARTION 2
		if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 2 + PARTION_SYSTEM_ID] == PARTION_EMPTY){
				//表示主分区0不存在。那么其他主分区肯定也不存在了。
				return 0;
			}else{
				p_hd_info -> prime_partion[0].bootable = hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 2];//主扇区状态

				p = (u32 *)&hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 2 + PARTION_BASE_SEC];//主分区起始扇区和长度

				p_hd_info -> prime_partion[2].base_sec = *p;	//主分区2的起始扇区
				p_hd_info -> prime_partion[2].num_sec = *(p + 1);	//主分区2的扇区个数

				if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 2 + PARTION_SYSTEM_ID] == PARTION_EXTEND){
					//如果主分区是扩展分区。将要继续递归查找
					get_hd_partion(hd_buff , driver , *p , 2);	//这是递归查找主分区2的逻辑分区
				}

			}

		//PRIME_PARTION 3
		if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 3 + PARTION_SYSTEM_ID] == PARTION_EMPTY){
				//表示主分区0不存在。那么其他主分区肯定也不存在了。
				return 0;
			}else{
				p_hd_info -> prime_partion[0].bootable = hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 3];//主扇区状态

				p = (u32 *)&hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 3 + PARTION_BASE_SEC];//主分区起始扇区和长度

				p_hd_info -> prime_partion[3].base_sec = *p;	//主分区3的起始扇区
				p_hd_info -> prime_partion[3].num_sec = *(p + 1);	//主分区3的扇区个数

				if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 3 + PARTION_SYSTEM_ID] == PARTION_EXTEND){
					//如果主分区是扩展分区。将要继续递归查找
					get_hd_partion(hd_buff , driver , *p , 3);	//这是递归查找主分区3的逻辑分区
				}

			}

	}else{	//考察主分区所有的逻辑分区。注意：逻辑分区与扩展分区的嵌套。一个扩展分区包含一个逻辑分区和一个扩展分区(如果有)

		//LOGIC PARTION
		if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0 + PARTION_SYSTEM_ID] == PARTION_EMPTY){
			//该扩展分区的逻辑分区不存在。那么可以结束搜索了。
			return 0;
		}else{

			//注意p_hd_info -> index_sub[prime_partion]是和传入的主分区号一致的。表示该主分所拥有的逻辑分区
			p_hd_info -> logic_partion[p_hd_info -> index_sub[prime_partion]].bootable =			//逻辑分区的状态
								hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0];



			p = (u32 *)&hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 0 + PARTION_BASE_SEC];//逻辑分区起始扇区和长度

			p_hd_info -> logic_partion[p_hd_info -> index_sub[prime_partion]].base_sec = *p + abs_sec;	//逻辑分区的起始扇区.是相对所在扩展扇区
			p_hd_info -> logic_partion[p_hd_info -> index_sub[prime_partion]].num_sec = *(p + 1);	//逻辑分区的扇区个数

			p_hd_info -> index_sub[prime_partion]++;
		}

		//EXTEND PARTION
		if(hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 1 + PARTION_SYSTEM_ID] == PARTION_EXTEND){	//若是扩展分区。继续搜索
			//注意。一个扩展分区存在情况是：一个逻辑分区+另一个扩展分区；一个逻辑分区；一个扩展分区。所以最多两个分区。
			//扩展分区里的扩展分区与主分区的扩展分区是不同的，它不再算作一个单独分区计算

			p = (u32 *)&hd_buff[PARTION_TABLE_START + PARTION_ITEM_SIZE * 1 + PARTION_BASE_SEC];//扩展分区起始扇区和长度

			get_hd_partion(hd_buff , driver , *p + p_hd_info -> prime_partion[prime_partion].base_sec , prime_partion);
								//这是递归查找主分区prime_partion的逻辑分区
								//注意*p是相对扇区，需要加上所在主分区的base_sec才得到所在的绝对扇区号


		}


	}



}


/*
 * static void print_hd_info(HD_INFO *p_hd_info);
 * 用于打印硬盘分区信息
 */
static void print_hd_info(HD_INFO *p_hd_info){
	int i;



	for(i = 0; i < MAX_PRIME_PARTION; i++){
		if(p_hd_info -> prime_partion[i].num_sec == 0)
			break;
		printf("--------PRIME%d--------/n" , i);
		printf("Able to Boot:  ");
		switch(p_hd_info -> prime_partion[i].bootable){
			case 0x00:
				printf("NO ");
				break;
			case 0x80:
				printf("OK ");
				break;
			default:
				printf("- ");
				break;
		}

		printf("BASE:  %d  " , p_hd_info -> prime_partion[i].base_sec);

		printf("SIZE:  %d/n" , p_hd_info -> prime_partion[i].num_sec);

	}

	//Logic Partion of Prime Partion0
	if((p_hd_info -> index_sub[0]) > 0){

		printf("--------LOGIC in PRIME0--------/n");
		for(i = 0; i < (p_hd_info -> index_sub[0]); i++){

			printf("Able to Boot:  ");
			switch(p_hd_info -> logic_partion[i].bootable){
				case 0x00:
					printf("NO ");
					break;
				case 0x80:
					printf("OK ");
					break;
				default:
					printf("- ");
					break;
			}

			printf("BASE:  %d  " , p_hd_info -> logic_partion[i].base_sec);

			printf("SIZE:  %d/n" , p_hd_info -> logic_partion[i].num_sec);



		}
	}

	//Logic Partion of Prime Partion1
	if((p_hd_info -> index_sub[1]) > 16){
		printf("--------LOGIC in PRIME1--------/n");
		for(i = 16; i < (p_hd_info -> index_sub[1]); i++){

			printf("Able to Boot:  ");
			switch(p_hd_info -> logic_partion[i].bootable){
				case 0x00:
					printf("NO ");
					break;
				case 0x80:
					printf("OK ");
					break;
				default:
					printf("- ");
					break;
			}

			printf("BASE:  %d  " , p_hd_info -> logic_partion[i].base_sec);

			printf("SIZE:  %d/n" , p_hd_info -> logic_partion[i].num_sec);


		}
	}

	//Logic Partion of Prime Partion2
	if((p_hd_info -> index_sub[0]) > 32){
		printf("--------LOGIC in PRIME2--------/n");
		for(i = 32; i < (p_hd_info -> index_sub[2]); i++){

			printf("Able to Boot:  ");
				switch(p_hd_info -> logic_partion[i].bootable){
					case 0x00:
						printf("NO ");
						break;
					case 0x80:
						printf("OK ");
						break;
					default:
						printf("- ");
						break;
				}

			printf("BASE:  %d  " , p_hd_info -> logic_partion[i].base_sec);

			printf("SIZE:  %d/n" , p_hd_info -> logic_partion[i].num_sec);


		}
	}

	//Logic Partion of Prime Partion3
	if((p_hd_info -> index_sub[0]) > 48){
		printf("--------LOGIC in PRIME3--------/n");
		for(i = 48; i < (p_hd_info -> index_sub[3]); i++){

			printf("Able to Boot:  ");
				switch(p_hd_info -> logic_partion[i].bootable){
					case 0x00:
						printf("NO ");
						break;
					case 0x80:
						printf("OK ");
						break;
					default:
						printf("- ");
						break;
				}

			printf("BASE:  %d  " , p_hd_info -> logic_partion[i].base_sec);

			printf("SIZE:  %d/n" , p_hd_info -> logic_partion[i].num_sec);

		}
	}


}






















