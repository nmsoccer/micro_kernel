#include "const.h"
#include "asm_function.h"
#include "process.h"
#include "message.h"
#include "signal.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "assert.h"
#include "fcntl.h"
#include "hd.h"
#include "sys/fs.h"
#include "global.h"
////////////////////全局变量////////////////////////////////////
//const 修饰的是指针地址不是地址指向的内容

SUP_BLOCK * const p_sup_block = (SUP_BLOCK *)0x5FFA00;	//superblock 被加载进内存的位置
unsigned int * const fat32	= (unsigned int *)0x5FFC00;						//fat表被加载进内存的位置。可能不能被完全加载。一次加载一个扇区
INODE * const inode_table = (INODE *)0x5FFE00;	//inode_array被加载进内存的位置。也可能不被完全加载。一次加载一个扇区
u8 * const fs_buff	= (u8 *)0x600000;					//文件系统的缓冲区。用于查找搜索放置数据。不能用于文件数据缓冲。
u8 * const file_buff_in = (u8 *)0x600200;					//文件输入缓冲区（从磁盘到内存）
u8 * const file_buff_out = (u8 *)0x600400;					//文件输出缓冲区（从内存到磁盘）

FD * const fd_table = (FD *)0x600A00;				//放置FD_TABLE的内存位置。预计占据sizeof(FD) * MAX_NR_FD

/////////////////文件内部变量/////////////////////////////////////
static RECORD_IN_MEM record_in_mem;	//记录FAT32和inode_table放入内存的块号

static u32 index_1st_dsec_fat32;		//存储数据的磁盘在FAT32中的起始下标。因为FAT32中起始1+1+2048+1028个项目是不能被文件使用的。
												//它们存储了boot,supblock ，FAT32 , INODE_TABLE
static u32 block_1st_dsec_fat32;		//第一个数据扇区在FAT32的块号

//static u32 dsec_in_mem;					//放入内存的数据扇区号

static char *seg_path[MAX_DEPTH_PATH];		//保存路径中出现的目录名、文件名首地址指针

//////////////////////////////////////////////////////////////////
////////////////////////文件系统管理进程////////////////////////



void task_fs(){
	MSG *p_msg;

	int iresult = 0;	//用于标示结果成功与否
	u32 uresult = 0;

	u32 mode = 0;		//权限

//	u32 i;


//	mkfs();

	init_fs();

	init_fd_table();


//	test();
//	rm_file("/etc/passwd" , 1 , 0);
//	rm_file("/etc" , 1 , 1);
//	create_file("/home" , I_D_RDWT | I_D_RDWT_G | I_D_RDWT_O , USR_PID);
//	rm_file("/home/leiming/.profile" , 1 , 0);

//	pause();
	while(1){
		p_msg = recv_msg();

		if(p_msg == (MSG *)NULL){	//如果消息包为空。重新循环

			continue;
		}
		//消息包非空
		switch(p_msg->type){

			case MSG_CREATE:
				uresult = create_file(p_msg->str_info , (u32)p_msg->int_info[0] , USR_PID);

				if(uresult != 0){	//如果创建成功返回成功信号
					kill(p_msg->send_pid , SIG_SUCCESS);
				}else{	//失败返回失败信号
					kill(p_msg->send_pid , SIG_FAILED);
				}
				del_msg(p_msg);
				break;

			case MSG_REMOVE:
				iresult = rm_file(p_msg->str_info , USR_PID, RM_F);

				if(iresult == 0){	//成功函数返回0
					kill(p_msg->send_pid , SIG_SUCCESS);
				}else{	//失败返回-1
					kill(p_msg->send_pid , SIG_FAILED);
				}

				del_msg(p_msg);
				break;

			case MSG_MKDIR:
				//因为输入的是文件的权限。需要改变为目录的权限。
				mode = (((u32)p_msg->int_info[0]) & (u32)0x0FFF) | ((u32)I_FLAG_D << 12);

				uresult = create_file(p_msg->str_info , mode , USR_PID);

				if(uresult != 0){	//如果创建成功返回成功信号
					kill(p_msg->send_pid , SIG_SUCCESS);
				}else{	//失败返回失败信号
					kill(p_msg->send_pid , SIG_FAILED);
				}	//end if-else

				del_msg(p_msg);
				break;

			case MSG_RMDIR:
				iresult = rm_file(p_msg->str_info , USR_PID , RM_D);

				if(iresult == 0){	//成功函数返回0
					kill(p_msg->send_pid , SIG_SUCCESS);
				}else{	//失败返回-1
					kill(p_msg->send_pid , SIG_FAILED);
				}
				del_msg(p_msg);
				break;

			case MSG_CHMOD:
				//因为输入的是文件的权限。需要改变为目录的权限。
				mode = (((u32)p_msg->int_info[0]) & (u32)0x0FFF) | ((u32)I_FLAG_D << 12);

				iresult = change_mod(p_msg->str_info , mode , USR_PID);

				if(iresult == 0){	//成功函数返回0
					kill(p_msg->send_pid , SIG_SUCCESS);
				}else{	//失败返回-1
					kill(p_msg->send_pid , SIG_FAILED);
				}

				del_msg(p_msg);
				break;

			case MSG_OPEN:
//				printf("PID:%d" , p_msg->send_pid);

				do_open(p_msg->str_info , p_msg->int_info[0] , p_msg->send_pid);
				del_msg(p_msg);
				break;

			case MSG_CLOSE:

				do_close(p_msg->int_info[0] , p_msg->send_pid);
				del_msg(p_msg);
				break;

			case MSG_WRITE:

				iresult = do_write(p_msg->int_info[0] , (const char *)p_msg->int_info[1] , (size_t)p_msg->int_info[2]);
				kill(p_msg->send_pid , iresult);
				del_msg(p_msg);
				break;

			case MSG_READ:

				iresult = do_read(p_msg->int_info[0] , (char *)p_msg->int_info[1] , (size_t)p_msg->int_info[2]);
				kill(p_msg->send_pid , iresult);
				del_msg(p_msg);
				break;

			case MSG_LIST:

				iresult = do_list_dir(p_msg->str_info , p_msg->int_info[0]);
				kill(p_msg->send_pid , iresult);
				del_msg(p_msg);
				break;

			case MSG_CHECK:

				iresult = is_file_dir(p_msg->str_info , p_msg->int_info[0]);
				kill(p_msg->send_pid , iresult);
				del_msg(p_msg);
				break;

			default:
				del_msg(p_msg);
				break;
		}



	}
}


static void test(void){
	char dest[20];
	char *src = "leiming";
	char *c;
	int i;

//	i = is_file_dir("/etc/passwda" , 1);

//	c = strrchr(src , 'l');
//	printf("result is %s" , c);
//	do_list_dir("/home/leiming" , 0);
//	i = search_file("/home/leiming");
//	strncpy(dest , src , strlen(src));
//	strcat(dest , src);
//	dest[20] = (char)0x00;
//	printf("It is %d" , i);


}



//////////////////////////////文件系统封装了消息的接口函数/////////////////////////////








//////////////////////////////文件系统内部专有函数/////////////////////////////////

/*
 *static int mkfs(void);
 * 产生文件系统。
 * 成功返回0失败返回-1
 * 在磁盘还没有文件系统的时候生成。生成之后将不再使用
 */

static int mkfs(void){
	DIR_ENTRY root_entry;
	char *root_name = "/";

//	PASSWD_ITEM item;
//	u32 n_inode;
//	u32 index_inode;

	memset(p_sup_block , 0 , SEC_SIZE , 0);
	memset(fat32 , 0 , SEC_SIZE , 0);
	memset(inode_table , 0 , SEC_SIZE , 0);

	memset(fs_buff , 0 , SEC_SIZE , 0);
	memset(file_buff_in , 0 , SEC_SIZE , 0);
	memset(file_buff_out , 0 , SEC_SIZE , 0);

	record_in_mem.record_fat32 = 0;	//fat32在内存中的扇区序号
	record_in_mem.record_inode_table = 0;	//inode_table在内存中的扇区序号

	hd_open();	//打开磁盘驱动。将分区信息存入内存的HD_INFO结构之中
	wait(SIG_READY);
	printf("Making File System....../n");

//	delay(5000);



	init_sup_block();	//初始化超级块

	//初始化INODE_TABLE
	inode_table[0].i_mode = I_SPECIAL;
	inode_table[1].i_mode = I_SPECIAL;
	inode_table[2].i_mode = I_SPECIAL;
	inode_table[3].i_mode = I_SPECIAL;

	//创造根目录。根目录初始情况下只有一条目录，就是它本身。因此只占用一个扇区。

	//在INODE_TABLE中注册
	inode_table[p_sup_block->n_inode_root].i_mode = (I_D_RDWT | I_D_RDWT_G | I_D_RDWT_O);	//根目录.确定后不再改变
	inode_table[p_sup_block->n_inode_root].n_sec_i = p_sup_block->n_1st_dsec;	//根目录起始扇区。确定后不再改变

	inode_table[p_sup_block->n_inode_root].usr_pid = 0;//以后会动态地改变
	inode_table[p_sup_block->n_inode_root].i_size = DIR_ENTRY_SIZE * 1;//以后会动态地改变

	syn_inode_table();


	//在FAT32中注册扇区的分配。
	index_1st_dsec_fat32 = get_index_fat32(p_sup_block->n_1st_dsec);	//加载含有第一个数据扇区的FAT32块进入内存。并设置起始下标。以后不再改变
	block_1st_dsec_fat32 = record_in_mem.record_fat32;						//设置block。之后不会再改变

	fat32[index_1st_dsec_fat32] = p_sup_block->n_1st_dsec;	//只有一个扇区。因此对应项目填自身
	syn_fat32();

	//在根目录数据区写入第一个目录项
	root_entry.n_inode = p_sup_block->n_inode_root;

	strncpy(root_entry.f_name , root_name , 0);


	memcpy(file_buff_out , (u8 *)&root_entry , DIR_ENTRY_SIZE);
	do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + p_sup_block->n_1st_dsec);

	//创建用户及密码文件/etc/passwd
	create_file("/etc" , I_D_RDWT | I_D_RDWT_G | I_D_RDWT_O , USR_PID);
	delay(500);
	create_file("/etc/passwd" , I_F_RDWT | I_F_RDWT_G | I_F_RDWT_O , USR_PID);

	delay(500);
	//创建主目录/home
	printf("making /home");
	create_file("/home" , I_D_RDWT | I_D_RDWT_G | I_D_RDWT_O , USR_PID);


	pause();
}	//end f




