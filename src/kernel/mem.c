#include <kernel/mem.h>
#include <kernel/atag.h>
#include <common/stdlib.h>
#include <stdint.h>
#include <stddef.h>

// Heap stuffs
static void heap_init(uint32_t heap_start);
/* Implement kmalloc as a linked list of allocated segments.
   Segments should be 4 byte aligned.
   Use best fit algorithm to find an allocation. */
typedef struct heap_segment {
    struct heap_segment *next;
    struct heap_segment *prev;
    uint32_t is_allocated;
    uint32_t segment_size; // Including this header
} heap_segment_t;

static heap_segment_t *heap_segment_list_head;

// End heap stuffs

extern uint8_t __end;
static uint32_t num_pages;

DEFINE_LIST(page);
IMPLEMENT_LIST(page);

static page_t *all_pages_array;
page_list_t free_pages;

void mem_init(atag_t *atags) {
    uint32_t mem_size, page_array_len, kernel_pages, page_array_end, i;

    // Get total num of pages
    mem_size = get_mem_size(atags);
    num_pages = mem_size / PAGE_SIZE;

    // Allocate space for all those pages' metadata.
    // Start this block just after the kernel image is finished.
    page_array_len = sizeof(page_t) * num_pages;
    all_pages_array = (page_t *)&__end;
    bzero(all_pages_array, page_array_len);
    INITIALIZE_LIST(free_pages);

    // Mark appropriate flags for all pages
    // Start with kernel pages
    kernel_pages = ((uint32_t)&__end) / PAGE_SIZE;
    for(i = 0; i < kernel_pages; i++) {
        all_pages_array[i].vaddr_mapped = i * PAGE_SIZE; // Identity map the kernel pages
        all_pages_array[i].flags.allocated = 1;
        all_pages_array[i].flags.kernel_page = 1;
    }

    // Reserve KERNEL_HEAP_SIZE for the kernel heap
    for(; i < kernel_pages + (KERNEL_HEAP_SIZE / PAGE_SIZE); i++) {
        all_pages_array[i].vaddr_mapped = i * PAGE_SIZE; // Identity map the kernel pages
        all_pages_array[i].flags.allocated = 1;
        all_pages_array[i].flags.kernel_heap_page = 1;
    }

    // Map the rest of the pages as unallocated, and add them to the free list
    for(; i < num_pages; i++) {
        all_pages_array[i].flags.allocated = 0;
        append_page_list(&free_pages, &all_pages_array[i]);
    }

    // Init the heap
    // TODO: is this for the kernel?
    page_array_end = (uint32_t)&__end + page_array_len;
    heap_init(page_array_end);
}

void *alloc_page(void) {
    page_t *page;
    void *page_mem;

    if(size_page_list(&free_pages) == 0) {
        return 0;
    }

    // Get a free page
    page = pop_page_list(&free_pages);
    page->flags.kernel_page = 1;
    page->flags.allocated = 1;

    // Get the address the physical page metadata refers to
    page_mem = (void *)((page - all_pages_array) * PAGE_SIZE);

    // Zero the page, so data doesn't leak between users and kernel
    bzero(page_mem, PAGE_SIZE);

    return page_mem;
}

void free_page(void *ptr) {
    page_t *page;

    // Get page metadata from the physical address
    page = all_pages_array + ((uint32_t)ptr / PAGE_SIZE);

    // Mark the page as free
    page->flags.allocated = 0;
    append_page_list(&free_pages, page);
}

static void heap_init(uint32_t heap_start) {
    heap_segment_list_head = (heap_segment_t *)heap_start;
    bzero(heap_segment_list_head, sizeof(heap_segment_t));
    heap_segment_list_head->segment_size = KERNEL_HEAP_SIZE;
}
