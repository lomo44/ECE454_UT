#ifndef _ggQuadTree_h
#define _ggQuadTree_h
#include "ggTypes.h"

#define GG_MINIMUM_NODE_DIMENSION 2
#define GG_LEAF_NODE_COUNT 16

typedef enum __ggeQuadTreePosition{
    eQuadTreePosition_TL,
    eQuadTreePosition_TR,
    eQuadTreePosition_BL,
    eQuadTreePosition_BR,
    eQuadTreePosition_Result,
    eQuadTreePosition_Count,
    eQuadTreePosition_Root
} ggeQuadTreePosition;

typedef struct __ggQuadTreeNode{
    ggeQuadSize m_eSize;
    ggeQuadTreePosition m_NodePosition;
    struct __ggQuadTreeNode* m_pChildNodes[eQuadTreePosition_Count];
} ggQuadTreeNode;

//TODO: Implement this
//ggQuadTreeNode* ggCreateQuadTreeFromBoard(ggBoard* in_pBoard, ggPosition in_pTopLeft, int in_iSize);
/*
 *Function: ggQuadTree_InitLeaf
 * ----------------------------
 *  Initialize a array to store leaf nodes for 2x2 blocks with 16 entries
 *
 *  returns: the pointer to array
 */
ggQuadTreeNode* ggQuadTree_InitLeaf();

#endif