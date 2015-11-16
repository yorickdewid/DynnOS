/*
 * DynnOS Operating System
 * Project DynnOS, Quenza
 * Copyright (c) 2010-2012
 * All rights reserved.
 *
 * All files, published and unpublished, are property of
 * the DynnOS poject. Source is reserved for official
 * developers only. Project developers are named in the
 * next section.
 *
 * Developed by: Dinux, Team17, team1717 and Quenza Development Team
 *
 * Description:	Kernel memory allocator
 * Date:		21-02-2012
 * Timestamp:	12-09-2012
 * DUID:		{Y17-IFEE4R53-W5GC3-4O}->?
 *
 * = Simple Quenza Common License 2012 =
 *
 * All files, published and unpublished, are property of the named owner
 * and / or the copyright holders. It is forbidden to use, change, copy,
 * modify, merge, publish, distribute, promote, sublicense, and/or sell
 * anything that can be called software or is related to information technology
 * relevant to the Software. Software as defined here includes source code
 * executable code, documentation and / or data.
 * Unless specified otherwise by the licensor, the following conditions
 * must apply at all time:
 * 1)	Each component, file, program, document, documentation and associated
 *		projects must include the official QCL or SQCL license, while the
 *		license must be supplied separately.
 * 2)	Each project / Software must have at least one Quenza Unique Identity code
 *		(QUID) which can be obtained from the licensor. The QUID need to be
 *		mentioned in the concerning project / Software files
 *		and the overall description.
 * 3)	The project / Software description must contain	the creation date
 *		and recent timestamp.
 * 4)	The copyright notice and this permission notice shall be
 *		included in all copies or substantial portions of the Software.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * For additional information see DynnOS.com/license
 */

#include <kconf/config.h>
#include <kdebug/debug.h>
#include <kcore/kernel.h>
#include <kdebug/log.h>
#include <kbase/cpu.h>
#include <kbase/isr.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kcommon/ordered_table.h>
#include <kmm/paging.h>
#include <kmm/kheap.h>

extern unsigned int end;
unsigned int placement_address = (unsigned int) &end;
extern struct page_directory *kernel_directory;
struct heap *kheap = 0;

unsigned int kmalloc_int(unsigned int sz, int align, unsigned int *phys)
{
    if(kheap != 0)
    {
        void *addr = alloc(sz, (unsigned char)align, kheap);
        if (phys != 0)
        {
            struct page *page = get_page((unsigned int)addr, 0, kernel_directory);
            *phys = page->frame*0x1000 + ((unsigned int)addr & 0xFFF);
        }
        return (unsigned int)addr;
    }
    else
    {
        if (align == 1 && (placement_address & 0xFFFFF000) )
        {
            // Align the placement address;
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }
        if (phys)
        {
            *phys = placement_address;
        }
        unsigned int tmp = placement_address;
        placement_address += sz;
        return tmp;
    }
}

void kfree(void *p)
{
    free(p, kheap);
}

unsigned int kmalloc_a(unsigned int sz)
{
    return kmalloc_int(sz, 1, 0);
}

unsigned int kmalloc_p(unsigned int sz, unsigned int *phys)
{
    return kmalloc_int(sz, 0, phys);
}

unsigned int kmalloc_ap(unsigned int sz, unsigned int *phys)
{
    return kmalloc_int(sz, 1, phys);
}

unsigned int kmalloc(unsigned int sz)
{
    return kmalloc_int(sz, 0, 0);
}

static void expand(unsigned int new_size, struct heap *heap)
{
    // Sanity check.
    //ASSERT(new_size > heap->end_address - heap->start_address);
    if(new_size <= heap->end_address - heap->start_address)
    {
        kprintf("Expansion mismatch\n");
    }

    // Get the nearest following page boundary.
    if ((new_size) & (0xFFFFF000 != 0))
    {
        new_size &= 0xFFFFF000;
        new_size += 0x1000;
    }

    // Make sure we are not overreaching ourselves.
    //ASSERT(heap->start_address+new_size <= heap->max_address);
    if((heap->start_address+new_size) > heap->max_address)
    {
        kprintf("Expansion limit\n");
    }

    // This should always be on a page boundary.
    unsigned int old_size = heap->end_address-heap->start_address;

    unsigned int i = old_size;
    while (i < new_size)
    {
        alloc_frame( get_page(heap->start_address+i, 1, kernel_directory),
                     (heap->supervisor)?1:0, (heap->readonly)?0:1);
        i += 0x1000 /* page size */;
    }
    heap->end_address = heap->start_address+new_size;
}

