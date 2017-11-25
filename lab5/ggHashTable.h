#ifndef _ggHashTable_h
#define _ggHashTable_h

#include "ggQuadTree.h"
/** Function definition for hash tables
 * Currently target implementation is hashing with chaining
 * Future improvement:
 * 1. Multiple Hashing algorithm
 * 2. Double hashing
 * 3. Multi-threading/ threadsafe hashing
 **/
#define HASHTABLE_SIZE 4096
#define ggHash(__key) (((__key)/37)%HASHTABLE_SIZE) 
#define ggShift(__x,__shift) (((__x>>2)&0xFFFF)<<__shift)
#define ggKey(__TL,__TR,__BL,__BR) ((ggShift(__TL,48))|(ggShift(__TR,32))|(ggShift(__BL,16))|(ggShift(__BR,0))) 

typedef unsigned long long ggHashKey;

typedef enum __ggeHashingAlgorithm{
    ggeHashingAlgorithm_Default,
    ggeHashingAlgorithm_DoubleHashing
} ggeHashingAlgorithm;

typedef struct __ggHashTableEntry{
    ggHashKey m_pkey;
    ggQuadTreeNode* m_pPayload;
    struct __ggHashTableEntry* m_pNextEntry;
} ggHashTableEntry;

typedef struct __ggHashTable {
    ggHashTableEntry* m_pEntries[HASHTABLE_SIZE];
    ggeHashingAlgorithm m_eAlgorithm;
} ggHashTable;
/*
 *Function: ggHashTable_Create
 * ----------------------------
 *  Initialize a hash function
 *
 *  in_eAlgorithm: a enum define the type of hashfunction
 *
 *  returns: the pointer to a hashtable
 */
ggHashTable*    ggHashTable_Create(ggeHashingAlgorithm in_eAlgorithm);
/*
 *Function: ggHashTable_Search
 * ----------------------------
 *  Find a node store inside hashtable 
 *
 *  in_pHashTable: pointer to the hashtable
 *  m_pNodeTL: the top-left child ndes
 *  m_pNodeTR: the top-right child ndes
 *  m_pNodeBL: the bottom-left child ndes
 *  m_pNodeBR: the bottom-right child ndes
 *  
 *  returns: the pointer to the node
 */
ggQuadTreeNode* ggHashTable_Search( ggHashTable* in_pHashTable,
                                    ggQuadTreeNode* m_pNodeTL,
                                    ggQuadTreeNode* m_pNodeTR,
                                    ggQuadTreeNode* m_pNodeBL,
                                    ggQuadTreeNode* m_pNodeBR);
/*
 *Function: ggHashTable_Insert
 * ----------------------------
 *  Find a node store inside hashtable 
 *
 *  in_pHashTable: pointer to the hashtable
 *  m_pPayLoad: pointer to the data ndes
 *  
 */                                  
void            ggHashTable_Insert(ggHashTable* in_pHashTable, ggQuadTreeNode* m_pPayLoad);
/*
 *Function: ggHashTable_Free
 * ----------------------------
 *  Free the hash table 
 *
 *  in_pHashTable: pointer to the hashtable
 */
void            ggHashTable_Free(ggHashTable* in_pHashTable);
/*
 *Helping Function: ggHashTable_CmpNode
 * ----------------------------
 *  check if the parent node contains four child node 
 *
 *  m_pNodeTL: the top-left child ndes
 *  m_pNodeTR: the top-right child ndes
 *  m_pNodeBL: the bottom-left child ndes
 *  m_pNodeBR: the bottom-right child ndes
 *  
 *  Return: return 1 if parent node contain four child nodes
 */   
 int            ggHashTable_CmpNode(    ggQuadTreeNode* m_pNodeP,                                    
                                        ggQuadTreeNode* m_pNodeTL,
                                        ggQuadTreeNode* m_pNodeTR,
                                        ggQuadTreeNode* m_pNodeBL,
                                        ggQuadTreeNode* m_pNodeBR);

#endif