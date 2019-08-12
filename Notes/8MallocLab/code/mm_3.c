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
#define GET_SIZE(p)		(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)	((char *)(bp) - WSIZE)
#define FTRP(bp)	((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

#define GET_PRED(p)			(GET(p))
#define PUT_PRED(p, val)    (PUT(p, val))
#define GET_SUCC(p)			(*(unsigned int *)((char *)(p) + WSIZE))
#define PUT_SUCC(p, val)    (*(unsigned int *)((char *)(p) + WSIZE) = (val))

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(8*WSIZE)) == (void *)-1)
		return -1;
    PUT(heap_listp, 0);								/* ALignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(2*DSIZE, 1));  /* Explicit Free Lists header */
    PUT(heap_listp + (2*WSIZE), 0);					/* Explicit Free Lists pred pointer */
    PUT(heap_listp + (3*WSIZE), 0);					/* Explicit Free Lists succ pointer */
    PUT(heap_listp + (4*WSIZE), PACK(2*DSIZE, 1));	/* Explicit Free Lists footer */
    PUT(heap_listp + (5*WSIZE), PACK(DSIZE, 1));    /* Epilogue header */
    PUT(heap_listp + (6*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (7*WSIZE), PACK(0, 1));	    /* Epilogue header */
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
    PUT_PRED(bp, 0);					/* Free block pred pointer */
    PUT_SUCC(bp, 0);					/* Free block succ pointer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block was free */
    if (!GET_ALLOC(HDRP(PREV_BLKP(bp)))){		// 若前面的块是空闲的，当前分配块与前面的块合并，该空闲块在空闲链表上的位置不变，大小改变
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));	
		return PREV_BLKP(bp);
    }
	// 若前面的块是已分配的，将当前分配块插入在空闲链表首位，注意判断空闲链表是否为空
    if (GET_SUCC(heap_listp) == 0){
		PUT_PRED(bp, heap_listp);
		PUT_SUCC(bp, 0);
		PUT_SUCC(heap_listp, bp);
    } else {
		PUT_PRED(bp, heap_listp);
		PUT_SUCC(bp, GET_SUCC(heap_listp));
		PUT_PRED(GET_SUCC(heap_listp), bp);
		PUT_SUCC(heap_listp, bp);
    }
    return bp;
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

    //displayEmptylist();
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
    //printf("+ find_fit\n");
    char *bp = GET_SUCC(heap_listp);
    while (bp != 0){		// 搜索空闲链表，首次适配方式
		if (GET_SIZE(HDRP(bp)) >= asize)
			return bp;
		else
			bp = GET_SUCC(bp);
    }
    return NULL;
}

