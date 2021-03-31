#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myalloc.h"
#include "list.h"

struct Myalloc {
    enum allocation_algorithm aalgorithm;
    int size;
    void* memory;
    struct nodeStruct *alloc_blocks;
    struct nodeStruct *free_blocks;
};

struct Myalloc myalloc;

void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
    assert(_size > 0);
    myalloc.aalgorithm = _aalgorithm;
    myalloc.size = _size;
    myalloc.memory = malloc((size_t)myalloc.size);

    memset(myalloc.memory, 0, myalloc.size);

    void* curr_memory = myalloc.memory;
    long block_size = _size;
    memcpy(curr_memory, &block_size, 8); // header

    curr_memory += 8;
    myalloc.free_blocks = List_createNode(curr_memory);

}

void destroy_allocator() {
    free(myalloc.memory);

    // free other dynamic allocated memory to avoid memory leak
}

// Allocate memory from myalloc.memory 
// ptr = address of allocated memory
void* allocate(int _size) {
    void* ptr = NULL;
    void* hptr = NULL;
    long alloc_block_size;
    long free_block_size;

    struct nodeStruct *fit_free_block = myalloc.free_blocks;

    // FIRST_FIT satisfies the allocation request from the first available memory
    // block (from left) that is at least as large as the requested size.

    // BEST_FIT satisfies the allocation request from the available memory
    // block that at least as large as the requested size and that results
    // in the smallest remainder fragment.

    // WORST_FIT satisfies the allocation request from the available memory
    // block that at least as large as the requested size and that results
    // in the largest remainder fragment.
    if (myalloc.aalgorithm == FIRST_FIT) { }
    else if (myalloc.aalgorithm == BEST_FIT) {
        List_ascend_sort(&myalloc.free_blocks);
    }
    else if (myalloc.aalgorithm == WORST_FIT) {
        List_descend_sort(&myalloc.free_blocks);
    }

    while (fit_free_block != NULL) {
        if (*(long *)(fit_free_block->blockptr - 8) >= _size + 8) {
            ptr = fit_free_block->blockptr;                 // pointer to user-visible memory
            hptr = ptr - 8;                                 // pointer to header
            alloc_block_size = _size + 8;                   // allocated bytes
            free_block_size = *(long *)hptr - (_size + 8);  // unallocated bytes

            // Unallocated bytes can become a free block with an 8-byte header.
            if (free_block_size > 8) {
                // insert block to allocated block list
                memcpy(hptr, &alloc_block_size, 8);
                struct nodeStruct *new_alloc_block = List_createNode(ptr);
                List_insertTail(&myalloc.alloc_blocks, new_alloc_block);

                // modify the fitted free block in free block list
                hptr += (_size + 8);
                memcpy(hptr, &free_block_size, 8);
                fit_free_block->blockptr = hptr + 8;
            }
            // Unallocated bytes cannot become a free block with an 8-byte header.
            else {
                // insert block to allocated block list
                alloc_block_size += free_block_size;
                memcpy(hptr, &alloc_block_size, 8);
                struct nodeStruct *new_alloc_block = List_createNode(ptr);
                List_insertTail(&myalloc.alloc_blocks, new_alloc_block);

                // delete the fitted free block in free block list
                List_deleteNode(&myalloc.free_blocks, fit_free_block);
            }
            
            break;
        }
        fit_free_block = fit_free_block -> next;
    }

    return ptr;
}

// Free allocated memory
// _ptr points to the user-visible memory. The size information is
// stored at (char*)_ptr - 8.
void deallocate(void* _ptr) {
    if (_ptr == NULL) { 
        return;
    }

    // delete allocated block from list
    struct nodeStruct *dealloc_block = List_findNode(myalloc.alloc_blocks, _ptr);
    List_deleteNode(&myalloc.alloc_blocks, dealloc_block);

    // insert free block to list
    struct nodeStruct *new_free_block = List_createNode(_ptr);
    List_insertTail(&myalloc.free_blocks, new_free_block);

    // clear contents of deallocated block,  keep the header
    memset(_ptr, 0, *(long *)(_ptr - 8) - 8);

    // merge contiguous free blocks
    List_sort(&myalloc.alloc_blocks); // sort the allocated list in ascending order of address
    struct nodeStruct *free_block = myalloc.free_blocks;
    while (free_block != NULL && free_block->next != NULL) {
        int is_contiguous = 0;
        long block_size = *(long *)(free_block->blockptr - 8);

        // if free_block is contiguous with free_block->next
        if (free_block->blockptr + block_size == free_block->next->blockptr) {
            is_contiguous = 1;
        }

        if (is_contiguous) {
            // merge
            block_size += *(long *)(free_block->next->blockptr - 8);
            memcpy(free_block->blockptr - 8, &block_size, 8); // update the header of free_block
            List_deleteNode(&myalloc.free_blocks, free_block->next); // delete the free_block->next from list
            memset(free_block->next->blockptr - 8, 0, 8);     // clear the header of free_block->next
        }
        free_block = free_block->next;
    }
}

