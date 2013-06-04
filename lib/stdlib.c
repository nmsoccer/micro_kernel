
//int itoa(int i , char *buff);
//将整数i转化为字符串放入buff开始的字符串中
//返回转化成字符的个数
int itoa(int data , char *buff){
	char str[10] = {0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF};//整数最长10位
	int i = 0;
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

	//从str[index]到str[9]为有效字符

	for(; index < 10; index++){
		*(buff + i) = str[index];
		i++;
	}

	return i;

}
