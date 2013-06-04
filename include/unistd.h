#ifndef _UNISTD_H
#define _UNISTD_H

#include"sys/types.h"
/*
 * int close(int fd);
 *
 * 关闭文件。
 * 如果关闭文件之后还有进程处于打开队列之中将会唤醒下一个进程
 * 如果没有进程了将撤销该fd
 *
 *@param:通过open返回的文件描述符
 *
 * 返回值：
 * 成功返回0失败返回－1
 */
extern int close(int fd);


/*
 * extern ssize_t read(int fd , char *buff , size_t count);
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
extern ssize_t read(int fd , char *buff , size_t count);






/*
 * extern ssize_t write(int fd , const char *buff , size_t count);
 *
 * 往打开的文件里写入buff里的count个字节
 * count为0则该函数不起作用
 *
 * @param0:打开的文件描述符
 * @param1:字符缓冲区buff
 * @param2:试图写入的字节数
 *
 *
 *
 * 返回值：
 * 成功返回写入的字节数。失败返回-1
 */
extern ssize_t write(int fd , const char *buff , size_t count);


/*
 * int pause();
 * 使自身进程处于挂起状态
 * 其他进程向其发送信号将使它激活
 *
 * 成功返回信号类型。失败返回-1
 */
extern int pause();






#endif