// compact allocated memory
// update _before, _after and compacted_size
int compact_allocation(void** _before, void** _after) {
    int compacted_size = 0;

    long block_size;
    void* before_ptr;
    void* after_ptr;
    long offset = 0;

    List_sort(&myalloc.alloc_blocks); // sort the allocated list in ascending order of address

    struct nodeStruct *prev_list = myalloc.alloc_blocks;
    
    // go through the sorted allocated list
    while (prev_list != NULL) {
        block_size = *(long *)(prev_list->blockptr - 8); // read block size from header
        before_ptr = prev_list->blockptr;                // read block ptr from list
        _before[compacted_size] = before_ptr;            // write prev addr in _before

        memcpy(myalloc.memory + offset, &block_size, 8); // set new header

        offset += 8;
        after_ptr = myalloc.memory + offset;
        _after[compacted_size] = after_ptr;              // write new addr in _after

        memmove(myalloc.memory + offset, before_ptr, block_size - 8); // copy contents of allocated block
        memset(before_ptr - 8, 0, block_size);           // clear prev allocated block

        offset += block_size - 8;
        compacted_size += 1;

        prev_list->blockptr = after_ptr;                 // update allocated list
        prev_list = prev_list->next;
    }

    // delete all free block nodes
    struct nodeStruct *temp = myalloc.free_blocks;
    while (temp != NULL) {
        struct nodeStruct *temp_next = temp->next;
        List_deleteNode(&myalloc.free_blocks, temp);
        temp = temp_next;
    }
    
    // set header of the compacted free block
    long compacted_free_size = myalloc.size - offset;
    memcpy(myalloc.memory + offset, &compacted_free_size, 8);
    
    // add the compacted free block to list
    struct nodeStruct *compacted_free_block = List_createNode(myalloc.memory + offset + 8);
    List_insertTail(&myalloc.free_blocks, compacted_free_block);

    return compacted_size;
}

int available_memory() {
    int available_memory_size = 0;
    // Calculate available memory size
    struct nodeStruct *f = myalloc.free_blocks;
    while (f != NULL) {
        available_memory_size += (int)*(long *)(f->blockptr - 8) - 8;

        f = f->next;
    }
    return available_memory_size;
}

void print_statistics() {
    int allocated_size = 0;
    int allocated_chunks = 0;
    int free_size = 0;
    int free_chunks = 0;
    int smallest_free_chunk_size = 0;
    int largest_free_chunk_size = 0;

    // Calculate the statistics
    struct nodeStruct *a = myalloc.alloc_blocks;
    struct nodeStruct *f = myalloc.free_blocks;
    long block_size;
    while (a != NULL) {
        block_size = *(long *)(a->blockptr - 8);
        allocated_size += (int)block_size - 8;
        allocated_chunks += 1;

        a = a->next;
    }

    while (f != NULL) {
        block_size = *(long *)(f->blockptr - 8);
        free_size += (int)block_size - 8;
        free_chunks += 1;

        f = f->next;
    }

    if (myalloc.free_blocks != NULL) {
        List_ascend_sort(&myalloc.free_blocks);
        smallest_free_chunk_size = (int)*(long *)(myalloc.free_blocks->blockptr - 8) - 8;
        List_descend_sort(&myalloc.free_blocks);
        largest_free_chunk_size = (int)*(long *)(myalloc.free_blocks->blockptr - 8) - 8;
    }

    printf("Allocated size = %d\n", allocated_size);
    printf("Allocated chunks = %d\n", allocated_chunks);
    printf("Free size = %d\n", free_size);
    printf("Free chunks = %d\n", free_chunks);
    printf("Largest free chunk size = %d\n", largest_free_chunk_size);
    printf("Smallest free chunk size = %d\n", smallest_free_chunk_size);
}
