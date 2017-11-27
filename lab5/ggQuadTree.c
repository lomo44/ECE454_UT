#include "ggQuadTree.h"
#include <stdio.h>

// ggQuadTreeNode* ggCreateQuadTreeFromBoard(ggBoard* in_pBoard, ggPosition in_pTopLeft, int in_iSize){
//     // if(in_iSize >= GG_MINIMUM_NODE_DIMENSION){
//     //     // Create sub tree;
//     //     int i;
//     //     int newsize = in_iSize>>1;
        
//     //     int new_x = in_pTopLeft.m_iX+newsize;
//     //     int new_y = in_pTopLeft.m_iY+newsize;
//     //     ggPosition nw_tl = in_pTopLeft;
//     //     ggPosition ne_tl = in_pTopLeft;
//     //     ne_tl.m_iX = new_x;
//     //     ggPosition sw_tl = in_pTopLeft;
//     //     sw_tl.m_iY = new_y;
//     //     ggPosition se_tl = in_pTopLeft;
//     //     se_tl.m_iX = new_x;
//     //     se_tl.m_iY = new_y;
        
//     //     ggQuadTreeNode* nw = ggCreateQuadTreeFromBoard(in_pBoard,nw_tl,newsize);
//     //     ggQuadTreeNode* ne = ggCreateQuadTreeFromBoard(in_pBoard,ne_tl,newsize);
//     //     ggQuadTreeNode* sw = ggCreateQuadTreeFromBoard(in_pBoard,sw_tl,newsize);
//     //     ggQuadTreeNode* se = ggCreateQuadTreeFromBoard(in_pBoard,se_tl,newsize);
//     //     ggQuadTreeNode* newNode = ggAlloc(ggQuadTreeNode);
//     //     newNode->m_NodePosition = eQuadTreePosition_Root;
//     //     newNode->m_pChildNodes[eQuadTreePosition_NW] = nw;
//     //     newNode->m_pChildNodes[eQuadTreePosition_NE] = ne;
//     //     newNode->m_pChildNodes[eQuadTreePosition_SW] = sw;
//     //     newNode->m_pChildNodes[eQuadTreePosition_SE] = se;
//     //     else{

//     //     }
//     // }
//     // else{
//     //     // leaf node, create normal node

    // }
//     return NULL;
// }
/*
 *Function: ggHashTable_two_Create
 * ----------------------------
 *  Initialize a array to store leaf nodes for 2x2 blocks with 16 entries
 *
 *  returns: the pointer to array
 */
ggQuadTreeNode* ggQuadTree_InitLeaf(){
    ggQuadTreeNode* array_output = (ggQuadTreeNode*) malloc (sizeof(ggQuadTreeNode)*GG_LEAF_NODE_COUNT);
    for (unsigned long long  i = 0; i < 16; i++){
        array_output[i].m_eSize = eQuadSize_2;
        array_output[i].m_pChildNodes[eQuadTreePosition_TL] = (ggQuadTreeNode*)((i >> 3) & 0b1);
        array_output[i].m_pChildNodes[eQuadTreePosition_TR] = (ggQuadTreeNode*)((i >> 2) & 0b1);
        array_output[i].m_pChildNodes[eQuadTreePosition_BL] = (ggQuadTreeNode*)((i >> 1) & 0b1);
        array_output[i].m_pChildNodes[eQuadTreePosition_BR] = (ggQuadTreeNode*)( i & 0b1);
    }
    return array_output;
}
ggQuadTreeNode* ggQuadTree_GetLeaf(ggQuadTreeNode* in_pLeafNodes,int in_iTL,int in_iTR,int in_iBL,int in_iBR){
    int index = ((in_iTL & 0b1) << 3) + ((in_iTR & 0b1) << 2) + ((in_iBL & 0b1) << 1) + ((in_iBR & 0b1));
    return &in_pLeafNodes[index]; 
}


