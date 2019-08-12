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

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t aszie);
static void place(void *bp, size_t asize);

static char *heap_listp;    /* points to prologue block of heap */

/* Basic constants and macros */
#define WSIZE	    4		/* Word and header/footer size (bytes) */
#define DSIZE	    8		/* Double word size (bytes) */
#define CHUNKSIZE   (1<<12)	/* Extend heap by this amount (bytes) */

#define MAX(x, y)   ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)   ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)		(*(unsigned int *)(p))
#define PUT(p, val)	(*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)	(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)	((char *)(bp) - WSIZE)
#define FTRP(bp)	((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
	return -1;
    PUT(heap_listp, 0);				    /* ALignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));	    /* Epilogue header */
    heap_listp += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
	return -1;
    return 0;
}

/* 
 * extend_heap - extends the heap with a new free block.
 *     To maintain alignment, extend_heap rounds up the requested size to 
 *     the nearest multiple of 2 words (8 bytes) and then requests the 
 *     additional heap space from the memory system.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
	return NULL;
    
    //printf("  --- extend_heap: %d\n", size);
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));	    /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));	    /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;	/* Adiusted block size */
    size_t extendsize;	/* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
	return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
	asize = 2*DSIZE;
    else
	asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);
    
    //printf("\n>>> mm_malloc size: %d\n", asize);
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL){
	//printf("  --- find_fit: %p\n", bp);
	place(bp, asize);
	return bp;
    }
    
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
	return NULL;
    //printf("  --- extend_heap: %p\n", bp);
    place(bp, asize);
    return bp;
}

static void *find_fit(size_t asize){
    char *bp = heap_listp + DSIZE;
    while (GET(HDRP(bp)) != 1){
	if (!(GET_ALLOC(HDRP(bp))) && (GET_SIZE(HDRP(bp)) >= asize))
	    return bp;
	else
	    bp = NEXT_BLKP(bp);
    }
    return NULL;
}

static void place(void *bp, size_t asize){
    size_t leftsize = GET_SIZE(HDRP(bp)) - asize;
    if (leftsize >= 2*DSIZE){
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(leftsize, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(leftsize, 0));
    } else {
	PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
	PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
    }
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    //printf("\n>>> mm_free: %p\n", bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc){
	//printf("  --- coalesce: %p\n", bp);
	return bp;
    }
    else if (prev_alloc && !next_alloc){
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	//PUT(HDRP(bp), PACK(size, 0));
	//PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc){
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	//PUT(FTRP(bp), PACK(size, 0));
	//PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }
    else {
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
	//PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	//PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    //printf("  --- coalesce: %p\n", bp);
    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
/*void *mm_realloc(void *ptr, size_t size)
{
    size_t cursize;
    char *bp;
    
    if (ptr == NULL)
	return mm_malloc(size);

    if (size == 0){
	mm_free(ptr);
	return NULL;
    }

    cursize = GET_SIZE(HDRP(ptr));
    if (size < cursize)	cursize = size;
    bp = mm_malloc(size);
    memcpy(bp, ptr, cursize);
    mm_free(ptr);

    return bp;
}
*/
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    char *bp;
    size_t asize, cursize, presize, nextsize, allsize;
    unsigned int prealloc, nextalloc, flag;
    
    //printf("\n>>> mm_realloc: %p\n", ptr);
    if (ptr == NULL)
	return mm_malloc(size);

    if (size == 0){
	mm_free(ptr);
	return NULL;
    }

    if (size <= DSIZE)
	asize = 2*DSIZE;
    else
	asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);
    
    cursize = GET_SIZE(HDRP(ptr));
    if (cursize == asize)
	return ptr;

    prealloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    nextalloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    presize = GET_SIZE(HDRP(PREV_BLKP(ptr)));
    nextsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    flag = (prealloc << 1) | nextalloc;
    
    if (cursize > asize){
	switch(flag){
	    case 0:
		allsize = presize + cursize + nextsize;
		bp = PREV_BLKP(ptr);
		memcpy(bp, ptr, asize);
		break;
	    case 1:
		allsize = presize + cursize;
		bp = PREV_BLKP(ptr);
		memcpy(bp, ptr, asize);
		break;
	    case 2:
		allsize = cursize + nextsize;
		bp = ptr;
		break;
	    case 3:
		allsize = cursize;
		if ((cursize - asize) < 2*DSIZE){
		    PUT(HDRP(ptr), PACK(allsize, 1));
		    PUT(FTRP(ptr), PACK(allsize, 1));
		    return ptr;
		}
		break;
	    default:
		printf("mm_realloc error!\n");
		break;
	}
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(allsize-asize, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(allsize-asize, 0));
    } else {
	switch(flag){
	    case 0:
		allsize = presize + cursize + nextsize;
		bp = PREV_BLKP(ptr);
		break;
	    case 1:
		allsize = presize + cursize;
		bp = PREV_BLKP(ptr);
		break;
	    case 2:
		allsize = cursize + nextsize;
		bp = ptr;
		break;
	    case 3:
		allsize = cursize;
		bp = ptr;
		break;
	    default:
		printf("mm_realloc error!\n");
	}
	if (allsize < asize){
	    bp = mm_malloc(size);
	    memcpy(bp, ptr, cursize);
	    mm_free(ptr);
	} else {
	    //printf("prealloc, nextalloc, flag: %d, %d, %d\n", prealloc, nextalloc, flag);
	    //printf("bp,ptr,cursize: %p, %p, %d\n", bp, ptr, cursize);
	    //memcpy(bp, ptr, cursize);
	    memmove(bp, ptr, cursize);
	    if ((allsize - asize) >= 2*DSIZE){
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(bp)), PACK(allsize-asize, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(allsize-asize, 0));
	    } else {
		PUT(HDRP(bp), PACK(allsize, 1));
		PUT(FTRP(bp), PACK(allsize, 1));
	    }  
	}
    }
    //printf("  --- mm_realloc: %p\n", bp);
    return bp;
}



