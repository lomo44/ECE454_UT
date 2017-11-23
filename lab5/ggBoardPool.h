#ifndef _ggBoardPool_h
#define _ggBoardPool_h

// Header for bulk-alloc boards

#include "ggTypes.h"
#include "ggBoard.h"

#define GG_BULK_ALLOC_STEP 30

typedef struct _ggBulkChain {
    int m_iTotalChunkCount;
    int m_iUsedChunkCount;
    int m_iChunkSize;
    void* m_pMem;
    struct __ggBulkChain* m_pNextBulk;
} ggBulkChain;

ggBulkChain* gBoadPool[eQuadSize_Count]; 

//TODO: Implement

void ggInitBoardPool();
void* ggAllocFromBulk(ggBulkChain* in_pBulkChain);
ggBulkChain* ggAllocBulk(ggeQuadSize in_eQuadSize);
ggBoard* ggAllocBoard(ggeQuadSize in_eQuadSize);
void ggFreePool();
#endif