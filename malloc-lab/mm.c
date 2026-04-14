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
    "dolggul2",
    /* First member's full name */
    "Seokje Kim",
    /* First member's email address */
    "dolggul2@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/*********************************************************
 * NOTE TO Users: bp - payload pointer, p - header or footer pointer
 ********************************************************/
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* single word */
#define WORD 4

/* double word */
#define DWORD 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

/* get size which aligned with two word*/
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* set packed value*/
#define PACK(size, allowed) ((size) | (allowed))
#define PUT(p, packed_value) (*((unsigned int *)(p)) = (packed_value))

/* read p packed value*/
#define GET(p) (*(unsigned int *)(p))

/* get size info*/
#define GET_SIZE(p) (GET(p) & ~0x7)

/* get allowed info*/
#define GET_ALLOC(p) (GET(p) & 0x1)

/* get current block header pointer*/
#define HDRP(bp) ((char *)(bp) - WORD)

/* get current block footer pointer */
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DWORD)

/* get next block payload pointer */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))

/* get prev block payload pointer */
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DWORD))

/*********************************************************
 *  set degug mode
 *********************************************************/
#define DEBUG 1
#if DEBUG
#define CHECKHEAP(where) mm_checkheap(where)
#else
#define CHECKHEAP(where)
#endif

/*********************************************************
 * set alloc mode
 *********************************************************/
#define MODE_IMPLICIT 1
#define MODE_EXPLICIT 2
#define MODE_SEGLIST 3

#define ALLOC_MODE MODE_EXPLICIT

/*********************************************************
 * function declaration
 *********************************************************/
static void *find_fit(size_t asize);
static void *coalesce(void *ptr);
static void *extend_heap(size_t words);
static void *place(void *bp, size_t size);

static int implicit_mm_init(void);
static void *implicit_mm_malloc(size_t size);
static void *implicit_find_fit(size_t asize);
static void *implicit_extend_heap(size_t words);
static void *implicit_place(void *bp, size_t size);
static void implicit_mm_free(void *ptr);
static void *implicit_coalesce(void *ptr);

static int explicit_mm_init(void);
static void *explicit_extend_heap(size_t words);
static void *explicit_coalesce(void *ptr);
static void implicit_mm_free(void *ptr);

static void mm_checkheap(const char *where);
static int in_heap(const void *p);
static int aligned(const void *p);

/*********************************************************
 * global variable
 *********************************************************/
static void *free_list_head = NULL;

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        return implicit_mm_init();
    }
    else if (ALLOC_MODE == MODE_EXPLICIT)
    {
        return explicit_mm_init();
    }
    else
    {
        return -1;
    }
}

static int implicit_mm_init(void)
{
    void *p;
    // memory for padding, prologue, epilogue to align DWORD
    if ((p = mem_sbrk(2 * DWORD)) == (void *)-1)
    {
        return -1;
    }

    // set prologue header and footer
    *(unsigned int *)((char *)p + 1 * WORD) = PACK(DWORD, 1);
    *(unsigned int *)((char *)p + 2 * WORD) = PACK(DWORD, 1);

    // set epilogue footer
    *(unsigned int *)((char *)p + 3 * WORD) = PACK(0, 1);
    return 0;
}

static int explicit_mm_init(void)
{
    char *p;
    size_t words;

    // memory for padding, prologue, epilogue to align DWORD
    if ((p = mem_sbrk(4 * WORD)) == (void *)-1)
    {
        return -1;
    }

    // set prologue header and footer
    *(unsigned int *)(p + 1 * WORD) = PACK(DWORD, 1);
    *(unsigned int *)(p + 2 * WORD) = PACK(DWORD, 1);

    // set epilogue footer
    *(unsigned int *)(p + 3 * WORD) = PACK(0, 1);

    // calc words
    words = (4 * DWORD) / WORD;

    // alloc block
    extend_heap(words);

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        return implicit_mm_malloc(size);
    }
    else
    {
        return NULL;
    }
}

static void *implicit_mm_malloc(size_t size)
{
    size_t asize;
    void *bp;

    // no need to allocate heap memory
    if (size == 0)
    {
        return NULL;
    }

    // get asize value
    if (size < DWORD)
    {
        // header(4) + aligned payload(8) + footer(4)
        asize = 2 * DWORD;
    }
    else
    {
        // header(4) + aligned payload + footer(4)
        asize = DWORD + ALIGN(size);
    }

    // find block to be used
    bp = find_fit(asize);

    // if there is no block which is bigger or equal than asize
    // extend_heap -> bp 반환
    if (bp == NULL)
    {
        // get words
        int words;
        if (asize % WORD == 0)
        {
            words = asize / WORD;
        }
        else
        {
            words = (asize / WORD) + 1;
        }

        // extend heap with words amount
        if ((bp = extend_heap(words)) == NULL)
        {
            return NULL;
        }
    }

    // call place
    bp = place(bp, asize);

    return (void *)bp;
}

