#include "ggHashLife.h"

ggQuadTreeNode* ggEvolveTree(ggQuadTreeNode* in_pTreeNode, int m_iIteration, ggHashLifeContext* in_pContext){
    return NULL;
}
void ggConvertTreeToBoard(ggQuadTreeNode* in_pTreeNode, ggBoard* in_pBoard,  ggHashLifeContext* in_pContext){
    return;
}

ggQuadTreeNode* ggCreateNode(ggBoard* in_pBoard, ggeQuadSize in_eSize, ggPosition in_pTLPosition, ggHashLifeContext* in_pContext){
    if(in_eSize==eQuadSize_2){
        // Almost base node type, need to scan the board
        int TL = in_pBoard->m_pBoard[in_pTLPosition.m_iY*in_pBoard->m_iNumOfRow+in_pTLPosition.m_iX];
        int TR = in_pBoard->m_pBoard[in_pTLPosition.m_iY*in_pBoard->m_iNumOfRow+in_pTLPosition.m_iX+1]; 
        int BL = in_pBoard->m_pBoard[(in_pTLPosition.m_iY+1)*in_pBoard->m_iNumOfRow+in_pTLPosition.m_iX]; 
        int BR = in_pBoard->m_pBoard[(in_pTLPosition.m_iY+1)*in_pBoard->m_iNumOfRow+in_pTLPosition.m_iX+1]; 
        return ggQuadTree_GetLeaf(in_pContext->m_pLeafNodes,TL,TR,BL,BR);
    }
    else{
        int sub_block_size = ggGetSizeFromQuadSize(in_eSize) >> 1; 
        ggPosition TR = in_pTLPosition;
        TR.m_iX += sub_block_size;
        ggPosition BL = in_pTLPosition;
        BL.m_iY += sub_block_size;
        ggPosition BR = BL;
        BR.m_iX += sub_block_size;
        // Getting the childs of the node
        ggQuadTreeNode* node_TL = ggCreateNode(in_pBoard, in_eSize-1,in_pTLPosition, in_pContext);
        ggQuadTreeNode* node_TR = ggCreateNode(in_pBoard, in_eSize-1,TR,in_pContext);
        ggQuadTreeNode* node_BL = ggCreateNode(in_pBoard, in_eSize-1,BL,in_pContext);
        ggQuadTreeNode* node_BR = ggCreateNode(in_pBoard, in_eSize-1,BR,in_pContext);
        // Check the hash table node and see if it exist
        ggQuadTreeNode* existingNode = ggHashTable_Search(in_pContext->m_pPatternTables[in_eSize],node_TL,node_TR,node_BL,node_BR);
        if(existingNode!=NULL){
            // Node exists, dont need to recreate
            return existingNode;
        }
        else{
            // Node does not exists, need to create one and insert it into the hash table
            ggQuadTreeNode* newNode = ggAlloc(ggQuadTreeNode);
            newNode->m_eSize = in_eSize;
            newNode->m_pChildNodes[eQuadTreePosition_TL] = node_TL;
            newNode->m_pChildNodes[eQuadTreePosition_TR] = node_TR;
            newNode->m_pChildNodes[eQuadTreePosition_BL] = node_BL;
            newNode->m_pChildNodes[eQuadTreePosition_BR] = node_BR;
            // Since the future results are unknown, we left it blank
            newNode->m_pChildNodes[eQuadTreePosition_Result] = NULL;
            // Insert it into the hashtable for future usage
            ggHashTable_Insert(in_pContext->m_pPatternTables[in_eSize],newNode);
            return newNode;
        }
    }
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
    ggHashLifeContext* context = ggCreateContext();
    ggPosition origin;
    origin.m_iX = 0;
    origin.m_iY = 0;
    ggQuadTreeNode* currentTree = ggCreateNode(newBoard,ggGetQuadSizeFromSize(ncols), origin, context);
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