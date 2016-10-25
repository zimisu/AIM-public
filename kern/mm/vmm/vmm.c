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
#include <sys/param.h>
//#include <aim/sync.h>
#include <aim/vmm.h>
#include <aim/pmm.h>
#include <aim/mmu.h>	/* PAGE_SIZE */
#include <aim/panic.h>
#include <aim/console.h>
#include <libc/string.h>
/************ For slab algorithm **************/
#define MF_USED 0xffaa0055
#define MF_FREE 0x0055ffaa

struct block_header {
	unsigned long bh_flags;
	union {
		unsigned long ubh_length;
		struct block_header *fbh_next;
	} vp;
};

#define bh_length vp.ubh_length
#define bh_next   vp.fbh_next
#define BH(p) ((struct block_header *)(p))

struct page_descriptor {
	struct page_descriptor *next;
	struct block_header *firstfree;
	int order;
	int nfree;
};

#define PAGE_DESC(p) ((struct page_descriptor *)(((uint32_t)p) >> 12 << 12))

struct size_descriptor {
	struct page_descriptor *firstfree;
	int size;
	int nblocks;

	int nmallocs;
	int nfrees;
	int nbytesmalloced;
	int npages;
};

struct size_descriptor sizes[] = { 
	{ NULL,  32,127, 0,0,0,0 },
	{ NULL,  64, 63, 0,0,0,0 },
	{ NULL, 128, 31, 0,0,0,0 },
	{ NULL, 252, 16, 0,0,0,0 },
	{ NULL, 508,  8, 0,0,0,0 },
	{ NULL,1020,  4, 0,0,0,0 },
	{ NULL,2040,  2, 0,0,0,0 },
	{ NULL,4080,  1, 0,0,0,0 },
	{ NULL,   0,  0, 0,0,0,0 }
};

#define NBLOCKS(order)          (sizes[order].nblocks)
#define BLOCKSIZE(order)        (sizes[order].size)

int get_order (int size)
{
	int order;

	/* Add the size of the header */
	size += sizeof (struct block_header); 
	for (order = 0;BLOCKSIZE(order);order++)
		if (size <= BLOCKSIZE (order))
			return order; 
	return -1;
}

/**********************************************/

/* dummy implementations */
static void *__simple_alloc(size_t size, gfp_t flags) {
	int order,i,sz, tries = 0;
	struct block_header *p;
	struct page_descriptor *page;

/* Sanity check... */
	order = get_order (size);
	if (order < 0) {
    	kprintf("kmalloc of too large a block (%d bytes).\n",size);
    	return (NULL);
    }
    for (; tries < 2; tries++) {
	    if ((page = sizes[order].firstfree) && (p = page->firstfree)) {
	        if (p->bh_flags == MF_FREE) {
	            page->firstfree = p->bh_next;
	            page->nfree--;
	            if (!page->nfree) {
					sizes[order].firstfree = page->next;
	                page->next = NULL;
	            }

	            sizes [order].nmallocs++;
	            sizes [order].nbytesmalloced += size;
	            p->bh_flags =  MF_USED; /* As of now this block is officially in use */
	            p->bh_length = size;
	            return p+1; /* Pointer arithmetic: increments past header */
	        }
	        kprintf("Problem: block on freelist at %08lx isn't free.\n",(long)p);
	        return (NULL);
	    }
	    /* Now we're in trouble: We need to get a new free page..... */

	    sz = BLOCKSIZE(order);
	    page = (struct page_descriptor *)(uint32_t)pgalloc();
	    if (!page) {
	    	kprintf("Couldn't get a free page.....\n");
	        return NULL;
	    }
	    sizes[order].npages++;
	    for (i=NBLOCKS(order),p=BH (page+1);i > 1;i--,p=p->bh_next) {
	        p->bh_flags = MF_FREE; 
	        p->bh_next = BH ( ((long)p)+sz);
	    }
	    /* Last block: */
	    p->bh_flags = MF_FREE;
	    p->bh_next = NULL;

	    page->order = order;
	    page->nfree = NBLOCKS(order); 
	    page->firstfree = BH(page+1);

	    page->next = sizes[order].firstfree;
	    sizes[order].firstfree = page;
	}

    return p;
}

static void __simple_free(void *obj) {
	int order;
	register struct block_header *p=((struct block_header *)obj) -1;
	struct page_descriptor *page,*pg2;

	page = PAGE_DESC(p);
	order = page->order;

	p->bh_flags = MF_FREE; 


	p->bh_next = page->firstfree;
	page->firstfree = p;
	page->nfree ++;


	if (page->nfree == 1) { 
		/* Page went from full to one free block: put it on the freelist */
   		if (page->next) {
        	kprintf("Page %p already on freelist dazed and confused....\n", page);
        } else {
        	page->next = sizes[order].firstfree;
        	sizes[order].firstfree = page;
        }
    }

	if (page->nfree == NBLOCKS (page->order)) {
    	if (sizes[order].firstfree == page) {
        	sizes[order].firstfree = page->next;
        } else {
        	for (pg2=sizes[order].firstfree;
                (pg2 != NULL) && (pg2->next != page);
				pg2=pg2->next)
            	/* Nothing */;
        	if (pg2 != NULL)
            	pg2->next = page->next;
        	else
            	kprintf("Ooops. page %p doesn't show on freelist.\n", page);
        }
    	pgfree((addr_t)(uint32_t)(page));
    }
}