/*
 *static void set_sup_block(void)
 * 初始化SUPERBLOCK
 */
static void init_sup_block(void){

	//设置超级块
	p_sup_block->magic = (u32)0x98;

	p_sup_block->base_sec_fs = hd_info.logic_partion[16].base_sec;
	p_sup_block->nr_sec_fs = hd_info.logic_partion[16].num_sec;

	p_sup_block->n_sec_sup_block = 1;	//扇区0是bootsec

	p_sup_block->n_sec_fat32 = p_sup_block->n_sec_sup_block + 1;	//相对与该文件系统而言。扇区0是bootsec。扇区1是super_block
	p_sup_block->nr_sec_fat32 = (u32)MAX_SEC_FAT32;

	p_sup_block->nr_inode = (u32)MAX_INODE_FS;
	p_sup_block->inode_size = (u32)INODE_SIZE;
	p_sup_block->n_sec_inode_table = (u32)(p_sup_block->n_sec_fat32 + p_sup_block->nr_sec_fat32);	//之前有bootsec，fat32
	p_sup_block->nr_sec_inode_table = (u32)MAX_SEC_INODE_TABLE;

	p_sup_block->n_inode_root = 4;	//根目录的inode序号 0为预留，1，2，3为TTY

	p_sup_block->n_1st_dsec = (u32)(p_sup_block->n_sec_inode_table + p_sup_block->nr_sec_inode_table);	//第一个数据区的相对扇区号
	p_sup_block->dir_entry_size = (u32)DIR_ENTRY_SIZE;

	do_write_hd((u8 *)p_sup_block , 0 , (p_sup_block->base_sec_fs + p_sup_block->n_sec_sup_block));

}	// end f




/*
 * static void init_fd_table(void)
 * 初始化fd_table。文件描述符列表
 */
static void init_fd_table(void){
	int i;
	FD *p_fd = fd_table;	//指向fd_table首地址

	p_fd->n_inode = 1;	//fd0 STDIN
	p_fd++;

	p_fd->n_inode = 2;	//fd1 STDOUT
	p_fd++;

	p_fd->n_inode = 3;	//fd2 STDERR
	p_fd++;


	for(i=3; i<MAX_NR_FD; i++){
		p_fd->n_inode = 0;
		p_fd->flags = 0;
		p_fd->f_pos = 0;
		p_fd->r_pos = 0;
		//不需要初始化f_name
		p_fd->fd_proc_line.head = 0;
		p_fd->fd_proc_line.tail = 0;
		p_fd->fd_proc_line.counter = 0;
		//不需要初始化p_fd->fd_proc_line.pid_line

		p_fd++;

	}	//end f

}	//end f



/*
 *static u32 gain_empty_dsec();
 * 通过扫描FAT32获得一块空闲的数据扇区。
 *
 * 返回值：
 * 成功返回数据扇区号。失败返回0
 */
static u32 gain_empty_dsec(){
	u32 block;	//FAT32的块
	u32 index;	//块的扫描下标

	//从第一个数据区所在块开始搜索
	for(block=block_1st_dsec_fat32; block<MAX_SEC_FAT32; block++){	//FAT32的所有块

		if(block != record_in_mem.record_fat32){	//如果块已经加入内存将不用再加载了
			do_read_hd(fat32 , 0 , p_sup_block->base_sec_fs + p_sup_block->n_sec_fat32 + block);
		}


		if(block == block_1st_dsec_fat32){	//如果是第一数据块。那么搜索起始点是不一样的

			for(index=index_1st_dsec_fat32; index<ITEM_FAT32_PER_SEC; index++){

				if(fat32[index] == 0){	//如果为0。表示可以使用

					return get_n_sec(index);

				}

			}	//end for

			continue;	//第一数据块搜索完毕。没有找到合适的。搜寻下一块

		}	//end if


		for(index=0; index<ITEM_FAT32_PER_SEC; index++){	//不是第一数据块就从下标0开始搜索

			if(fat32[index] == 0){	//如果为0。表示可以使用

				return get_n_sec(index);

			}

		}// end for

		//该数据块搜索完毕。加载下一块继续搜索


	}	//end ouside for


}	//end f



/*
 *static int free_dsec(u32 n_dsec);
 * 清空以n_dsec开始的一连串磁盘扇区。磁盘扇区的数据信息会被清空
 *如果该扇区链接有下一个扇区，那么之后的扇区也将被依次回收，直到最末扇区
 *
 * @param:首扇区号
 *
 * 返回值：
 * 成功放回0，失败返回-1
 *
 */
static int free_dsec(u32 n_dsec){
	u32 index;
	u32 n_dsec_next;	//下一个扇区号。如果有

	//如果删除的不是数据扇区 将发生错误
	if(n_dsec < p_sup_block->n_1st_dsec){
		printf("Illegal Operation!");
		return -1;
	}

	while(1){	//根据FAT32表逐次的删除n_dsec及其后续扇区
		index = get_index_fat32(n_dsec);
		n_dsec_next = fat32[index];

		//删除此扇区
		fat32[index] = 0;	//将对应的置0
		if(syn_fat32() == -1){	//将修改写入FAT32
			printf("Synchronize FAT32 Failed!");
			return -1;
		}
		memset(file_buff_out , 0 , SEC_SIZE , 0);
		if(do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec) == -1){	//将对应磁盘信息清零
			printf("Free Dsec Failed!");
			return -1;
		}

		//查看该扇区是否为最末扇区
		if(n_dsec_next == n_dsec){	//表示是最末一个扇区。删除后返回
			return 0;
		}
		//不是最末的扇区，那么删除下一个扇区
		n_dsec = n_dsec_next;

	}	//end while
/*
	index = get_index_fat32(n_dsec);

	fat32[index] = 0;	//将对应的置0

	if(syn_fat32() == -1){	//将修改写入FAT32
		printf("Synchronize FAT32 Failed!");
		return -1;
	}

	memset(file_buff_out , 0 , SEC_SIZE , 0);

	if(do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec) == -1){	//将对应磁盘信息清零
		printf("Free Dsec Failed!");
		return -1;
	}

	return 0;

*/
}	//end f




/*
 *static int get_index_fat32(u32 n_sec);
 * 获得扇区在FAT32表对应的下标号.
 * @param0:扇区号。注意，是相对于文件系统而言。而不是绝对扇区号
 * 成功返回对应的下标号0~ITEM_FAT32_PER_SEC。失败返回-1
 */
static int get_index_fat32(u32 n_sec){
	u32 n_block = 0;
	u32 abs_sec = 0;

	n_block = n_sec / (ITEM_FAT32_PER_SEC);	//这里为n_sec/128因为每个扇区有128项目。这里求得FAT32的块号

	if(record_in_mem.record_fat32 == n_block){//如果目的块已经在内存了。就不用再读扇区，直接返回序号
		return (n_sec % (ITEM_FAT32_PER_SEC));
	}


	//表示目的块并未在内存之中
	//求绝对扇区号=文件系统的扇区号 + FAT32表在文件系统中的偏移扇区号 + FAT32的块号
	abs_sec = p_sup_block->base_sec_fs + p_sup_block->n_sec_fat32 + n_block;


	if(do_read_hd(fat32 , 0 , abs_sec) == -1){
		return -1;
	}else{
		record_in_mem.record_fat32 = n_block;	//记录放入内存的块号

		return (n_sec % (ITEM_FAT32_PER_SEC));

	}	// end if-else



}	// end f




/*
 *static u32 get_n_sec(u32 index_fat32);
 *获得FAT32下标所指的扇区。注意FAT[INDEX]表示与INDEX所指的扇区有逻辑关系的下一个扇区。区分INDEX所指的扇区与FAT[INDEX]的区别
 *@param0:FAT32的下标
 *返回该下标对应的扇区。
 */
static u32 get_n_sec(u32 index_fat32){

	return ((record_in_mem.record_fat32 * (ITEM_FAT32_PER_SEC)) + index_fat32);


}	// end f



/*
 *static int syn_fat32();
 * 将放入内存的FAT32表某块写入对应的磁盘扇区(同步)
 * 成功返回0.失败返回-1
 */
static int syn_fat32(){
	u32 abs_sec;


	abs_sec = p_sup_block->base_sec_fs + p_sup_block->n_sec_fat32 + record_in_mem.record_fat32;


	return do_write_hd(fat32 , 0 , abs_sec);

} //end f


/*
 *static u32 gain_empty_inode();
 * 获取一个空inode以供使用。并返回对应的n_inode
 *
 * 返回值：
 * 成功返回一个大于0的的n_inode号。
 * 失败返回0
 */
