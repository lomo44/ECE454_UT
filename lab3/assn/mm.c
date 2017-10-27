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

#define HEAP_CHECK_ENABLE 0
// All codes for lab3 goes inside here


#define WORD_TO_BYTES(x) ((x)<<3)
#define BYTES_TO_WORD(x) ((x)>>3)
#define WORD_TO_DWORD(x) ((x)>>1)

typedef uintptr_t *Heap_ptr;
typedef uintptr_t *Data_ptr;
typedef int DWORD;
typedef int WORD;
typedef int BYTE;

#define BIN_SIZE 160
#define MALLOC_ALIGNMENT 4            // 16 bytes alignments
#define MIN(x, y) ((x) < (y)?(x) :(y))
#define PTR_ALIGNMENT (sizeof(void*))   // 8 bytes for 64bit machine

#define MAGIC_NUMBER 889999
#define GUARD_SIZE 4
#define NEXT_PTR_OFFSET 1
#define HEADER_OFFSET 1
#define PREVIOUS_PTR_OFFSET 1
#define BLOCK_ALLOCATED 1
#define BLOCK_FREE 0
#define INVALID_HEAP_PTR ((Heap_ptr)-1)
#define FAST_BLOCK_SIZE 10
#define SPLIT_THRESHOLD 768

#define PROLOGUE_OFFSET (HEADER_OFFSET+NEXT_PTR_OFFSET)
#define EPILOGUE_OFFSET (HEADER_OFFSET+PREVIOUS_PTR_OFFSET)
#define META_DATA_WORD (PROLOGUE_OFFSET+EPILOGUE_OFFSET)
#define META_SIZE_DWORD (META_DATA_WORD >> 1)


#define llGetDataPtrFromHeapPtr(x) ((Data_ptr)(x+NEXT_PTR_OFFSET+HEADER_OFFSET))
#define llGetHeapPtrFromDataPtr(x) ((Heap_ptr)(x-PROLOGUE_OFFSET))
#define llGetDataSizeFromHeader(x) (GET(x)>>MALLOC_ALIGNMENT)
#define llGetPrevHeapPtrFromHeapPtr(x) ((Heap_ptr)(x-llGetDataSizeFromHeader(x-1)-META_DATA_WORD))
#define llGetNextHeapPtrFromHeapPtr(x) ((Heap_ptr)(x+META_DATA_WORD+llGetDataSizeFromHeader(x)))

#define llSetNextBlock(x, target) (PUT((Heap_ptr)(x+HEADER_OFFSET),target))
#define llGetNextBlock(x) ((Heap_ptr)(GET(x+HEADER_OFFSET)))

//#define llSetPrivBlock(x, target) PUT((x+PROLOGUE_OFFSET+llGetDataSizeFromHeader(x)),target)
#define llGetPrivBlock(x) ((Heap_ptr)(GET(x+PROLOGUE_OFFSET+llGetDataSizeFromHeader(x))))

void llSetPrivBlock(Heap_ptr in_pInput, Heap_ptr in_ptarget){
    PUT(((in_pInput)+(PROLOGUE_OFFSET)+(llGetDataSizeFromHeader(in_pInput))),(in_ptarget));
}

//#define llDisconnectBlock(y) {\
//    Heap_ptr __next = llGetNextBlock(y);\
//    Heap_ptr __prev = llGetPrivBlock(y);\
//    if(__next!=NULL){\
//        llSetPrivBlock(__next, __prev);\
//    }\
//    if(__prev!=NULL){\
//        llSetNextBlock(__prev, __next);\
//    }\
//    llSetNextBlock(y, NULL);\
//    llSetPrivBlock(y, NULL);\
//}

#define llPush(x, head){\
    llSetNextBlock((x), (head));\
    llSetPrivBlock((x), NULL);\
    if((head)!= NULL)\
        llSetPrivBlock((head), x);\
}

