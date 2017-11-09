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
#include <pthread.h>

#include "malloc.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
name_t my_name = {
        /* Team name */
        "NSFW",
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


/*********************************************
 *  Type define and constant define
 *********************************************/
// All codes for lab3 goes inside here
// heap pointer points to a heap block (data block + meta data)
typedef uintptr_t *Heap_ptr;
// data pointer points to a data block inside heap block
typedef uintptr_t *Data_ptr;
// if the integer is WORD type, means it is the size in words (8 bytes)
// this is also the default size in the code
typedef int WORD;
// if the integer is WORD type, means it is the size in bytes
typedef int BYTE;


// Size of each entries of hte Bin, 8 bytes for 64bit machine
#define PTR_ALIGNMENT (sizeof(void*))
// the alignment of address -16 bytes alignments
#define MALLOC_ALIGNMENT 4
/* the size (words) of guard blocks
 * guard block is a block has only meta data and 0 real data block
 * it locates at top and end of heap area
 * it is used to protected merge unexpected memory space when llfree() is called
 */
#define GUARD_SIZE 4
//Size in words of  header META data (Size of block + allocation)
#define HEADER_OFFSET 1
//Size in words of pointer to next free block in META data
#define NEXT_PTR_SIZE 1
//Size in words of  pointer to previous free block in META data
#define PREVIOUS_PTR_OFFSET 1
// Size of prologue in words
#define PROLOGUE_OFFSET (HEADER_OFFSET+NEXT_PTR_SIZE)
// Size of epilogue in words
#define EPILOGUE_OFFSET (HEADER_OFFSET+PREVIOUS_PTR_OFFSET)
/*
 * Size of meta data in words
 * Over all a block (8 bytes each) will have the data structure like following
 * 63______________________3___________1_________0
 * |    Block Size (bytes) |unused bits|alloc bit|
 * |_______________________|___________|_________|
 * |    Pointer  to next free block in the BIN   |
 * |_____________________________________________|
 * |                                             |
 * |                                             |
 * |       data payload 16 byte aligned          |
 * |_____________________________________________|
 * |  Pointer to previous free block in the BIN  |
 * |_____________________________________________|
 * |    Block Size (bytes) |unused bits|alloc bit|
 * |_____________________________________________|
 * BLOCK 1: Header [60'b data size in word,3'b000,allocation bit]
 * BLOCK 2: Pointer to next free block
 * BLOCK 3: data block align with 16 bytes
 * BLOCK 4: Pointer to previous free block
 * BLOCK 5: Footer block [60'b data size in word,3'b000,allocation bit]
 */
#define BLOCK_META_SIZE_WORD (PROLOGUE_OFFSET+EPILOGUE_OFFSET)
//Define of block is used
#define BLOCK_ALLOCATED 1
//Define of block is free
#define BLOCK_FREE 0
#define INVALID_HEAP_PTR ((Heap_ptr)-1)

/*********************************************
 *  Configurations
 *********************************************/
//this enables heap check function
//Warning: it is super slow to turn on heap check, please do not turn on
#define HEAP_CHECK_ENABLE 0
// Enable Fast block direct allocation
#define FAST_BLOCK_DIRECT_ALLOC_ENABLE 0
// Enable Chunk Extend when calling mem_sbrk;
#define CHUNK_EXTEND_ENABLE 210
// Size of the BIN holding a list of pointers to free blocks
#define BIN_SIZE 88
//blocks under FAST_BLOCK_SIZE will not be merged because it is too small
#define FAST_BLOCK_SIZE 12
//blocks with size under SPLIT_THRESHOLD after split will not be split
#define SPLIT_THRESHOLD 8

/*********************************************
 *  Macro functions define
 *********************************************/
//macro returns the minimum value of (x,y)
#define MIN(x, y) ((x) < (y)?(x) :(y))
// Get Ptr from address
#define GET_PTR(x) (Heap_ptr)(GET(x))
// Put Ptr in address
#define PUT_PTR(x,y) PUT((x),(uintptr_t)(y))

//macro change size in words (8 bytes) into size in bytes
#define WORD_TO_BYTES(x) ((x)<<3)
//macro change size in bytes into size in words (8 bytes)
#define BYTES_TO_WORD(x) ((x)>>3)
//macro change size in words (8 bytes) into size in words (16 bytes)
#define WORD_TO_DWORD(x) ((x)>>1)

// returns the pointer to data blocks in side a heap block
#define llGetDataPtrFromHeapPtr(x) ((x+NEXT_PTR_SIZE+HEADER_OFFSET))
// returns the pointer to heap blocks contain the data blocks
#define llGetHeapPtrFromDataPtr(x) ((x-PROLOGUE_OFFSET))
// returns the data size in the heap blocks
#define llGetDataSizeFromHeader(x) (GET(x)>>MALLOC_ALIGNMENT)
// return the pointer to previous heap block in memory space
#define llGetPrevHeapPtrFromHeapPtr(x) ((x-llGetDataSizeFromHeader(x-1)-BLOCK_META_SIZE_WORD))
// return the pointer to nexp heap block in memory space
#define llGetNextHeapPtrFromHeapPtr(x) ((x+BLOCK_META_SIZE_WORD+llGetDataSizeFromHeader(x)))

// set the next  free block linked in the BIN
#define llSetNextBlock(x, target) (PUT_PTR((x+HEADER_OFFSET),target))
// returns the next free block linked in the BIN
#define llGetNextBlock(x) (GET_PTR(x+HEADER_OFFSET))
// set the previous  free block linked in the BIN
#define llSetPrevBlock(x, target) (PUT_PTR((x+PROLOGUE_OFFSET+llGetDataSizeFromHeader(x)),target))
// returns the previous free block linked in the BIN
#define llGetPrevBlock(x) ((Heap_ptr)(GET(x+PROLOGUE_OFFSET+llGetDataSizeFromHeader(x))))

// disconnect a block inside the link list and set its NextBloack and PrivBloack to NULL
#define llDisconnectBlock(y) {\
    Heap_ptr __next = llGetNextBlock(y);\
    Heap_ptr __prev = llGetPrevBlock(y);\
    if(__next!=NULL){\
        llSetPrevBlock(__next, __prev);\
    }\
    if(__prev!=NULL){\
        llSetNextBlock(__prev, __next);\
    }\
    llSetNextBlock(y, NULL);\
    llSetPrevBlock(y, NULL);\
}