static void *find_fit(size_t asize)
{
    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        return implicit_find_fit(asize);
    }
    else
    {
        return NULL;
    }
}

static void *implicit_find_fit(size_t asize)
{
    for (void *bp = (void *)((char *)mem_heap_lo() + 4 * WORD); GET_SIZE(HDRP(bp)) > 0; bp = (void *)NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
        {
            return (void *)bp;
        }
    }

    return NULL;
}

static void *extend_heap(size_t words)
{
    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        return implicit_extend_heap(words);
    }
    else
    {
        return NULL;
    }
}

static void *implicit_extend_heap(size_t words)
{
    size_t asize;
    void *bp;

    // 8배수가 되도록 짝수 words 값을 구하고 byte로 변환
    if ((words % 2) == 1)
    {
        asize = (words + 1) * WORD;
    }
    else
    {
        asize = words * WORD;
    }

    // 반환되는 주소는 epilogue_header 마지막 주소이므로, 사실 payload를 가리키는 자리
    if ((bp = mem_sbrk(asize)) == (void *)-1)
    {
        return NULL;
    }

    // init header, footer
    PUT(HDRP(bp), PACK(asize, 0));
    PUT(FTRP(bp), PACK(asize, 0));

    // update epilogue header
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    bp = coalesce(bp);
    CHECKHEAP("after extend_heap");
    return (void *)bp;
}

static void *explicit_extend_heap(size_t words)
{
    size_t asize;
    void *bp;

    // 8배수가 되도록 짝수 words 값을 구하고 byte로 변환
    if ((words % 2) == 1)
    {
        asize = (words + 1) * WORD;
    }
    else
    {
        asize = words * WORD;
    }

    // 반환되는 주소는 epilogue_header 마지막 주소이므로, 사실 payload를 가리키는 자리
    if ((bp = mem_sbrk(asize)) == (void *)-1)
    {
        return NULL;
    }

    // init header, footer
    PUT(HDRP(bp), PACK(asize, 0));
    PUT(FTRP(bp), PACK(asize, 0));

    // init prev, next
    *(void **)bp = NULL;
    *(void **)((char *)bp + sizeof(void *)) = free_list_head;

    // append free_list
    free_list_head = bp;

    // update epilogue header
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    bp = coalesce(bp);
    CHECKHEAP("after extend_heap");
    return (void *)bp;
}

static void *place(void *bp, size_t size)
{
    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        return implicit_place(bp, size);
    }
    else
    {
        return NULL;
    }
}

static void *implicit_place(void *bp, size_t size)
{
    assert(bp != NULL);
    assert(in_heap(bp));
    assert(aligned(bp));
    assert(GET_ALLOC(HDRP(bp)) == 0);
    assert(GET_SIZE(HDRP(bp)) >= size);

    // bp의 크기에서 size 만큼 뺀 것이 2*DWORD 보다 같거나 크다면 split
    int origin_size = GET_SIZE(HDRP(bp));

    if ((origin_size - size) >= (2 * DWORD))
    {
        // size 만큼 사용
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));

        // 나머지는 free 설정
        PUT(HDRP(NEXT_BLKP(bp)), PACK(origin_size - size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(origin_size - size, 0));
    }
    // 그렇지 않다면, 블록 크기만큼을 사용하도록 만들어주자
    else
    {
        PUT(HDRP(bp), PACK(origin_size, 1));
        PUT(FTRP(bp), PACK(origin_size, 1));
    }

    CHECKHEAP("after place");
    return (void *)bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        implicit_mm_free(ptr);
    }
}

static void implicit_mm_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    assert(in_heap(ptr));
    assert(aligned(ptr));
    assert(GET_ALLOC(HDRP(ptr)) == 1);

    PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
    PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
    coalesce(ptr);
}

/*
 * coalesce - merge with adjacent free block
 */
static void *coalesce(void *ptr)
{
    if (ALLOC_MODE == MODE_IMPLICIT)
    {
        return implicit_coalesce(ptr);
    }
    else
    {
        return NULL;
    }
}

