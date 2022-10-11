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
#define CHUNKSIZE (1 << 12)     /* Extend hea by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)


/* Given block ptr bp, compte address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

/*
 * find_fit algorithm:
 * 1. first fit
 * 2. next fit
 * 3. best fit
 */
#define FIRST_FIT 1
#define NEXT_FIT 2
#define BEST_FIT 3

#define FIT_ALGO 2

static char *heap_listp;

#if FIT_ALGO == NEXT_FIT
char *cur_listp;
#endif

static void place(void *bp, size_t size){
    size_t origin_size = GET_SIZE(HDRP(bp));
    if (origin_size - size >= 2 * DSIZE) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));

        PUT(HDRP(NEXT_BLKP(bp)), PACK(origin_size - size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(origin_size - size, 0));
    } else {
        PUT(HDRP(bp), PACK(origin_size, 1));
        PUT(FTRP(bp), PACK(origin_size, 1));
    }
    return;
}

/* 
 * util: 74%
 * secs: 0.4585
 * Perf index = 44 (util) + 16 (thru) = 61 / 100
 */
#if FIT_ALGO == FIRST_FIT
static void *find_fit(size_t size) {
    void *tmp_listp = heap_listp;
    while (GET_SIZE(HDRP(tmp_listp)) < size || GET_ALLOC(HDRP(tmp_listp))) {
        if (GET_SIZE(HDRP(tmp_listp)) == 0) {
            return NULL;
        }
        tmp_listp = NEXT_BLKP(tmp_listp);
    }
    return tmp_listp;
}
/* 
 * util: 73%
 * secs: 0.0918
 * Perf index = 44 (util) + 40 (thru) = 84/100
 */
# elif FIT_ALGO == NEXT_FIT
static void *find_fit(size_t size) {
    void *tmp_listp;
    for (tmp_listp = cur_listp; GET_SIZE(HDRP(tmp_listp)) != 0; tmp_listp = NEXT_BLKP(tmp_listp)) {
        if (!GET_ALLOC(HDRP(tmp_listp)) && GET_SIZE(HDRP(tmp_listp)) >= size) {
            cur_listp = tmp_listp;
            return tmp_listp;
        }
    }
    for (tmp_listp = heap_listp; tmp_listp != cur_listp; tmp_listp = NEXT_BLKP(tmp_listp)) {
        if (!GET_ALLOC(HDRP(tmp_listp)) && GET_SIZE(HDRP(tmp_listp)) >= size) {
            cur_listp = tmp_listp;
            return tmp_listp;
        }
    }
    return NULL;
}
/*
 * util: 75%
 * secs: 0.4660
 * Perf index = 45 (util) + 16 (thru) = 61/100
 */
# elif FIT_ALGO == BEST_FIT
static void *find_fit(size_t size) {
    void *best_listp = NULL;
    size_t best_size = 0x7fffffff;
    for (void *tmp_listp = heap_listp; GET_SIZE(HDRP(tmp_listp)) != 0; tmp_listp = NEXT_BLKP(tmp_listp)) {
        size_t cur_size = GET_SIZE(HDRP(tmp_listp));
        if (!GET_ALLOC(HDRP(tmp_listp)) && cur_size >= size) {
            if (cur_size == size) {
                return tmp_listp;
            }
            if (cur_size < best_size) {
                best_size = cur_size;
                best_listp = tmp_listp;
            }
        }
    }
    return best_listp;
}

# endif

/*
 * coalesce - Coalesce free block next by
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc)
        return bp;
    if (!next_alloc && prev_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(bp), PACK(size, 0));
    }
    if (!prev_alloc && next_alloc) {
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    if (!prev_alloc && !next_alloc) {
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    };
    
    // size_t size = GET_SIZE(HDRP(bp)) + (1 - prev_alloc) * GET_SIZE(FTRP(PREV_BLKP(bp))) + (1 - next_alloc) * GET_SIZE(HDRP(NEXT_BLKP(bp)));
    // void *end_bp = bp;
    // if (!next_alloc)
    //     end_bp = NEXT_BLKP(end_bp);
    // PUT(FTRP(end_bp), PACK(size, 0));
    // if (!prev_alloc)
    //     bp = PREV_BLKP(bp);
    // PUT(HDRP(bp), PACK(size, 0));

#if FIT_ALGO == NEXT_FIT
    if (cur_listp > bp && cur_listp < NEXT_BLKP(bp))
        cur_listp = bp;
#endif

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
    PUT(heap_listp, 0);                                     /* Alignment */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));          /* Prologue block's size is 8 and is always alloced */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));              /* Epilogue header */
    heap_listp += (2 * WSIZE);
    
#if FIT_ALGO == NEXT_FIT
    cur_listp = heap_listp;
#endif

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

    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    size = GET_SIZE(HDRP(oldptr));
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    copySize = GET_SIZE(HDRP(newptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize - WSIZE);
    mm_free(oldptr);
    return newptr;
}














