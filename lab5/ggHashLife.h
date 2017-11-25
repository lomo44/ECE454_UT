#ifndef _ggHashLife_h
#define _ggHashLife_h

#include "ggTypes.h"
#include "ggHashTable.h"
#include "ggQuadTree.h"



ggQuadTreeNode* ggCreateTreeFromBoard(ggBoard in_pBoard);
ggQuadTreeNode* ggEvolveTree(ggQuadTreeNode* in_pTreeNode, int m_iIteration);
void ggConvertTreeToBoard(ggQuadTreeNode* in_pTreeNode, ggBoard in_pBoard);


#endif