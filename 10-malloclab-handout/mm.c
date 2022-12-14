/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* Basic constants and macros */
#define WSIZE 4                 /* Word and header/footer size (bytes) */
#define DSIZE 8                 /* Double word size (bytes) */
#define MIN_BLOCK_SIZE 16
#define CHUNKSIZE (1 << 12)     /* Extend hea by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/*
 * Block struct:
 * +-+-+-+-+-+-+-+-+-+-+
 * | Block Size  | 00A | A: Alloc
 * +-+-+-+-+-+-+-+-+-+-+
 * |       pred        |
 * +-+-+-+-+-+-+-+-+-+-+
 * |       succ        |
 * +-+-+-+-+-+-+-+-+-+-+
 * |      Payload      |
 * +-+-+-+-+-+-+-+-+-+-+
 * |      Padding      |
 * +-+-+-+-+-+-+-+-+-+-+
 * | Block Size  | 00A | A: Alloc
 * +-+-+-+-+-+-+-+-+-+-+
 */

/* Read and write a word at address p */
#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)


/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

/* Given block ptr bp, compute pred and succ block */
#define FD(bp) (*(char**)(bp))
#define BK(bp) (*(char**)((bp) + WSIZE))

/* Given block ptr bp, compute pred and succ ptr address*/
#define FD_PTR(bp) ((char*)(bp))
#define BK_PTR(bp) ((char*)(bp + WSIZE))

#define SET_FD(bp, val) (*(unsigned int*)(bp) = (unsigned int)(val))
#define SET_BK(bp, val) (*(unsigned int*)(bp+WSIZE) = (unsigned int)(val))

static char *heap_listp;

#define FREE_LIST_SIZE 10
void *free_lists[FREE_LIST_SIZE] = {NULL};


/* Following funcions was method of free_lists */
size_t size_to_type(size_t size) {
    size_t x = 0;
    if   (size < 16)        ;
    else if (size <= 32)    x = 0;
    else if (size <= 64)    x = 1;
    else if (size <= 128)   x = 2;
    else if (size <= 256)   x = 3;
    else if (size <= 512)   x = 4;
    else if (size <= 1024)  x = 5;
    else if (size <= 2048)  x = 6;
    else if (size <= 4096)  x = 7;
    else                    x = 8;
    return x;
}

static void insert_node_to_free_list(void *bp, size_t size){
    size_t list_num = size_to_type(size);
    size_t last_head = (size_t)free_lists[list_num];
    SET_BK(bp, last_head);
    if (last_head != NULL) {
        SET_FD(last_head, bp);
    }
    SET_FD(bp, 0);
    free_lists[list_num] = bp;
    return;
}

static void delete_node_from_free_list(void *bp, size_t size) {
    if (FD(bp) != NULL)
        SET_BK(FD(bp), BK(bp));
    else
        free_lists[size_to_type(size)] = BK(bp);
    if (BK(bp) != NULL)
        SET_FD(BK(bp), FD(bp));
    return;
}

/* place the block to a free block */
static void place(void *bp, size_t size){
    size_t origin_size = GET_SIZE(HDRP(bp));
    delete_node_from_free_list(bp, origin_size);
    if (origin_size - size >= 2 * DSIZE) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        size_t remain_size = origin_size - size;
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
        insert_node_to_free_list(NEXT_BLKP(bp), remain_size);
    } else {
        PUT(HDRP(bp), PACK(origin_size, 1));
        PUT(FTRP(bp), PACK(origin_size, 1));
    }
    return;
}

/* find a fit free block (first fit) */
static void *find_fit(size_t size) {
    size_t list_num = size_to_type(size);
    for (int i = list_num; i < FREE_LIST_SIZE; ++i)
        if (free_lists[i] != NULL)
            for (void* tmp_ptr = free_lists[i]; tmp_ptr != NULL; tmp_ptr = BK(tmp_ptr))
                if (GET_SIZE(HDRP(tmp_ptr)) >= size)
                    return tmp_ptr;
    return NULL;
}