//push x into the head of the link list
#define llPush(x, head){\
    llSetNextBlock((x), (head));\
    llSetPrevBlock((x), NULL);\
    if((head)!= NULL)\
        llSetPrevBlock((head), x);\
}

//check is the block is used(return 0) or free(return 1)
#define llIsBlockFree(x) (!(GET(x) & 1))
//calculate the index of a block in the BIN
#define llGetBinningIndex(x) (MIN((WORD_TO_DWORD(llGetDataSizeFromHeader(x))-1),(BIN_SIZE-1)))

/*********************************************
 * Enum define
 *********************************************/
// enum of different error type
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
	eLLError_arena_busy = 10,
	eLLError_arena_full = 11,
	eLLError_target_initialized = 12,
    eLLError_None = 0
} eLLError;

//structure used in split block function contains the division of the block
typedef struct _llSplitRecipe {
    int m_iBlockASize;
    int m_iBlockBSize;
} llSplitRecipe;

#define LAB4_START 1
#if LAB4_START
#define NUM_OF_AREANA 4
#define ARENA_INITIAL_SIZE 8000
#define ARENA_PROLOGUE_SIZE 1
#define ARENA_EPILOGUE_SIZE 1
#define ARENA_META_SIZE (ARENA_EPILOGUE_SIZE+ARENA_PROLOGUE_SIZE)
typedef int llArenaID;

typedef struct _llArenaContext {
	llArenaID m_iArenaID;
	int m_iArenaSize;
	Heap_ptr m_pBins[BIN_SIZE];
	pthread_mutex_t m_ArenaLock;
} llArenaContext;

typedef struct _llControlContext {
	llArenaContext m_pArenas[NUM_OF_AREANA];
	pthread_mutex_t m_iHeapLock;
} llControlContext;

/** New global variables **/
llControlContext* gControlContext = NULL;


