#ifndef _ggHashTable_h
#define _ggHashTable_h

/** Function definition for hash tables
 * Currently target implementation is hashing with chaining
 * Future improvement:
 * 1. Multiple Hashing algorithm
 * 2. Double hashing
 * 3. Multi-threading/ threadsafe hashing
 **/

typedef enum __ggeHashingAlgorithm{
    ggeHashingAlgorithm_Default,
    ggeHashingAlgorithm_DoubleHashing
} ggeHashingAlgorithm;

typedef struct __ggHashTableEntry{
    char* m_pkey;
    void* m_pPayload;
    struct __ggHashTableEntry* m_pNextEntry;
} ggHashTableEntry;

typedef struct __ggHashTable {
    int m_iSize;
    ggHashTableEntry* m_pEntries;
    ggeHashingAlgorithm m_eAlgorithm;
} ggHashTable;

//TODO: Implement
ggHashTable*    ggHashTable_Create(int m_iSize, ggeHashingAlgorithm in_eAlgorithm);
//TODO: Implement
void            ggHashTable_Free(ggHashTable* in_pHashTable);
//TODO: Implement
void*           ggHashTable_Search(ggHashTable* in_pHashTable, char* m_pKey);
//TODO: Implement
void            ggHashTable_Insert(ggHashTable* in_pHashTable, char* m_pKey, void* m_pPayLoad);

#endif