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
#include <aim/initcalls.h>
#include <aim/console.h>
#include <aim/panic.h>
#include <aim/trap.h>
#include <proc.h>
#include <aim/pmm.h>
#include <aim/vmm.h>

struct segdesc gdt[NSEGS] = {
	SEG(0x0, 0x0, 0x0, 0x0),			// null seg
	SEG(STA_X|STA_R, 0, 0xffffffff, 0),		// kernel code
	SEG(STA_W, 0, 0xffffffff, 0),			// kernel data
	SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER),	// user code
	SEG(STA_W, 0, 0xffffffff, DPL_USER)		// user data
};

static void startothers(void);

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
	lapicinit();
	ioapicinit();
	do_early_initcalls();
	do_initcalls();

	mpinit();
	startothers();

	panic("The kernel finished!!!\n");
}

// Common CPU setup code.
extern struct gatedesc idt[256];
static void
mpmain(void)
{
	struct cpu *cpu= get_cpu();

 	kprintf("----------------\ncpu%d: starting\n---------\n", cpunum());
	lidt(idt, sizeof(idt)); // load idt register
	xchg(&cpu->started, 1); // tell startothers() we're up
  //scheduler();     // start running processes
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
	lgdt(gdt, sizeof(gdt));
	lapicinit();
	mpmain();
}

void run_mp() {
	asm("hlt;");
	kputs("haha!!!!!!!!!!  another cpu!\n");
	mpmain();
}

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  extern uchar start[], entryother_end[];
  extern int ncpu;
  uchar *code;
  struct cpu *c;
  char *stack;

  void * mp_addr = run_mp;
  kprintf("run_mp addr: %x,  %x\n", mp_addr, &mp_addr);
  memcpy(P2V(0x7000 - 8), &mp_addr, 4);

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memcpy(code, start, (uint)(entryother_end - start));

  kprintf("ncpu: %d\n", ncpu);
  for(c = cpus; c < cpus+ncpu ; c++){
  	kprintf("cpunum: %d\n", cpunum());
    if(c == cpus+cpunum())  // We've started already.
      continue;
  	kprintf("cpu num: %x\n", c);

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = pgalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void**)(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(c->started == 0)
      ;
  }
}