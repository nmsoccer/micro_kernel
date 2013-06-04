/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: assert.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */


#ifndef _ASSERT_H
#define _ASSERT_H




#define assert(exp) if(exp); else {printf("Assertion Failed:");printf("%s" , #exp);pause();}	//使进程挂起












#endif
