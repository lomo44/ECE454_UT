/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
        /* Team name */
        "NSFW1",
        /* First member's full name */
        "LI ZHUANG",
        /* First member's email address */
        "johnnn.li@mail.utoronto.ca",
        /* Second member's full name (leave blank if none) */
        "LIN GUJIANG",
        /* Second member's email address (leave blank if none) */
        "gujiang.lin@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x, y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p, val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define LAB3_START 1

#ifdef LAB3_START
// All codes for lab3 goes inside here
#define RETURN_IF_ERROR(x) {if((x)!=eLLError_None) return (x);}
#define RET_IF_RUN_ERROR(x,y) {(gError=(x));RETURN_IF_ERROR((y))}
#define BYTES_TO_DWORD(x) (x>>3)

typedef void* Heap_ptr;
typedef void* Data_ptr;


#define BIN_SIZE 1024
#define MALLOC_ALIGNMENT 4            // 16 bytes alignments
#define MIN(x, y) ((x) < (y)?(x) :(y))
#define PTR_ALIGNMENT (sizeof(void*))   // 8 bytes for 64bit machine

#define MAGIC_NUMBER 889999

#define NEXT_PTR_OFFSET 1
#define HEADER_OFFSET 1
#define MAGIC_NUMBER_OFFSET 1
#define BLOCK_ALLOCATED 1
#define BLOCK_FREE 0
#define INVALID_HEAP_PTR ((Heap_ptr)-1)
#define SPLIT_THRESH_HOLD

#define PROLOGUE_OFFSET (HEADER_OFFSET+NEXT_PTR_OFFSET)
#define EPILOGUE_OFFSET (HEADER_OFFSET+MAGIC_NUMBER_OFFSET)
#define META_DATA_SIZE (PROLOGUE_OFFSET+EPILOGUE_OFFSET)

#define llGetLinkedBlock(x) ((Heap_ptr)(GET(x+HEADER_OFFSET)))
#define llGetDataPtrFromHeapPtr(x) ((Data_ptr)(x+NEXT_PTR_OFFSET+HEADER_OFFSET))
#define llGetHeapPtrFromDataPtr(x) ((Heap_ptr)(x-PROLOGUE_OFFSET))
#define llGetDataSizeFromHeader(x) (GET(x)>>MALLOC_ALIGNMENT)
#define llGetPrevBlockPtrFromHeapPtr(x) ((Heap_ptr)(x-META_DATA_SIZE-llGetDataSizeFromHeader(x)))
#define llGetNextBlockPtrFromHeapPtr(x) ((Heap_ptr)(x+META_DATA_SIZE+llGetDataSizeFromHeader(x)))
#define llSetLinkedBlock(x,target) (PUT((Heap_ptr)(x+HEADER_OFFSET),(uintptr_t)target))
#define llIsBlockFree(x) (!(GET(x) & 0))


typedef enum _eLLError{
    eLLError_heap_extend_fail,
    eLLError_allocation_fail,
    eLLError_search_fail,
    eLLError_None = 0
} eLLError;

typedef struct _llSplitRecipe{
    int m_iBlockASize;
    int m_iBlockBSize;
} llSplitRecipe;

Heap_ptr gBin;
int gBinSize;
Heap_ptr gHeapPtr;
size_t gHeapSize = 0;
eLLError gError;

/*llGetAllignedSizeInBytes
 * -------------------------------------------------------------
 * Get aligned size from in_iInput
 * in_iInput:input size
 * in_iAlignment: alignment, must be a power of 2
 *
 * Return aligned size
 */
int llGetAllignedSizeInBytes(int in_iInput, int in_iAlignment){
    if ((in_iInput & ((1 << in_iAlignment) - 1)) > 0)
        return ((in_iInput >> in_iAlignment) + 1) << in_iAlignment;
    else
        return  in_iInput;
}
Heap_ptr llExtendHeap(int in_iExtendSize){
    Heap_ptr ret = mem_sbrk(in_iExtendSize);
    if(ret!=INVALID_HEAP_PTR){
        gHeapSize += in_iExtendSize >> PTR_ALIGNMENT;
        return ret;
    }
    else{
        return INVALID_HEAP_PTR;
    }
}

