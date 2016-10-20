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
#include <aim/panic.h>

extern uint32_t _end;

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES];
void mmu_init(pgindex_t *boot_page_index) {
}

void early_mm_init(void) {
	page_index_early_map((pgindex_t*)V2P(entrypgdir), 0, (void*)0, (uint32_t)&_end - KERN_BASE);
	page_index_early_map((pgindex_t*)V2P(entrypgdir), 0, (void*)P2V(0), (uint32_t)&_end - KERN_BASE);
}

int page_index_early_map(pgindex_t *boot_page_index, addr_t paddr,
	void* vaddr, size_t size) {
	uint32_t a, last;
	a = (uint32_t) vaddr;
	last = ((uint32_t) vaddr) + size - 1;
	for (; a < last; a += (1 << 22), paddr += (1 << 22)) {
		boot_page_index[a >> PDXSHIFT] = paddr | PTE_P | PTE_W | PTE_PS;
	}
	return 0;
}