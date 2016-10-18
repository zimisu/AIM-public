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
#include <aim/mmu.h>
#include <arch-mmu.h>
#include <mmu.h>
#include <util.h>

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERN_BASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

void mmu_init(pgindex_t *boot_page_index) {
}

void write_pgdir_addr() {
 	pde_t *addr = V2P_WO(entrypgdir);
 	asm (
 		"movl    %%cr4, %%eax;"
 		"orl     $(#CR4_PSE), %%eax;"
 		"movl    %%eax, %%cr4;"
 		"movl    %0, %%eax;"
 		"movl    %%eax, %%cr3;"
 		"movl    %cr0, %eax;"
 		"orl     $((#CR0_PG)|(#CR0_WP), %eax;"
 		"movl    %eax, %cr0;"
 		: "=r"(addr)
 	);
}


/* Initialize a page index table and fill in the structure @pgindex */
pgindex_t *init_pgindex(void) {
	write_pgdir_addr();
}
/* Destroy the page index table itself assuming that everything underlying is
 * already done with */
//void destroy_pgindex(pgindex_t *pgindex);
/* Map virtual address starting at @vaddr to physical pages at @paddr, with
 * VMA flags @flags (VMA_READ, etc.) Additional flags apply as in kmmap.h.
 */
//int map_pages(pgindex_t *pgindex, void *vaddr, addr_t paddr, size_t size,
 //   uint32_t flags);