eLLError llInitBin(size_t in_iBinSize){
    // Extend heap for bin pointers
    gBin = llExtendHeap(BIN_SIZE*PTR_ALIGNMENT);
    if(gBin == INVALID_HEAP_PTR){
        return eLLError_heap_extend_fail;
    }
    else{
        // Bin Allocation success;
        gBinSize = BIN_SIZE;
        // Initialize bin
        int i;
        for(i = 0; i < gBinSize; i++){
           PUT(gBin+i,(uintptr_t)INVALID_HEAP_PTR);
        }
        return eLLError_None;
    }
}
/*
 * Return data size in dword from header
 */
eLLError llMarkBlockAllocationBit(Heap_ptr in_pBlockPtr, int in_bAllocated){
    int data_size_in_dword = llGetDataSizeFromHeader(in_pBlockPtr);
    PUT(in_pBlockPtr, PACK(data_size_in_dword << MALLOC_ALIGNMENT, in_bAllocated));
    PUT(in_pBlockPtr+PROLOGUE_OFFSET+data_size_in_dword+MAGIC_NUMBER_OFFSET, PACK(data_size_in_dword << MALLOC_ALIGNMENT,in_bAllocated));
    return eLLError_None;
}
eLLError llDeAllocBlock(Heap_ptr in_pInputPtrA){
    llMarkBlockAllocationBit(in_pInputPtrA,0);
    return eLLError_None;
}


/*
 * Function llInitBlock
 * ------------------------
 * Initialize the block (with all meta data)
 * in_pInputPtr: pointer to data block
 * in_iBlockSizeInQword: desire block size with meta data
 *
 * Return: error message
 */
eLLError llInitBlock(Heap_ptr in_pInputPtr, int in_iBlockSizeInQword){
    int data_size_in_dword = in_iBlockSizeInQword-META_DATA_SIZE;
    // Place the header
    PUT(in_pInputPtr, PACK(data_size_in_dword << MALLOC_ALIGNMENT,1));
    // initialize the link list pointer
    PUT(in_pInputPtr+HEADER_OFFSET, -1);
    // initialize the magic number
    PUT(in_pInputPtr+data_size_in_dword+PROLOGUE_OFFSET,MAGIC_NUMBER);
    // initialize the footer
    PUT(in_pInputPtr+PROLOGUE_OFFSET+data_size_in_dword+MAGIC_NUMBER_OFFSET, PACK(data_size_in_dword << MALLOC_ALIGNMENT,1));
    return eLLError_None;
}
/*
 * Function llAllocFromHeap
 * ------------------------
 * Initialize the block from heap(with all meta data)
 * io_pOutputPtr: pointer to data block
 * in_iSizeInBytes: desire block size with meta data
 *
 * Return: error message
 */
eLLError llAllocFromHeap(size_t in_iSizeInBytes, Data_ptr * io_pOutputPtr){
    // get the corresponding alignment size
    int aligned_size = llGetAllignedSizeInBytes(in_iSizeInBytes, MALLOC_ALIGNMENT);
    // Calculate the real block size
    int block_size = aligned_size + META_DATA_SIZE;
    // Allocate a new block from heap
    Heap_ptr bp = llExtendHeap(block_size);
    if(bp ==INVALID_HEAP_PTR){
        return eLLError_heap_extend_fail;
    }
    else{
        // Initialize the block
        llInitBlock(bp,BYTES_TO_DWORD(block_size));
        // return the block address
        *io_pOutputPtr = llGetDataPtrFromHeapPtr(bp);
        return eLLError_None;
    }
}

/*
 * Function llAllocFromHeap
 * ------------------------
 * Initialize the block from bin (initialized free block)(with all meta data)
 * io_pOutputPtr: pointer to data block
 * in_iSizeInBytes: desire block size with meta data
 *
 * Return: error message
 */