void llDisconnectBlock(Heap_ptr x){
    Heap_ptr __next = llGetNextBlock((x));
    Heap_ptr __prev = llGetPrivBlock((x));
    if(__next!=NULL){
        //llPrintBlock(__next);
        llSetPrivBlock(__next, __prev);
    }
    if(__prev!=NULL){
        llSetNextBlock(__prev, __next);
    }
    llSetNextBlock((x), NULL);
    llSetPrivBlock((x), NULL);
}

#define llIsBlockFree(x) (!(GET(x) & 1))
#define llGetBinningIndex(x) (MIN((WORD_TO_DWORD(llGetDataSizeFromHeader(x))-1),(BIN_SIZE-1)))

typedef enum _eLLError {
    eLLError_heap_extend_fail = 1,
    eLLError_allocation_fail = 2,
    eLLError_search_fail = 3,
    eLLError_header_footer_size_inconsistent = 4,
    eLLError_header_footer_allocation_inconsistent = 5,
    eLLError_mismatch_magic_number = 6,
    eLLError_Non_Empty_Next_Ptr = 7,
    eLLError_data_not_align = 8,
    eLLError_zero_size_block = 9,
    eLLError_None = 0
} eLLError;

typedef struct _llSplitRecipe {
    int m_iBlockASize;
    int m_iBlockBSize;
} llSplitRecipe;

Heap_ptr gBin;
int gBinSize;
Heap_ptr gHeapStart;
Heap_ptr gHeapEnd;
Heap_ptr gTop = NULL;
eLLError gError;

void llPrintBlock(Heap_ptr in_pBlockPtr) {
    // Place the header
    Heap_ptr end = in_pBlockPtr + PROLOGUE_OFFSET + PREVIOUS_PTR_OFFSET + (llGetDataSizeFromHeader(in_pBlockPtr));
    printf("[Header: %llx]Size: %d, Allocated: %d\n",in_pBlockPtr,llGetDataSizeFromHeader(in_pBlockPtr), !llIsBlockFree(in_pBlockPtr));
    printf("Next Ptr: %llx\n", GET(in_pBlockPtr + 1));
    printf("Priv Ptr: %llx\n", llGetPrivBlock(in_pBlockPtr));
    printf("[Footer: %llx]Size: %d, Allocated: %d\n",end, llGetDataSizeFromHeader(end), !llIsBlockFree(end));
}

/*
 * Function: llGetAllignedSizeInBytes
 * -------------------------------------------------------------
 * Get aligned size from in_iInput
 * in_iInput:input size
 * in_iAlignment: alignment, must be a power of 2
 *
 * Return aligned size
 */
BYTE llGetAllignedSizeInBytes(BYTE in_iInput, int in_iAlignment) {
    if ((in_iInput & ((1 << in_iAlignment) - 1)) > 0)
        return ((in_iInput >> in_iAlignment) + 1) << in_iAlignment;
    else
        return in_iInput;
}

/*
 * Function: llExtendHeap
 * -------------------------------------------------------------
 * extend heap size
 * in_iExtendSize:request extra heap size
 *
 * Return: Error message
 */

Heap_ptr llExtendHeap(BYTE in_iExtendSize) {
#define FAST_HEAP 0
#if FAST_HEAP
    int extend_chunk = 16384;
    if(gTop==NULL){
        gTop = mem_sbrk(extend_chunk);
        gHeapRemainSize+=extend_chunk;
        gTotalHeapSize+=extend_chunk;
    }
    while(gHeapRemainSize < in_iExtendSize){
        void* a = mem_sbrk(extend_chunk);
        gHeapRemainSize += extend_chunk;
        gTotalHeapSize+=extend_chunk;
    }
    gHeapRemainSize-=in_iExtendSize;

    Heap_ptr ret = gTop;
    gTop+=(BYTES_TO_WORD(in_iExtendSize));
    printf("Extend %d, remain: %d\n",in_iExtendSize,gHeapRemainSize);
    printf("Total extend: %d,%d\n",mem_heapsize(),gTotalHeapSize);
    return ret;
#endif
    Heap_ptr ret = mem_sbrk(in_iExtendSize);
    //printf("Address: %llx\n",ret);
    if (ret != INVALID_HEAP_PTR) {
        return ret;
    } else {
        return INVALID_HEAP_PTR;
    }

}