/*
 * coalesce - Coalesce free block next by
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
        return bp;

    /* coalesce with next block */
    if (!next_alloc && prev_alloc) {
        size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node_from_free_list(bp, size);
        delete_node_from_free_list(NEXT_BLKP(bp), next_size);
        size += next_size;
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(bp), PACK(size, 0));
    }

    /* coalesce with prev block */
    if (!prev_alloc && next_alloc) {
        size_t prev_size = GET_SIZE(FTRP(PREV_BLKP(bp)));
        delete_node_from_free_list(bp, size);
        delete_node_from_free_list(PREV_BLKP(bp), prev_size);
        size += prev_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    /* coalesce with prev and next block */
    if (!prev_alloc && !next_alloc) {
        size_t prev_size = GET_SIZE(FTRP(PREV_BLKP(bp)));
        size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node_from_free_list(bp, size);
        delete_node_from_free_list(PREV_BLKP(bp), prev_size);
        delete_node_from_free_list(NEXT_BLKP(bp), next_size);
        size += prev_size + next_size;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    };

    insert_node_to_free_list(bp, size);
    return bp;
}

/*
 * extend_heap - extend heap with a free block
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;
    /* Allocate and even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    insert_node_to_free_list(bp, size);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)   /* sbrk() function returns the start address of the new area. */
        return -1;
    for (int i = 0; i < FREE_LIST_SIZE; ++i)
        free_lists[i] = NULL;
    PUT(heap_listp, 0);                                     /* Alignment */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));          /* Prologue block's size is 8 and is always alloced */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));              /* Epilogue header */
    heap_listp += (2 * WSIZE);
    
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /* Ignore spurious requests */
    if (size <= 0)
        return NULL;
    if (size <= DSIZE)
        size = 2 * DSIZE;
    size_t newsize = size + DSIZE + ((-size) % DSIZE);
    void* bp;
    // int newsize = ALIGN(size + SIZE_T_SIZE);
    if ((bp = find_fit(newsize)) != NULL) {
        place(bp, newsize);
        return bp;
    }
    size_t extendsize = MAX(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
	   return NULL;
    place(bp, newsize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    insert_node_to_free_list(ptr, size);
    coalesce(ptr);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size){
    void *nblock = ptr;
    int reamin;
    if (size == 0) return NULL;
    if (size <= DSIZE) size = 2 * DSIZE;
    else size = ALIGN(size + 8);
    if ((reamin = GET_SIZE(HDRP(ptr)) - size) >= 0){
        return ptr;
    }
    else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) || !GET_SIZE(HDRP(NEXT_BLKP(ptr)))){
        if ((reamin = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - size) < 0){
            if (extend_heap(MAX(-reamin, CHUNKSIZE)/WSIZE) == NULL)
                return NULL;
            reamin += MAX(-reamin, CHUNKSIZE);
        }
        delete_node_from_free_list(NEXT_BLKP(ptr), GET_SIZE(HDRP(NEXT_BLKP(ptr))));
        PUT(HDRP(ptr), PACK(size + reamin, 1));
        PUT(FTRP(ptr), PACK(size + reamin, 1));
    }
    else{
        nblock = mm_malloc(size);
        memcpy(nblock, ptr, GET_SIZE(HDRP(ptr)));
        mm_free(ptr);
    }
    return nblock;
}

// Results for mm malloc:
// trace  valid  util     ops      secs  Kops
//  0       yes   98%    5694  0.000162 35235
//  1       yes   94%    5848  0.000171 34199
//  2       yes   98%    6648  0.000200 33273
//  3       yes   99%    5380  0.000141 38102
//  4       yes   66%   14400  0.000268 53771
//  5       yes   89%    4800  0.000182 26446
//  6       yes   86%    4800  0.000184 26130
//  7       yes   55%   12000  0.000198 60759
//  8       yes   51%   24000  0.000446 53848
//  9       yes   99%   14401  0.000141101774
// 10       yes   67%   14401  0.000127113215
// Total          82%  112372  0.002218 50652

// Perf index = 49 (util) + 40 (thru) = 89/100