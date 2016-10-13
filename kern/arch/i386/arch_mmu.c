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
#include <util.h>

void mmu_init(pgindex_t *boot_page_index)
{

 	pde_t *addr = V2P_WO(boot_page_index);
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

