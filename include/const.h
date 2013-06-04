/*
 * Copyright (c) 2010 , LeiMing
 * All rights reserved
 *
 * File name: const.h
 * FIle ID:
 * Abstract:
 *
 * Version:	0.1
 * Time: 2010.5.
 */
#ifndef CONST_H
#define CONST_H


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int	 u32;

/*
 * SOME COMMON CONSTS
 */
#define NONE 0
#define NULL 0

/*
 * VIDEO SETTINGS
 */
#define CRT_ADDR_REG	0x3D4
#define CRT_DATA_REG	0x3D5

//INDEX writes into CRT_ADDR_REG
#define INDEX_ST_VMEM_H	0x0C
#define INDEX_ST_VMEM_L	0x0D

#define INDEX_CURSOR_H	0x0E
#define INDEX_CURSOR_L	0x0F

/* 8259A interrupt controller ports. */
#define	INT_M_CTL	0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

/* 中断向量 */
#define	INT_VECTOR_IRQ0			0x20
#define	INT_VECTOR_IRQ8			0x28

#endif
