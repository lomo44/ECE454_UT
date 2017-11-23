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

//     // }
// }