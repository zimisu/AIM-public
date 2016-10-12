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

#include "arch-boot.h"
#include <sys/types.h>
#include <aim/init.h>

 void clear_bss() {
 	extern int __bss_start, __bss_end;
 	//int *p = &__bss_start;

 	stosb(&__bss_start, 0, (&__bss_end - &__bss_start) * 4);
 	//for (; p < &__bss_end; p++)
 		//*p = 0;
 }

__noreturn
void master_early_init(void)
{
	clear_bss();

	arch_early_init();
	goto panic;

panic:
	//while (1);
	hlt();
}