#define llCreateOnHeap(__type) (__type*)(mem_sbrk(sizeof(__type)))
#define llGetArenaIDFromHeapPtr(x) ((llArenaID)((GET(x) & 15) >> 1))
#define llSetArenaIDToHeapPtr(x,id) PUT(x,(GET(x)& ((~15)+1) | (id << 1))) 

eLLError llInitArena(Heap_ptr in_pHeapPtr, int in_iArenaSizeInWord){
    // Set prologue 
    PUT(in_pHeapPtr,PACK((in_iArenaSizeInWord) << MALLOC_ALIGNMENT,1));
    // Set epilogue
    PUT(in_pHeapPtr+in_iArenaSizeInWord, PACK((in_iArenaSizeInWord) << MALLOC_ALIGNMENT,1));
}
eLLError llExtendArena(llArenaID in_iArenaID, int in_iSizeInWord){
    int target_size = MAX(in_iSizeInWord, ARENA_INITIAL_SIZE);
    Heap_ptr* extended_ptr = mem_sbrk(WORD_TO_BYTES(target_size));
    if(extended_ptr!=NULL){
        llInitArena(extended_ptr,in_iSizeInWord);
        return eLLError_None;
    }
    else{
        return eLLError_allocation_fail;
    }
}
eLLError llInitControlContext(){
	if(gControlContext==NULL){
		Heap_ptr* tmpControlContext = llCreateOnHeap(llControlContext);
		if(tmpControlContext!=NULL){
			gControlContext = tmpControlContext;
			return eLLError_None;
		}
		return eLLError_allocation_fail;
	}
	return eLLError_target_initialized;
}
eLLError llInitArenaContexts(){
	if(gControlContext!=NULL){
		int i;
		for(i = 0 ; i < NUM_OF_AREANA; i++){
            // Initialize arena ID
            gControlContext->m_pArenas[i].m_iArenaID = i;
            // Initialize arena lock
            pthread_mutex_init(&gControlContext->m_pArenas[i].m_ArenaLock,NULL);
            // Initialize arena size
            gControlContext->m_pArenas[i].m_iArenaSize = ARENA_INITIAL_SIZE;
        }
	}
}



/** Call this function to try lock one of the arenas **/
eLLError llLockArena(llArenaID* io_iArenaID){
    llArenaID target_arena = -1;
    int i;
    // iterate through every possible arena and try to gain a lock for it.
    for(int i = 0; i < NUM_OF_AREANA; i++){
        if(pthread_mutex_trylock(&gControlContext->m_pArenas[i].m_ArenaLock)==0){
            target_arena = i;
        }
    }
    if(target_arena==-1){
        // We have not aquired any lock, wait for an arena based on its thread id
        pthread_t id = pthread_self();
        target_arena = id&NUM_OF_AREANA;
        pthread_mutex_lock(&gControlContext->m_pArenas[target_arena].m_ArenaLock);
    }
    *io_iArenaID = target_arena;
    return eLLError_None;
}
eLLError llUnlockArena(llArenaID in_iArenaID){
    pthread_mutex_unlock(&gControlContext->m_pArenas[in_iArenaID].m_ArenaLock);
}


#endif

/*********************************************
 * Global values define
 *********************************************/
// pointer to the BIN
Heap_ptr gBin;
// How many entries in the Bin
int gBinSize;
// Start point of the heap usage, used for heap check
Heap_ptr gHeapStart;
// End point of the heap usage, used for heap check
Heap_ptr gHeapEnd;
//Error Message
eLLError gError;

/*********************************************
 * Helping functions
 *********************************************/

/* Function: llPrintBlock
 * -------------------------------------------------------------
 * Print out information of one block, use for sel debug only
 * Input:
 * in_pBlockPtr: pointer to a heap block
 */