static void *implicit_coalesce(void *ptr)
{
    assert(ptr != NULL);
    assert(in_heap(ptr));
    assert(aligned(ptr));
    assert(GET_ALLOC(HDRP(ptr)) == 0);

    char *bp = (char *)ptr;
    void *prev_bp = PREV_BLKP(ptr);
    void *next_bp = NEXT_BLKP(ptr);
    unsigned char prev_allowed = GET_ALLOC(HDRP(prev_bp));
    unsigned char next_allowed = GET_ALLOC(HDRP(next_bp));

    if (prev_allowed && next_allowed)
    {
        CHECKHEAP("after coalesce");
        return (void *)bp;
    }

    else if (prev_allowed && !next_allowed)
    {
        unsigned int asize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(next_bp));
        PUT(HDRP(bp), PACK(asize, 0));
        PUT(FTRP(bp), PACK(asize, 0));
        CHECKHEAP("after coalesce");
        return (void *)bp;
    }

    else if (!prev_allowed && next_allowed)
    {
        unsigned int asize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(prev_bp));
        PUT(HDRP(prev_bp), PACK(asize, 0));
        PUT(FTRP(prev_bp), PACK(asize, 0));
        CHECKHEAP("after coalesce");
        return (void *)prev_bp;
    }

    else
    {
        unsigned int asize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(prev_bp)) + GET_SIZE(HDRP(next_bp));
        PUT(HDRP(prev_bp), PACK(asize, 0));
        PUT(FTRP(prev_bp), PACK(asize, 0));
        CHECKHEAP("after coalesce");
        return (void *)prev_bp;
    }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    /*
    parameters:
        ptr: old bp, which indicates payload start address
        size: payload size to be allocated

    return: NULL if failee else new bp
    */

    void *old_ptr = ptr;
    void *new_ptr;
    size_t old_size;
    size_t copy_size;

    // ask memory with 0 size, no need to allocate memory
    if (size == 0)
    {
        mm_free(old_ptr);
        CHECKHEAP("after mm_realloc");
        return NULL;
    }

    // mm_malloc(size)
    if (old_ptr == NULL)
    {
        if ((new_ptr = mm_malloc(size)) == NULL)
        {
            CHECKHEAP("after mm_realloc");
            return NULL;
        }
        CHECKHEAP("after mm_realloc");
        return new_ptr;
    }

    // when ask same size
    old_size = GET_SIZE(HDRP(old_ptr)) - DWORD;
    if (old_size == size)
    {
        CHECKHEAP("after mm_realloc");
        return old_ptr;
    }

    // compare asked size with old size and choose smaller
    copy_size = (old_size < size) ? old_size : size;

    // allocate
    if ((new_ptr = mm_malloc(size)) == NULL)
    {
        CHECKHEAP("after mm_realloc");
        return NULL;
    }

    // copy payload
    memcpy(new_ptr, old_ptr, copy_size);

    // free old ptr
    mm_free(old_ptr);

    CHECKHEAP("after mm_realloc");
    return new_ptr;
}

static void mm_checkheap(const char *where)
{
    char *bp;

    // check prologue
    if (GET((char *)mem_heap_lo() + WORD) != PACK(DWORD, 1))
    {
        fprintf(stderr, "[%s] bad prologue header\n", where);
        abort();
    }

    if (GET((char *)mem_heap_lo() + 2 * WORD) != PACK(DWORD, 1))
    {
        fprintf(stderr, "[%s] bad prologue footer\n", where);
        abort();
    }

    // check block with payload
    for (bp = (char *)mem_heap_lo() + 4 * WORD; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        // check alignment
        if (!aligned(bp))
        {
            fprintf(stderr, "[%s] bad alignment at %p\n", where, bp);
            abort();
        }

        // check in heap address
        if (!in_heap(HDRP(bp)) || !in_heap(FTRP(bp)))
        {
            fprintf(stderr, "[%s] block out of heap at %p\n", where, bp);
            abort();
        }

        // check headder, footer have same value
        if (GET(HDRP(bp)) != GET(FTRP(bp)))
        {
            fprintf(stderr, "[%s] header/footer mismatch at %p\n", where, bp);
            abort();
        }

        // check block is aligned
        if ((GET_SIZE(HDRP(bp)) % ALIGNMENT) != 0)
        {
            fprintf(stderr, "[%s] bad size alignment at %p\n", where, bp);
            abort();
        }

        // check coalesce is working
        if (!GET_ALLOC(HDRP(bp)) &&
            GET_SIZE(HDRP(NEXT_BLKP(bp))) > 0 &&
            !GET_ALLOC(HDRP(NEXT_BLKP(bp))))
        {
            fprintf(stderr, "[%s] uncoalesced free blocks at %p\n", where, bp);
            abort();
        }
    }

    // check epilogue
    if (GET_SIZE(HDRP(bp)) != 0 || !GET_ALLOC(HDRP(bp)))
    {
        fprintf(stderr, "[%s] bad epilogue\n", where);
        abort();
    }
}

static int in_heap(const void *p)
{
    return (char *)p >= (char *)mem_heap_lo() && (char *)p <= (char *)mem_heap_hi();
}

static int aligned(const void *p)
{
    return ((size_t)p % ALIGNMENT) == 0;
}
