/*
 * --------------------------------------STRING.C----------------------
 * --------------------------------------------关于字符串操作的库函数-----------------------
 */
#include "const.h"
#include "string.h"

/*
 * int strncpy(char *dest , char *src , int number);
 * 成功返回拷贝的字符个数 ， 错误返回1
 * number = 0 表示复制src直到遇见0x00(不会拷贝0x00)
 * number > 0 表示复制src一共number个字符到dest之中
 *
 * 注意dest要有足够的空间。如果number大于src的长度可能会出现错误
 *
 */
int strncpy(char *dest , const char *src , int number){
	int i = 0;
	char c;

	if(number < 0){
		return -1;
	}

	if(number == 0){

		while(1){
			if(*(src + i) == 0x00){
//				*(dest + i) = 0x00;	//不会拷贝0x00
				break;
			}
			*(dest + i) = *(src + i);

			i++;
		}
		return i;	//返回的是拷贝的字符个数（不包括0x00）注意使从1开始算而不是0
	}

	if(number > 0){
		for(i = 0; i < number; i++){
			*(dest + i) = *(src + i);
		}
//		*(dest + i) = 0x00;	//末尾需要补一个结束标识符
		return number;
	}

}




/*
 * int strlen(char *str);
 * 返回以str开头的字符串的个数，以0x00结束。
 * 但是不包括0x00
 */
int strlen(char *str){
	int i = 0;

	while(1){
		if(*(str + i) == 0x00)
			break;
		i++;
	}

	return i;	//这里与人的认识保持一致。实际是0~i-1 ， 但是我们认为是从1~i

}



/*
 * int strcmp(const char *str1 , const char *str2);
 *
 * @param0:参与比较的字符串1
 * @param1:参与比较的字符串2
 *
 * 返回值：
 * -1:第一个字符串小于第二个字符串
 * =0:第一个字符串等于第二个字符串
 * =1:第一个字符串大于第二个字符串
 */
int strcmp(const char *str1 , const char *str2){
	u32 index = 0;

	while(1){
		//它们同时到达字符串末尾。而且此时暗含之前的字符全部相同。两字符串必定相同
		if(((u8)str1[index] == (u8)0x00) && ((u8)str2[index] == (u8)0x00)){
			return 0;
		}

		//str1字符小于str2字符。如果str1到达字符串末尾，而str2还没有，那么一定str1<str2
		if((u8)str1[index] < (u8)str2[index]){
			return -1;
		}

		//str1字符大于str2字符。如果str2到达字符串末尾，而str1还没有，那么一定str1>str2
		if((u8)str1[index] > (u8)str2[index]){
			return 1;
		}

		index++;

	}	//end while

}	//end f



/*
 * char *strcat(char *dest , char *src);
 * 连结两个字符串，将第二个字符串连在第一个字符串末尾。注意将要剔出第一个字符串的末尾0x00(如果有)
 * 第一个字符串要有足够的空间存储复制的字符串
 *
 * @param0:
 * @param1:
 *
 * 返回值：
 * 成功返回第一个字符串的指针，失败返回NULL
 */
char *strcat(char *dest , char *src){
	int i;
	i = strlen(dest);

	strncpy(&dest[i] , src , strlen(src));

	return dest;
}	//end f


/*
 * char *strchr(char *s , int c);
 * 查找字符串s中第一个出现目标字符c的位置，返回其地址
 *
 * @param0:字符串
 * @param1:目标字符
 *
 * 返回值:
 * 成功返回地址。失败返回NULL
 *
 */
char *strchr(char *s , int c){
	int len;	//字符串之长度
	int i;

	len = strlen(s);

	for(i=0; i<len; i++){
		if(s[i] == (char)c){	//找到目标。返回其位置
			return &s[i];
		}

	}
	//没有找到。返回NULL
	return (char *)NULL;
}	//end f



/*
 * char *strrchr(char *s , int c);
 *
  * 查找字符串s中最后一个出现目标字符c的位置，返回其地址
 *
 * @param0:字符串
 * @param1:目标字符
 *
 * 返回值:
 * 成功返回地址。失败返回NULL
 *
 */
char *strrchr(char *s , int c){
	int len;	//字符串长度
	int i;

	len = strlen(s);

	for(i=(len-1); i>=0; i--){	//从字符串末尾往前查找
		if(s[i] == (char)c){	//找到目标字符
			return &s[i];
		}
	}	//end for

	//没有找到 返回NULL
	return (char *)NULL;

}	//end f