void llPrintBlock(Heap_ptr in_pBlockPtr) {
    // Place the header
    Heap_ptr end = in_pBlockPtr + PROLOGUE_OFFSET + PREVIOUS_PTR_OFFSET + (llGetDataSizeFromHeader(in_pBlockPtr));
    printf("[Header: %llx]Size: %d, Allocated: %d\n",in_pBlockPtr,llGetDataSizeFromHeader(in_pBlockPtr), !llIsBlockFree(in_pBlockPtr));
    printf("Next Ptr: %llx\n", GET(in_pBlockPtr + 1));
    printf("Priv Ptr: %llx\n", llGetPrevBlock(in_pBlockPtr));
    printf("[Footer: %llx]Size: %d, Allocated: %d\n",end, llGetDataSizeFromHeader(end), !llIsBlockFree(end));
}

/* Function: llGetAllignedSizeInBytes
 * -------------------------------------------------------------
 * Get aligned size from in_iInput
 * Input:
 * in_iInput:input size in bytes
 * in_iAlignment: alignment, must be a power of 2
 *
 * Return: aligned size in bytes
 */
BYTE llGetAllignedSizeInBytes(BYTE in_iInput, int in_iAlignment) {
    if ((in_iInput & ((1 << in_iAlignment) - 1)) > 0)
        return ((in_iInput >> in_iAlignment) + 1) << in_iAlignment;
    else
        return in_iInput;
}

/* Function: llExtendHeap
 * -------------------------------------------------------------
 * extend heap size
 * Input:
 * in_iExtendSize:request extra heap size
 *
 * Return: the point to the beginning of extended heap or error if failed
 */
Heap_ptr llExtendHeap(BYTE in_iExtendSize) {
    Heap_ptr ret = mem_sbrk(in_iExtendSize);
    //printf("Address: %llx\n",ret);
    if (ret != INVALID_HEAP_PTR) {
        return ret;
    } else {
        return INVALID_HEAP_PTR;
    }
}

/* Function: llInitBin
 * -------------------------------------------------------------
 * Initialize Bin for free blocks
 *
 * Return: Error message
 */
eLLError llInitBin() {
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
            PUT_PTR(gBin + i, NULL);
        }
        return eLLError_None;
    }
}

/* Function: llMarkBlockAllocationBit
 * -----------------------------------------
 * change the allocation inform of a block
 * Input:
 * in_pBlockPtr: pointer to a block
 * in_bAllocated: allocation inform (0=not used,1= used)
 *
 * Return: Error message
 */
eLLError llMarkBlockAllocationBit(Heap_ptr in_pBlockPtr, int in_bAllocated) {
    WORD data_size = llGetDataSizeFromHeader(in_pBlockPtr);
    PUT(in_pBlockPtr, PACK(data_size << MALLOC_ALIGNMENT, in_bAllocated));
    PUT(in_pBlockPtr + PROLOGUE_OFFSET + data_size + PREVIOUS_PTR_OFFSET,
        PACK(data_size << MALLOC_ALIGNMENT, in_bAllocated));
    return eLLError_None;
}

/* Function llInitBlock
 * ------------------------
 * Initialize the block (with all meta data)
 * Input:
 * in_pInputPtr: pointer to heap block
 * in_iBlockSizeInWord: desire block size with meta data
 *
 * Return: error message
 */
eLLError llInitBlock(Heap_ptr in_pInputPtr, WORD in_iBlockSizeInWord) {
    //llPrintBlock(llGetPrevHeapPtrFromHeapPtr(in_pInputPtr));
    WORD data_size_in_dword = in_iBlockSizeInWord - BLOCK_META_SIZE_WORD;
    // Place the header
    PUT(in_pInputPtr, PACK(data_size_in_dword << MALLOC_ALIGNMENT, 1));
    // initialize the previous ptr
    llSetPrevBlock(in_pInputPtr,NULL);
    // initialize the next ptr
    llSetNextBlock(in_pInputPtr,NULL);
    // initialize the footer
    PUT(in_pInputPtr + PROLOGUE_OFFSET + data_size_in_dword + PREVIOUS_PTR_OFFSET,
        PACK(data_size_in_dword << MALLOC_ALIGNMENT, 1));
    return eLLError_None;
}

/* Function llAllocFromHeap
 * ------------------------
 * Initialize the block from heap(with all meta data)
 * Input:
 * io_pOutputPtr: pointer to data block
 * in_iSizeInBytes: desire block size with meta data
 *
 * Return: error message
 */