static u32 gain_empty_inode(){
	u32 block;	 //放入内存的块号
	u32 index; //遍历块的指针

	//从第0块开始查找。
	for(block = 0; block<MAX_SEC_INODE_TABLE; block++){	//外层循环用于更新块

		if(block != record_in_mem.record_inode_table){	//如果block就是当前放入内存的块那么不用加载
			do_read_hd(inode_table , 0 , p_sup_block->base_sec_fs + p_sup_block->n_sec_inode_table + block);
			record_in_mem.record_inode_table = block;
		}


		for(index=0; index<INODE_PER_SEC; index++){	//内层循环用于查找每个块。

			if(inode_table[index].i_mode == I_NO_EXIST){	//找到可以利用的n_inode

				return get_n_inode(index);	//通过index获得对应的n_inode号

			}

		}//	end inside for

		//表示上一块已经满了。回到循环首部加载下一块继续查找(如果有)



	}// end ouside for

}	//end f



/*
 *static int del_inode(u32 n_inode);
 * 删除一个inode
 *
 * @param:欲删除的inode的n_inode号
 *
 * 返回值：
 * 成功返回0。失败返回-1
 */
static int del_inode(u32 n_inode){
	u32 index;
	u32 i_mode;

	index = get_index_inode_table(n_inode);

	i_mode = inode_table[index].i_mode;
	if(i_mode == I_SPECIAL){	//特殊文件inode不能被删除
		printf("Can not remove special file inode!");
		return -1;
	}else{
		inode_table[index].i_mode = I_NO_EXIST;
		inode_table[index].n_sec_i = 0;
		inode_table[index].usr_pid = 0;
		inode_table[index].i_size = 0;

		syn_inode_table();	//同步将修改写入磁盘

		return 0;

	}// end if-esle


}	//end f






/*
 *static int get_index_inode_table(u32 n_inode);
 * 获得inode在inode_table表对应的下标号.
 * @param0:inode序号。
 * 成功返回对应的下标号0~INODE_PER_SEC。失败返回-1
 */
static int get_index_inode_table(u32 n_inode){
	u32 n_block = 0;
	u32 abs_sec = 0;

	n_block = n_inode / (INODE_PER_SEC);

	if(record_in_mem.record_inode_table == n_block){//如果目的块已经在内存了。就不用再读扇区，直接返回序号
		return (n_inode % (ITEM_FAT32_PER_SEC));
	}


	//表示目的块并未在内存之中

	abs_sec = p_sup_block->base_sec_fs + p_sup_block->n_sec_inode_table + n_block;


	if(do_read_hd(inode_table , 0 , abs_sec) == -1){
		return -1;
	}else{
		record_in_mem.record_inode_table = n_block;

		return (n_inode % (INODE_PER_SEC));
	}	//end if-else

}	//end f


/*
 *static u32 get_n_inode(u32 index_inode_table);
 *获得对应index_inode_table的n_inode。
 *@param0:inode_table的下标
 *返回该下标对应的n_inode。
 */
static u32 get_n_inode(u32 index_inode_table){

	return (record_in_mem.record_inode_table * (INODE_PER_SEC) + index_inode_table);
}	//end f



/*
 *static int syn_inode_table();
 * 将放入内存的INODE_TABLE表某块写入对应的磁盘扇区(同步)
 * 成功返回0.失败返回-1
 */
static int syn_inode_table(){

	u32 abs_sec;

	abs_sec = p_sup_block->base_sec_fs + p_sup_block->n_sec_inode_table + record_in_mem.record_inode_table;

	return do_write_hd(inode_table , 0 , abs_sec);
}	//end f




/*
 *static int read_dsec(u32 dsec);
 *
 * 将数据扇区的内容读入内存的磁盘缓冲区file_buff_in中
 *
 * @param0:读取的扇区。注意是相对扇区不是绝对扇区
 *
 * 返回值：
 * 成功返回0
 * 失败返回-1
 *
 */
static int read_dsec(u32 dsec){
	int i;
	i = do_read_hd(file_buff_in , 0 , p_sup_block->base_sec_fs + dsec);

	if(i == 0){
		return 0;
	}else{
		printf("Read data sec failed!");
		return -1;
	}

}



/*
 *static int write_dsec(u32 dsec);
 *
 * 将放入内存的数据扇区内容写回磁盘
 * 凡是写回的内容都放在fille_buff_out之中
 *
 *@param0:写入的扇区号。注意是相对扇区不是绝对扇区
 *
 * 返回值：
 * 成功返回0
 * 失败返回-1
 *
 */
static int write_dsec(u32 dsec){
	int i;

	i = do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + dsec);

	if(i == 0){
		return 0;	//成功写入
	}else{
		printf("Write data failed!");
		return -1;
	}

}






////////////
/*
 *static void init_fs();
 * 初始化文件系统。
 * 设置相应内存
 * 加载特殊块进入内存。主要是加载超级块。FAT32数据扇区起始块。INODE_TABLE的起始块等
 */
static void init_fs(){
	memset(p_sup_block , 0 , SEC_SIZE , 0);
	memset(fat32 , 0 , SEC_SIZE , 0);
	memset(inode_table , 0 , SEC_SIZE , 0);
	memset(fs_buff , 0 , SEC_SIZE , 0);
	memset(file_buff_in , 0 , SEC_SIZE , 0);
	memset(file_buff_out , 0 , SEC_SIZE , 0);

	record_in_mem.record_fat32 = 0;	//fat32在内存中的扇区序号
	record_in_mem.record_inode_table = 0;	//inode_table在内存中的扇区序号

	hd_open();	//打开磁盘驱动。将分区信息存入内存的HD_INFO结构之中

	wait(SIG_READY);	//收到信息。表示磁盘驱动已经成功打开。hd_info分区信息已经被正确填写。

	printf("Loading File System....../n");


	do_read_hd(p_sup_block , 0 , hd_info.logic_partion[16].base_sec + 1);	//读入超级块放入内存缓冲区中


	index_1st_dsec_fat32 = get_index_fat32(p_sup_block->n_1st_dsec);	//加载含有第一个数据扇区的FAT32块进入内存。并设置起始下标。以后不再改变
	block_1st_dsec_fat32 = record_in_mem.record_fat32;						//设置block。之后不会再改变

	do_read_hd(inode_table , 0 , (p_sup_block->base_sec_fs + p_sup_block->n_sec_inode_table));	//将INODE_TABLE的第一个块放入内存相应位置.注意此时不用修改相应的内存块


//	dsec_in_mem = 0;	//放入内存的扇区号初始化


}




/*
 *static void sep_path(char *path , char **seg_path , char delim);
 * 路径始终是以/dir/dir/.../file的形式出现的.
 * 解析文件路径。将出现的delim置0x00。并将分割的字符串首地址存入seg_path之中
 * 注意，第一个'/'代表根目录，而其他的'/'代表分隔符。所以第一个'/'将不会被设置为0。系统默认从根目录开始查找
 *
 * @param0:路径
 * @param1:放置路径子字符串首地址的结构
 * @param2:分隔符
 *
 * 返回值为子字符串的个数
 */
static int sep_path(char *path , char **seg_path , char delim){
	int nr_seg = 0;	//记录子字符串的个数
	u32 index  = 0;	//搜索path的下标

	while(1){
		if(path[index] == 0x00){
			return nr_seg;
		}

		if(path[index] != delim){
			index++;
			continue;
		}

		if(path[index] == delim){	// 找到分割符
			path[index] = 0x00;		//将分割符所在位置设置为0x00

			index++;					//考察分隔符后的第一个字符

			if(path[index] != 0x00){	//如果不是0x00则说明存在子字符串，将其放入seg_path之中
				*(seg_path + nr_seg) = &path[index];

				index++;
				nr_seg++;

			}else{						//如果是0x00则说明字符串已经结束
				return nr_seg;
			}

		}


	}


}

/*
 *static u32 search_f_in_dir(const char *f_name , const u32 n_inode_dir);
 * 在指定的目录内寻找文件名为f_name的文件(目录)。
 *
 * @param0:查找的文件名。
 * @param1:目录的n_inode号。
 * *
 * 返回值：
 * 成功返回文件名对应的n_inode号
 * 失败返回0
 */
static u32 search_f_in_dir(const char *f_name , const u32 n_inode_dir){
	DIR_ENTRY *p_dir_entry;	//目录项
	u32 i;						//遍历目录

	u32 index_inode_table;	//目录结点在inode_table中的位置
	u32 n_sec_dir;			//目录的起始扇区

	u32 sec_dir;				//目录所占的扇区号。初始值为n_sec_dir，之后可能会随着读FAT32而改变

	u32 index_fat32;			//目录在FAT32中的位置。是可以动态改变的。初始值为目录初始扇区的标号


	index_inode_table = get_index_inode_table(n_inode_dir);

	//检测是否是目录
	if((inode_table[index_inode_table].i_mode >> 12) & (u32)0x0F != I_FLAG_D){

		printf("It is not a dir");
		return 0;
	}



	n_sec_dir = inode_table[index_inode_table].n_sec_i;

	//循环变量的初始化
	sec_dir = n_sec_dir;
	index_fat32 = get_index_fat32(n_sec_dir);

	while(1){
		memset(fs_buff , 0 , SEC_SIZE , 0);

		do_read_hd(fs_buff , 0 , p_sup_block->base_sec_fs + sec_dir);
//		read_dsec(sec_dir);

		//遍历目录一个扇区的所有DIR_ENTRY（如果有）
		for(i=0; i<NR_ENTRY_PER_SEC; i++){
			p_dir_entry = (DIR_ENTRY *)(fs_buff + (DIR_ENTRY_SIZE) * i);

			//如果某目录项不存在。那么之后的目录项是完全可能存在的。还要继续搜索。直到找到最后一个扇区为止
			if(strcmp(p_dir_entry->f_name , f_name) == 0){	//找到。返回对应的n_inode
				return p_dir_entry->n_inode;
			}

		}	//end for

		//表示目录的上一个扇区没有找到。那么加载下一个扇区(如果有)

		if(fat32[index_fat32] == sec_dir){	//如果sec_dir的下一个扇区就是本身，表示没有下一个扇区了。没有找到返回
			return 0;
		}

		sec_dir = fat32[index_fat32];	//找到下一个扇区的扇区号。
		index_fat32 = get_index_fat32(sec_dir);	//加载下一个扇区所在FAT32的块
	}	//end while

}	//end f




