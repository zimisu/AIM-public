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

#include <sys/types.h>
#include <aim/init.h>
#include <arch-mmu.h>
#include <x86.h>
#include <mmu.h>
#include <aim/pmm.h>
#include <aim/vmm.h>
#include <arch-trap.h>
#include <lapic.h>
#include <ioapic.h>

struct segdesc gdt[NSEGS] = {
	SEG(0x0, 0x0, 0x0, 0x0),			// null seg
	SEG(STA_X|STA_R, 0, 0xffffffff, 0),		// kernel code
	SEG(STA_W, 0, 0xffffffff, 0),			// kernel data
	SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER),	// user code
	SEG(STA_W, 0, 0xffffffff, DPL_USER)		// user data
};


__noreturn void open_4MB_page();
void arch_early_init(void)
{
	lgdt(gdt, sizeof(gdt));

	early_mm_init();
	open_4MB_page();
/*
	asm("movl    %cr4, %eax;");
	asm("orl     $(#CR4_PSE), %eax;");
	
 		"movl    %eax, %cr4;"
 		"movl    $(entrypgdir - #KERN_BASE), %eax;"
 		"movl    %%eax, %%cr3;"
 		"movl    %%cr0, %%eax;"
 		"orl     $((#CR0_PG)|(#CR0_WP)), %%eax;"
 		"movl    %%eax, %%cr0;"

		"mov		$(kstack_top), %esp;"
		"mov 	%%esp, %%ebp;"
		"ljmp	$(SEG_KCODE<<3), $panic"
		);*/
}

void run_on_high_addr() {
	kputs("high addr!\n");

	page_allocator_init();
	trap_init();
	//lapicinit();
	ioapicinit();

	asm("int $0x80;");
	/*
	struct pages a = {
		0xffffffff,
		4096,
		0
	};
	struct pages b = a, c = a, d = a;
	int ret;
	ret = alloc_pages(&a);
	kprintf("return value: %d\n", ret);
	kprintf("alloc addr: %x\n-----\n", ((uint32_t)a.paddr));
	ret = alloc_pages(&b);
	kprintf("return value: %d\n", ret);
	kprintf("alloc addr: %x\n-----\n", ((uint32_t)b.paddr));
	ret = alloc_pages(&c);
	kprintf("return value: %d\n", ret);
	kprintf("alloc addr: %x\n-----\n", ((uint32_t)c.paddr));
	free_pages(&a);
	ret = alloc_pages(&d);
	kprintf("return value: %d\n", ret);
	kprintf("alloc addr: %x\n-----\n", ((uint32_t)d.paddr));

	addr_t a, b, c, d;
	a = kmalloc(2, GFP_ZERO);
	kprintf("addr: 0x%llx   size: %llu\n", a, ksize(a));
	b = kmalloc(5, GFP_ZERO);
	kprintf("addr: 0x%llx   size: %llu\n", b, ksize(b));
	kfree(a);
	c = kmalloc(15, GFP_ZERO);
	kprintf("addr: 0x%llx   size: %llu\n", c, ksize(c));
	d = kmalloc(55, GFP_ZERO);
	kprintf("addr: 0x%llx   size: %llu\n", d, ksize(d));
	d = kmalloc(56, GFP_ZERO);
	kprintf("addr: 0x%llx   size: %llu\n", d, ksize(d));*/
	panic("The kernel finished!!!\n");
}