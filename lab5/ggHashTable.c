#include "ggHashTable.h"
#include <stdlib.h>

/*
 *Function: ggHashTable_Create
 * ----------------------------
 *  Initialize a hash function
 *
 *  in_eAlgorithm: a enum define the type of hashfunction
 *
 *  returns: the pointer to a hashtable
 */
ggHashTable*    ggHashTable_Create(ggeHashingAlgorithm in_eAlgorithm){
    ggHashTable* hash_output = (ggHashTable*) malloc (sizeof(ggHashTable));
    hash_output->m_eAlgorithm = ggeHashingAlgorithm_Default;
    return hash_output;
}
/*
 *Function: ggHashTable_Free
 * ----------------------------
 *  Free the hash table 
 *
 *  in_pHashTable: pointer to the hashtable
 */
void ggHashTable_Free(ggHashTable* in_pHashTable){
    free (in_pHashTable);
    return;
}
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
                                    ggQuadTreeNode* m_pNodeBR) {
    //generate hashtag by cobime digit 2-17 of the child node adress;
    ggHashKey m_pKey = ggKey((ggHashKey)m_pNodeTL,(ggHashKey)m_pNodeTR,(ggHashKey)m_pNodeBL,(ggHashKey)m_pNodeBR);
    int index = ggHash(m_pKey);
    ggHashTableEntry* node_output = in_pHashTable->m_pEntries[index];
    //search the hashed entry in hash table 
    while (node_output!= NULL){
        if (node_output->m_pkey == m_pKey){
            if (ggHashTable_CmpNode (node_output->m_pPayload,m_pNodeTL, m_pNodeTR, m_pNodeBL, m_pNodeBR)){
                return node_output->m_pPayload;
            }         
        }
        node_output = node_output->m_pNextEntry;
    }
    return NULL;
}
/*
 *Function: ggHashTable_Insert
 * ----------------------------
 *  Find a node store inside hashtable 
 *
 *  in_pHashTable: pointer to the hashtable
 *  m_pPayLoad: pointer to the data ndes
 *  
 */
void ggHashTable_Insert(ggHashTable* in_pHashTable, ggQuadTreeNode* m_pPayLoad){
    ggHashKey m_pkey = ggKey((ggHashKey)m_pPayLoad->m_pChildNodes[eQuadTreePosition_TL],
                             (ggHashKey)m_pPayLoad->m_pChildNodes[eQuadTreePosition_TR],
                             (ggHashKey)m_pPayLoad->m_pChildNodes[eQuadTreePosition_BL],
                             (ggHashKey)m_pPayLoad->m_pChildNodes[eQuadTreePosition_BR]);
    int index = ggHash(m_pkey);
    ggHashTableEntry* node_temp = (ggHashTableEntry*) malloc (sizeof(ggHashTableEntry));
    node_temp->m_pkey = m_pkey;
    node_temp->m_pPayload = m_pPayLoad;
    node_temp->m_pNextEntry = in_pHashTable->m_pEntries[index];
    in_pHashTable->m_pEntries[index] = node_temp;
    return;
}

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
 int ggHashTable_CmpNode (  ggQuadTreeNode* m_pNodeP,                                    
                            ggQuadTreeNode* m_pNodeTL,
                            ggQuadTreeNode* m_pNodeTR,
                            ggQuadTreeNode* m_pNodeBL,
                            ggQuadTreeNode* m_pNodeBR){
    if ((m_pNodeP->m_pChildNodes[eQuadTreePosition_TL] == m_pNodeTL) &&
        (m_pNodeP->m_pChildNodes[eQuadTreePosition_TR] == m_pNodeTR) &&
        (m_pNodeP->m_pChildNodes[eQuadTreePosition_BL] == m_pNodeBL) &&
        (m_pNodeP->m_pChildNodes[eQuadTreePosition_BR] == m_pNodeBR)) {
        
            return 1;
    }     
    return 0;
}

ggQuadTreeNode*            ggHashTable_CreateAndInsert(ggHashTable* in_pHashTable, ggeQuadSize in_eSize, 
                                            ggQuadTreeNode* in_pTL,
                                            ggQuadTreeNode* in_pTR,
                                            ggQuadTreeNode* in_pBL,
                                            ggQuadTreeNode* in_pBR){
    
    ggQuadTreeNode* m1 = ggAlloc(ggQuadTreeNode);
    ggQuadTreeNode_Init(m1,in_eSize,in_pTL,in_pTR,in_pBL,in_pBR);
    ggHashTable_Insert(in_pHashTable,m1);
    return m1;
}