/*
 *static int reg_f_in_dir(const u32 n_inode_f , const char *f_name , const u32 n_inode_dir);
 * 在指定的目录内创建一个指向文件的目录项。
 *
 * @param0:创建文件的n_inode号
 * @param1:创建文件的文件名
 * @param2:目录的n_inode号
 *
 * 返回值：
 * 创建成功返回0 失败返回-1
 */
static int reg_f_in_dir(const u32 n_inode_f , const char *f_name , const u32 n_inode_dir){

	DIR_ENTRY *p_dir_entry;	//目录项
	u32 i;						//遍历目录

	u32 index_inode_table;	//目录结点在inode_table中的位置
	u32 n_sec_dir;			//目录的起始扇区

	u32 sec_dir;				//目录所占的扇区号。初始值为n_sec_dir，之后可能会随着读FAT32而改变

	u32 index_fat32;			//目录在FAT32中的位置。是可以动态改变的。初始值为目录初始扇区的标号


	index_inode_table = get_index_inode_table(n_inode_dir);

	//检测是否是目录
	if((inode_table[index_inode_table].i_mode >> 12) & (u32)0x0F != I_FLAG_D){

		printf("It is not a dir");
		return -1;
	}

	n_sec_dir = inode_table[index_inode_table].n_sec_i;

	//循环变量的初始化
	sec_dir = n_sec_dir;
	index_fat32 = get_index_fat32(n_sec_dir);



	while(1){
		memset(fs_buff , 0 , SEC_SIZE , 0);

		do_read_hd(fs_buff , 0 , p_sup_block->base_sec_fs + sec_dir);
//		read_dsec(sec_dir);


		//遍历目录一个扇区的所有DIR_ENTRY（如果有）
		for(i=0; i<NR_ENTRY_PER_SEC; i++){
			p_dir_entry = (DIR_ENTRY *)(fs_buff + (DIR_ENTRY_SIZE) * i);



			if(p_dir_entry-> n_inode == 0){	//表示该目录项不存在。那么可以注册

				p_dir_entry->n_inode = n_inode_f;

				strncpy(p_dir_entry->f_name , f_name , 0);	//注意是没有拷贝最后的0x00

				do_write_hd(fs_buff , 0 , p_sup_block->base_sec_fs + sec_dir);

				//还要修改目录的inode内容。因为大小发生了改变
				inode_table[index_inode_table].i_size += (DIR_ENTRY_SIZE);
				syn_inode_table();
				return 0;
			}else{

				if(strcmp(p_dir_entry->f_name , f_name) == 0)	{//找到。表示文件已经存在。
					printf("File %s is existed!" , f_name);
					return -1;
				}

			}	//end if-else

		}	//end for


		//表示目录的上一个扇区没有找到空的项目。那么加载下一个扇区(如果没有就分配)

		if(fat32[index_fat32] == sec_dir){	//如果sec_dir的下一个扇区就是本身，表示没有下一个扇区了。
			//分配一个扇区
			sec_dir = gain_empty_dsec();
			fat32[index_fat32] = sec_dir;	//目前的index指向新分配的扇区
			syn_fat32();

			index_fat32 = get_index_fat32(sec_dir);
			continue;
		}

		sec_dir = fat32[index_fat32];	//找到下一个扇区的扇区号。
		index_fat32 = get_index_fat32(sec_dir);	//加载下一个扇区所在FAT32的块


	}	//end while

}

/*
 *static u32 del_f_in_dir(const char *f_name , const u32 n_inode_dir);
 * 在指定的目录内删除一个文件的目录项。
 *
 * @param0:欲文件的文件名
 * @param1:目录的n_inode号
 *
 * 返回值：
 * 删除成功返回文件的n_inode号。 失败返回0
 */
static u32 del_f_in_dir(const char *f_name , const u32 n_inode_dir){

	DIR_ENTRY *p_dir_entry;	//目录项
	u32 i;						//遍历目录

	u32 index_inode_table;	//目录结点在inode_table中的位置
	u32 n_sec_dir;			//目录的起始扇区

	u32 sec_dir;				//目录所占的扇区号。初始值为n_sec_dir，之后可能会随着读FAT32而改变

	u32 index_fat32;			//目录在FAT32中的位置。是可以动态改变的。初始值为目录初始扇区的标号

	u32 n_inode_f;			//文件的n_inode号

	index_inode_table = get_index_inode_table(n_inode_dir);

	//检测是否是目录
	if((inode_table[index_inode_table].i_mode >> 12) & (u32)0x0F != I_FLAG_D){

		printf("It is not a dir");
		return 0;

	}

	n_sec_dir = inode_table[index_inode_table].n_sec_i;

	//循环变量的初始化
	sec_dir = n_sec_dir;
	index_fat32 = get_index_fat32(n_sec_dir);



	while(1){
		memset(fs_buff , 0 , SEC_SIZE , 0);

		do_read_hd(fs_buff , 0 , p_sup_block->base_sec_fs + sec_dir);

		//遍历目录一个扇区的所有DIR_ENTRY（如果有）
		for(i=0; i<NR_ENTRY_PER_SEC; i++){
			p_dir_entry = (DIR_ENTRY *)(fs_buff + (DIR_ENTRY_SIZE) * i);

			//如果某目录项不存在。那么之后的目录项是完全可能存在的。还要继续搜索。直到找到最后一个扇区为止

			if(strcmp(p_dir_entry->f_name , f_name) == 0){	//找到。删除之。

				n_inode_f = p_dir_entry->n_inode;	//保存文件的n_inode号

				memset(p_dir_entry , 0 , DIR_ENTRY_SIZE , 0);	//将此目录项目全部置0
				do_write_hd(fs_buff , 0 , p_sup_block->base_sec_fs + sec_dir);	//重新写入磁盘相应位置。

				//还要修改目录的inode内容。因为大小发生了改变
				inode_table[index_inode_table].i_size -= (DIR_ENTRY_SIZE);
				syn_inode_table();

				return n_inode_f;

			}


		}	//end for


		//表示目录的上一个扇区没有找到。那么加载下一个扇区(如果有)

		if(fat32[index_fat32] == sec_dir){	//如果sec_dir的下一个扇区就是本身，表示没有下一个扇区了。没有找到返回
			return 0;
		}

		sec_dir = fat32[index_fat32];	//找到下一个扇区的扇区号。
		index_fat32 = get_index_fat32(sec_dir);	//加载下一个扇区所在FAT32的块


	}	//end while


}










/*
 *static u32 create_file(char *abs_path , u32 mode , int usr_pid)
 * 创建文件。
 *
 * @param0:绝对路径。已经包含了文件名。
 * @param1:模式。比如设定是只读文件或者目录等
 * @param2:创建者的进程号
 *
 * 返回值：
 * 成功返回创建文件的n_inode，失败返回0
 */
static u32 create_file(char *abs_path , u32 mode , int usr_pid){
	int nr_seg;	//子目录（文件）的个数。也是目录嵌套的层数

	u32 n_inode_d;
	u32 n_inode_f;
	u32 index_inode_f;

	u32 n_dsec;	//分配给文件的扇区号
	u32 index_fat32;	//分配扇区在FAT32的下标

	DIR_ENTRY *p_dir_entry;	//如果建立的是目录。需要初始化两个目录项: .与..

	int i;

	nr_seg = sep_path(abs_path , seg_path , '/');

	if(nr_seg == 0){	//表示只有根目录。不用再创建了。形如"/"
		printf("Root Dir is existed!");
		return 0;
	}

	//初始化循环变量
	n_inode_d = p_sup_block->n_inode_root;	//总是从根目录开始搜索

	for(i=0; i<nr_seg; i++){

		if(i != (nr_seg -1)){	//文件之前的所有目录
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			if(n_inode_f == 0){	//表示该子目录不存在。退出
				printf("%s is not existed!" , seg_path[i]);
				return 0;
			}

			//子目录存在.那么作为下一次查找的父目录
			n_inode_d = n_inode_f;

			continue;

		}	//end if

		if(i == (nr_seg - 1)){	//表示文件名
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			//目标文件存在。不能创建
			if(n_inode_f != 0){
				printf("%s is existed HAHA" , seg_path[i]);
				return 0;
			}

			//目标文件不存在。可以创建

			//开始创建

			//首先获取一个空白扇区。然后注册到FAT32表之中。注意，创建文件开始都是分配一个扇区。
			n_dsec = gain_empty_dsec();
			index_fat32 = get_index_fat32(n_dsec);
			fat32[index_fat32] = n_dsec;
			syn_fat32();

			//然后寻找一个空白的inode并获取n_inode
			n_inode_f = gain_empty_inode();
			index_inode_f = get_index_inode_table(n_inode_f);
				//填充相关的信息
			inode_table[index_inode_f].i_mode = mode;
			inode_table[index_inode_f].n_sec_i = n_dsec;
			inode_table[index_inode_f].usr_pid = (u32)usr_pid;

			if(((mode >> 12) & 0x0000000F) == I_FLAG_D){	//如果是目录
				inode_table[index_inode_f].i_size = (DIR_ENTRY_SIZE) * 2;
			}else{
				inode_table[index_inode_f].i_size = 0;
			}
			syn_inode_table();

			//在父目录里注册文件
			reg_f_in_dir(n_inode_f , seg_path[i] , n_inode_d);

			//如果建立的是目录那么还需要初始化两个dir_entry . 与 ..
			if(((mode >> 12) & 0x0000000F) == I_FLAG_D){
//				do_read_hd(fs_buff , 0 , p_sup_block->base_sec_fs + n_dsec);//将文件的扇区读入fs_buff
				//因为目的扇区空白所以不需要读只需要写
				memset(fs_buff , 0 , SEC_SIZE , 0);
				p_dir_entry = (DIR_ENTRY *)fs_buff;

				//建立 .
				p_dir_entry->n_inode = n_inode_f;
				strncpy(p_dir_entry->f_name , "." , 0);

				p_dir_entry++;
				//建立..
				p_dir_entry->n_inode = n_inode_d;

				strncpy(p_dir_entry->f_name , ".." , 0);

				do_write_hd(fs_buff , 0 , p_sup_block->base_sec_fs + n_dsec);
			}

			return n_inode_f;

		}	//end if


	}	//end for


}	//end f


