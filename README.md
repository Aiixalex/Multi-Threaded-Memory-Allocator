# Multi-Threaded-Memory-Allocator
A multi-threaded memory allocator.

## Install

Install the repository.

```shell
$ make
$ ./myalloc
```

## Usage

### Initialization

API:

```c
void initialize_allocator(int size, enum allocation_algorithm);
```

The allocator needs to know the total size of the contiguous memory that is assumed for the rest of the program. Any requests for allocation and deallocation requests will be served from this contiguous chunk. The memory chunk is allocated using `malloc()` and its content is initialized to 0 using `memset`. and the memory allocation algorithm to be used. 

This memory allocator uses the following memory allocation algorithms:

* FIRST_FIT satisfies the allocation request from the first available memory block (from left) that is at least as large as the requested size.

* BEST_FIT satisfies the allocation request from the available memory block that at least as large as the requested size and that results in the smallest remainder fragment.

* WORST_FIT satisfies the allocation request from the available memory block that at least as large as the requested size and that results in the largest remainder fragment.

All information of the contiguous memory chunk is represented in `struct Myalloc`:

```c
struct Myalloc {
    enum allocation_algorithm aalgorithm;
    int size;
    void* memory;
    struct nodeStruct *alloc_blocks;
    struct nodeStruct *free_blocks;
};
```

`alloc_blocks` and `free_blocks` are two separate singly linked lists (implemented in `list.h` and `list.c`) to manage free/allocated space. When a block gets allocated by `allocate()`, its metadata must be inserted to the list of allocated blocks. Similarly when a block gets freed by `deallocate()`, its metadata must be inserted to the list of free blocks. Note that the free list must never maintain contiguous free blocks.

![1](1.bmp)

### Metadata Management

The size of each allocated/freed memory block is stored in header. The header should only contain a single 8-byte word that denotes the size of the actual allocation. For example, if the request asks for 16 bytes of memory, you should actually allocate 8 + 16 bytes, and use the first 8-byte to store the size of the total allocation (24 bytes) and return a pointer to the user-visible 16-byte.

### Compaction

Compaction is performed by grouping the allocated memory blocks in the beginning of the memory chunk and combining the free memory at the end of the memory chunk.

![2](2.bmp)

The compaction API as shown below:

```c
int compact_allocation(void** _before, void** _after);
```

The API accepts `_before` and `_after` arrays of `void*` pointers and insert the previous address and new address of each relocated block in `_before` and `_after`. The return integer is the total number of pointers inserted in the `_before` or `_after` array.

### Multi-threading Support

The allocator uses a simple design that uses a global `pthread_mutex` to protect the entire allocator. That is, all the functions must first acquire the mutex before continuing to do its work, and must release the mutex before returning.

### Example

The tested code is in `main.c`.

```shell
$ ./myalloc
Using first fit algorithm on memory size 100
p[0] = 0x7fffeee71268 ; *p[0] = 0
p[1] = 0x7fffeee71274 ; *p[1] = 1
p[2] = 0x7fffeee71280 ; *p[2] = 2
p[3] = 0x7fffeee7128c ; *p[3] = 3
p[4] = 0x7fffeee71298 ; *p[4] = 4
p[5] = 0x7fffeee712a4 ; *p[5] = 5
p[6] = 0x7fffeee712b0 ; *p[6] = 6
p[7] = 0x7fffeee712bc ; *p[7] = 7
Allocation failed
Allocation failed
Allocated size = 36
Allocated chunks = 8
Free size = 0
Free chunks = 0
Largest free chunk size = 0
Smallest free chunk size = 0
Freeing p[1]
Freeing p[3]
Freeing p[5]
Freeing p[7]
Freeing p[9]
Allocated size = 16
Allocated chunks = 4
Free size = 20
Free chunks = 4
Largest free chunk size = 8
Smallest free chunk size = 4
available_memory 20
Allocated size = 4
Allocated chunks = 4
Free size = 44
Free chunks = 1
Largest free chunk size = 44
Smallest free chunk size = 44
```