eLLError llAllocFromHeap(size_t in_iSizeInBytes, Data_ptr *io_pOutputPtr) {
    // get the corresponding alignment size
    WORD aligned_size = BYTES_TO_WORD(llGetAllignedSizeInBytes(in_iSizeInBytes, MALLOC_ALIGNMENT));
    // Calculate the real block size
    WORD block_size = aligned_size + BLOCK_META_SIZE_WORD;
    // Allocate a new block from heap
    Heap_ptr bp = llExtendHeap(WORD_TO_BYTES(block_size));
    if (bp == INVALID_HEAP_PTR) {
        return eLLError_heap_extend_fail;
    } else {
        // Initialize the block
        // use bp-GUARD_SIZE re over-write the last bottom heap guard block
        llInitBlock(bp - GUARD_SIZE, block_size);
        gHeapEnd  = bp-GUARD_SIZE+block_size;
        // re-init a bottom guard block
        llInitBlock((bp - GUARD_SIZE + block_size), GUARD_SIZE);
        *io_pOutputPtr = llGetDataPtrFromHeapPtr(bp - GUARD_SIZE);
        return eLLError_None;
    }
}


/* Main Sub Function llAllocFromHeap
 * ------------------------
 * Initialize the block from bin (initialized free block)(with all meta data)
 * Input:
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

#if FAST_BLOCK_DIRECT_ALLOC_ENABLE
    if(BYTES_TO_WORD(in_iSizeInBytes) <= FAST_BLOCK_SIZE && GET(gBin + start_index) == NULL){
        return eLLError_search_fail;
    }
#endif

    Heap_ptr ret = NULL;
    //go though the first part of the BIN to check is there is available block
    while (start_index < gBinSize-1) {
        if (GET_PTR(gBin + start_index) != NULL) {
            // valid bin found, reconstruct the link list
            ret = GET_PTR(gBin + start_index);
            // set the head of the linked list to the next element
            PUT_PTR(gBin + start_index, llGetNextBlock(ret));
            llDisconnectBlock(ret);
            break;
        }
        start_index++;
    }
    // ALL list except for last entry is find and there is no usable blocks
    // This is special case cause last entries contains all blocks greater than certian value
    //nor all blocks in the entry is larger than the request size
    if(ret == NULL) {
        Heap_ptr cur_ptr= GET_PTR(gBin+gBinSize-1);
        if(cur_ptr!=NULL){
            Heap_ptr next_ptr = llGetNextBlock(cur_ptr);
            if (WORD_TO_BYTES(llGetDataSizeFromHeader(cur_ptr))>= in_iSizeInBytes)
            {
                // Head is the target, need to change the head
                ret = cur_ptr;
                PUT_PTR((gBin+gBinSize-1), next_ptr);
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
    // find a usable block
    if (ret != NULL) {
        // modified the allocation bit
        llMarkBlockAllocationBit(ret, BLOCK_ALLOCATED);
        *io_pOutputPtr = llGetDataPtrFromHeapPtr(ret);
        return eLLError_None;
    } else {
        //fails to find a usable block return error
        *io_pOutputPtr = NULL;
        return eLLError_allocation_fail;
    }

}


/* Helping Function: llGetSplitedRemainderSize
 * -----------------------------------
 * calculate the remaining data size (no meta) after split
 * Input:
 * in_iTotalDataSize: total block size (with meta) available
 * in_iTargetSize: desire data (no meta)size
 *
 * Return: remain data size (no meta)
 */
WORD llGetSplitedRemainderSize(WORD in_iTotalDataSize, WORD in_iTargetSize) {
    WORD remain_size = in_iTotalDataSize - in_iTargetSize - BLOCK_META_SIZE_WORD;
    if (remain_size < 4) {
        //printf("Error: Try to split size %d from size %d\n", in_iTargetSize, in_iTotalDataSize);
        return 0;
    } else
        return remain_size;

}