/*
 *static u32 search_file(char *abs_path)
 *根据文件名搜寻文件（目录）
 *
 *@param:文件（目录）绝对路径
 *
 *返回值：
 *成功返回目标文件（目录）的INODE号。失败返回0
 */
static u32 search_file(char *abs_path){
	int nr_seg;	//子目录（文件）的个数。也是目录嵌套的层数

	u32 n_inode_d;
	u32 n_inode_f;

	int i;

	nr_seg = sep_path(abs_path , seg_path , '/');

	if(nr_seg == 0){	//表示只查找到根目录。形如"/"
//		printf("Root Dir is existed!");
		return p_sup_block->n_inode_root;
//		return 0;
	}

	//初始化循环变量
	n_inode_d = p_sup_block->n_inode_root;	//总是从根目录开始搜索

	for(i=0; i<nr_seg; i++){

		if(i != (nr_seg -1)){	//文件之前的所有目录
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			if(n_inode_f == 0){	//表示该子目录不存在。退出
				printf("dir %s is not existed!" , seg_path[i]);
				return 0;
			}

			//子目录存在.那么作为下一次查找的父目录
			n_inode_d = n_inode_f;

			continue;

		}	//end if

		if(i == (nr_seg - 1)){	//表示文件(目录)名
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			//存在目标文件(目录)。返回n_inode号
			if(n_inode_f != 0){
				return n_inode_f;
			}

			//目标文件(目录)不存在。
			printf("file %s is not existed" , seg_path[i]);
			return 0;


		}	//end if


	}	//end for



}	//end f




/*
 * static int delete_file(const char *f_name , u32 n_inode_parent , int usr_pid)
 * 删除文件。
 *
 * @param0:文件名。
 * @param1:文件父目录n_inode号
 * @param2:想要删除该文件的用户PID
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
static int delete_file(const char *f_name , u32 n_inode_parent , int usr_pid){
	u32 n_inode;	//文件的n_inode号
	u32 index_inode;
	u32 index_fat32;
	u32 n_dsec;

//	u32 temp;	//记录临时变量

	n_inode = search_f_in_dir(f_name , n_inode_parent);	//获取文件的n_inode号。同时删除在父目录的目录项

	if(!n_inode){	//如果n_inode等于零。表示文件不存在

		printf("%s is not existed!" , f_name);
		return -1;
	}

	index_inode = get_index_inode_table(n_inode);


	//如果不是普通文件无法删除
	if(((inode_table[index_inode].i_mode >> 12) & 0x0000000F) != I_FLAG_F){
		printf("%s is not a file!" , f_name);
		return -1;
	}

	//如果是只读文件无法删除。 文件的只读只写只执行属性只由拥有者的一组属性决定
	//就是说，如果其他用户将该文件设为只读是不能阻止该文件被删除。
	if(((inode_table[index_inode].i_mode) & 0xFF00) == I_F_RD_ONLY){
		printf("Can not remove read-only file!");
		return -1;
	}

	//如果想要删除文件的非创始用户.要检查其他用户对文件是否具有写权限。
	if((inode_table[index_inode].usr_pid) != usr_pid){
		//如果组用户有写权限可以删除
		if((inode_table[index_inode].i_mode & (u32)I_FLAG_WO_G) != 0){
			goto del_file;
		}
		//如果其他用户有写权限可以删除
		if((inode_table[index_inode].i_mode & (u32)I_FLAG_WO_O) != 0){
			goto del_file;
		}

		//没有写权限无法删除
		printf("You have not right to write!");
		return -1;
	}

del_file:
	n_dsec = inode_table[index_inode].n_sec_i;//获取文件的数据起始扇区

	assert(del_inode(n_inode) == 0);	//删除该文件在inode_table之中的项目

	del_f_in_dir(f_name , n_inode_parent);	//在父目录中注销

	//删除以n_dsec开始的数据扇区链直到最末的扇区
	return free_dsec(n_dsec);
/*
	while(1){
		index_fat32 = get_index_fat32(n_dsec);

		//如果该扇区为文件最后一个扇区。删除之然后退出
		if(fat32[index_fat32] == n_dsec){

			return free_dsec(n_dsec);

		}

		temp = fat32[index_fat32];	//下一个扇区保存到临时变量。

		//删除该扇区。
		free_dsec(n_dsec);

		//取得下一个扇区号
		n_dsec = temp;


	}	//end while
*/

}	//end f



/*
 * static int delete_dir(const char *dir_name , u32 n_inode_parent , int usr_pid)
 * 删除一个目录。
 * 注意目录必须为空。
 *
 * @param0:目录名。
 * @param1:目录父目录n_inode号
 *	@param2:删除目录的用户PID
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
static int delete_dir(const char *dir_name , u32 n_inode_parent , int usr_pid){
	u32 n_inode;
	u32 index_inode;
	u32 n_dsec;

	n_inode = search_f_in_dir(dir_name , n_inode_parent);	//获取目录的n_inode号

	if(!n_inode){	//目录不存在

		printf("%s is not existed!" , dir_name);
		return -1;
	}

	index_inode = get_index_inode_table(n_inode);

	//如果不是目录无法删除。
	if(((inode_table[index_inode].i_mode >> 12) & 0x0000000F) != I_FLAG_D){
		printf("%s is not a directory!" , dir_name);
		return -1;
	}


	//如果目录非空。不能删除。注意如果只有.与..是可以删除的
	if(inode_table[index_inode].i_size > ((DIR_ENTRY_SIZE) * 2)){
		printf("Dir is not empty!");
		return -1;
	}


	//如果是只读目录无法删除。 目录的只读只写属性只由拥有者的一组属性决定
	//就是说，如果其他用户将该目录设为只读是不能阻止该目录被删除。
	if(((inode_table[index_inode].i_mode) & 0xFF00) == I_D_RD_ONLY){
		printf("Cant not remove read-only dir!");
		return -1;
	}

	//如果想要删除目录的是非创始用户.要检查其他用户对目录是否具有写权限。
	if((inode_table[index_inode].usr_pid) != usr_pid){
		//如果组用户有写权限可以删除
		//因为写权限是bit1  0010
		//检测写位是否为0
		if(((inode_table[index_inode].i_mode) & (u32)I_FLAG_WO_G) != 0){
			goto del_dir;
		}
		//如果其他用户有写权限可以删除
		if((inode_table[index_inode].i_mode & (u32)I_FLAG_WO_O) != 0){
			goto del_dir;
		}

		//没有写权限无法删除
		printf("You have not right to write!");
		return -1;
	}

del_dir:


	n_dsec = inode_table[index_inode].n_sec_i;//获取目录的数据起始扇区

	assert(del_inode(n_inode) == 0);	//删除该目录在inode_table之中的项目

	del_f_in_dir(dir_name , n_inode_parent);	//删除在父目录的项

	//因为只有一个扇区。删除之
	return free_dsec(n_dsec);

}	//end f




/*
 * static int rm_file(char *abs_path , int usr_pid , int opt);
 * 删除文件。
 *
 * @param0:文件的绝对路径
 * @param1:删除文件的用户ID
 *	@param2:选项。0删除文件。1删除目录
 *
 * 返回值：
 * 成功返回0.
 * 失败返回-1
 */
static int rm_file(char *abs_path , int usr_pid , int opt){
	int nr_seg;	//子目录（文件）的个数。也是目录嵌套的层数

	u32 n_inode_d;
	u32 n_inode_f;

//	u32 index_inode;

	int i;

	nr_seg = sep_path(abs_path , seg_path , '/');

	if(nr_seg == 0){	//表示只有根目录。不能删除。形如"/"
		printf("Root Dir can't be removed!");
		return -1;
	}

	//初始化循环变量
	n_inode_d = p_sup_block->n_inode_root;	//总是从根目录开始搜索

	for(i=0; i<nr_seg; i++){

		if(i != (nr_seg -1)){	//文件之前的所有目录
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			if(n_inode_f == 0){	//表示该子目录不存在。退出
				printf("Dir %s is not existed!" , seg_path[i]);
				return -1;
			}

			//子目录存在.那么作为下一次查找的父目录
			n_inode_d = n_inode_f;

			continue;

		}	//end if

		if(i == (nr_seg - 1)){	//表示文件名

			if(opt == RM_F){	//如果删除的是文件

				return delete_file(seg_path[i] , n_inode_d , usr_pid);

			}
			if(opt == RM_D){	//如果删除的是目录

				return delete_dir(seg_path[i] , n_inode_d , usr_pid);

			}

			return -1;
		}


	}	//end for


}	//end f



