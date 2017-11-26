#include "ggHashLife.h"

ggQuadTreeNode *ggCreateTreeFromBoard(ggBoard *in_pBoard)
{
    return NULL;
}
ggQuadTreeNode* ggEvolveTree(ggQuadTreeNode* in_pTreeNode, int m_iIteration, ggHashLifeContext* in_pContext){
    return NULL;
}
void ggConvertTreeToBoard(ggQuadTreeNode* in_pTreeNode, ggBoard* in_pBoard,  ggHashLifeContext* in_pContext){
    return;
}

void ggFreeContext(ggHashLifeContext* in_pContext){
    // decallocate the leaf nodes array
    free(in_pContext->m_pPatternTables);
    // de-allocate all of the hash tabls
    for(int i = 0; i < eQuadSize_Count;i++){
        ggHashTable_Free(in_pContext->m_pPatternTables[i]);
    }
}

ggHashLifeContext* ggCreateContext(){
    ggHashLifeContext* newContext = ggAlloc(ggHashLifeContext);
    // Initialize leafnodes array
    newContext->m_pLeafNodes = ggQuadTree_InitLeaf();
    // Initialize hash tables for different nodes
    for(int i =0 ; i < eQuadSize_Count;i++){
        newContext->m_pPatternTables[i] = ggHashTable_Create(ggeHashingAlgorithm_Default);
    }
    return newContext;
}

char *
hash_game_of_life(char *inBoard, char *outboard,
                  const int nrows,
                  const int ncols,
                  const int gens_max)
{
    // Allocate a new board and assigned corresponding properties
    ggBoard* newBoard = ggAlloc(ggBoard);
    newBoard->m_pBoard = inBoard;
    newBoard->m_iNumOfCol = ncols;
    newBoard->m_iNumOfRow = nrows;

    ggQuadTreeNode* currentTree = ggCreateTreeFromBoard(newBoard);
    ggHashLifeContext* context = ggCreateContext();

    int current_iteration = 0;
    int current_stride = nrows/4;
    while(current_iteration!=gens_max){
        while((gens_max-current_iteration)>current_stride){
            current_stride = current_stride/2;
        }
        currentTree = ggEvolveTree(currentTree,current_stride,context);
        current_iteration+=current_stride;
    }
    ggConvertTreeToBoard(currentTree,newBoard,context);
    ggFreeContext(context);
    char* ret = newBoard->m_pBoard;
    free(newBoard);
    return ret;
}