/* Helping Function: llGetMaximumExtendableSize
 * -----------------------------------
 * calculate the maximum extendable size of heap
 * Input:
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
            max_size += (llGetDataSizeFromHeader(cur_ptr) + BLOCK_META_SIZE_WORD);
        } else {
            break;
        }
        cur_ptr = llGetNextHeapPtrFromHeapPtr(cur_ptr);
    }
    return max_size;
}

/* Helping Function: llThrowInBin
 * -----------------------------------
 * free a used data block and put it back to Bin
 * Input:
 * in_pHeapPtr: pointer to the block
 *
 * Return: Error Message
 */
eLLError llThrowInBin(Heap_ptr in_pHeapPtr) {

    int index = llGetBinningIndex(in_pHeapPtr);
    llPush(in_pHeapPtr,(Heap_ptr)GET(gBin+index));
    PUT_PTR(gBin + index, in_pHeapPtr);
    return eLLError_None;
}

/* Helping Function llPullFromBin
 * ---------------------
 * This function pull one specific block from a specific link list in BIN
 * Input:
 * in_pHeapPtr: pointer to the block
 *
 * Return: Error message
 */
eLLError llPullFromList(Heap_ptr in_pBin, Heap_ptr in_pTarget) {
    if(GET_PTR(in_pBin) == in_pTarget)
        PUT_PTR(in_pBin,llGetNextBlock(in_pTarget));
    llDisconnectBlock(in_pTarget);
    return eLLError_None;
}

/* Helping Function llPullFromBin
 * ---------------------
 * This function pull one specific block from BIN
 * Input:
 * in_pHeapPtr: pointer to the block
 *
 * Return: Error message
 */
eLLError llPullFromBin(Heap_ptr in_pHeapPtr) {
    int index = llGetBinningIndex(in_pHeapPtr);
    return llPullFromList(gBin + index, in_pHeapPtr);

}

/* Helping Function llMergeBlock
 * ---------------------
 * Merge up to three free block into one. assume merged block is free and contain no data
 * Input:
 * in_pInputPtrA: pointer to the free block A
 * in_pInputPtrB: pointer to the free block B
 * io_pOutputPtr: pointer to the merged new block
 *
 * Return: Error message
 */