/*
 * static int change_mod(char *abs_path , u32 mode , int usr_pid);
 * 更改文件(目录)权限
 *
 * @param0:文件（目录）的绝对路径
 * @param1:修改成的权限。I_FLAG_RO/I_FLAG_WO/I_FLAG_RW I_FLAG_RO_G/I_FLAG_WO_G/I_FLAG_RW_G
 * I_FLAG_RO_O/I_FLAG_WO_O/I_FLAG_RW_O
 * @param2:试图修改权限的用户ID（非属主无法修改）
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
static int change_mod(char *abs_path , u32 mode , int usr_pid){
	int nr_seg;	//子目录（文件）的个数。也是目录嵌套的层数

	u32 n_inode_d;
	u32 n_inode_f;

	u32 index_inode;

	int i;

	nr_seg = sep_path(abs_path , seg_path , '/');

	if(nr_seg == 0){	//表示只有根目录。不能改变权限。形如"/"
		printf("Root Dir can't be removed!");
		return -1;
	}

	//初始化循环变量
	n_inode_d = p_sup_block->n_inode_root;	//总是从根目录开始搜索

	for(i=0; i<nr_seg; i++){

		if(i != (nr_seg -1)){	//文件之前的所有目录
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			if(n_inode_f == 0){	//表示该子目录不存在。退出
				printf("Dir %s is not existed!" , seg_path[i]);
				return -1;
			}

			//子目录存在.那么作为下一次查找的父目录
			n_inode_d = n_inode_f;

			continue;

		}	//end if

		if(i == (nr_seg - 1)){	//表示文件名
			n_inode_f = search_f_in_dir(seg_path[i] , n_inode_d);

			if(n_inode_f == 0){	//文件不存在
				printf("%s is not existed!" , seg_path[i]);
				return -1;
			}
			//文件存在。准备修改权限

			index_inode = get_index_inode_table(n_inode_f);

			if((inode_table[index_inode].usr_pid) == (u32)usr_pid){
				//只有属主可以修改

				//如果0~3位不为0。增加其他用户权限
				if((mode & (u32)I_MASK_OTH) != 0){
					inode_table[index_inode].i_mode = ((inode_table[index_inode].i_mode & (u32)0xFFF0) | (mode & 0x000F));
					mode = mode & 0xFFF0;//将mode相应位清零
				}
				//如果4～7位不为0。增加组用户权限
				if((mode & (u32)I_MASK_GRP) != 0){
					inode_table[index_inode].i_mode = ((inode_table[index_inode].i_mode & (u32)0xFF0F) | (mode & 0x00F0));
					mode = mode & 0xFF0F;//将mode相应位清零
				}
				//如果8～11位不为0。增加用户权限
				if((mode & (u32)I_MASK_USR) != 0){
					inode_table[index_inode].i_mode = ((inode_table[index_inode].i_mode & (u32)0xF0FF) | (mode & 0x0F00));

				}


//				inode_table[index_inode].i_mode = I_F_RDWT;
				syn_inode_table();

				return 0;
			}
			//非属主无法修改
			printf("sorry!You are not the owner!");
			return -1;

		}	//end if


	}	//end for


}	//end f



/*
 * static int gain_empty_fd();
 * 从fd_table获得一个空闲的fd号
 *
 * 返回值：
 * 成功返回fd号，失败返回-1
 */
static int gain_empty_fd(){
	int i;
	FD *p_fd = fd_table;

	//前面三个已经被占据了 无法更改
	for(i=0; i<MAX_NR_FD; i++){

		if(p_fd->n_inode == 0){	//如果p_fd->n_inode为0表示没有占用
			return i;
		}

		p_fd++;

	}	//end for
	return -1;
}	//end f


/*
 * static int gain_fd_by_name(char *abs_path)
 * 根据文件的绝对路径获取文件在fd_table中的fd号。
 * 如果文件存在但没有fd号将在fd_table中选择一个号返回。
 *
 * @param:文件绝对路径
 *
 * 返回值：
 * 成功返回文件在fd_table中的fd号。失败返回-1
 */
static int gain_fd_by_name(char *abs_path){
	int i;
	u32 n_inode;

	//获取文件对应n_inode号
	n_inode = search_file(abs_path);

//	printf("n_inode%d" , n_inode);

	//如果文件本来不存在。返回-1
	if(n_inode == 0){
		return -1;
	}

	//文件是存在的。考察是否已经打开(在fd_table中注册)
	for(i=0; i<MAX_NR_FD; i++){

		//找到对应的n_inode号返回。
		if(fd_table[i].n_inode == n_inode){
			return i;
		}

	}	//end for

	//没有对应的号,那么在fd_table之中选择一个赋予该文件
	i = gain_empty_fd();

	if(i != -1){	//如果找到了空的fd号
		fd_table[i].n_inode = n_inode;
		return i;
	}

	return -1;

}	//end f


/*
 * static int do_open(char *abs_path , int flags , int pid);
 * 打开文件。
 *
 * @param0:文件路径。
 * @param1:打开标志
 * @param2:打开文件进程号
 *
 * 返回值：
 * 成功返回0，失败返回-1
 */
static int do_open(char *abs_path , int flags , int pid){
	int fd;
	FD * p_fd;
	u32 index_inode;	//文件n_inode在inode_table中的标号
	u32 index_fat32;	//文件首扇区的FAT32编号

	fd = gain_fd_by_name(abs_path);

//	printf("FD is:%d" , fd);
//	printf("NICE!");
	if(fd == -1){	//寻找文件失败
//		printf("FAILED@");
		kill(pid , SIG_FAILED);
		return -1;

	}

	p_fd = &fd_table[fd];

	//检测等待的队列是否超过最大值，否则将进程号与标志入队列
	if(p_fd->fd_proc_line.counter >= FD_WAIT_PID){
		printf("Beyond the max queue of fd");
		kill(pid , SIG_FAILED);
		return -1;
	}

	index_inode = get_index_inode_table(p_fd->n_inode);	//获取文件的index_inode
	//根据flags与pid进行文件权限检查
	if(pid == inode_table[index_inode].usr_pid){	//如果是文件的创建者

		if((flags & O_RDWR_MASK) == O_RDONLY){	//如果以只读方式打开文件

			if((inode_table[index_inode].i_mode & I_MASK_USR) == I_FLAG_WO){	//只读方式是不能打开只写文件的
				printf("Can not open write-only file using read-only method!");
				kill(pid , SIG_FAILED);
				return -1;
			}//可以读只执行文件

		}else if((flags & O_RDWR_MASK) == O_WRONLY){//以只写方式打开

			if((inode_table[index_inode].i_mode & I_MASK_USR) == I_FLAG_RO){	//只写方式是不能打开只读文件的
				printf("Can not open read-only file using write-only method!");
				kill(pid , SIG_FAILED);
				return -1;
			}//可以写只执行文件

		}//以读写方式打开则任何情况都满足

	}else{	//不是文件的创建者

		if((inode_table[index_inode].i_mode & I_MASK_OTH) == 0){	//表示并没有设定其他的权限
			printf("Can not open file which offers no rights to others");
			kill(pid , SIG_FAILED);
			return -1;
		}

		if((flags & O_RDWR_MASK) == O_RDONLY){	//如果以只读方式打开文件

			if((inode_table[index_inode].i_mode & I_MASK_OTH) == I_FLAG_WO_O){	//只读方式是不能打开只写文件的
				printf("Can not open write-only file using read-only method!");
				kill(pid , SIG_FAILED);
				return -1;
			}//可以读只执行文件

		}else if((flags & O_RDWR_MASK) == O_WRONLY){//以只写方式打开

			if((inode_table[index_inode].i_mode & I_MASK_OTH) == I_FLAG_RO_O){	//只写方式是不能打开只读文件的
				printf("Can not open read-only file using write-only method!");
				kill(pid , SIG_FAILED);
				return -1;
			}//可以写只执行文件

		}//以读写方式打开则任何情况都满足

	}	//end if-else




	//如果有写入标志而没有指定写入方式那么打开错误
	if(((flags&O_RDWR_MASK)!=O_RDONLY) && ((flags&O_POS_MASK)!=O_APPEND) &&((flags&O_POS_MASK)!=O_TRUNC)){
		printf("Can not open without writing method");
		kill(pid , SIG_FAILED);
		return -1;
	}


	p_fd->fd_proc_line.item_fd_line[p_fd->fd_proc_line.head].pid = pid;
	p_fd->fd_proc_line.item_fd_line[p_fd->fd_proc_line.head].flags = flags;
	p_fd->fd_proc_line.counter++;
	p_fd->fd_proc_line.head = (p_fd->fd_proc_line.head + 1) % FD_WAIT_PID;

	//如果这是第一个打开文件的进程，将该进程唤醒。否则挂起直到close函数唤醒
	if(p_fd->fd_proc_line.counter == 1){

		//根据flags设定文件的读写位置
		//设定读位置为文件头
		p_fd->r_pos = 0;

		//根据flags设定写位置
		if(((flags & O_POS_MASK) == O_APPEND) && ((flags & O_RDWR_MASK) != O_RDONLY)){	//在文件末尾添加
			p_fd->f_pos = inode_table[index_inode].i_size;	//文件大小正表示文件的末尾

		}else if(((flags & O_POS_MASK) == O_TRUNC) && ((flags & O_RDWR_MASK) != O_RDONLY)){
			//从文件头打开并且以可写的方式打开文件。文件以前的内容作废
			inode_table[index_inode].i_size = 0;

			//只保留该文件的首扇区。删除后续扇区（如果有）
			index_fat32 = get_index_fat32(inode_table[index_inode].n_sec_i);
			if(fat32[index_fat32] != inode_table[index_inode].n_sec_i){	//如果还有后续扇区那么删除以下一个扇区开始的扇区链
				free_dsec(fat32[index_fat32]);
				index_fat32 = get_index_fat32(inode_table[index_inode].n_sec_i);
				fat32[index_fat32] = inode_table[index_inode].n_sec_i;	//因为后续扇区已经被删除了。所以该扇区作为末扇区
				printf("FREE%d" , fat32[index_fat32]);
				syn_fat32();

			}
			//因为上一个函数会改变fat32缓冲区


			p_fd->f_pos = 0;
		}else{	//在这种情况下只有O_RDONLY。
			p_fd->f_pos = inode_table[index_inode].i_size;	//由此来标志一个文件的末尾
		}
		p_fd->flags = flags;
		kill(pid , fd);
	}

}	//end f