eLLError llAllocFromBin(size_t in_iSizeInBytes, Data_ptr* io_pOutputPtr){
    // Calculate the aligned bucket size
    int aligned_size = llGetAllignedSizeInBytes(in_iSizeInBytes, MALLOC_ALIGNMENT);
    // Calculate the bucket index;
    int start_index = aligned_size >> MALLOC_ALIGNMENT;
    // Iterate through bin and get the best fit bucket
    Heap_ptr ret = NULL;
    while(start_index != gBinSize){
        if(GET(gBin+start_index)!=-1){
            // valid bin found, reconstruct the link list
            ret = (Heap_ptr)GET(gBin+start_index);
            // set the head of the linked list to the next element
            PUT(gBin+start_index,(uintptr_t)llGetLinkedBlock(ret));
            // modified the allocation bit
            llMarkBlockAllocationBit(ret, BLOCK_ALLOCATED);
        }
        start_index++;
    }
    if(ret!=NULL){
        *io_pOutputPtr = llGetDataPtrFromHeapPtr(ret);
        return eLLError_None;
    }
    else{
        return eLLError_allocation_fail;
    }
}


/*
 * Function: llGetSplitedRemainderSize
 * -----------------------------------
 * calculate the remaining data size (no meta) after split
 * in_iTotalDataSize: total block size (with meta) available
 * in_iTargetSize: desire data (no meta)size
 *
 * Return: remain data size (no meta)
 */
int      llGetSplitedRemainderSize(int in_iTotalDataSize, int in_iTargetSize) {
    int remain_size = in_iTotalDataSize - in_iTargetSize - META_DATA_SIZE *2;
    if (remain_size < 2) {
        printf("Error: Try to split size %d from size %d", in_iTargetSize, in_iTotalDataSize);
        return 0;
    } else
        return remain_size;

}
int      llGetMaximumExtendableSize(Heap_ptr in_pPtr){
    int max_size = 0;
    Heap_ptr heap_end = gHeapPtr + gHeapSize;
    Heap_ptr cur_ptr = llGetNextBlockPtrFromHeapPtr(in_pPtr);
    while(cur_ptr < heap_end){
        if(llIsBlockFree(cur_ptr)){
            max_size += llGetDataSizeFromHeader(cur_ptr);
        }
        else{
            break;
        }
    }
    return max_size;
}


eLLError llThrowInBin(Heap_ptr in_pHeapPtr){
    int index = MIN(llGetDataSizeFromHeader(in_pHeapPtr)>>MALLOC_ALIGNMENT,BIN_SIZE-1);
    llMarkBlockAllocationBit(in_pHeapPtr,BLOCK_FREE);
    llSetLinkedBlock(in_pHeapPtr,GET(gBin+index));
    PUT(gBin+index,(uintptr_t)in_pHeapPtr);
    return eLLError_None;
}
eLLError llPullFromList(Heap_ptr in_pBin, Heap_ptr in_pTarget){
    Heap_ptr cur_ptr= (Heap_ptr)GET(in_pBin);
    Heap_ptr next_ptr = llGetLinkedBlock(cur_ptr);
    if(cur_ptr == in_pTarget){
        // Head is the target, need to change the head
        llSetLinkedBlock(cur_ptr,NULL);
        PUT(cur_ptr,(uintptr_t)next_ptr);
        return eLLError_None;
    }
    else{
        while(next_ptr != NULL){
            if(next_ptr==in_pTarget){
                Heap_ptr next_next = llGetLinkedBlock(next_ptr);
                llSetLinkedBlock(cur_ptr,next_next);
                llSetLinkedBlock(next_ptr,NULL);
                return eLLError_None;
            }
            else{
                cur_ptr = next_ptr;
                next_ptr = llGetLinkedBlock(next_ptr);
            }
        }
    }
    return eLLError_search_fail;
}

eLLError llPullFromBin(Heap_ptr in_pHeapPtr){
    int index = MIN(llGetDataSizeFromHeader(in_pHeapPtr)>>MALLOC_ALIGNMENT,BIN_SIZE-1);
    return llPullFromList(in_pHeapPtr+index,in_pHeapPtr);

}

