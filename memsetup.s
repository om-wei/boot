/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the Samsung SMDK2410 by
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


/* some parameters for the board */

/*
 *
 * Taken from linux/arch/arm/boot/compressed/head-s3c2410.S
 *
 * Copyright (C) 2002 Samsung Electronics SW.LEE  <hitchcar@sec.samsung.com>
 *
 */

.set BWSCON, 	0x48000000

/* BWSCON */
.set DW8, 		 	(0x0)
.set DW16, 		 	(0x1)
.set DW32, 		 	(0x2)
.set WAIT, 		 	(0x1<<2)
.set UBLB, 		 	(0x1<<3)

.set B1_BWSCON, 	  	(DW32)
.set B2_BWSCON, 	  	(DW16)
.set B3_BWSCON, 	  	(DW16)
.set B4_BWSCON, 	  	(DW16 + WAIT + UBLB)
.set B5_BWSCON, 	  	(DW16)
.set B6_BWSCON, 	  	(DW32)
.set B7_BWSCON, 	  	(DW32)

/* BANK0CON */
.set B0_Tacs, 		 	0x0	/*  0clk */
.set B0_Tcos, 		 	0x0	/*  0clk */
.set B0_Tacc, 		 	0x7	/* 14clk */
.set B0_Tcoh, 		 	0x0	/*  0clk */
.set B0_Tah, 		 	0x0	/*  0clk */
.set B0_Tacp, 		 	0x0
.set B0_PMC, 		 	0x0	/* normal */

/* BANK1CON */
.set B1_Tacs, 		 	0x0	/*  0clk */
.set B1_Tcos, 		 	0x0	/*  0clk */
.set B1_Tacc, 		 	0x7	/* 14clk */
.set B1_Tcoh, 		 	0x0	/*  0clk */
.set B1_Tah,		 	0x0	/*  0clk */
.set B1_Tacp, 		 	0x0
.set B1_PMC, 		 	0x0

.set B2_Tacs, 		 	0x0
.set B2_Tcos, 		 	0x0
.set B2_Tacc, 		 	0x7
.set B2_Tcoh, 		 	0x0
.set B2_Tah, 		 	0x0
.set B2_Tacp, 		 	0x0
.set B2_PMC, 		 	0x0

.set B3_Tacs, 		 	0x0	/*  0clk */
.set B3_Tcos, 		 	0x0	/*  0clk */
.set B3_Tacc, 		 	0x7	/* 14clk */
.set B3_Tcoh, 		 	0x0	/*  0clk */
.set B3_Tah, 		 	0x0	/*  0clk */
.set B3_Tacp, 		 	0x0
.set B3_PMC, 		 	0x0	/* normal */

.set B4_Tacs, 		 	0x0	/*  0clk */
.set B4_Tcos, 		 	0x3	/*  4clk */
.set B4_Tacc, 		 	0x7	/* 14clk */
.set B4_Tcoh, 		 	0x1	/*  1clk */
.set B4_Tah, 		 	0x0	/*  0clk */
.set B4_Tacp, 		 	0x3     /*  6clk */
.set B4_PMC, 		 	0x0	/* normal */

.set B5_Tacs, 		 	0x0	/*  0clk */
.set B5_Tcos, 		 	0x0	/*  0clk */
.set B5_Tacc, 		 	0x7	/* 14clk */
.set B5_Tcoh, 		 	0x0	/*  0clk */
.set B5_Tah, 		 	0x0	/*  0clk */
.set B5_Tacp, 		 	0x0
.set B5_PMC, 		 	0x0	/* normal */

.set B6_MT, 		 	0x3	/* SDRAM */
.set B6_Trcd, 	 	 	0x1
.set B6_SCAN, 		 	0x1	/* 9bit */

.set B7_MT, 		 	0x3	/* SDRAM */
.set B7_Trcd, 		 	0x1	/* 3clk */
.set B7_SCAN, 		 	0x1	/* 9bit */

/* REFRESH parameter */
.set REFEN, 		 	0x1	/* Refresh enable */
.set TREFMD, 		 	0x0	/* CBR(CAS before RAS)/Auto refresh */
.set Trp, 			 	0x0	/* 2clk */
.set Trc, 			 	0x3	/* 7clk */
.set Tchr, 			 	0x2	/* 3clk */
.set REFCNT, 		 	1269/* period=7.8us, HCLK=100Mhz, (2048+1-7.8*100) */
/**************************************/

_TEXT_BASE:
	.word	TEXT_BASE

.globl memsetup
memsetup:
	/* memory control configuration */
	/* make r0 relative the current location so that it */
	/* reads SMRDATA out of FLASH rather than memory ! */
	ldr     r0, =SMRDATA
	ldr		r1, _TEXT_BASE
	sub		r0, r0, r1
	ldr		r1, =BWSCON	/* Bus Width Status Controller */
	add     r2, r0, #13*4
0:
	ldr     r3, [r0], #4
	str     r3, [r1], #4
	cmp     r2, r0
	bne     0b

	/* everything is fine now */
	mov	pc, lr

	.ltorg
/* the literal pools origin */

SMRDATA:
    .word (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
    .word ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))
    .word ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))
    .word ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))
    .word ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))
    .word ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))
    .word ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))
    .word ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))
    .word ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))
    .word ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)
    .word 0x32
    .word 0x30
    .word 0x30