/*
 * int do_close(int fd , int pid);
 *
 * 关闭文件。
 * 如果关闭文件之后还有进程处于打开队列之中将会唤醒下一个进程
 * 如果没有进程了将撤销该fd
 *
 *@param0:通过open返回的文件描述符
 *@param1:试图关闭文件的进程pid
 *同时，如果打开标志有写入标志同时要求更新相关文件的inode的f_pos标志
 *
 * 返回值：
 * 成功返回0失败返回－1
 */
static int do_close(int fd , int pid){
	FD *p_fd;
	u32 index_inode;
	u32 index_fat32;

	p_fd = &fd_table[fd];

	//如果申请关闭文件的进程不是当前打开文件的进程，则关闭失败
	if(p_fd->fd_proc_line.item_fd_line[p_fd->fd_proc_line.tail].pid != pid){
		kill(pid , SIG_FAILED);
		return -1;
	}

	index_inode = get_index_inode_table(p_fd->n_inode);	//获取文件n_inode的序号

	//如果打开标志有写标志，那么需要更新文件的大小即文件末尾
	if((p_fd->flags & O_RDWR_MASK) != O_RDONLY){
//		index_inode = get_index_inode_table(p_fd->n_inode);
		inode_table[index_inode].i_size = p_fd->f_pos;
		syn_inode_table();
	}

	kill(pid , SIG_SUCCESS);	//向申请进程发送成功的信号。表示关闭成功

	p_fd->fd_proc_line.counter--;
	p_fd->fd_proc_line.tail = (p_fd->fd_proc_line.tail + 1) % FD_WAIT_PID;

	if(p_fd->fd_proc_line.counter == 0){	//如果只有这一个进程，将销毁fd
		//销毁fd
		fd_table[fd].n_inode = 0;
		return 0;
	}

	//如果还有打开文件的进程阻塞就唤醒下一个进程
	p_fd->flags = p_fd->fd_proc_line.item_fd_line[p_fd->fd_proc_line.tail].flags;


	//根据flags设定文件的读写位置
	//设定读位置为文件头
	p_fd->r_pos = 0;

	//根据flags设定写位置
	if(((p_fd->flags & O_POS_MASK) == O_APPEND) && ((p_fd->flags & O_RDWR_MASK) != O_RDONLY)){	//在文件末尾添加
//		index_inode = get_index_inode_table(p_fd->n_inode);
		p_fd->f_pos = inode_table[index_inode].i_size;	//文件大小正表示文件的末尾

	}else if(((p_fd->flags & O_POS_MASK) == O_TRUNC) && ((p_fd->flags & O_RDWR_MASK) != O_RDONLY)){
		//从文件头打开并且以可写的方式打开文件。文件以前的内容作废
//		index_inode = get_index_inode_table(p_fd->n_inode);
		inode_table[index_inode].i_size = 0;

		//只保留该文件的首扇区。删除后续扇区（如果有）
		index_fat32 = get_index_fat32(inode_table[index_inode].n_sec_i);
		if(fat32[index_fat32] != inode_table[index_inode].n_sec_i){	//如果还有后续扇区那么删除以下一个扇区开始的扇区链
			free_dsec(fat32[index_fat32]);
		}
		fat32[index_fat32] = inode_table[index_inode].n_sec_i;	//因为后续扇区已经被删除了。所以该扇区作为末扇区

		p_fd->f_pos = 0;
	}else{	//就只有O_RDONLY
		p_fd->f_pos = inode_table[index_inode].i_size;	//由此来标志一个文件的末尾
	}


	//因为此时进程陷于阻塞，发送信号将其激活。注意fd作为信号量内容传给了目标进程
	kill(p_fd->fd_proc_line.item_fd_line[p_fd->fd_proc_line.tail].pid , fd);
	return 0;
}




/*
 * static u32 get_n_dsec(u32 n_inode , u32 n_fsec);
 *
 * 根据文件内容的扇区号得到该扇区所在的数据扇区号
 *
 *@param0:文件的inode号
 *@param1:文件内容的扇区号。（文件所占用的扇区数从0开始编号）
 *
 * 返回值：
 * 成功返回数据扇区号，失败返回0
 */
static u32 get_n_dsec(u32 n_inode , u32 n_fsec){
	u32 i;
	u32 n_dsec;
//	u32 n_dsec_next;
	int index_fat32;

	u32 index_inode;

	index_inode = get_index_inode_table(n_inode);
	n_dsec = inode_table[index_inode].n_sec_i;	//获取文件起始扇区
	i = 0;

	while(1){
		if(i > n_fsec){	//如果i大于n_fsec表示出现了错误
			return 0;
		}


		if(i == n_fsec){	//直到到达文件内容扇区号 返回其数据扇区号
			return n_dsec;
		}

		//如果i小于n_fsec 那么取下一个扇区号
		index_fat32 = get_index_fat32(n_dsec);
		n_dsec = fat32[index_fat32];
		i++;
	}	//end while

}	//end f






/*
 * static ssize_t do_write(int fd , const char *buff , size_t count);
 *
 * 往打开的文件的末尾里面写入buff之中共count个字节
 * count如果为0则不起作用
 *
 *@param0:已经打开的文件描述符
 *@param1:调用该函数的进程所提供的缓冲区
 *@param2:试图写入的字节数
 *@param3:调用函数的进程PID
 *
 *返回值：
 *成功则返回实际写入的字节数。失败返回－1
 *
 */
static ssize_t do_write(int fd , const char *buff , size_t count){
	u32 old_size;		//最初的文件大小

	u32 index_buff;	//用于指示buff的标志符.始终指向下一个要写入的字符
	u32 start_writing;	//用于标识将buff写入文件最末扇区的起始位置
								//注意，当以O_TRUNC方式打开文件时首扇区就是最末扇区，而分配给文件的其他扇区将被回收)
	u32 f_pos;	//文件结尾的标识也是文件的大小（B）每当写入时将会发生改变，在函数返回前赋值给fd
	u32 n_dsec;	//文件最末一个数据扇区号
	u32 n_dsec_new;	//新开辟的数据扇区号
	u32 index_fat32;	//FAT32的下标号

	if(count == 0){
		return 0;
	}


	//根据打开文件的标志判断是否可以写
	if((fd_table[fd].flags & O_RDWR_MASK) == O_RDONLY){	//打开标识为只读，那么不能够写
		printf("Flag is RDONLY.Can not write!");
		return -1;
	}


	index_buff = 0;
	old_size = fd_table[fd].f_pos;
	f_pos = old_size;
	n_dsec = get_n_dsec(fd_table[fd].n_inode , f_pos / (SEC_SIZE));	//f_pos / (SEC_SIZE)表示文件的内容扇区号
	start_writing = f_pos % (SEC_SIZE);


	//专门处理文件结尾所在的扇区。该扇区特殊是因为有部分文件内容而且还有部分空间。
	//剩余空间大于0个字节，那么需要利用该扇区的剩余空间
	//如果剩余空间没有了。重新开辟一个扇区。
	if((SEC_SIZE - start_writing) > 0){
		do_read_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec);

		if((SEC_SIZE - start_writing) >= count){//如果空白空间大于想要写入的字符个数
//			memcpy(&file_buff_out[start_writing] , buff , count);

			strncpy(&file_buff_out[start_writing] , buff , count);
			do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec);

			f_pos += count;
			fd_table[fd].f_pos = f_pos;	//更新文件大小

			return count;
		}else{	//如果剩余空间小于count
//			memcpy(&file_buff_out[start_writing] , buff , SEC_SIZE - start_writing);
			strncpy(&file_buff_out[start_writing] , buff , SEC_SIZE - start_writing);
			do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec);

			f_pos += (SEC_SIZE - start_writing);	//改变文件大小标识符
		}
		index_buff = SEC_SIZE - start_writing;	//buff标志移动
	}	//end if

	//buff还有剩余字节需要写入。要分配新的扇区供写入

	while(1){	//一次循环写入一个扇区。直到写完或者出现错误（比如空间不足）

		//重新开辟一个新的扇区。并且在FAT32中注册

		//先设定新扇区，该扇区暂时为末扇区
		n_dsec_new = gain_empty_dsec();	//该函数会改变fat32缓冲扇区
		index_fat32 = get_index_fat32(n_dsec_new);
		fat32[index_fat32] = n_dsec_new;
		syn_fat32();
		//设定新扇区为上一个扇区的下一个扇区
		index_fat32 = get_index_fat32(n_dsec);
		fat32[index_fat32] = n_dsec_new;
		syn_fat32();

		n_dsec = n_dsec_new;

		//写入新开辟的扇区
		if((count - index_buff) <= SEC_SIZE){	//如果buff剩余的字节小于一个扇区那么写入后可以返回
//			memcpy(file_buff_out , &buff[index_buff] , count - index_buff);
			strncpy(file_buff_out , &buff[index_buff] , count - index_buff);
			do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec);
			f_pos += (count - index_buff);
			fd_table[fd].f_pos = f_pos;
			return (f_pos - old_size);
		}else{		//如果count - index_buff 大于一个扇区。分配一个扇区写入
//			memcpy(file_buff_out , &buff[index_buff] , SEC_SIZE);
			strncpy(file_buff_out , &buff[index_buff] , SEC_SIZE);
			do_write_hd(file_buff_out , 0 , p_sup_block->base_sec_fs + n_dsec);
			f_pos += (SEC_SIZE);
			index_buff += (SEC_SIZE);
		}	//end if-else

	}	//end while

}	//end f