eLLError llMergeBlock(Heap_ptr in_pInputPtrA, Heap_ptr in_pInputPtrB, Heap_ptr *io_pOutputPtr) {
    WORD block_A_size = llGetDataSizeFromHeader (in_pInputPtrA) + BLOCK_META_SIZE_WORD;
    WORD block_B_size = llGetDataSizeFromHeader (in_pInputPtrB) + BLOCK_META_SIZE_WORD;
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

/* Helping Function llSplitBlock
 * ---------------------
 * Split one free block into two. assume split legal
 * Input:
 * in_pInputPtr: pointer to the free block
 * in_pRecipe: desire division of the block
 * io_pOutputPtrA: pointer to the free block A
 * io_pOutputPtrB: pointer to the free block B
 *
 * Return: Error message
 */
eLLError llSplitBlock(Heap_ptr in_pInputPtr, llSplitRecipe *in_pRecipe, Heap_ptr *io_pOutputPtrA, Heap_ptr *io_pOutputPtrB) {
    WORD block_A_size = in_pRecipe->m_iBlockASize + BLOCK_META_SIZE_WORD;
    WORD block_B_size = in_pRecipe->m_iBlockBSize + BLOCK_META_SIZE_WORD;
    *io_pOutputPtrA = in_pInputPtr;
    llInitBlock(*io_pOutputPtrA, block_A_size);
    *io_pOutputPtrB = in_pInputPtr + block_A_size;
    llInitBlock(*io_pOutputPtrB, block_B_size);
    llMarkBlockAllocationBit(*io_pOutputPtrB,BLOCK_FREE);
    return eLLError_None;
}

/* Helping Function llCopyBlock
 * ---------------------
 * one block's data into another block. assume copy legal
 * Input:
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

/*********************************************
 * Heap Consistency Checker
 *********************************************/

/* Debug Function llIsBlockInList
 * --------------------------------
 * This function is used to check the correctness of a specific block
 * This is ued only when HEAP_CHECK_ENABLE == 1
 * Input:
 * in_pBlockPtr: pointer to the heap block
 *
 * Return: Error message
 */
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

/* Debug Function llIsBlockInList
 * --------------------------------
 * This function is used to check if one specific block is in the BIN
 * This is ued only when HEAP_CHECK_ENABLE == 1
 * Input:
 * in_pBin: pointer to the BIN
 * in_pBlockPtr: pointer to the heap block
 *
 * Return: Error message
 */
int llIsBlockInList(Heap_ptr in_pBin, Heap_ptr in_pBlockPtr) {
    Heap_ptr start_block = (Heap_ptr)GET(in_pBin);
    while (start_block != NULL) {
        if (start_block == in_pBlockPtr) {
            return 1;
        }
        start_block = llGetNextBlock(start_block);
    }
    return 0;
}

/* Debug Function llValidBining
 * --------------------------------
 * This function is used to check the correctness of a block in the BIN
 * This is ued only when HEAP_CHECK_ENABLE == 1
 * Input:
 * in_pBlockPtr: pointer to BIN
 *
 * Return: Error message
 */
eLLError llValidBining(Heap_ptr in_pBlockPtr) {
    int index = llGetBinningIndex(in_pBlockPtr);
    if (llIsBlockFree(in_pBlockPtr)) {
        return llIsBlockInList(gBin + index, in_pBlockPtr)==1?eLLError_None:eLLError_search_fail;
    }
    else{
        if(llGetNextBlock(in_pBlockPtr) != NULL || llGetPrevBlock(in_pBlockPtr) != NULL){
            return eLLError_Non_Empty_Next_Ptr;
        }
        return eLLError_None;
    }
}

/* Debug Function llCheckHeap
 * --------------------------------
 * This function is used to check the correctness of the heap mem area
 * This is ued only when HEAP_CHECK_ENABLE == 1
 *
 * Return: Error message
 */
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

/********************************************************
 * Main functions of link list memory allocation (llaloc)
 ********************************************************/

/* Main Function llFree
 * --------------------------------
 * This function will free a data blocks
 * When program frees a block, it will looks at previous block and next block in the heap to check
 * if it is possible to merge. After merging, it will put the blocks into the bin for future re-use
 * Input:
 * in_pDataPtr: pointer to the data bloack needs to be free
 *
 * Return: Error message
 */
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
    if (is_prev_free != BLOCK_FREE && llGetDataSizeFromHeader(prev_ptr) > FAST_BLOCK_SIZE) {
        // Previous block is free, merge current block with previous block
        llPullFromBin(prev_ptr);
        llMergeBlock(ret, prev_ptr, &ret);
    }
    if (is_next_free != BLOCK_FREE && llGetDataSizeFromHeader(next_ptr) > FAST_BLOCK_SIZE) {
        // Next block is free
        llPullFromBin(next_ptr);
        llMergeBlock(ret, next_ptr, &ret);
    }
    // Throw the merged block into bin
    llThrowInBin(ret);
#if HEAP_CHECK_ENABLE
    printf("Free: %llx\n", ret);
    gError = llCheckHeap();
    if (gError != eLLError_None)
        printf("Heap Error: %d\n", gError);
#endif
    return gError;
}

/* Main Function llAlloc
 * --------------------------------
 * This function will allocate a heap block contain data block with size (in_iSize)
 * This function will find if there is possible free blocks from the BIN
 * If there is no usable blocks from the BIN, the program will extend the heap (at least 256 bytes)to alloc the block
 * The free second-hand blocks may have different size from the design size, so the program will check kif the free
 * second-hand block is splittable. If yes, it will split the block and put the not used on back to BIN
 * Input:
 * in_iSize: desire size
 *
 * Return pointer to the data block
 */