/*
 * Function: llInitBin
 * -------------------------------------------------------------
 * Initialize Bin for free blocks
 * in_iBinSize:Size of Bin
 *
 * Return: Error message
 */
eLLError llInitBin(size_t in_iBinSize) {
    // Extend heap for bin pointers
    gBin = llExtendHeap(BIN_SIZE * PTR_ALIGNMENT);
    if (gBin == INVALID_HEAP_PTR) {
        return eLLError_heap_extend_fail;
    } else {
        // Bin Allocation success;
        gBinSize = BIN_SIZE;
        // Initialize bin
        int i;
        for (i = 0; i < gBinSize; i++) {
            PUT(gBin + i, NULL);
        }
        return eLLError_None;
    }
}

/*
 * Function: llMarkBlockAllocationBit
 * -----------------------------------------
 * change the allocation inform of a block
 * in_pBlockPtr: pointer to a block
 * in_bAllocated: allocation inform (0=not used,1= used)
 *
 * Return: Error message
 */
eLLError llMarkBlockAllocationBit(Heap_ptr in_pBlockPtr, int in_bAllocated) {
    int data_size_in_dword = llGetDataSizeFromHeader(in_pBlockPtr);
    PUT(in_pBlockPtr, PACK(data_size_in_dword << MALLOC_ALIGNMENT, in_bAllocated));
    PUT(in_pBlockPtr + PROLOGUE_OFFSET + data_size_in_dword + PREVIOUS_PTR_OFFSET,
        PACK(data_size_in_dword << MALLOC_ALIGNMENT, in_bAllocated));
    return eLLError_None;
}

/*
 * Function: llDeAllocBlock
 * -----------------------------------------
 * deallocate a block
 * in_pBlockPtr: pointer to a block
 * Return: Error message
 */
