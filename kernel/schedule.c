/*
 * -----------------------------SCHEDULE.C-------------------------------------
 */

#include "global.h"

void schedule();


/*
 * void schedule()
 * process schedule when irq0 happens
 */

void schedule(){
	int i;
	int j;

	PROC_TABLE *p = &proc_table;

	PCB *p_pcb;

	p -> index_ready--;		//汇编的使用与内部使用相差一

	j = p -> index_ready;	//store current index
	p_pcb = p -> p_pcb[j];

	if(p_pcb -> slices > 0){	//每响应一次时间中断，时间片都会减少，直到为0

		p_pcb -> slices--;

	}else{
		p_pcb -> priority = 0;	//如果时间片为零，优先级将降为0
	}

	/*
	 * 基于优先级的时间片轮转法
	 */
	if(j == (MAX_PROCESS - 1)){	//这种情况下只需要查询从0到MAX_PROCESS - 1

		for(i = 0; i < MAX_PROCESS; i++){	//轮询proc_table的每个进程PCB
			if((p -> p_pcb[i] == 0) || (p -> p_pcb[i] -> p_flag == P_FLAG_SUS)){	//proc_table may not be full
				//如果该处进程表为空 或者进程状态处于SUSPEND。不参与调度

				continue;
			}

			if((p -> p_pcb[i] -> priority) >= (p -> p_pcb[p -> index_ready] -> priority)){
				p -> index_ready = i;
			}
		}

	}else if(j == 0){	//这种情况下只需从j+1到MAX_PROCESS-1
		for(i = j + 1; i < MAX_PROCESS; i++){	//轮询proc_table的每个进程PCB
			if((p -> p_pcb[i] == 0) || (p -> p_pcb[i] -> p_flag == P_FLAG_SUS)){	//proc_table may not be full
				//如果该处进程表为空 或者进程状态处于SUSPEND。不参与调度
				continue;
			}

			if((p -> p_pcb[i] -> priority) >= (p -> p_pcb[p -> index_ready] -> priority)){
				p -> index_ready = i;
			}
		}

	}else{	//j处于中间，就考察从j+1 到 MAX_PROCESS - 1 和 0 到 j-1
		for(i = j + 1; i < MAX_PROCESS; i++){	//轮询proc_table的每个进程PCB
			if((p -> p_pcb[i] == 0) || (p -> p_pcb[i] -> p_flag == P_FLAG_SUS)){	//proc_table may not be full
				//如果该处进程表为空 或者进程状态处于SUSPEND。不参与调度
				continue;
			}

			if((p -> p_pcb[i] -> priority) >= (p -> p_pcb[p -> index_ready] -> priority)){
				p -> index_ready = i;
			}
		}
		for(i = 0; i < j; i++){	//轮询proc_table的每个进程PCB
			if((p -> p_pcb[i] == 0) || (p -> p_pcb[i] -> p_flag == P_FLAG_SUS)){	//proc_table may not be full
				//如果该处进程表为空 或者进程状态处于SUSPEND。不参与调度
				continue;
			}

			if((p -> p_pcb[i] -> priority) >= (p -> p_pcb[p -> index_ready] -> priority)){
				p -> index_ready = i;
			}
		}


	}


	/*
	错误在于，在优先级都为0的情况下只会最后两个进程被轮转
	for(i = 0; i < MAX_PROCESS; i++){	//轮询proc_table的每个进程PCB
		if(i == j){								//不与当前的比较，因为它们是一样的
			continue;
		}

		if(p -> p_pcb[i] == 0){	//proc_table may not be full

			continue;
		}


		if((p -> p_pcb[i] -> priority) >= (p -> p_pcb[p -> index_ready] -> priority)){
			p -> index_ready = i;
		}


	}



/*
 * 无优先级的时间片轮转法
	i = j;
	while(1){
		i = (i + 1) % MAX_PROCESS;
		if(p -> p_pcb[i] == 0){
		}else{
			p -> index_ready = i;
			break;
		}
	}
*/
	p -> index_ready++;	//



}
