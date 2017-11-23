#include "ggBoardPool.h"
#include <stdlib.h>
#include <string.h>

void ggInitBoardPool(){
    memset(gBoadPool,0,sizeof(ggBulkChain*)*eQuadSize_Count);
}
ggBulkChain* ggAllocBulk(ggeQuadSize in_eQuadSize){
    ggBulkChain* ret = ggAlloc(ggBulkChain);
    ret->m_iTotalChunkCount = GG_BULK_ALLOC_STEP;
    ret->m_iUsedChunkCount = 0;
    ret->m_iChunkSize = (1 << (in_eQuadSize+2))*(1 << (in_eQuadSize+2))*sizeof(char);
    ret->m_pMem = malloc(ret->m_iTotalChunkCount*ret->m_iChunkSize);
    ret->m_pNextBulk = NULL;
    return ret;
}
void* ggAllocFromBulk(ggBulkChain* in_pBulkChain){
    void* ret = in_pBulkChain->m_pMem+in_pBulkChain->m_iUsedChunkCount*
    in_pBulkChain->m_iChunkSize;
    in_pBulkChain->m_iUsedChunkCount++;

    return ret;
}
ggBoard* ggAllocBoard(ggeQuadSize in_eQuadSize){
    ggBulkChain* current_chain = gBoadPool[in_eQuadSize];
    ggBulkChain* previous_chain = NULL;
    while(current_chain!=NULL){
        if(current_chain->m_iUsedChunkCount < current_chain->m_iTotalChunkCount){
            // Chain is not full, use this one
            break;
        }
        previous_chain = current_chain;
        current_chain = current_chain->m_pNextBulk;
    }
    if(current_chain!=NULL){
        // Free Chain not found, need to alloc a new one
        ggBulkChain* newChunk = ggAllocBulk(in_eQuadSize);
        if(previous_chain==NULL){
            /** Newly allocated block, should update the head but not
             * The previous ptr
             * */
            gBoadPool[in_eQuadSize] = newChunk; 
        }
        else{
            previous_chain->m_pNextBulk = newChunk;
        }
        current_chain = newChunk;
    }
    void* data = ggAllocFromBulk(current_chain);
    ggBoard* ret =  ggAlloc(ggBoard);
    ret->m_eBoardSize = in_eQuadSize;
    ret->m_pBoardData = data;
    ret->m_pFutureBoard = NULL;
    return ret;
}

void ggFreeBulk(ggBulkChain* in_pBulkChain){
    ggBulkChain* current_ptr = in_pBulkChain;
    ggBulkChain* previous_ptr = NULL;
    while(current_ptr!=NULL){
        // Free the first one, 
        previous_ptr= current_ptr;
        current_ptr = current_ptr->m_pNextBulk;
        free(previous_ptr->m_pMem);
        free(previous_ptr);
    }
}
void ggFreePool(){
    int i;
    for(i = 0; i < eQuadSize_Count;i++){
        ggFreeBulk(gBoadPool[i]);
    }
}