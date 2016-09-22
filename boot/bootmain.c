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

	elf = (struct elfhdr*)0x10000;

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

