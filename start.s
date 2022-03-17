/*
 *  armboot - Startup Code for ARM920 CPU-core
 *
 *  Copyright (c) 2001	Marius Gr?ger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Z?pke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
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

.set CONFIG_STACKSIZE,      (128*1024)      /* regular stack */
.set CONFIG_STACKSIZE_IRQ,    (4*1024)    /* IRQ stack */
.set CONFIG_STACKSIZE_FIQ,    (4*1024)    /* FIQ stack */

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start:	b       reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq

	.balignl 16,0xdeadbeef



/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

/*
 * CFG_MEM_END is in the board dependent config-file (configs/config_BOARD.h)
 */
_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * Note: armboot_end is defined by the (board-dependent) linker script
 */
.globl _armboot_end
_armboot_end:
	.word armboot_end

/*
 * _armboot_real_end is the first usable RAM address behind armboot
 * and the various stacks
 */
 
.globl _armboot_real_end
_armboot_real_end:
	.word FIQ_STACK_START + 4

.globl IRQ_STACK_START
IRQ_STACK_START:
	.word (_armboot_end + CONFIG_STACKSIZE + CONFIG_STACKSIZE_IRQ - 4)

.globl FIQ_STACK_START
FIQ_STACK_START:
	.word (IRQ_STACK_START + CONFIG_STACKSIZE_FIQ)


/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

/* turn off the watchdog */
.set pWTCON,		0x53000000
/* Interupt-Controller base addresses */
.set INTMSK,		0x4A000008
.set INTSUBMSK,		0x4A00001C
/* clock divisor register */
.set CLKDIVN,		0x4C000014

/* PLL control register */
.set MPLLCON,		0x4C000004
/* Mpll = (2 * m * Fin) / (p * 2^S ) */
/* m = (MDIV + 8), p = (PDIV + 2), s = SDIV */
/* Mpll = (2 * (92+8) * 12) / ((1+2) * 2^1) = 400MHz */
.set V_MPLLCON,		((92<<12) + (1<<4) + 1)

	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]

	ldr	r1, =0x3ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]

	/* cpu set IRQ enable */
	mrs	r0,cpsr
	bic	r0,r0,#0x80
	msr	cpsr,r0


	/* FCLK:HCLK:PCLK = 1:4:8 */
	ldr	r0, =CLKDIVN
	mov	r1, #5
	str	r1, [r0]

	/* set FCLK is 400 MHz ! */
	ldr r1, =V_MPLLCON
	ldr r0, =MPLLCON
	str r1, [r0]

	/* FastBusMode to AsyncBusMode, CPU use FCLK instand of HCLK. */
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#0xc0000000
	mcr p15,0,r0,c1,c0,0


	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
	bl	cpu_init_crit

# NORFLASH USE THIS CODE
#relocate:
# 	/*
# 	 * relocate armboot to RAM
# 	 */
# 	adr	r0, _start		/* r0 <- current position of code */
# 	ldr	r2, _armboot_start
# 	ldr	r3, _armboot_end
# 	sub	r2, r3, r2		/* r2 <- size of armboot */
# 	ldr	r1, _TEXT_BASE		/* r1 <- destination address */
# 	add	r2, r0, r2		/* r2 <- source end address */
# 
# 	/*
# 	 * r0 = source address
# 	 * r1 = target address
# 	 * r2 = source end address
# 	 */
#copy_loop:
# 	ldmia	r0!, {r3-r10}
# 	stmia	r1!, {r3-r10}
# 	cmp	r0, r2
# 	ble	copy_loop

	/* set up the stack */
	ldr	r0, _armboot_end
	add	r0, r0, #CONFIG_STACKSIZE
	sub	sp, r0, #12		/* leave 3 words for abort-stack */

.globl	Ram_pr				/* global var used for nandFlash read&write pr */
Ram_pr:					/* place it here, for NOT beyond 4K RAM */
	.word(0xA5A5)
	bl	relocate_ram		/* NANDFLASH USE THIS CODE */

	ldr	pc, _start_armboot

_start_armboot:	.word start_armboot


/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */


cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	mcr	p15, 0, r0, c1, c0, 0


	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a memsetup.S in your board directory.
	 */
	mov	ip, lr
	bl	memsetup
	mov	lr, ip

	mov	pc, lr


/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
.set S_FRAME_SIZE,	72

.set S_OLD_R0,	68
.set S_PSR,		64
.set S_PC,		60
.set S_LR,		56
.set S_SP,		52

.set S_IP,		48
.set S_FP,		44
.set S_R10,		40
.set S_R9,		36
.set S_R8,		32
.set S_R7,		28
.set S_R6,		24
.set S_R5,		20
.set S_R4,		16
.set S_R3,		12
.set S_R2,		8
.set S_R1,		4
.set S_R0,		0

.set MODE_SVC,	0x13
.set I_BIT,	 	0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add     r8, sp, #S_PC

	ldr	r2, _armboot_end
	add	r2, r2, #CONFIG_STACKSIZE
	sub	r2, r2, #8
	ldmia	r2, {r2 - r4}                   @ get pc, cpsr, old_r0
	add	r0, sp, #S_FRAME_SIZE		@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r4}                   @ save sp_SVC, lr_SVC, pc, cpsr, old_r
	mov	r0, sp
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add     r8, sp, #S_PC
	stmdb   r8, {sp, lr}^                   @ Calling SP, LR
	str     lr, [r8, #0]                    @ Save calling PC
	mrs     r6, spsr
	str     r6, [r8, #4]                    @ Save CPSR
	str     r0, [r8, #8]                    @ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_end		@ setup our mode stack
	add	r13, r13, #CONFIG_STACKSIZE	@ resides at top of normal stack
	sub	r13, r13, #8

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str     lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
@	bl 	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
@	bl 	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
@	bl 	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
@	bl 	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
@	bl 	do_not_used

@ #ifdef CONFIG_USE_IRQ

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl 	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
@	bl 	do_fiq
	irq_restore_user_regs
@ #else

/*
	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl 	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl 	do_fiq

#endif
*/

	.align	5
.globl reset_cpu
reset_cpu:
	mov     ip, #0
	mcr     p15, 0, ip, c7, c7, 0           @ invalidate cache
	mcr     p15, 0, ip, c8, c7, 0           @ flush TLB (v4)
	mrc     p15, 0, ip, c1, c0, 0           @ get ctrl register
	bic     ip, ip, #0x000f                 @ ............wcam
	bic     ip, ip, #0x2100                 @ ..v....s........
	mcr     p15, 0, ip, c1, c0, 0           @ ctrl register
	mov     pc, r0
