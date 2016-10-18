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
#include <aim/panic.h>
#include <mmu.h>
#include <arch-mmu.h>

.text

.globl _start
_start:
	/* same as bootloader */
	cli
	cld
	/* Set up the stack pointer */
	mov	$(kstack_top-KERN_BASE), %esp
	xor	%eax, %eax
	push	%eax		// eip=0
	push	%eax		// ebp=0
	mov	%esp, %ebp

	/* and there we go */
	call	master_early_init

.globl open_4MB_page
open_4MB_page:

 	movl    %cr4, %eax
 	orl     $(CR4_PSE), %eax;
 	movl    %eax, %cr4
 	movl    $(entrypgdir - KERN_BASE), %eax
 	movl    %eax, %cr3
 	movl    %cr0, %eax
 	orl     $((CR0_PG)|(CR0_WP)), %eax
 	movl    %eax, %cr0

	mov		$(kstack_top), %esp
	mov 	%esp, %ebp
	ljmp	$(SEG_KCODE<<3), $panic

.globl _start_addr
_start_addr = _start - KERN_BASE