Data_ptr llAlloc(int in_iSize) {

    Data_ptr ret = NULL;
    // Try to allocate a block from bin
    eLLError error = llAllocFromBin(in_iSize, &ret);
    if(error!=eLLError_None){
#if CHUNK_EXTEND_ENABLE
        error =  llAllocFromHeap(MAX(CHUNK_EXTEND_ENABLE,in_iSize),&ret);
#else
        error =  llAllocFromHeap(in_iSize,&ret);
#endif
    }
    if (error != eLLError_allocation_fail) {
        // Successfully found a free block inside the list, now we need to check if the block is splittable
        Heap_ptr heap_ret = llGetHeapPtrFromDataPtr(ret);
        // Get the allocated block size
        int block_size = llGetDataSizeFromHeader(heap_ret);
        if(block_size > FAST_BLOCK_SIZE){
            // Get the alligned data size
            int real_data_size = BYTES_TO_WORD(llGetAllignedSizeInBytes(in_iSize, MALLOC_ALIGNMENT));
            // Get the remainder size after spliting the main block

            if(block_size-BLOCK_META_SIZE_WORD > real_data_size){
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

#if HEAP_CHECK_ENABLE
    printf("Alloc: %llx\n",llGetHeapPtrFromDataPtr(ret));
    gError = llCheckHeap();
    if (gError != eLLError_None)
        printf("Heap Error: %d\n", gError);
#endif
    return ret;
}

/* Main Function llInit
 * --------------------------------
 * Initialize the Bin and the guard block
 *
 * Return Error message
 */
eLLError llInit() {
    llInitBin();
    Heap_ptr ret = llExtendHeap(WORD_TO_BYTES(GUARD_SIZE) * 2);
    //initialize a top guard block
    llInitBlock(ret, 4);
    gHeapStart = ret+GUARD_SIZE;
    //initialize a bottom guard block
    llInitBlock(ret + 4, 4);
    gHeapEnd = ret+GUARD_SIZE;
    return eLLError_None;
}

/* Main Function llRealloc
 * -------------------------------------------
 * This function will re-alloc a data block with following cases
 * 1)input pointer = NULL                                       => alloc a new block
 * 2)desire size= 0                                             => free the block
 * 3)same or smaller desire size                                => do nothing
 * 4)larger desire size
 *   1. if the block is following by a large enough free block  => extend the block
 *   2. if the block not available to extend                    => allocate a new block, copy data, free old block
 *
 * Input:
 * in_pDataPtr: pointer to the data block
 * in_iSize: the desire new size (in bytes, not include meta data)
 *
 * Return: the pointer to the data block
 */
Data_ptr llRealloc(Data_ptr in_pDataPtr, int in_iSize) {
    //cover the data block to heap pointer
    Heap_ptr ptr = llGetHeapPtrFromDataPtr(in_pDataPtr);
    // Getting the current data size
    WORD current_data_size = WORD_TO_BYTES(llGetDataSizeFromHeader(ptr));
    Data_ptr ret = in_pDataPtr;
    if (in_pDataPtr == NULL) {
        // If input data ptr is NULL, then allocate a new block
        ret = llAlloc(in_iSize);
    } else if (in_iSize == 0) {
        // Size equal zero, basically free
        llFree(in_pDataPtr);
    } else if (current_data_size >= in_iSize) {
        // Size doesn't change or has enough size, ignore
    } else {
        // Final Reallocation
        // Check next block
        Heap_ptr  next_block = llGetNextHeapPtrFromHeapPtr(ptr);
        if(llIsBlockFree(next_block)){
            WORD potential_size = llGetDataSizeFromHeader(next_block)+BLOCK_META_SIZE_WORD+llGetDataSizeFromHeader(ptr);
            if(WORD_TO_BYTES(potential_size)>=in_iSize){
                llPullFromBin(next_block);
                llInitBlock(ptr,potential_size+BLOCK_META_SIZE_WORD);
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
    #if HEAP_CHECK_ENABLE
        gError = llCheckHeap();
        if (gError != eLLError_None)
            printf("Heap Error: %d\n", gError);
    #endif
    return ret;
}

#endif

pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void) {
    if (dseg_lo == NULL && dseg_hi == NULL) {
		mem_init();
	}
    return llInit();
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) {
    pthread_mutex_lock(&malloc_lock);
    llFree(bp);
    pthread_mutex_unlock(&malloc_lock);
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
    Data_ptr ll_data_point;
    pthread_mutex_lock(&malloc_lock);
    ll_data_point= llAlloc(size);
    pthread_mutex_unlock(&malloc_lock);
    return ll_data_point;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void) {
    return llCheckHeap();
}