/*
 * Function llMergeBlock
 * ---------------------
 * Merge up to three free block into one. assume merged block is free and contain no data
 * in_pInputPtrA: pointer to the free block A
 * in_pInputPtrB: pointer to the free block B
 * io_pOutputPtr: pointer to the merged new block
 *
 * Return: Error message
 */
eLLError llMergeBlock(Heap_ptr in_pInputPtrA,Heap_ptr in_pInputPtrB,Heap_ptr* io_pOutputPtr){
    int block_A_size = llGetDataSizeFromHeader (in_pInputPtrA) + META_DATA_SIZE;
    int block_B_size = llGetDataSizeFromHeader (in_pInputPtrB) + META_DATA_SIZE;
    //let the merged get be the previous block
    if ( in_pInputPtrA < in_pInputPtrB) {
        io_pOutputPtr = in_pInputPtrA;
    } else {
        io_pOutputPtr = in_pInputPtrB;
    }
    //re-initialize the block
    llInitBlock(io_pOutputPtr,block_A_size + block_B_size);
    return eLLError_None;
}
/*
 * Function llSplitBlock
 * ---------------------
 * Split one free block into two. assume split legal
 * in_pInputPtr: pointer to the free block
 * in_pRecipe: desire division of the block
 * io_pOutputPtrA: pointer to the free block A
 * io_pOutputPtrB: pointer to the free block B
 *
 * Return: Error message
 */
eLLError llSplitBlock(Heap_ptr in_pInputPtr,llSplitRecipe* in_pRecipe, Heap_ptr* io_pOutputPtrA,Heap_ptr* io_pOutputPtrB){
    int block_A_size = in_pRecipe->m_iBlockASize + META_DATA_SIZE;
    int block_B_size = in_pRecipe->m_iBlockBSize + META_DATA_SIZE;
    io_pOutputPtrA = in_pInputPtr;
    llInitBlock(io_pOutputPtrA,block_A_size);
    io_pOutputPtrB = in_pInputPtr + block_A_size;
    llInitBlock(io_pOutputPtrB,block_B_size);
    return eLLError_None;
}
/*
 * Function llExtendBlock
 * ---------------------
 * Extend one free block into desire size. assume extend legal
 * in_pInputPtr: pointer to the free block
 * in_iExtendSize: desire new size of the block
 *
 * Return: Error message
 */
eLLError llExtendBlock(Heap_ptr in_pInputPtr, int in_iExtendSize){
    llInitBlock(in_pInputPtr,in_iExtendSize);
    return eLLError_None;
}
/*
 * Function llCopyBlock
 * ---------------------
 * one block's data into another blcok. assume copy legal
 * in_pFrom: pointer to the block hold the data
 * in_pTo: pointer to the empty block
 * in_iCopySize: data size need to copy in 8 bytes
 *
 * Return: Error message
 */
eLLError llCopyBlock(Heap_ptr in_pFrom, Heap_ptr in_pTo, int in_iCopySize){
    memcpy(in_pFrom + PROLOGUE_OFFSET,in_pTo + PROLOGUE_OFFSET,in_iCopySize *8);
    return eLLError_None;
}

/*
 * Main Allocation functions
 */