static size_t __simple_size(void *obj) { 
	register struct block_header *p=((struct block_header *)obj) -1;
	return p->bh_length;
}


int simple_allocator_bootstrap(void *pt, size_t size) {

	return 0;
}

int simple_allocator_init(void) {

	return 0;
}

static struct simple_allocator __simple_allocator = {
	.alloc	= __simple_alloc,
	.free	= __simple_free,
	.size	= __simple_size
};

void *kmalloc(size_t size, gfp_t flags)
{
	//unsigned long intr_flags;
	void *result;

	if (size > PAGE_SIZE / 2)
		panic("kmalloc: size %lu too large\n", size);
	//recursive_lock_irq_save(&memlock, intr_flags);
	result = __simple_allocator.alloc(size, flags);
	//recursive_unlock_irq_restore(&memlock, intr_flags);
	if (flags & GFP_ZERO)
		memset(result, 0, size);
	return result;
}

void kfree(void *obj)
{
	//unsigned long flags;
	if (obj != NULL) {
		//recursive_lock_irq_save(&memlock, flags);
		/* Junk filling is in flff.c since we need the gfp flags */
		__simple_allocator.free(obj);
		//recursive_unlock_irq_restore(&memlock, flags);
	}
}

size_t ksize(void *obj)
{
	if (obj != NULL)
		return __simple_allocator.size(obj);
	else
		return 0;
}

void set_simple_allocator(struct simple_allocator *allocator)
{
	if (allocator == NULL)
		return;
	memcpy(&__simple_allocator, allocator, sizeof(*allocator));
}

void get_simple_allocator(struct simple_allocator *allocator)
{
	if (allocator == NULL)
		return;
	memcpy(allocator, &__simple_allocator, sizeof(*allocator));
}

/* dummy implementation again */
static int __caching_create(struct allocator_cache *cache) { return EOF; }
static int __caching_destroy(struct allocator_cache *cache) { return EOF; }
static void *__caching_alloc(struct allocator_cache *cache) { return NULL; }
static int __caching_free(struct allocator_cache *cache, void *obj)
{ return EOF; }
static void __caching_trim(struct allocator_cache *cache) {}

struct caching_allocator __caching_allocator = {
	.create		= __caching_create,
	.destroy	= __caching_destroy,
	.alloc		= __caching_alloc,
	.free		= __caching_free,
	.trim		= __caching_trim
};

void set_caching_allocator(struct caching_allocator *allocator)
{
	if (allocator == NULL)
		return;
	memcpy(&__caching_allocator, allocator, sizeof(*allocator));
}

int cache_create(struct allocator_cache *cache)
{
	if (cache == NULL)
		return EOF;
	//spinlock_init(&cache->lock);
	return __caching_allocator.create(cache);
}

int cache_destroy(struct allocator_cache *cache)
{
	//unsigned long flags;

	if (cache == NULL)
		return EOF;
	//spin_lock_irq_save(&cache->lock, flags);
	int retval = __caching_allocator.destroy(cache);
	//spin_unlock_irq_restore(&cache->lock, flags);
	return retval;
}

void *cache_alloc(struct allocator_cache *cache)
{
	//unsigned long flags;
	if (cache == NULL)
		return NULL;
	//spin_lock_irq_save(&cache->lock, flags);
	void *retval = __caching_allocator.alloc(cache);
	//spin_unlock_irq_restore(&cache->lock, flags);
	if (cache->flags & GFP_ZERO)
		memset(retval, 0, cache->size);
	return retval;
}

int cache_free(struct allocator_cache *cache, void *obj)
{
	//unsigned long flags;
	if (cache == NULL)
		return EOF;
	if (!(cache->flags & GFP_UNSAFE))
		memset(obj, JUNKBYTE, cache->size);
	//spin_lock_irq_save(&cache->lock, flags);
	int retval = __caching_allocator.free(cache, obj);
	//spin_unlock_irq_restore(&cache->lock, flags);
	return retval;
}

void cache_trim(struct allocator_cache *cache)
{
	//unsigned long flags;
	if (cache == NULL)
		return;
	//spin_lock_irq_save(&cache->lock, flags);
	__caching_allocator.trim(cache);
	//spin_unlock_irq_restore(&cache->lock, flags);
}

