#include "ggHashLife.h"

ggQuadTreeNode* ggEvolveTree(ggQuadTreeNode* in_pTreeNode, int m_iIteration, ggHashLifeContext* in_pContext){
    return NULL;
}

ggQuadTreeNode* ggGetNode(ggeQuadSize in_eSize, ggQuadTreeNode* in_pTL,
                                                ggQuadTreeNode* in_pTR,
                                                ggQuadTreeNode* in_pBL,
                                                ggQuadTreeNode* in_pBR, ggHashLifeContext* in_pContext){
    ggQuadTreeNode* m1 = ggHashTable_Search(in_pContext->m_pPatternTables[in_eSize],in_pTL,in_pTR,in_pBL,in_pBR);
    if(m1==NULL){
        m1 = ggHashTable_CreateAndInsert(in_pContext->m_pPatternTables[m1->m_eSize],in_eSize,in_pTL,in_pTR,in_pBL,in_pBR);
    }
    return m1;
}

void ggGenerateResultNode(ggQuadTreeNode* in_pTreeNode, ggHashLifeContext* in_pContext){
    /**
     * ABCD
     * EFGH
     * IJKL
     * MNOP
     * */
    // Grabing the corresponding quadtree nodes
    ggeQuadSize result_size = in_pTreeNode->m_eSize-1;
    if(in_pTreeNode->m_eSize!=eQuadSize_4){
        //TODO: implement
    }
    else{
        ggQuadTreeNode* A = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TL]->m_pChildNodes[eQuadTreePosition_TL];
        ggQuadTreeNode* B = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TL]->m_pChildNodes[eQuadTreePosition_TR];
        ggQuadTreeNode* C = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TL]->m_pChildNodes[eQuadTreePosition_BL];
        ggQuadTreeNode* D = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TL]->m_pChildNodes[eQuadTreePosition_BR];
        ggQuadTreeNode* E = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_TL];
        ggQuadTreeNode* F = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_TR];
        ggQuadTreeNode* G = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_BL];
        ggQuadTreeNode* H = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_BR];
        ggQuadTreeNode* I = in_pTreeNode->m_pChildNodes[eQuadTreePosition_BL]->m_pChildNodes[eQuadTreePosition_TL];
        ggQuadTreeNode* J = in_pTreeNode->m_pChildNodes[eQuadTreePosition_BL]->m_pChildNodes[eQuadTreePosition_TR];
        ggQuadTreeNode* K = in_pTreeNode->m_pChildNodes[eQuadTreePosition_BL]->m_pChildNodes[eQuadTreePosition_BL];
        ggQuadTreeNode* L = in_pTreeNode->m_pChildNodes[eQuadTreePosition_BL]->m_pChildNodes[eQuadTreePosition_BR];
        ggQuadTreeNode* M = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_TL];
        ggQuadTreeNode* N = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_TR];
        ggQuadTreeNode* O = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_BL];
        ggQuadTreeNode* P = in_pTreeNode->m_pChildNodes[eQuadTreePosition_TR]->m_pChildNodes[eQuadTreePosition_BR];
        
        ggQuadTreeNode* m1 = ggGetNode(result_size,A,B,E,F,in_pContext);
        if(m1->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m1,in_pContext);
        }
        ggQuadTreeNode* m2 = ggGetNode(result_size,B,C,F,G,in_pContext);
        if(m2->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m2,in_pContext);
        }
        ggQuadTreeNode* m3 = ggGetNode(result_size,C,D,G,H,in_pContext);
        if(m3->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m3,in_pContext);
        }
        ggQuadTreeNode* m4 = ggGetNode(result_size,E,F,I,J,in_pContext);
        if(m4->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m4,in_pContext);
        }
        ggQuadTreeNode* m5 = ggGetNode(result_size,F,G,J,K,in_pContext);
        if(m5->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m5,in_pContext);
        }
        ggQuadTreeNode* m6 = ggGetNode(result_size,G,H,K,L,in_pContext);
        if(m6->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m6,in_pContext);
        }
        ggQuadTreeNode* m7 = ggGetNode(result_size,I,J,M,N,in_pContext);
        if(m7->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m7,in_pContext);
        }
        ggQuadTreeNode* m8 = ggGetNode(result_size,J,K,N,O,in_pContext);
        if(m8->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m8,in_pContext);
        }
        ggQuadTreeNode* m9 = ggGetNode(result_size,K,L,O,P,in_pContext);
        if(m9->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(m9,in_pContext);
        }
        /**
         * m1m2m3
         * m4m5m6
         * m7m8m9
         * */
        // Generate final node
        ggQuadTreeNode* f1 = ggGetNode(result_size,m1,m2,m4,m5,in_pContext);
        if(f1->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(f1,in_pContext);
        }
        ggQuadTreeNode* f2 = ggGetNode(result_size,m2,m3,m5,m6,in_pContext);
        if(f2->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(f2,in_pContext);
        }
        ggQuadTreeNode* f3 = ggGetNode(result_size,m4,m5,m7,m8,in_pContext);
        if(f3->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(f3,in_pContext);
        }
        ggQuadTreeNode* f4 = ggGetNode(result_size,m5,m6,m8,m9,in_pContext);
        if(f4->m_pChildNodes[eQuadTreePosition_Result]==NULL){
            ggGenerateResultNode(f4,in_pContext);
        }
        ggQuadTreeNode* final = ggGetNode(result_size,f1,f2,f3,f4,in_pContext);
        in_pTreeNode->m_pChildNodes[eQuadTreePosition_Result] = final;
    }
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

ggQuadTreeNode* ggCreateWrappedNode(ggQuadTreeNode* in_pNode, ggHashLifeContext* in_pContext){
    // Create a wrapped tree node
    /**
     * DCDC
     * BABA
     * DCDC
     * BABA                 
     * */
    ggQuadTreeNode* A = in_pNode->m_pChildNodes[eQuadTreePosition_TL];
    ggQuadTreeNode* B = in_pNode->m_pChildNodes[eQuadTreePosition_TR];
    ggQuadTreeNode* C = in_pNode->m_pChildNodes[eQuadTreePosition_BL];
    ggQuadTreeNode* D = in_pNode->m_pChildNodes[eQuadTreePosition_BR];

    ggQuadTreeNode* res = ggHashTable_Search(in_pContext->m_pPatternTables[in_pNode->m_eSize],D,C,B,A);
    if(res==NULL){
        res = ggAlloc(ggQuadTreeNode);
        ggQuadTreeNode_Init(res,in_pNode->m_eSize,D,C,B,A);
        ggHashTable_Insert(in_pContext->m_pPatternTables[in_pNode->m_eSize],res);
    }
    ggQuadTreeNode* parentNode = ggAlloc(ggQuadTreeNode);
    ggQuadTreeNode_Init(parentNode,in_pNode->m_eSize+1,res,res,res,res);
    ggHashTable_Insert(in_pContext->m_pPatternTables[parentNode->m_eSize],res);
    return parentNode;
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