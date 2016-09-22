/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of AIMv6.
 *
 * AIMv6 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIMv6 is distributed in the hope that it will be useful,
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
#include <aim/boot.h>

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

__noreturn
void bootmain(void)
{
	struct elfhdr *elf;
	struct proghdr *ph, *eph;
	void (*entry)(void);
	uchar* pa;
	struct part_ent *partition_entry;

	//get second partition entry from mbr
	partition_entry = (part_ent*)(mbr + 446 + 16);
	/*todo: read from disk*/

	readsect(elf, partition_entry->rel_sector);
	//elf = (struct elfhdr*)0x10000;

	readseg((uchar*)elf, 4096, 0);

	if (elf->magic != ELF_MAGIC) return;

	ph = (struct proghdr*)((uchar*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (; ph < eph; ph++) {
		pa = (uchar*)ph->paddr;
		readseg(pa, ph->filesz, ph->off);
		if (ph->memsz > ph->filesz)
			stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
	}

	entry = (void(*)(void))(elf->entry);
	entry();

	while (1);
}


void waitdisk(void) {
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, uint offset) {
	waitdisk();
	outb(0x1f2, 1);
	outb(0x1f3, offset);
 	outb(0x1F4, offset >> 8);
 	outb(0x1F5, offset >> 16);
 	outb(0x1F6, (offset >> 24) | 0xE0);
 	outb(0x1F7, 0x20);  

 	waitdisk();
 	insl(0x1f0, dst, SECTSIZE / 4);
}

void readseg(uchar* pa, uint count, uint offset) {
	uchar* epa;

	epa = pa + count;

	pa -= offset % SECTSIZE;

	offset = (offset / SECTSIZE) + 1;

	for (; pa < epa; pa += SECTSIZE, offset++) 
		readsect(pa, offset);
}