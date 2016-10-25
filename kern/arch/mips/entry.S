/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of AIM.
 *
 * AIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asm.h>

.text

/*
 * IMPORTANT NOTE:
 * Unless explicitly stated (either by filename, assembler directives,
 * or comments), all code inside kern/arch/mips directory should work
 * on both MIPS32 and MIPS64 architectures.
 */

/*
 * Here is the entry of kernel, as stated by vmaim.lds.S.
 *
 * Usually on a multiprocessor machine, only one processor (we call it
 * the master processor) is brought in to the kernel entry, while
 * other processors (called slave processors) are suspended by firmware
 * and wait for some sort of signal from the master.
 */
BEGIN(_start)
	/* PIC environment preparation */
setup_pic:
	/*
	 * MIPS did not consider PIC at time of design.  When the concept
	 * of PIC was mentioned, MIPS ABI designated register 't9' to store
	 * the address of entry of current routine the processor is running.
	 * Furthermore, MIPS ABI stated that register 'gp' points to the
	 * Global Offset Table (GOT).
	 *
	 * Unfortunately, we cannot assume that BIOS and/or bootloaders would
	 * set up a PIC environment (i.e. 'gp' and 't9') for us, as most
	 * operating systems including Linux runs a non-PIC kernel on MIPS.
	 * So we have to set up both registers by our own.
	 */

	/* We need to obtain the entry of __start dynamically first, by
	 * calling a Branch-And-Link, retrieving the return address, and
	 * compute the address manually.  To ensure correctness, optimization
	 * and instruction reordering should be turned off. */
	.set	noreorder
	bal	1f
	nop
	/* Idea from UBoot: we embed the value of _gp in the entry code, and
	 * fetch the value during execution there.
	 * Note for MIPS64: we know that _gp is above KSEG0, so storing
	 * the low 32 bits would suffice, as LW instruction would sign-extend
	 * the high 32 bits with all 1s.
	 */
	.word	_gp		/* at __start + 8 = t9 + 8 */
1:
	SUBU	t9, ra, 8
	lw	gp, 8(t9)
	.set	reorder

	/* Ensure that we are inside kernel mode. */
	MFC032	a0, CP0_STATUS
	or	a0, ST_EXCM
	xor	a0, ST_EXCM
#ifdef __LP64__
	or	a0, ST_KX | ST_SX | ST_UX /* enable 64-bit address space */
#endif
	or	a0, ST_IM
	MTC032	a0, CP0_STATUS
	/*
	 * The kernel is already inside RAM, so .text section, .rodata
	 * section and .data section are in their right position with
	 * correct values.
	 * However, .bss section is still not guaranteed to be initialized
	 * to 0, so we are going to clean it first.
	 */
init_bss:
	LA	s1, _bss_begin
	LA	s2, _bss_end
1:	bge	s1, s2, 9f
	sw	zero, (s1)
	ADDU	s1, 4
	b	1b
9:
init_master_stack:
	/* boot_stack points to stack top.  Defined in vmaim.lds.S */
	LA	sp, kstack_top

main_entry:
	/*
	 * We know that we won't return from master_early_init(), so
	 * we don't need to save anything prior to calling the function.
	 *
	 * PIC note: ordinarily we need to save gp register since jumping
	 * to another global function would probably clobber the register.
	 * However, we are not returning here, so we'll just let it happen.
	 *
	 * The .cprestore directive, which saves gp onto stack, is a hack to
	 * suppress the warning message from assembler.  We don't care about
	 * the value anyway.
	 */
	.cprestore
	jal	master_early_init

1:	b	1b
END(_start)