eLLError llDeAllocBlock(Heap_ptr in_pInputPtrA) {
    llMarkBlockAllocationBit(in_pInputPtrA, BLOCK_FREE);
    llSetNextBlock(in_pInputPtrA,NULL);
    llSetPrivBlock(in_pInputPtrA,NULL);
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
eLLError llInitBlock(Heap_ptr in_pInputPtr, WORD in_iBlockSizeInWord) {
    //llPrintBlock(llGetPrevHeapPtrFromHeapPtr(in_pInputPtr));
    WORD data_size_in_dword = in_iBlockSizeInWord - META_DATA_WORD;
    // Place the header
    PUT(in_pInputPtr, PACK(data_size_in_dword << MALLOC_ALIGNMENT, 1));
    // initialize the previous ptr
    llSetPrivBlock(in_pInputPtr,NULL);
    // initialize the next ptr
    llSetNextBlock(in_pInputPtr,NULL);
    // initialize the footer
    PUT(in_pInputPtr + PROLOGUE_OFFSET + data_size_in_dword + PREVIOUS_PTR_OFFSET,
        PACK(data_size_in_dword << MALLOC_ALIGNMENT, 1));
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
eLLError llAllocFromHeap(size_t in_iSizeInBytes, Data_ptr *io_pOutputPtr) {
    // get the corresponding alignment size
    int aligned_size = BYTES_TO_WORD(llGetAllignedSizeInBytes(in_iSizeInBytes, MALLOC_ALIGNMENT));
    // Calculate the real block size
    int block_size = aligned_size + META_DATA_WORD;
    // Allocate a new block from heap
    Heap_ptr bp = llExtendHeap(WORD_TO_BYTES(block_size));
    if (bp == INVALID_HEAP_PTR) {
        return eLLError_heap_extend_fail;
    } else {
        // Initialize the block
        llInitBlock(bp - META_DATA_WORD, block_size);
        gHeapEnd  = bp-META_DATA_WORD+block_size;
        llInitBlock((bp - META_DATA_WORD + block_size), 4);
        *io_pOutputPtr = llGetDataPtrFromHeapPtr(bp - 4);
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
eLLError llAllocFromBin(BYTE in_iSizeInBytes, Data_ptr *io_pOutputPtr) {


    // Calculate the aligned bucket size
    int aligned_size = llGetAllignedSizeInBytes(in_iSizeInBytes, MALLOC_ALIGNMENT);
    // Calculate the bucket index;
    int start_index = MIN((aligned_size >> MALLOC_ALIGNMENT)-1,BIN_SIZE-1);
    // Iterate through bin and get the best fit bucket

    Heap_ptr ret = NULL;
    while (start_index < gBinSize-1) {
        if (GET(gBin + start_index) != NULL) {
            // valid bin found, reconstruct the link list
            ret = GET(gBin + start_index);
            // set the head of the linked list to the next element
            PUT(gBin + start_index, llGetNextBlock(ret));
            llDisconnectBlock(ret);
            break;
        }
        start_index++;
    }
    if(ret == NULL) {
        Heap_ptr cur_ptr= (Heap_ptr)GET(gBin+gBinSize-1);
        if(cur_ptr!=NULL){
            Heap_ptr next_ptr = llGetNextBlock(cur_ptr);
            if (WORD_TO_BYTES(llGetDataSizeFromHeader(cur_ptr)>= in_iSizeInBytes))
            {
                // Head is the target, need to change the head
                ret = cur_ptr;
                PUT((gBin+gBinSize-1), next_ptr);
                llDisconnectBlock(ret);
            } else {
                cur_ptr = next_ptr;
                next_ptr = cur_ptr != NULL?llGetNextBlock(cur_ptr):NULL;
                while (cur_ptr != NULL) {
                    if (WORD_TO_BYTES(llGetDataSizeFromHeader(cur_ptr)) >= in_iSizeInBytes) {
                        ret = cur_ptr;
                        llDisconnectBlock(ret);
                        break;
                    } else {
                        cur_ptr = next_ptr;
                        next_ptr = cur_ptr != NULL?llGetNextBlock(cur_ptr):NULL;
                    }
                }
            }
        }
    }

    if (ret != NULL) {
        // modified the allocation bit
        llMarkBlockAllocationBit(ret, BLOCK_ALLOCATED);
        *io_pOutputPtr = llGetDataPtrFromHeapPtr(ret);
        return eLLError_None;
    } else {
        *io_pOutputPtr = NULL;
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
WORD llGetSplitedRemainderSize(WORD in_iTotalDataSize, WORD in_iTargetSize) {
    int remain_size = in_iTotalDataSize - in_iTargetSize - META_DATA_WORD;
    if (remain_size < 4) {
        //printf("Error: Try to split size %d from size %d\n", in_iTargetSize, in_iTotalDataSize);
        return 0;
    } else
        return remain_size;

}

/*
 * Function: llGetMaximumExtendableSize
 * -----------------------------------
 * calculate the maximum extendable size of heap
 * in_pPtr: pointer to a heap
 * in_iTargetSize: desire data (no meta)size
 *
 * Return: maximum size in dword
 */
WORD llGetMaximumExtendableSize(Heap_ptr in_pPtr) {
    WORD max_size = 0;
    Heap_ptr cur_ptr = llGetNextHeapPtrFromHeapPtr(in_pPtr);
    while (cur_ptr < gHeapEnd) {
        if (llIsBlockFree(cur_ptr)) {
            max_size += (llGetDataSizeFromHeader(cur_ptr) + META_DATA_WORD);
        } else {
            break;
        }
        cur_ptr = llGetNextHeapPtrFromHeapPtr(cur_ptr);
    }
    return max_size;
}

/*
 * Function: llThrowInBin
 * -----------------------------------
 * free a used data block and put it back to Bin
 * in_pHeapPtr: pointer to the bloack
 *
 * Return: Error Message
 */
eLLError llThrowInBin(Heap_ptr in_pHeapPtr) {

    int index = llGetBinningIndex(in_pHeapPtr);
    llPush(in_pHeapPtr,GET(gBin+index));
    PUT(gBin + index, (uintptr_t)in_pHeapPtr);
    //llPrintBlock(in_pHeapPtr);
    return eLLError_None;
}


eLLError llPullFromList(Heap_ptr in_pBin, Heap_ptr in_pTarget) {
    if(GET(in_pBin) == in_pTarget)
        PUT(in_pBin,llGetNextBlock(in_pTarget));
    llDisconnectBlock(in_pTarget);
    return eLLError_None;
}

eLLError llPullFromBin(Heap_ptr in_pHeapPtr) {
    //llPrintBlock(in_pHeapPtr);
    int index = llGetBinningIndex(in_pHeapPtr);
    //printf("Pull: %x in %d\n",in_pHeapPtr,index);
    return llPullFromList(gBin + index, in_pHeapPtr);

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
eLLError llMergeBlock(Heap_ptr in_pInputPtrA, Heap_ptr in_pInputPtrB, Heap_ptr *io_pOutputPtr) {
    int block_A_size = llGetDataSizeFromHeader (in_pInputPtrA) + META_DATA_WORD;
    int block_B_size = llGetDataSizeFromHeader (in_pInputPtrB) + META_DATA_WORD;
    //let the merged get be the previous block
    if (in_pInputPtrA < in_pInputPtrB) {
        *io_pOutputPtr = in_pInputPtrA;
    } else {
        *io_pOutputPtr = in_pInputPtrB;
    }
    //re-initialize the block
    llInitBlock(*io_pOutputPtr, block_A_size + block_B_size);
    llMarkBlockAllocationBit(*io_pOutputPtr,BLOCK_FREE);
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
eLLError
llSplitBlock(Heap_ptr in_pInputPtr, llSplitRecipe *in_pRecipe, Heap_ptr *io_pOutputPtrA, Heap_ptr *io_pOutputPtrB) {
    int block_A_size = in_pRecipe->m_iBlockASize + META_DATA_WORD;
    int block_B_size = in_pRecipe->m_iBlockBSize + META_DATA_WORD;
    *io_pOutputPtrA = in_pInputPtr;
    llInitBlock(*io_pOutputPtrA, block_A_size);
    *io_pOutputPtrB = in_pInputPtr + block_A_size;
    llInitBlock(*io_pOutputPtrB, block_B_size);
    llMarkBlockAllocationBit(*io_pOutputPtrB,BLOCK_FREE);
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
eLLError llExtendBlock(Heap_ptr in_pInputPtr, WORD in_iExtendSize) {
    llInitBlock(in_pInputPtr, in_iExtendSize);
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
eLLError llCopyBlock(Heap_ptr in_pFrom, Heap_ptr in_pTo, int in_iCopySize) {
    memcpy(in_pTo + PROLOGUE_OFFSET,in_pFrom + PROLOGUE_OFFSET, in_iCopySize);
    return eLLError_None;
}


eLLError llCheckBlock(Heap_ptr in_pBlockPtr) {

    // Check the consistency of the block
    int size_in_header = llGetDataSizeFromHeader(in_pBlockPtr);
    int size_in_footer = llGetDataSizeFromHeader(in_pBlockPtr + PROLOGUE_OFFSET + size_in_header + PREVIOUS_PTR_OFFSET);
    int allocation_in_header = (GET(in_pBlockPtr) & 1);
    int allocation_in_footer = (GET(in_pBlockPtr + PROLOGUE_OFFSET + size_in_header + PREVIOUS_PTR_OFFSET) & 1);
    // Check if the size in the header is the same as the footer
    if (size_in_footer != size_in_header) {
        return eLLError_header_footer_size_inconsistent;
    }
    if (size_in_header == 0){
        return eLLError_zero_size_block;
    }
    // Check header's footer's allocation bit
    if (allocation_in_footer != allocation_in_header) {
        return eLLError_header_footer_allocation_inconsistent;
    }
    if ((long long) in_pBlockPtr % 16 != 0) {
        return eLLError_data_not_align;
    }
    return eLLError_None;
}

int llIsBlockInList(Heap_ptr in_pBin, Heap_ptr in_pBlockPtr) {
    Heap_ptr start_block = GET(in_pBin);
    while (start_block != NULL) {
        if (start_block == in_pBlockPtr) {
            return 1;
        }
        start_block = llGetNextBlock(start_block);
    }
    return 0;
}

eLLError llValidBining(Heap_ptr in_pBlockPtr) {
    int index = llGetBinningIndex(in_pBlockPtr);
    if (llIsBlockFree(in_pBlockPtr)) {
        return llIsBlockInList(gBin + index, in_pBlockPtr)==1?eLLError_None:eLLError_search_fail;
    }
    else{
        if(llGetNextBlock(in_pBlockPtr) != NULL || llGetPrivBlock(in_pBlockPtr) != NULL){
            return eLLError_Non_Empty_Next_Ptr;
        }
        return eLLError_None;
    }
}

eLLError llCheckHeap() {
    // valid every block
    Heap_ptr cur_ptr = gHeapStart;
    eLLError error = eLLError_None;
    while (llGetDataSizeFromHeader(cur_ptr)!=0) {
        printf("Checking Block %x size %d Free %d. Start: %x, End: %x\n",cur_ptr,llGetDataSizeFromHeader(cur_ptr),llIsBlockFree(cur_ptr),gHeapStart,gHeapEnd);
        error = llCheckBlock(cur_ptr);
        if (error != eLLError_None) {
            printf("Check Block Failed"
                           ". Error: %d\n",error);
            llPrintBlock(cur_ptr);
            assert(0);
            return error;
        }
        error = llValidBining(cur_ptr);
        if (error != eLLError_None) {
            printf("Binning Failed. Error: %d\n",error);
            llPrintBlock(cur_ptr);
            assert(0);
            return error;
        }
        cur_ptr = llGetNextHeapPtrFromHeapPtr(cur_ptr);
    }
    return error;
}
eLLError llFree(Data_ptr in_pDataPtr) {

    // Convert the given data ptr to heap ptr
    Heap_ptr cur_ptr = llGetHeapPtrFromDataPtr(in_pDataPtr);
    // Deallocate the current block

    llMarkBlockAllocationBit(cur_ptr,BLOCK_FREE);
    // Get the previous block ptr
    Heap_ptr prev_ptr = llGetPrevHeapPtrFromHeapPtr(cur_ptr);
    // Get the next block ptr
    Heap_ptr next_ptr = llGetNextHeapPtrFromHeapPtr(cur_ptr);
    // Check if both block are free
    int is_prev_free = llIsBlockFree(prev_ptr);
    int is_next_free = llIsBlockFree(next_ptr);
    Heap_ptr ret = cur_ptr;
    // Check if previous block is free
    if (is_prev_free != 0 && llGetDataSizeFromHeader(prev_ptr) > FAST_BLOCK_SIZE) {
        // Previous block is free, merge current block with previous block
        llPullFromBin(prev_ptr);
        llMergeBlock(ret, prev_ptr, &ret);
    }
    if (is_next_free != 0 && llGetDataSizeFromHeader(next_ptr) > FAST_BLOCK_SIZE) {
        // Next block is free
        llPullFromBin(next_ptr);
        llMergeBlock(ret, next_ptr, &ret);
    }
    // Throw the merged block into bin
    llThrowInBin(ret);

#if HEAP_CHECK_ENABLE
    gError = llCheckHeap();
    if (gError != eLLError_None)
        printf("Heap Error: %d\n", gError);
#endif
    return gError;
}

Data_ptr llAlloc(int in_iSize) {

    Data_ptr ret = NULL;
    // Try to allocate a block from bin
    eLLError error = llAllocFromBin(in_iSize, &ret);
    if(error!=eLLError_None)
        error =  llAllocFromHeap(MAX(32,in_iSize),&ret);
    if (error != eLLError_allocation_fail) {
        // Successfully found a free block inside the list, now we need to check if the block is splittable
        Heap_ptr heap_ret = llGetHeapPtrFromDataPtr(ret);
        // Get the allocated block size
        int block_size = llGetDataSizeFromHeader(heap_ret);
        if(block_size > FAST_BLOCK_SIZE){
            // Get the alligned data size
            int real_data_size = BYTES_TO_WORD(llGetAllignedSizeInBytes(in_iSize, MALLOC_ALIGNMENT));
            // Get the remainder size after spliting the main block

            if(block_size-META_DATA_WORD > real_data_size){
                int remainder_size = llGetSplitedRemainderSize(block_size, real_data_size);
                // If the remainder size is greater than 0 (splitable)
                if (remainder_size > SPLIT_THRESHOLD) {
                    // Blk is splittable, then split the block and put residual into the bin
                    Heap_ptr outptrA;
                    Heap_ptr outptrB;
                    // Creating a new split recipe
                    llSplitRecipe newRecipe;
                    newRecipe.m_iBlockASize = real_data_size;
                    newRecipe.m_iBlockBSize = remainder_size;
                    //then split the current plock
                    llSplitBlock(heap_ret, &newRecipe, &outptrA, &outptrB);
                    // Throw the remainder size into the bin
                    llThrowInBin(outptrB);
                }
            }
        }

    }
//    else {
//        // Cannot found a proper free block on the list, extend the heap and allocate a new block
//        llAllocFromHeap(MAX(512,in_iSize), &ret);
//        llFree(ret);
//        llAllocFromBin(in_iSize,&ret);
//        //llAllocFromHeap(in_iSize,&ret);
//    }
#if HEAP_CHECK_ENABLE
    gError = llCheckHeap();
    if (gError != eLLError_None)
        printf("Heap Error: %d\n", gError);
#endif
    return ret;
}

/*
 * Main Allocation functions
 */
eLLError llInit() {
    llInitBin(BIN_SIZE);
    Heap_ptr ret = llExtendHeap(WORD_TO_BYTES(GUARD_SIZE) * 2);
    llInitBlock(ret, 4);
    gHeapStart = ret+GUARD_SIZE;
    llInitBlock(ret + 4, 4);
    gHeapEnd = ret+GUARD_SIZE;
    return eLLError_None;
}




Data_ptr llRealloc(Data_ptr in_pDataPtr, int in_iSize) {
    // Getting the current data size
    //printf("Realloc\n");
    Heap_ptr ptr = llGetHeapPtrFromDataPtr(in_pDataPtr);
    int current_data_size = WORD_TO_BYTES(llGetDataSizeFromHeader(ptr));
    Data_ptr ret = in_pDataPtr;
    if (in_pDataPtr == NULL) {
        // If input data ptr is NULL, then allocate a new block
        ret = llAlloc(in_iSize);
    } else if (in_iSize == 0) {
        // Size equal zero, basically free
        llFree(in_pDataPtr);
    } else if (current_data_size == in_iSize) {
        // Size doesn't change, ignore
    } else {
        // Final Reallocation
        // Get the maximum extending size by
        if (WORD_TO_BYTES(llGetDataSizeFromHeader(ptr)) >= in_iSize) {
            // Local resizing is possible, extend the current block
        } else {
            // Check next block
            Heap_ptr  next_block = llGetNextHeapPtrFromHeapPtr(ptr);
            if(llIsBlockFree(next_block)){
                //llPrintBlock(next_block);
                int potential_size = llGetDataSizeFromHeader(next_block)+META_DATA_WORD+llGetDataSizeFromHeader(ptr);
                if(WORD_TO_BYTES(potential_size)>=in_iSize){
                    llPullFromBin(next_block);
                    llInitBlock(ptr,potential_size+META_DATA_WORD);
//                    int remainder_size = llGetSplitedRemainderSize(potential_size+META_DATA_WORD, in_iSize);
////                    // If the remainder size is greater than 0 (splitable)
//                    if (remainder_size > SPLIT_THRESHOLD) {
//                        // Blk is splittable, then split the block and put residual into the bin
//                        Heap_ptr outptrA;
//                        Heap_ptr outptrB;
//                        // Creating a new split recipe
//                        llSplitRecipe newRecipe;
//                        newRecipe.m_iBlockASize = in_iSize;
//                        newRecipe.m_iBlockBSize = remainder_size;
//                        //then split the current plock
//                        llSplitBlock(ptr, &newRecipe, &outptrA, &outptrB);
//                        // Throw the remainder size into the bin
//                        llThrowInBin(outptrB);
//                    }

#if HEAP_CHECK_ENABLE
                    gError = llCheckHeap();
    if (gError != eLLError_None)
        printf("Heap Error: %d\n", gError);
#endif
                    return llGetDataPtrFromHeapPtr(ptr);
                }
            }
            Data_ptr new_block = llAlloc(in_iSize);
            Heap_ptr new_heap_ptr = llGetHeapPtrFromDataPtr(new_block);
            // copy the old data into the new data block
            llCopyBlock(ptr, new_heap_ptr, current_data_size);
            // Free the old data block
            llFree(in_pDataPtr);
            ret = new_block;
        }
    }
#if HEAP_CHECK_ENABLE
    gError = llCheckHeap();
    if (gError != eLLError_None)
        printf("Heap Error: %d\n", gError);
#endif
    return ret;
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
    return llInit();
//    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) -1)
//        return -1;
//    PUT(heap_listp, 0);                         // alignment padding
//    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
//    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
//    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
//    heap_listp += DSIZE;
//    return 0;
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
    llFree(bp);
//    if (bp == NULL) {
//        return;
//    }
//    size_t size = GET_SIZE(HDRP(bp));
//
//    PUT(HDRP(bp), PACK(size, 0));
//    PUT(FTRP(bp), PACK(size, 0));
//    coalesce(bp);
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
    return llAlloc(size);

//    size_t asize; /* adjusted block size */
//    size_t extendsize; /* amount to extend heap if no fit */
//    char *bp;
//    /* Ignore spurious requests */
//    if (size == 0)
//        return NULL;
//
//    /* Adjust block size to include overhead and alignment reqs. */
//    if (size <= DSIZE)
//        asize = 2 * DSIZE;
//    else
//        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
//
//    /* Search the free list for a fit */
//    if ((bp = find_fit(asize)) != NULL) {
//        place(bp, asize);
//        return bp;
//    }
//
//    /* No fit found. Get more memory and place the block */
//    extendsize = MAX(asize, CHUNKSIZE);
//    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
//        return NULL;
//    place(bp, asize);
//    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) {
    return llRealloc(ptr, size);
//    /* If size == 0 then this is just free, and we return NULL. */
//    if (size == 0) {
//        mm_free(ptr);
//        return NULL;
//    }
//    /* If oldptr is NULL, then this is just malloc. */
//    if (ptr == NULL)
//        return (mm_malloc(size));
//
//    void *oldptr = ptr;
//    void *newptr;
//    size_t copySize;
//
//    newptr = mm_malloc(size);
//    if (newptr == NULL)
//        return NULL;
//
//    /* Copy the old data. */
//    copySize = GET_SIZE(HDRP(oldptr));
//    if (size < copySize)
//        copySize = size;
//    memcpy(newptr, oldptr, copySize);
//    mm_free(oldptr);
//    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void) {
    return 1;
}
