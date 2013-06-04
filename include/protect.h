/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: protect.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */


#ifndef PROTECT_H
#define PROTECT_H


/*
 * DESCRIPTOR
 */
typedef struct stc_descriptor{
	u16	low_limit;
	u16 	low_base;
	u8		mid_base;

	u8		low_attr;
	u8		high_attr_limit;

	u8		high_base;

}DESCRIPTOR;

#define DESCRIPTOR_LEN	8	//8B


#endif