static unsigned int contract(unsigned int new_size, struct heap *heap)
{
    // Sanity check.
    //ASSERT(new_size < heap->end_address-heap->start_address);
    if(new_size >= heap->end_address-heap->start_address)
    {
        kprintf("Contraction mismatch\n");
    }

    // Get the nearest following page boundary.
    if (new_size&0x1000)
    {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    // Don't contract too far!
    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    unsigned int old_size = heap->end_address-heap->start_address;
    unsigned int i = old_size - 0x1000;
    while (new_size < i)
    {
        free_frame(get_page(heap->start_address+i, 0, kernel_directory));
        i -= 0x1000;
    }

    heap->end_address = heap->start_address + new_size;
    return new_size;
}

static int find_smallest_hole(unsigned int size, unsigned char page_align, struct heap *heap)
{
    // Find the smallest hole that will fit.
    unsigned int iterator = 0;
    while (iterator < heap->index.size)
    {
        struct header *header = (struct header *)lookup_ordered_array(iterator, &heap->index);
        // If the user has requested the memory be page-aligned
        if (page_align > 0)
        {
            // Page-align the starting point of this header.
            unsigned int location = (unsigned int)header;
            int offset = 0;
            if ((location+sizeof(struct header)) & (0xFFFFF000 != 0))
                offset = 0x1000 /* page size */  - (location+sizeof(struct header))%0x1000;
            int hole_size = (int)header->size - offset;
            // Can we fit now?
            if (hole_size >= (int)size)
                break;
        }
        else if (header->size >= size)
            break;
        iterator++;
    }
    // Why did the loop exit?
    if (iterator == heap->index.size)
        return -1; // We got to the end and didn't find anything.
    else
        return iterator;
}

static char header_t_less_than(void*a, void *b)
{
    return (((struct header*)a)->size < ((struct header*)b)->size)?1:0;
}

struct heap *create_heap(unsigned int start, unsigned int end_addr, unsigned int max, unsigned char supervisor, unsigned char readonly)
{
    struct heap *heap = (struct heap *) kmalloc(sizeof(struct heap));

    // All our assumptions are made on startAddress and endAddress being page-aligned.
    //ASSERT(start%0x1000 == 0);
    if((start % 0x1000 != 0))
    {
        kprintf("start not page-aligned\n");
    }
    //ASSERT(end_addr%0x1000 == 0);
    if((end_addr % 0x1000) != 0)
    {
        kprintf("end not page-aligned\n");
    }
    
    // Initialise the index.
    heap->index = place_ordered_array((void*)start, HEAP_INDEX_SIZE, &header_t_less_than);
    
    // Shift the start address forward to resemble where we can start putting data.
    start += sizeof(type_t)*HEAP_INDEX_SIZE;

    // Make sure the start address is page-aligned.
    if((start) & (0xFFFFF000 != 0))
    {
        start &= 0xFFFFF000;
        start += 0x1000;
    }
    // Write the start, end and max addresses into the heap structure.
    heap->start_address = start;
    heap->end_address = end_addr;
    heap->max_address = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;
    strcpy(heap->flag, "XKRN");

    // We start off with one large hole in the index.
    struct header *hole = (struct header *)start;
    hole->size = end_addr-start;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array((void*)hole, &heap->index);     

	return heap;
}

void *alloc(unsigned int size, unsigned char page_align, struct heap *heap)
{

    // Make sure we take the size of header/footer into account.
    unsigned int new_size = size + sizeof(struct header) + sizeof(struct footer);
    // Find the smallest hole that will fit.
    int iterator = find_smallest_hole(new_size, page_align, heap);

    if (iterator == -1) // If we didn't find a suitable hole
    {
        // Save some previous data.
        unsigned int old_length = heap->end_address - heap->start_address;
        unsigned int old_end_address = heap->end_address;

        // We need to allocate some more space.
        expand(old_length+new_size, heap);
        unsigned int new_length = heap->end_address-heap->start_address;

        // Find the endmost header. (Not endmost in size, but in location).
        iterator = 0;
        // Vars to hold the index of, and value of, the endmost header found so far.
        int idx = -1; unsigned int value = 0x0;
        while ((unsigned int) iterator < heap->index.size)
        {
            unsigned int tmp = (unsigned int)lookup_ordered_array(iterator, &heap->index);
            if (tmp > value)
            {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        // If we didn't find ANY headers, we need to add one.
        if (idx == -1)
        {
            struct header *header = (struct header *)old_end_address;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;
            struct footer *footer = (struct footer *) (old_end_address + header->size - sizeof(struct footer));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void*)header, &heap->index);
        }
        else
        {
            // The last header needs adjusting.
            struct header *header = lookup_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;
            // Rewrite the footer.
            struct footer *footer = (struct footer *) ( (unsigned int)header + header->size - sizeof(struct footer) );
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
        // We now have enough space. Recurse, and call the function again.
        return alloc(size, page_align, heap);
    }

    struct header *orig_hole_header = (struct header *)lookup_ordered_array(iterator, &heap->index);
    unsigned int orig_hole_pos = (unsigned int)orig_hole_header;
    unsigned int orig_hole_size = orig_hole_header->size;
    // Here we work out if we should split the hole we found into two parts.
    // Is the original hole size - requested hole size less than the overhead for adding a new hole?
    if (orig_hole_size-new_size < sizeof(struct header)+sizeof(struct footer))
    {
        // Then just increase the requested size to the size of the hole we found.
        size += orig_hole_size-new_size;
        new_size = orig_hole_size;
    }

    // If we need to page-align the data, do it now and make a new hole in front of our block.
    if (page_align && orig_hole_pos&0xFFFFF000)
    {
        unsigned int new_location   = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(struct header);
        struct header *hole_header = (struct header *)orig_hole_pos;
        hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(struct header);
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        struct footer *hole_footer = (struct footer *) ( (unsigned int)new_location - sizeof(struct footer) );
        hole_footer->magic    = HEAP_MAGIC;
        hole_footer->header   = hole_header;
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->size;
    }
    else
    {
        // Else we don't need this hole any more, delete it from the index.
        remove_ordered_array(iterator, &heap->index);
    }

    // Overwrite the original header...
    struct header *block_header  = (struct header *)orig_hole_pos;
    block_header->magic     = HEAP_MAGIC;
    block_header->is_hole   = 0;
    block_header->size      = new_size;
    // ...And the footer
    struct footer *block_footer  = (struct footer *) (orig_hole_pos + sizeof(struct header) + size);
    block_footer->magic     = HEAP_MAGIC;
    block_footer->header    = block_header;

    // We may need to write a new hole after the allocated block.
    // We do this only if the new hole would have positive size...
    if (orig_hole_size - new_size > 0)
    {
        struct header *hole_header = (struct header *) (orig_hole_pos + sizeof(struct header) + size + sizeof(struct footer));
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        hole_header->size     = orig_hole_size - new_size;
        struct footer *hole_footer = (struct footer *) ( (unsigned int)hole_header + orig_hole_size - new_size - sizeof(struct footer) );
        if ((unsigned int)hole_footer < heap->end_address)
        {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }
        // Put the new hole in the index;
        insert_ordered_array((void*)hole_header, &heap->index);
    }
    
    // ...And we're done!
    return (void *) ( (unsigned int)block_header+sizeof(struct header) );
}

void free(void *p, struct heap *heap)
{
    // Exit gracefully for null pointers.
    if (p == 0)
        return;

    // Get the header and footer associated with this pointer.
    struct header *header = (struct header*) ( (unsigned int)p - sizeof(struct header) );
    struct footer *footer = (struct footer*) ( (unsigned int)header + header->size - sizeof(struct footer) );

    // Sanity checks.
    //ASSERT(header->magic == HEAP_MAGIC);
    //ASSERT(footer->magic == HEAP_MAGIC);
    if(header->magic != HEAP_MAGIC)
    {
        kprintf("Corrupt block\n");
    }
    if(footer->magic != HEAP_MAGIC)
    {
        kprintf("Corrupt block\n");
    }

    // Make us a hole.
    header->is_hole = 1;

    // Do we want to add this header into the 'free holes' index?
    char do_add = 1;

    // Unify left
    // If the thing immediately to the left of us is a footer...
    struct footer *test_footer = (struct footer *) ( (unsigned int)header - sizeof(struct footer) );
    if (test_footer->magic == HEAP_MAGIC &&
        test_footer->header->is_hole == 1)
    {
        unsigned int cache_size = header->size; // Cache our current size.
        header = test_footer->header;     // Rewrite our header with the new one.
        footer->header = header;          // Rewrite our footer to point to the new header.
        header->size += cache_size;       // Change the size.
        do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
    }

    // Unify right
    // If the thing immediately to the right of us is a header...
    struct header *test_header = (struct header*) ( (unsigned int)footer + sizeof(struct footer) );
    if (test_header->magic == HEAP_MAGIC &&
        test_header->is_hole)
    {
        header->size += test_header->size; // Increase our size.
        test_footer = (struct footer *) ( (unsigned int)test_header + // Rewrite it's footer to point to our header.
                                    test_header->size - sizeof(struct footer) );
        footer = test_footer;
        // Find and remove this header from the index.
        unsigned int iterator = 0;
        while ( (iterator < heap->index.size) &&
                (lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
            iterator++;

        // Make sure we actually found the item.
        if(iterator >= heap->index.size)
        {
            kprintf("object not found\n");
        }
        // Remove it.
        remove_ordered_array(iterator, &heap->index);
    }

    // If the footer location is the end address, we can contract.
    if ( (unsigned int)footer+sizeof(struct footer) == heap->end_address)
    {
        unsigned int old_length = heap->end_address-heap->start_address;
        unsigned int new_length = contract( (unsigned int)header - heap->start_address, heap);
        // Check how big we will be after resizing.
        if (header->size - (old_length-new_length) > 0)
        {
            // We will still exist, so resize us.
            header->size -= old_length-new_length;
            footer = (struct footer *) ( (unsigned int)header + header->size - sizeof(struct footer));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
        }
        else
        {
            // We will no longer exist :(. Remove us from the index.
            unsigned int iterator = 0;
            while ( (iterator < heap->index.size) &&
                    (lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
                iterator++;
            // If we didn't find ourselves, we have nothing to remove.
            if (iterator < heap->index.size)
                remove_ordered_array(iterator, &heap->index);
        }
    }

    // If required, add us to the index.
    if (do_add == 1)
        insert_ordered_array((void*)header, &heap->index);

}
