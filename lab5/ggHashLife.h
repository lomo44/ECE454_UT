#ifndef _ggHashLife_h
#define _ggHashLife_h

#include "ggTypes.h"
#include "ggHashTable.h"
#include "ggQuadTree.h"

// Context structure for HL
typedef struct __ggHashLifeContext{
    ggHashTable* m_pPatternTables[eQuadSize_Count];
    ggQuadTreeNode* m_pLeafNodes;
} ggHashLifeContext;


ggHashLifeContext* 	ggCreateContext();
void 				ggFreeContext(ggHashLifeContext* in_pContext);

ggQuadTreeNode*		ggCreateWrappedNode(ggQuadTreeNode* in_pNode);
ggQuadTreeNode* 	ggCreateTreeFromBoard(ggBoard* in_pBoard);
ggQuadTreeNode* 	ggEvolveTree(ggQuadTreeNode* in_pTreeNode, int m_iIteration, ggHashLifeContext* in_pContext);
void 				ggConvertTreeToBoard(ggQuadTreeNode* in_pTreeNode, ggBoard* in_pBoard,  ggHashLifeContext* in_pContext);

char*
hash_game_of_life (char* inBoard, char* outboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max);

#endif