/*
 * static ssize_t do_read(int fd , char *buff , size_t count);
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
static ssize_t do_read(int fd , char *buff , size_t count){
	u32 init_pos;		//开始读取文件的位置标识
	u32 r_pos;		//最后读取文件的位置表示
	u32 f_size;	//文件大小
	u32 start_reading;	//开始读取的位置
	u32 n_dsec;	//数据扇区号
	u32 index_fat32;	//FAT32表号
	u32 index_buff;	//指示buff的读取位置
	u32 limit;	//比较count与距离文件末剩余字节数大小，取最小值

	init_pos = fd_table[fd].r_pos;
	r_pos = init_pos;
	f_size = fd_table[fd].f_pos;

	if(r_pos >= f_size){	//已经到了文件末尾
		return -1;
	}

	if((f_size - r_pos) >= count){ //距离文件末尾字节数目如果大于count,limit则为count
		limit = count;
	}else{
		limit = (f_size - r_pos);	//如果距离末尾字节数小于count则limit为剩余的字节数
	}

	start_reading = r_pos % (SEC_SIZE);
	n_dsec = get_n_dsec(fd_table[fd].n_inode , r_pos / (SEC_SIZE));
	index_fat32 = get_index_fat32(n_dsec);
	index_buff = 0;


	//欲读取的第一个扇区要特殊处理，概这个扇区的读取位置不是扇区首
	do_read_hd(file_buff_in , 0 , p_sup_block->base_sec_fs + n_dsec);

	if((SEC_SIZE - start_reading) >= limit){//如果该扇区剩余空间大于想要读入的字符个数
		strncpy(buff , &file_buff_in[start_reading] , limit);
		r_pos += limit;
		fd_table[fd].r_pos = r_pos;	//更新读取
		return limit;
	}else{	//如果剩余空间小于limit
		strncpy(buff , &file_buff_in[start_reading] , SEC_SIZE - start_reading);
		limit -= (SEC_SIZE - start_reading);		//更新剩余读取的数目
		r_pos += (SEC_SIZE - start_reading);	//改变读取标识符
	}
	index_buff = SEC_SIZE - start_reading;	//buff标志移动



	//buff还有剩余字节需要读入。取下一个扇区
	while(1){	//一次循环读入一个扇区。直到读完

		//读取下一个扇区
		index_fat32 = get_index_fat32(n_dsec);
		n_dsec = fat32[index_fat32];

		//如果是最末一个扇区那么剩余的未读字节一定是小于扇区大小的
		if(limit <= SEC_SIZE){	//如果还没有读取的字节小于一个扇区那么读入后可以返回
			do_read_hd(file_buff_in , 0 , p_sup_block->base_sec_fs + n_dsec);
			strncpy(&buff[index_buff] , file_buff_in , limit);
			r_pos += limit;
			fd_table[fd].r_pos = r_pos;
			return (r_pos - init_pos);
		}else{		//如果没有读取的字节数大于一个扇区。读入一个扇区
			do_read_hd(file_buff_in , 0 , p_sup_block->base_sec_fs + n_dsec);
			strncpy(&buff[index_buff] , file_buff_in , SEC_SIZE);
			r_pos += (SEC_SIZE);
			index_buff += (SEC_SIZE);
			limit -= (SEC_SIZE);
		}	//end if-else



	}	//end while




}	//end f



/*
 * static ssize_t do_get_fsize(int fd);
 * 返回打开的文件大小
 *
 * @param:文件描述符
 *
 * 返回值:
 * 成功返回大小。失败返回-1
 */
static ssize_t do_get_fsize(){

}	//end f



/*
 * static int do_list_dir(char *dir_path , int opt);
 * 显示文件夹的内容。
 *
 * @param0:文件夹的路径
 * @param1:选择项目
 *
 * 返回值:
 * 成功返回0。失败返回－1
 */
static int do_list_dir(char *dir_path , int opt){
	u32 n_inode;
	u32 index_inode;

	u32 dir_size;	//目录的大小
	u32 nr_entry;	//目录项的个数

	u32 n_dsec;	//目录的首扇区
	u32 index_fat32;

	u32 mode;	//目标的模式

	char *init = "----------";	//文件前缀
	char *head = "- --- --- ---";
//	head[14]= (char)0x00;
//	char buff[SEC_SIZE];
	char buff[50];

	DIR_ENTRY *dir_entry;

	int i;
	int n;	//已经显示的entry

	i = strncpy(buff , dir_path , 0);
	buff[i] = (char)0x00;

	n = 0;

	n_inode = search_file(dir_path);	//获取目录的n_inode
	index_inode = get_index_inode_table(n_inode);
	n_dsec = inode_table[index_inode].n_sec_i;	//获取目录的首扇区号
	dir_size	= inode_table[index_inode].i_size;//获取目录的大小
	mode = inode_table[index_inode].i_mode;

	nr_entry = dir_size / (DIR_ENTRY_SIZE);	//获取目录项的个数

	//检测如果是文件。那么不用dir_enrty
	if(((mode&I_MASK_FD)>>12)  == I_FLAG_F){
		head[0] = 'f';

		//检测属主的权限
		if((mode&I_MASK_USR)  == I_FLAG_RO){
			head[1] = 'r';
		}else if((mode&I_MASK_USR)  == I_FLAG_WO){
			head[2] = 'w';
		}else if((mode&I_MASK_USR)  == I_FLAG_RW){
			head[1] = 'r';
			head[2] = 'w';
		}

		//检测组的权限
		if((mode&I_MASK_GRP)  == I_FLAG_RO_G){
			head[5] = 'r';
		}else if((mode&I_MASK_GRP)  == I_FLAG_WO_G){
			head[6] = 'w';
		}else if((mode&I_MASK_GRP)  == I_FLAG_RW_G){
			head[5] = 'r';
			head[6] = 'w';
		}

		//检测其他人的权限
		if((mode&I_MASK_OTH)  == I_FLAG_RO_O){
			head[9] = 'r';
		}else if((mode&I_MASK_OTH)  == I_FLAG_WO_O){
			head[10] = 'w';
		}else if((mode&I_MASK_OTH)  == I_FLAG_RW_O){
			head[9] = 'r';
			head[10] = 'w';
		}

		printf("%s%s" , head , buff);
		return 0;
	}	//end if





	dir_entry = (DIR_ENTRY *)file_buff_in;
	while(1){	//一次循环读取一个扇区。如果已经显示的entry已经等于目录项目的个数那么退出
		i = 0;
		do_read_hd(file_buff_in , 0 , p_sup_block->base_sec_fs + n_dsec);

		for(; i<NR_ENTRY_PER_SEC; i++){	//依次读取扇区的每个目录项目

			if(dir_entry[i].n_inode != 0){	//如果该项目非空
				index_inode = get_index_inode_table(dir_entry[i].n_inode);
				mode = inode_table[index_inode].i_mode;

				strncpy(head , init , 0);

				//检测如果是文件。
				if(((mode&I_MASK_FD)>>12)  == I_FLAG_F){
					head[0] = 'f';
				}
				//检测如果是目录。
				if(((mode&I_MASK_FD)>>12)  == I_FLAG_D){
					head[0] = 'd';
				}

				//检测属主的权限
				if((mode&I_MASK_USR)  == I_FLAG_RO){
					head[1] = 'r';
				}else if((mode&I_MASK_USR)  == I_FLAG_WO){
					head[2] = 'w';
				}else if((mode&I_MASK_USR)  == I_FLAG_RW){
					head[1] = 'r';
					head[2] = 'w';
				}

				//检测组的权限
				if((mode&I_MASK_GRP)  == I_FLAG_RO_G){
					head[5] = 'r';
				}else if((mode&I_MASK_GRP)  == I_FLAG_WO_G){
					head[6] = 'w';
				}else if((mode&I_MASK_GRP)  == I_FLAG_RW_G){
					head[5] = 'r';
					head[6] = 'w';
				}

				//检测其他人的权限
				if((mode&I_MASK_OTH)  == I_FLAG_RO_O){
					head[9] = 'r';
				}else if((mode&I_MASK_OTH)  == I_FLAG_WO_O){
					head[10] = 'w';
				}else if((mode&I_MASK_OTH)  == I_FLAG_RW_O){
					head[9] = 'r';
					head[10] = 'w';
				}

				printf("%s" , head);
				printf("%s/n" , dir_entry[i].f_name);
				n++;

				if(n == nr_entry){	//如果已经读取的项目已经等于总共的项目
					return 0;;
				}

			}	//end if

		}	//end for

		//如果下一个扇区仍然有搞头
		index_fat32 = get_index_fat32(n_dsec);

		if(fat32[index_fat32] == n_dsec){	//仍然有项目但却已是最后的一个扇区 说明出现了一个问题
			return -1;
		}

		n_dsec = fat32[index_fat32];

	}	//end while

//	printf("%s is existed ");
}	//end f


/*
 * static int is_file_dir(char *abs_path , int opt);
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
static int is_file_dir(char *abs_path , int opt){
	u32 n_inode;
	u32 index_inode;
	u32 mode;

	n_inode = search_file(abs_path);

	if(n_inode == 0){	//文件(目录)不存在
		return -1;
	}

	index_inode = get_index_inode_table(n_inode);
	mode = inode_table[index_inode].i_mode;
	//文件(目录存在)
	if(opt == 0){	//检测是否是文件
		if(((mode & I_MASK_FD) >> 12) == I_FLAG_F){
			return 0;
		}

		return -1;
	}
	if(opt == 1){	//检测是否是目录
		if(((mode & I_MASK_FD) >> 12) == I_FLAG_D){
			return 0;
		}

		return -1;
	}

	return -1;

}







