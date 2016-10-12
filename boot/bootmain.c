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
#include <aim/boot.h>
#include <elf.h>

#define SECTSIZE  512

void readseg(uint8_t*, uint32_t, uint32_t);

uint8_t *mbr;

__noreturn
void bootmain(void)
{
	struct elf32hdr *elf;
	struct elf32_phdr *ph, *eph;
	void (*entry)(void);
	uint8_t* pa;

	mbr = (uint8_t*)(0x7dbe);

	elf = (struct elf32hdr*)0x10000;

	//partition table starts from mbr + 16; 
	//the size of every entry is 8, so the second partition entry starts from mbr + 16 + 8
	uint32_t offset = (*(uint32_t *)(mbr + 16 + 8)) * SECTSIZE;

	readseg((uint8_t*)elf, 8192, offset);

	if (*((uint32_t *)(elf->e_ident)) != ELF_MAGIC) return;

	ph = (struct elf32_phdr*)((uint8_t*)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++) {
		pa = (uint8_t*)ph->p_paddr;
		readseg(pa, ph->p_filesz, ph->p_offset + offset);
		if (ph->p_memsz > ph->p_filesz)
			stosb(pa + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
	}

	entry = (void(*)(void))(elf->e_entry);
	entry();
}

