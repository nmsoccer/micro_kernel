/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: signal.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */


#ifndef SIGNAL_H
#define SIGNAL_H


/*
 * -------------SIGNAL.H----------------
 * -------信号相关的函数。实质上是处理消息--------
 * 是建立在消息之上的通信机制。主要是整形通信
 * 它的目的是对于用户进程，消息机制不可见。由函数封装
 *
 */

/*
 * 一些所有进程通用的信号
 */
#define SIG_READY	1
#define SIG_ERROR	2

#define SIG_SUCCESS	0
#define SIG_FAILED	-1
/*
 * void wait(int signal);
 * 等待目标的信号量出现
 * 等待过程中进程会被进程0暂停。直到进程的收件箱出现携带目标信号量的消息包。目标消息包随后被销毁
 *
 * @param:信号
 */
void wait(int signal);

/*
 * int kill(int pid , int signal);
 * 发送信号到指定的进程
 *
 * @param0:目标进程
 * @param1:信号
 *
 * 返回值：
 * 成功返回0.失败返回-1
 */
int kill(int pid , int signal);

/*
 * 使目标进程处于睡眠状态指定时间
 */
unsigned int sleep(unsigned int seconds);


#endif