eLLError llInit(){
    llInitBin(BIN_SIZE);
    gHeapPtr = gBin;
    return eLLError_None;
}
Data_ptr llAlloc(int in_iSize){
    Heap_ptr ret = NULL;
    // Try to allocate a block from bin
    eLLError error = llAllocFromBin(in_iSize,&ret);
    if(error!=eLLError_allocation_fail){
        // Successfully found a free block inside the list, now we need to check if the block is splittable

        // Get the allocated block size
        int block_size = llGetDataSizeFromHeader(ret);
        // Get the alligned data size
        int real_data_size = llGetAllignedSizeInBytes(in_iSize,MALLOC_ALIGNMENT);
        // Get the remainder size after spliting the main block
        int remainder_size = llGetSplitedRemainderSize(block_size,real_data_size);
        // If the remainder size is greater than 0 (splitable)
        if(remainder_size >= 0){
            // Blk is splittable, then split the block and put residual into the bin
            Heap_ptr outptrA;
            Heap_ptr outptrB;
            // Creating a new split recipe
            llSplitRecipe newRecipe;
            newRecipe.m_iBlockASize = real_data_size;
            newRecipe.m_iBlockBSize = remainder_size;
            //then split the current plock
            llSplitBlock(ret,&newRecipe,&outptrA,&outptrB);
            // Throw the remainder size into the bin
            llThrowInBin(outptrB);
        }
    }
    else{
        // Cannot found a proper free block on the list, extend the heap and allocate a new block
        llAllocFromHeap(in_iSize,&ret);
    }
    return ret;
}
eLLError llFree(Data_ptr in_pDataPtr){
    // Convert the given data ptr to heap ptr
    Heap_ptr cur_ptr = llGetHeapPtrFromDataPtr(in_pDataPtr);
    // Deallocate the current block
    RET_IF_RUN_ERROR(llDeAllocBlock(cur_ptr),gError);
    // Get the previous block ptr
    Heap_ptr prev_ptr = llGetPrevBlockPtrFromHeapPtr(cur_ptr);
    // Get the next block ptr
    Heap_ptr next_ptr = llGetNextBlockPtrFromHeapPtr(cur_ptr);
    // Check if both block are free
    int is_prev_free = llIsBlockFree(prev_ptr);
    int is_next_free = llIsBlockFree(next_ptr);
    Heap_ptr ret = cur_ptr;
    // Check if previous block is free
    if(is_prev_free!=0){
        // Previous block is free, merge current block with previous block
        RET_IF_RUN_ERROR(llMergeBlock(ret,prev_ptr,&ret),gError);
        llPullFromBin(prev_ptr);
    }
    if(is_next_free!=0){
        // Next block is free
        RET_IF_RUN_ERROR(llMergeBlock(ret,next_ptr,&ret),gError);
        llPullFromBin(next_ptr);
    }
    // Throw the merged block into bin
    RET_IF_RUN_ERROR(llThrowInBin(cur_ptr),gError);
    return gError;
}
Data_ptr llRealloc(Data_ptr in_pDataPtr, int in_iSize){
    // Getting the current data size
    int current_data_size = llGetDataSizeFromHeader(in_pDataPtr);
    if(in_pDataPtr == NULL){
        // If input data ptr is NULL, then allocate a new block
        return llAlloc(in_iSize);
    }
    else if(in_iSize == 0){
        // Size equal zero, basically free
        llFree(in_pDataPtr);
        return NULL;
    }
    else if(current_data_size == in_iSize){
        // Size doesn't change, ignore
        return in_pDataPtr;
    }
    else{
        // Final Reallocation
        // Converting the data ptr into heap ptr
        Heap_ptr ptr = llGetHeapPtrFromDataPtr(in_pDataPtr);
        // Get the maximum extending size by
        int maximum_extendable_size = llGetMaximumExtendableSize(ptr);
        if(maximum_extendable_size>=in_iSize){
            // Local resizing is possible, extend the current block
            llExtendBlock(ptr,maximum_extendable_size);
            return in_pDataPtr;
        }
        else{
            // local size is not enough for exending, alloc a new one and throw old one into bin
            Data_ptr new_block = llAlloc(in_iSize);
            Heap_ptr new_heap_ptr = llGetHeapPtrFromDataPtr(new_block);
            // copy the old data into the new data block
            llCopyBlock(ptr,new_heap_ptr,current_data_size);
            // Free the old data block
            llFree(in_pDataPtr);
            return new_block;
        }
    }
}

#endif

size_t max_size = 0;
void *heap_listp = NULL;

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void) {
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) -1)
        return -1;
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;
    return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        return bp;
    } else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    } else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    } else {            /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *) -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void *find_fit(size_t asize) {
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void *bp, size_t asize) {
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(bsize, 1));
    PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) {

    if (bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size) {
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) {
    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
        return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void) {
    return 1;
}
