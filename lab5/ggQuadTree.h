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
    struct __ggQuadTreeNode* m_pChildNodes[eQuadTreePosition_Count];
} ggQuadTreeNode;

/*
 *Function: ggQuadTree_InitLeaf
 * ----------------------------
 *  Initialize a array to store leaf nodes for 2x2 blocks with 16 entries
 *
 *  returns: the pointer to array
 */
ggQuadTreeNode* ggQuadTree_InitLeaf();


ggQuadTreeNode* ggQuadTree_GetLeaf(ggQuadTreeNode* in_pLeafNodes,int in_iTL,int in_iTR,int in_iBL,int in_iBR);
// // Some tree node utilities function
// ggQuadTreeNode* ggQuadTree_Get

#endif