/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
/*void *mm_realloc(void *ptr, size_t size)
{
    size_t asize, oldsize, tempsize;
    char *bp;
    
    if (ptr == NULL)
	return mm_malloc(size);

    if (size == 0){
	mm_free(ptr);
	return NULL;
    }

    if (size <= DSIZE)
	asize = 2*DSIZE;
    else
	asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);
    
    oldsize = GET_SIZE(HDRP(ptr));
    if (oldsize == asize)
	return ptr;

    if (oldsize < asize){
	if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) && (GET_SIZE(HDRP(NEXT_BLKP(ptr))) >= (asize-oldsize))){
	    tempsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	    if (tempsize  >= (asize-oldsize+2*DSIZE)){
		PUT(HDRP(ptr), PACK(asize, 1));
		PUT(FTRP(ptr), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(ptr)), PACK(tempsize+oldsize-asize, 0));
		PUT(FTRP(NEXT_BLKP(ptr)), PACK(tempsize+oldsize-asize, 0));
	    }else{
		PUT(HDRP(ptr), PACK(tempsize+oldsize, 1));
		PUT(FTRP(ptr), PACK(tempsize+oldsize, 1));
	    }
	}else{
	    bp = mm_malloc(asize);
	    memcpy(bp, ptr, oldsize);
	    mm_free(ptr);
	    return bp;
	}
    }else {
	if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))){
	    tempsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	    PUT(HDRP(ptr), PACK(asize, 1));
	    PUT(FTRP(ptr), PACK(asize, 1));
	    PUT(HDRP(NEXT_BLKP(ptr)), PACK(tempsize+oldsize-asize, 0));
	    PUT(FTRP(NEXT_BLKP(ptr)), PACK(tempsize+oldsize-asize, 0));
	}else{
	    if ((oldsize-asize) >= 2*DSIZE){
		PUT(HDRP(ptr), PACK(asize, 1));
		PUT(FTRP(ptr), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(ptr)), PACK(oldsize-asize, 0));
		PUT(FTRP(NEXT_BLKP(ptr)), PACK(oldsize-asize, 0));
	    }
	}
    }
    
    return ptr;
}
*/
