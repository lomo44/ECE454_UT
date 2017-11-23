#ifndef _ggQuadTree_h
#define _ggQuadTree_h
#include "ggTypes.h"

#define GG_MINIMUM_NODE_DIMENSION 2

typedef enum __ggeQuadTreePosition{
    eQuadTreePosition_NW,
    eQuadTreePosition_NE,
    eQuadTreePosition_SW,
    eQuadTreePosition_SE,
    eQuadTreePosition_Count,
    eQuadTreePosition_Root
} ggeQuadTreePosition;

typedef struct __ggQuadTreeNode{
    ggeQuadTreePosition m_NodePosition;
    struct __ggQuadTreeNode* m_pChildNodes[eQuadTreePosition_Count];
} ggQuadTreeNode;

//TODO: Implement this
//ggQuadTreeNode* ggCreateQuadTreeFromBoard(ggBoard* in_pBoard, ggPosition in_pTopLeft, int in_iSize);


#endif