static void place(void *bp, size_t asize){
    //printf("+ place: %p\n", bp);
    size_t leftsize = GET_SIZE(HDRP(bp)) - asize;
    if (leftsize >= 2*DSIZE){	// 剩余空间大于最小块空间，分割空闲块；剩余空闲块替换原空闲块在空闲链表中的位置
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(bp)), PACK(leftsize, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(leftsize, 0));
		PUT_PRED(NEXT_BLKP(bp), GET_PRED(bp));
		PUT_SUCC(NEXT_BLKP(bp), GET_SUCC(bp));
		PUT_SUCC(GET_PRED(bp), NEXT_BLKP(bp));
		if (GET_SUCC(bp) != 0)
			PUT_PRED(GET_SUCC(bp), NEXT_BLKP(bp));
    } else {					// 剩余空间小于最小块空间，不进行分割；将原空闲块在空闲链表中删除
		PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
		PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
		PUT_SUCC(GET_PRED(bp), GET_SUCC(bp));
		if (GET_SUCC(bp) != 0)
			PUT_PRED(GET_SUCC(bp), GET_PRED(bp));
	}
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    //displayEmptylist();
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
    //printf(" > coalesce\n");

    if (prev_alloc && next_alloc){			// 前面的块和后面的块都是已分配的；将该空闲块插入到空闲链表首位
		//printf("  --- coalesce: %p\n", bp);
		if (GET_SUCC(heap_listp) == 0){
			PUT_PRED(bp, heap_listp);
			PUT_SUCC(bp, 0);
			PUT_SUCC(heap_listp, bp);
		} else {
			PUT_PRED(bp, heap_listp);
			PUT_SUCC(bp, GET_SUCC(heap_listp));
			PUT_PRED(GET_SUCC(heap_listp), bp);
			PUT_SUCC(heap_listp, bp);
		}
		return bp;
    }
    else if (prev_alloc && !next_alloc){	// 前面的块已分配的，后面的块是空闲的；将该空闲块替换其后面的空闲块在空闲链表中的位置，并将两者合并
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		if (GET_SUCC(NEXT_BLKP(bp)) != 0)
			PUT_PRED(GET_SUCC(NEXT_BLKP(bp)), bp);
		PUT_SUCC(GET_PRED(NEXT_BLKP(bp)), bp);
		PUT_PRED(bp, GET_PRED(NEXT_BLKP(bp)));
		PUT_SUCC(bp, GET_SUCC(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc){	// 前面的块是空闲的，后面的块是已分配的；前面的块在空闲链表的位置不变，并将两者合并
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
    }
    else {									// 前面的块和后面的块都是空闲的；前面的块在空闲链表的位置不变，将后面的块从空闲链表中删除，并合并这三个空闲块
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		if (GET_SUCC(NEXT_BLKP(bp)) != 0)
			PUT_PRED(GET_SUCC(NEXT_BLKP(bp)), GET_PRED(NEXT_BLKP(bp)));
		PUT_SUCC(GET_PRED(NEXT_BLKP(bp)), GET_SUCC(NEXT_BLKP(bp)));
		bp = PREV_BLKP(bp);
    }

    //printf("  --- coalesce: %p\n", bp);
    return bp;
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t asize, cursize, nextsize, allsize;
    unsigned int nextalloc;
    char *bp;
    //displayEmptylist();
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

    //printf("\n>>> mm_realloc asize: %d\n", asize);
    cursize = GET_SIZE(HDRP(ptr));
    if (cursize == asize)
		return ptr;
   
    nextalloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    nextsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));

    if (cursize < asize){	// 当前块空间不足
		if (!nextalloc && ((cursize + nextsize) >= asize)){	// 后面的块是空闲的，且两者空间之和足够分配
			allsize = cursize + nextsize;	// 两者合并的总空间
			if ((allsize - asize) >= 2*DSIZE){	// 分配后剩余空间大于最小块空间，进行分割；剩余空闲块替换原后面的空闲块在空闲链表中的位置（此处应注意设置好相关块的前后指针，且注意堆块大小及分配标志位和相关块的前后指针设置顺序，防止取的覆盖原数据后的值）
				bp = NEXT_BLKP(ptr);
				PUT(HDRP(ptr), PACK(asize, 1));	// 注：代码顺序很重要！！
				PUT_PRED(NEXT_BLKP(ptr), GET_PRED(bp));
				PUT_SUCC(NEXT_BLKP(ptr), GET_SUCC(bp));
				if (GET_SUCC(bp) != 0)
					PUT_PRED(GET_SUCC(bp), NEXT_BLKP(ptr));
				PUT_SUCC(GET_PRED(bp), NEXT_BLKP(ptr));
				PUT(FTRP(ptr), PACK(asize, 1));
				PUT(HDRP(NEXT_BLKP(ptr)), PACK(allsize-asize, 0));
				PUT(FTRP(NEXT_BLKP(ptr)), PACK(allsize-asize, 0));
			} else {	// 分配后剩余空间不足以分配最小块空间，直接分配总的空间；从空闲链表中删除原后面的空闲块
				bp = NEXT_BLKP(ptr);
				if (GET_SUCC(bp) != 0)
					PUT_PRED(GET_SUCC(bp), GET_PRED(bp));
				PUT_SUCC(GET_PRED(bp), GET_SUCC(bp));
				PUT(HDRP(ptr), PACK(allsize, 1));
				PUT(FTRP(ptr), PACK(allsize, 1));
			}
		} else {		// 后面的块是已分配的。原地分配空间不足，需另外申请空间，且注意数据的拷贝
			bp = mm_malloc(asize);
			memcpy(bp, ptr, cursize);
			mm_free(ptr);
			//printf("  --- mm_realloc: %p\n", bp);
			return bp;
		}	
    } else {	// 当前块空间足够
		if (!nextalloc){	// 后面的块是空闲的，合并两个空间，并在分配后再进行分割；剩余空闲块取代原后面的空闲块在空闲链表中的位置
			bp = NEXT_BLKP(ptr);
			allsize = cursize + nextsize;
			PUT(HDRP(ptr), PACK(asize, 1));
			PUT(FTRP(ptr), PACK(asize, 1));
			PUT(HDRP(NEXT_BLKP(ptr)), PACK(allsize-asize, 0));
			PUT(FTRP(NEXT_BLKP(ptr)), PACK(allsize-asize, 0));
			PUT_PRED(NEXT_BLKP(ptr), GET_PRED(bp));
			PUT_SUCC(NEXT_BLKP(ptr), GET_SUCC(bp));
			if (GET_SUCC(bp) != 0)
				PUT_PRED(GET_SUCC(bp), NEXT_BLKP(ptr));
			PUT_SUCC(GET_PRED(bp), NEXT_BLKP(ptr));
		} else {		// 后面的块是已分配的	
			allsize = cursize;
			if ((cursize - asize) >= 2*DSIZE){	// 分配后剩余空间大于最小块空间，进行分割；剩余空闲块插入到空闲链表头部
				PUT(HDRP(ptr), PACK(asize, 1));
				PUT(FTRP(ptr), PACK(asize, 1));
				PUT(HDRP(NEXT_BLKP(ptr)), PACK(allsize-asize, 0));
				PUT(FTRP(NEXT_BLKP(ptr)), PACK(allsize-asize, 0));
				bp = NEXT_BLKP(ptr);
				if (GET_SUCC(heap_listp) == 0){
					PUT_PRED(bp, heap_listp);
					PUT_SUCC(bp, 0);
					PUT_SUCC(heap_listp, bp);
				} else {
					PUT_PRED(bp, heap_listp);
					PUT_SUCC(bp, GET_SUCC(heap_listp));
					PUT_PRED(GET_SUCC(heap_listp), bp);
					PUT_SUCC(heap_listp, bp);
				}
			}	    
		}
    }
    
    //printf("  --- mm_realloc: %p\n", ptr);
    return ptr;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    char *bp;
    //printf("\n>>> mm_realloc: %p\n", ptr);
    
    if (ptr == NULL)
	return mm_malloc(size);

    if (size == 0){
		mm_free(ptr);
		return NULL;
    }

    oldsize = GET_SIZE(HDRP(ptr));
    if (size < oldsize)	oldsize = size;
    bp = mm_malloc(size);
    memcpy(bp, ptr, oldsize);
    mm_free(ptr);

    return bp;
}

// 打印空闲链表，验证程序用
void displayEmptylist(){
    printf(" > displayEmptylist\n");
    char *bp = GET_SUCC(heap_listp);
    printf("heaplist");
    while (bp != 0){
	printf(" -> %p", bp);
	bp = GET_SUCC(bp);
    }
    printf("\n");
}
