#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

#define ENABLE_SIMD 0
#define SPEED_UP 1

#define TILE_SIZE 1000
#define PIXEL_SIZE 3
#define PIXEL_DATA_TYPE unsigned char
#define tmAlloc(type,size) (type*)malloc(sizeof(type)*size)

#define MATRIX_00 0
#define MATRIX_01 1
#define MATRIX_02 2
#define MATRIX_10 4
#define MATRIX_11 5
#define MATRIX_12 6
#define MATRIX_20 8
#define MATRIX_21 9
#define MATRIX_22 10
#define MATRIX_INDEX_TRANSFORM_X MATRIX_02
#define MATRIX_INDEX_TRANSFORM_Y MATRIX_12
#define VECTOR_X 0
#define VECTOR_Y 1
#define VECTOR_Z 2

///////////// Matrix Operation ////////////////////

typedef int tmMat4i;
typedef int tmVec4i;
typedef struct _tmIndexMap{
    int m_iWidth;
    int m_iLength;
    int* m_pArray;
} tmIndexMap;
typedef enum _tmMatrixType{
    eMatrixType_Identity,
    eMatrixType_CCW90,
    eMatrixType_CW90,
    eMatrixType_R180,
    eMatrixType_MirroX,
    eMatrixType_MirroY
} tmMatFlag;
typedef enum _tmVertexType{
    etop_left,
    etop_right,
    ebot_left,
    ebot_right,
} tmVertexType;

tmVec4i* gTempVec1 = NULL;
tmVec4i* gTempVec2 = NULL;
int      gVertex   = etop_left;

tmMat4i* gTempMul = NULL;
tmMat4i* gGlobalCW = NULL;
tmMat4i* gGlobalCCW = NULL;
tmMat4i* gGlobalMirrorYMatrix = NULL;
tmMat4i* gGlobalMirrorXMatrix = NULL;
tmMat4i* gGlobalR180 = NULL;
tmMat4i* gGlobalTransform = NULL;

void        tmCopyMat(tmMat4i* in_pA, tmMat4i* in_pB){
    memcpy(in_pA, in_pB,sizeof(int)*16);
}
void        tmMatMulVec(tmMat4i* in_pA, tmVec4i* in_pB, tmVec4i* io_pC){
#if ENABLE_SIMD
    
#else
    io_pC[VECTOR_X] = in_pA[MATRIX_00]*in_pB[VECTOR_X] + in_pA[MATRIX_01]*in_pB[VECTOR_Y] + in_pA[MATRIX_02]*in_pB[VECTOR_Z];
    io_pC[VECTOR_Y] = in_pA[MATRIX_10]*in_pB[VECTOR_X] + in_pA[MATRIX_11]*in_pB[VECTOR_Y] + in_pA[MATRIX_12]*in_pB[VECTOR_Z];
    io_pC[VECTOR_Z] = in_pA[MATRIX_20]*in_pB[VECTOR_X] + in_pA[MATRIX_21]*in_pB[VECTOR_Y] + in_pA[MATRIX_22]*in_pB[VECTOR_Z];
#endif
}
void        tmMatMulMat(tmMat4i* in_pA, tmMat4i* in_pB, tmMat4i* io_pC){
#if ENABLE_SIMD
    
#else
    io_pC[MATRIX_00] = in_pA[MATRIX_00]*in_pB[MATRIX_00] + in_pA[MATRIX_01]*in_pB[MATRIX_10] + in_pA[MATRIX_02]*in_pB[MATRIX_20];
    io_pC[MATRIX_01] = in_pA[MATRIX_00]*in_pB[MATRIX_01] + in_pA[MATRIX_01]*in_pB[MATRIX_11] + in_pA[MATRIX_02]*in_pB[MATRIX_21];
    io_pC[MATRIX_02] = in_pA[MATRIX_00]*in_pB[MATRIX_02] + in_pA[MATRIX_01]*in_pB[MATRIX_11] + in_pA[MATRIX_02]*in_pB[MATRIX_22];
    io_pC[MATRIX_10] = in_pA[MATRIX_10]*in_pB[MATRIX_00] + in_pA[MATRIX_11]*in_pB[MATRIX_10] + in_pA[MATRIX_12]*in_pB[MATRIX_20];
    io_pC[MATRIX_11] = in_pA[MATRIX_10]*in_pB[MATRIX_01] + in_pA[MATRIX_11]*in_pB[MATRIX_11] + in_pA[MATRIX_12]*in_pB[MATRIX_21];
    io_pC[MATRIX_12] = in_pA[MATRIX_10]*in_pB[MATRIX_02] + in_pA[MATRIX_11]*in_pB[MATRIX_11] + in_pA[MATRIX_12]*in_pB[MATRIX_22];
    io_pC[MATRIX_20] = in_pA[MATRIX_20]*in_pB[MATRIX_00] + in_pA[MATRIX_21]*in_pB[MATRIX_10] + in_pA[MATRIX_22]*in_pB[MATRIX_20];
    io_pC[MATRIX_21] = in_pA[MATRIX_20]*in_pB[MATRIX_01] + in_pA[MATRIX_21]*in_pB[MATRIX_11] + in_pA[MATRIX_22]*in_pB[MATRIX_21];
    io_pC[MATRIX_22] = in_pA[MATRIX_20]*in_pB[MATRIX_02] + in_pA[MATRIX_21]*in_pB[MATRIX_11] + in_pA[MATRIX_22]*in_pB[MATRIX_22];
#endif
}
void        tmMatMulVecInplace(tmMat4i* in_pA, tmVec4i* io_pB){
    tmMatMulMat(in_pA, io_pB, gTempMul);
    tmCopyMat(gTempMul,io_pB);
}

void        tmLoadMat(tmMat4i* in_pA, tmMatFlag in_eFlag){
    switch(in_eFlag){
        case eMatrixType_CCW90:{
            in_pA[MATRIX_00] = 0;
            in_pA[MATRIX_01] = -1;
            in_pA[MATRIX_10] = 1;
            in_pA[MATRIX_11] = 0;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_CW90:{
            in_pA[MATRIX_00] = 0;
            in_pA[MATRIX_01] = 1;
            in_pA[MATRIX_10] = -1;
            in_pA[MATRIX_11] = 0;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_MirroX:{
            in_pA[MATRIX_00] = -1;
            in_pA[MATRIX_01] = 0;
            in_pA[MATRIX_10] = 0;
            in_pA[MATRIX_11] = 1;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_MirroY:{
            in_pA[MATRIX_00] = 1;
            in_pA[MATRIX_01] = 0;
            in_pA[MATRIX_10] = 0;
            in_pA[MATRIX_11] = -1;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_R180:{
            in_pA[MATRIX_00] = -1;
            in_pA[MATRIX_01] = 0;
            in_pA[MATRIX_10] = 0;
            in_pA[MATRIX_11] = -1;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_Identity:
        default:{
            in_pA[MATRIX_00] = 1;
            in_pA[MATRIX_01] = 0;
            in_pA[MATRIX_10] = 0;
            in_pA[MATRIX_11] = 1;
            in_pA[MATRIX_22] = 1;
            break;
        }
    }
}
tmMat4i*    tmAllocMat(tmMatFlag in_eFlag){
    tmMat4i* ret = tmAlloc(int, 16);
    tmLoadMat(ret, in_eFlag);
    return ret;
}
void        tmFreeMat(tmMat4i* in_pA){
    free(in_pA);
}
tmVec4i*    tmAllocVec(){
    tmVec4i* ret = tmAlloc(int, 4);
    ret[VECTOR_Z] = 1;
    return ret;
}
void        tmFreeVec(tmVec4i* in_pA){
    free(in_pA);
}
void        tmMatrixIndexToVec(int in_iIndex,int in_iStrideSize, tmVec4i* io_pVec){
    io_pVec[VECTOR_X] = (in_iIndex % in_iStrideSize - in_iStrideSize) * 2;
    io_pVec[VECTOR_Y] = (in_iIndex / in_iStrideSize - in_iStrideSize) * 2;
    io_pVec[VECTOR_Z] = 1;
}
int         tmVecToMatrixIndex(tmVec4i* in_pVec,int in_iStrideSize){
    return (in_pVec[VECTOR_X]/2+in_iStrideSize)+ (in_pVec[VECTOR_Y]/2+in_iStrideSize)*in_iStrideSize;
}
void        tmExtractTransform(tmMat4i* in_pA, tmMat4i* io_pB){
    io_pB[MATRIX_INDEX_TRANSFORM_X] = in_pA[MATRIX_INDEX_TRANSFORM_X];
    io_pB[MATRIX_INDEX_TRANSFORM_Y] = in_pA[MATRIX_INDEX_TRANSFORM_Y];
}

tmIndexMap* tmAllocIndexMap(int in_iWidth, int in_iLength){
    tmIndexMap* retMap = tmAlloc(tmIndexMap, 1);
    retMap->m_iWidth = in_iWidth;
    retMap->m_iLength = in_iLength;
    retMap->m_pArray = tmAlloc(int,in_iWidth*in_iLength);
}
void        tmFreeIndexMap(tmIndexMap* in_pIndexMap){
    free(in_pIndexMap->m_pArray);
    free(in_pIndexMap);
}
void        tmLoadIndexMapFromTransFormMatrix(tmMat4i* in_pMat, tmIndexMap* io_pInputMap){
    int i,j;
    int total_indexes = io_pInputMap->m_iWidth * io_pInputMap->m_iLength; 
    for (i = 0; i < total_indexes; i++){
        tmMatrixIndexToVec(i,io_pInputMap->m_iWidth, gTempVec1);
        tmMatMulVec(in_pMat,gTempVec1,gTempVec2);
        io_pInputMap->m_pArray[i] = tmVecToMatrixIndex(gTempVec2,io_pInputMap->m_iWidth);
    }
}

void       tmUpdateVertex (tmMat4i* io_pMat){
    if ((io_pMat[MATRIX_00] == 1 && io_pMat[MATRIX_11] == 1)||(io_pMat[MATRIX_01] == -1 && io_pMat[MATRIX_10] == -1))
        gVertex = etop_left;
    else if ((io_pMat[MATRIX_00] == -1 && io_pMat[MATRIX_11] == 1)||(io_pMat[MATRIX_01] == 1 && io_pMat[MATRIX_10] == -1))
        gVertex = etop_right;
    else if ((io_pMat[MATRIX_00] == 1 && io_pMat[MATRIX_11] == -1)||(io_pMat[MATRIX_01] == -1 && io_pMat[MATRIX_10] == 1))
        gVertex = ebot_right;
    else if ((io_pMat[MATRIX_00] == -1 && io_pMat[MATRIX_11] == -1)||(io_pMat[MATRIX_01] == 1 && io_pMat[MATRIX_10] == 1))
        gVertex = ebot_left;
    else
        printf("Error: Update Vertex Fail");
    
}
/////// Tiling /////////////
typedef struct _tmTile{
    unsigned char* m_pBuffer;
    unsigned char** m_pRows;
} tmTile;
typedef struct _tmTiledBuffer{
    unsigned char* m_pBuffer;
    tmTile* m_pTiles;
    tmTile** m_pTilesMap;
    int m_iValidX;
    int m_iValidY;
    size_t m_iTiledDimension;
    size_t m_iTileOffset;
    size_t m_iTilesPerRow;
    size_t m_iTilesPerCol;
} tmTiledMemory;
typedef enum _tmRotionDirectionFlag{
    tmRotionDirectionFlagCCW,
    tmRotionDirectionFlagCW
} tmRotionDirectionFlag;
typedef enum _tmMoveDirectionFlag{
    tmMoveDirectionFlagUP,
    tmMoveDirectionFlagDown,
    tmMoveDirectionFlagLeft,
    tmMoveDirectionFlagRight,
} tmMoveDirectionFlag;
typedef enum _tmMirrorDirectionFlag{
    tmMirrorDirectionX,
    tmMirrorDirectionY,
} tmMirrorDirectionFlag;
// This tile is used for intra-tile remapping
tmTile* gAuxTiles = NULL;
// This tile arrays is used for inter-tile remapping
tmTile** gAuxTiledMemory = NULL;
tmIndexMap* gInterTileIndexMap = NULL;
tmIndexMap* gIntraTileIndexMap = NULL;
tmTiledMemory* gGlobalTiledMemory = NULL;
void tmSwapTile(tmTile* io_pA, tmTile* io_pB){
    unsigned char* temp = io_pA->m_pBuffer;
    io_pA->m_pBuffer = io_pB->m_pBuffer;
    io_pB->m_pBuffer = temp;
    
    unsigned char** temp_row = io_pA->m_pRows;
    io_pA->m_pRows = io_pB->m_pRows;
    io_pB->m_pRows = temp_row;
}
void tmMapTile(tmTile* io_Tile, tmIndexMap* in_pIndexMap){
    int row,col;
    int target_index;
    for(row = 0; row < TILE_SIZE; row++){
        for(col = 0; col < TILE_SIZE; col++){
            target_index = in_pIndexMap->m_pArray[row*TILE_SIZE+col];
            memcpy(gAuxTiles->m_pBuffer+target_index*PIXEL_SIZE,io_Tile->m_pRows[row]+col*PIXEL_SIZE, PIXEL_SIZE);
        }
    }
    tmSwapTile(io_Tile,gAuxTiles);
}
void tmMakeTile(tmTile* io_pTile, unsigned char* in_pBuffer){
    io_pTile->m_pBuffer = in_pBuffer;
    io_pTile->m_pRows = tmAlloc(unsigned char*, TILE_SIZE);
    int i;
    for (int i = 0 ; i < TILE_SIZE; i++){
        io_pTile->m_pRows[i] = io_pTile->m_pBuffer + i*TILE_SIZE;
    }
}
tmTile* tmAllocTile(int in_iSize){
    tmTile* ret = tmAlloc(tmTile, 1);
    ret->m_pBuffer = tmAlloc(PIXEL_DATA_TYPE, PIXEL_SIZE * in_iSize * in_iSize);
    ret->m_pRows = tmAlloc(PIXEL_DATA_TYPE*, in_iSize);
    int i;
    for (i = 0; i < in_iSize; i++){
        ret->m_pRows[i] = ret->m_pBuffer+i*in_iSize*PIXEL_SIZE;
    }
    return ret;
}
void tmMapTiledMemory(tmTiledMemory* io_Tile, tmIndexMap* in_pIndexMap){
    int total_tiles = io_Tile->m_iTilesPerCol * io_Tile->m_iTilesPerRow;
    int i;
    for (i = 0; i < total_tiles; i++){
        gAuxTiledMemory[in_pIndexMap->m_pArray[i]] = io_Tile->m_pTilesMap[i];
    }
    tmTile** temp = gAuxTiledMemory;
    temp = io_Tile->m_pTilesMap;
    io_Tile->m_pTilesMap = temp;
}
tmTiledMemory* tmAllocTiledMemory(size_t in_iTilesPerRow, size_t in_iTilesPerCol){
    size_t in_iNumOfTile = in_iTilesPerCol * in_iTilesPerRow;
    unsigned char* fullBuffer = tmAlloc(PIXEL_DATA_TYPE,TILE_SIZE*TILE_SIZE*in_iNumOfTile);
    tmTiledMemory* returnMemory = tmAlloc(tmTiledMemory,1);
    returnMemory->m_iTileOffset = PIXEL_SIZE * TILE_SIZE * TILE_SIZE;
    returnMemory->m_iTiledDimension = TILE_SIZE;
    returnMemory->m_pBuffer = fullBuffer;
    
    returnMemory->m_pTiles = tmAlloc(tmTile, in_iNumOfTile);
    returnMemory->m_pTilesMap = tmAlloc(tmTile*, in_iNumOfTile);
    returnMemory->m_iTilesPerCol = in_iTilesPerCol;
    returnMemory->m_iTilesPerRow = in_iTilesPerRow;
    // Allocate all the tiles
    int i,j;
    for (i = 0; i < in_iNumOfTile; i++){
        tmMakeTile(returnMemory->m_pTiles+i,fullBuffer+i*returnMemory->m_iTileOffset);
        returnMemory->m_pTilesMap[i] = returnMemory->m_pTiles+i;
    }
    return returnMemory;
}
void tmFreeTiledMemory(tmTiledMemory* in_pTiledMemory){
    free(in_pTiledMemory->m_pBuffer);
    free(in_pTiledMemory->m_pTiles);
    free(in_pTiledMemory->m_pTilesMap);
    free(in_pTiledMemory);
}
tmTiledMemory* tmAllocTiledMemoryFromFrame(unsigned char* in_pBuffer, int in_iSize){
    int num_of_tiles = in_iSize/TILE_SIZE + 1;
    tmTiledMemory* retMemory = tmAllocTiledMemory(num_of_tiles,num_of_tiles);
    int row,tiled_memory_size;
    tiled_memory_size = retMemory->m_iTileOffset;
    for (row = 0; row < in_iSize; row++){
        memcpy(retMemory->m_pBuffer+row*tiled_memory_size, in_pBuffer+row*in_iSize, in_iSize);
    }
}
void tmTiledMemoryToFrame(unsigned char* io_pBuffer, int in_iSize, tmTiledMemory* in_pTiled){
    int size = in_pTiled->m_iValidX;
    
    int x_offset;
    int y_offset;
    if (gVertex == etop_left || gVertex == etop_right) 
        x_offset == 0;
    else
        x_offset == TILE_SIZE - size % TILE_SIZE;
    
    if (gVertex == etop_left || gVertex == etop_left) 
        y_offset == 0;
    else
        y_offset == TILE_SIZE - size % TILE_SIZE;
    
    int tile_per_row;
    tile_per_row = in_pTiled->m_iTilesPerRow;

    int row_count;
    int col_count;
    int remain_row;
    int remain_col;
    int copied_row;
    int copied_col;
    int row_to_copy;
    int col_to_copy;
    int tile_y_offset;
    int tile_x_offset;
    int in_tile_row_count;
    int tile_index;
    
    while (remain_row > 0){
            remain_col = size;
            if ( y_offset != 0 && row_count ==0) {
                    row_to_copy = TILE_SIZE - y_offset;
                    tile_y_offset = y_offset;
            } else if (remain_row < TILE_SIZE) {
                    row_to_copy = remain_row;
                    tile_y_offset = 0;
            } else {
                    row_to_copy = TILE_SIZE;	
                    tile_y_offset = 0;
            }
            while (remain_col > 0){
                    copied_row = size - remain_row;
                    copied_col = size - remain_col;
                    tile_index = tile_per_row*row_count+col_count;
                    for (in_tile_row_count = 0; in_tile_row_count < row_to_copy ; in_tile_row_count ++){
                            if (x_offset != 0 && col_count == 0) {
                                    col_to_copy = TILE_SIZE - x_offset;
                                    tile_x_offset = x_offset;
                            } else if (remain_col <TILE_SIZE){
                                    col_to_copy = remain_col;
                                    tile_x_offset = 0;
                            } else {
                                    col_to_copy = TILE_SIZE;
                                    tile_x_offset = 0;
                            }
                            memcpy (io_pBuffer+(copied_row*size+copied_col)*PIXEL_SIZE,in_pTiled->m_pTilesMap[tile_index]->m_pRows[tile_y_offset+in_tile_row_count]+tile_x_offset,col_to_copy);
                            copied_row ++;
                    }
                    remain_col -=col_to_copy;
                    col_count ++;
            }
            remain_row -= row_to_copy;
            row_count++;
    }
    
}
void tmCleanTile(tmTile* io_pTile, int in_iOffset, tmMoveDirectionFlag in_eFlag) {
   
    int tile_row;
    
    if (in_eFlag == tmMoveDirectionFlagUP) { 
        for (tile_row = TILE_SIZE - in_iOffset; tile_row < TILE_SIZE; tile_row++) {
          memset (io_pTile->m_pRows[tile_row],0, TILE_SIZE * PIXEL_SIZE);   
        }
    } else if (in_eFlag == tmMoveDirectionFlagDown) { 
        for (tile_row = 0; tile_row < in_iOffset; tile_row++) {
          memset (io_pTile->m_pRows[tile_row],0, TILE_SIZE * PIXEL_SIZE);   
        }
    } else if (in_eFlag == tmMoveDirectionFlagLeft) { 
        int white_size = in_iOffset * PIXEL_SIZE;
        for (tile_row = 0; tile_row < TILE_SIZE; tile_row++) {
            memset (io_pTile->m_pRows[tile_row] + TILE_SIZE - in_iOffset, 0, white_size);       
        }
    //} else if (in_eFlag == tmMoveDirectionFlagRight) { 
    } else {
        int white_size = in_iOffset * PIXEL_SIZE;
        for (tile_row = 0; tile_row < TILE_SIZE; tile_row++) {
            memset (io_pTile->m_pRows[tile_row], 0, white_size);       
        }  
    }
    
}
/**
 * Global tiled memory initialization function, any type of global variable should be 
 * initialized here
 */
void tmInit(unsigned char* in_pFrameBuffer, int in_iBufferSize){
    // Initial global variable
    int in_iTilesPerRow, in_iTilesPerCol;
    in_iTilesPerRow = in_iBufferSize / TILE_SIZE + 1;
    in_iTilesPerCol = in_iTilesPerRow;
    gAuxTiles = tmAllocTile(TILE_SIZE);
    gAuxTiledMemory = tmAlloc(tmTile*, in_iTilesPerRow * in_iTilesPerCol );
    gGlobalCCW = tmAllocMat(eMatrixType_CCW90);
    gGlobalCW = tmAllocMat(eMatrixType_CW90);
    gGlobalMirrorXMatrix = tmAllocMat(eMatrixType_MirroX);
    gGlobalMirrorYMatrix = tmAllocMat(eMatrixType_MirroY);
    gGlobalR180 = tmAllocMat(eMatrixType_R180);
    gGlobalTransform = tmAllocMat(eMatrixType_Identity);
    gTempMul  =tmAllocMat(eMatrixType_Identity);
    gInterTileIndexMap = tmAllocIndexMap(TILE_SIZE, TILE_SIZE);
    gIntraTileIndexMap = tmAllocIndexMap(in_iTilesPerRow, in_iTilesPerCol);
    gGlobalTiledMemory = tmAllocTiledMemoryFromFrame(in_pFrameBuffer, in_iBufferSize);
}
void tmCleanUp(){
    tmFreeMat(gTempMul);
    tmFreeMat(gGlobalTransform);
    tmFreeMat(gGlobalCCW);
    tmFreeMat(gGlobalCW);
    tmFreeMat(gGlobalMirrorXMatrix);
    tmFreeMat(gGlobalMirrorYMatrix);
    tmFreeMat(gGlobalR180);
    tmFreeMat(gGlobalTransform);
    tmFreeVec(gTempVec1);
    tmFreeVec(gTempVec2);
    tmFreeIndexMap(gInterTileIndexMap);
    tmFreeIndexMap(gIntraTileIndexMap);
    tmFreeTiledMemory(gGlobalTiledMemory);
}
void tmMoveTile(tmTile* io_pFrom, tmTile* io_pTo, int in_iOffset, tmMoveDirectionFlag in_eFlag){
    if (in_iOffset == 0)
        return;
    
    int tile_row;
    
    if (in_eFlag == tmMoveDirectionFlagUP) { 
        for (tile_row = 0; tile_row < in_iOffset; tile_row++) {
            io_pTo->m_pRows[TILE_SIZE - in_iOffset + tile_row] = io_pFrom->m_pRows[tile_row]; 
        }
        for (tile_row = 0; tile_row < TILE_SIZE - in_iOffset; tile_row++) {
            io_pFrom->m_pRows[tile_row] = io_pFrom->m_pRows[tile_row + in_iOffset];  
        }
    } else if (in_eFlag == tmMoveDirectionFlagDown) {
        for (tile_row = 0; tile_row < in_iOffset; tile_row++) {
            io_pTo->m_pRows[tile_row] = io_pFrom->m_pRows[tile_row + TILE_SIZE - in_iOffset]; 
        }
        for (tile_row = 0; tile_row < TILE_SIZE - in_iOffset; tile_row++) {
            io_pFrom->m_pRows[tile_row + in_iOffset] = io_pFrom->m_pRows[tile_row];  
        }
    } else if (in_eFlag == tmMoveDirectionFlagLeft) {
        int shift_len = in_iOffset * PIXEL_SIZE;
        int shift_n_len = (TILE_SIZE - in_iOffset)* PIXEL_SIZE;
        char* row_start;
        for (tile_row = 0; tile_row < TILE_SIZE; tile_row++) {
            row_start = io_pFrom->m_pRows[tile_row];
            memmove (io_pTo->m_pRows[tile_row] + shift_n_len, row_start, shift_len);
            memmove (row_start, row_start + shift_len, shift_n_len);
        } 
    //} else if (in_eFlag == tmMoveDirectionFlagRight) {
        } else {
        int shift_len = in_iOffset * PIXEL_SIZE;
        int shift_n_len = (TILE_SIZE - in_iOffset)* PIXEL_SIZE;
        char* row_start;
        for (tile_row = 0; tile_row < TILE_SIZE; tile_row++) {
            row_start = io_pFrom->m_pRows[tile_row];
            memmove (io_pTo->m_pRows[tile_row], row_start + shift_n_len, shift_len);
            memmove (row_start + shift_len, row_start, shift_n_len);
        } 
    } 
}
void tmMoveTiledMemory(tmTiledMemory* io_pTiledMemory,int in_iOffset, tmMirrorDirectionFlag in_eFlag){
    if(io_pTiledMemory == NULL){
        return;
    }
    else{
        int tile_to_move = in_iOffset / TILE_SIZE;
        int tile_umove_offset = io_pTiledMemory->m_iTilesPerRow-tile_to_move;
        int total_tiles = io_pTiledMemory->m_iTilesPerCol * io_pTiledMemory->m_iTilesPerRow;
        int row,col;
        switch(in_eFlag){
            case tmMoveDirectionFlagLeft:{
                // Check if the offset is greater than the tile size, if it is greater then move tile first
                if (tile_to_move!=0){
                    for(row = 0; row < io_pTiledMemory->m_iTilesPerRow; row++){
                        tmTile** start = io_pTiledMemory->m_pTilesMap+row*io_pTiledMemory->m_iTilesPerRow;
                        memcpy(start,gAuxTiledMemory, tile_to_move*sizeof(tmTile*));
                        memmove(start,start+tile_to_move, sizeof(tmTile*)*tile_umove_offset);
                        memcpy(start+tile_umove_offset, gAuxTiledMemory, tile_to_move*sizeof(tmTile*));
                    }
                }
                // Individual tile movement
                in_iOffset %= TILE_SIZE;
                col = 0;
                row = 0;
                for (row = 0; row < io_pTiledMemory->m_iTilesPerRow; row++){
                    tmTile** start_tile = io_pTiledMemory->m_pTilesMap + row*io_pTiledMemory->m_iTilesPerRow;
                    for (col = 1; col < io_pTiledMemory->m_iTilesPerCol; col++){
                        tmMoveTile(start_tile[col],start_tile[col-1],in_iOffset,in_eFlag);
                    }
                    tmMoveTile(start_tile[0],start_tile[io_pTiledMemory->m_iTilesPerCol-1],in_iOffset,in_eFlag);
                }
                // White the tile
                row = io_pTiledMemory->m_iTilesPerCol-1;
                while(row < total_tiles){
                    tmCleanTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += io_pTiledMemory->m_iTilesPerRow;
                }
            }
            case tmMoveDirectionFlagRight:{
                if (tile_to_move!=0){
                    for(row = 0; row < io_pTiledMemory->m_iTilesPerRow; row++){
                        tmTile** start = io_pTiledMemory->m_pTilesMap+row*io_pTiledMemory->m_iTilesPerRow;
                        memcpy(start+tile_umove_offset,gAuxTiledMemory, tile_to_move*sizeof(tmTile*));
                        memmove(start+tile_to_move,start, sizeof(tmTile*)*tile_umove_offset);
                        memcpy(start, gAuxTiledMemory, tile_to_move*sizeof(tmTile*));
                    }
                }
                in_iOffset %= TILE_SIZE;
                col = 0;
                row = 0;
                for (row = 0; row < io_pTiledMemory->m_iTilesPerRow; row++){
                    tmTile** start_tile = io_pTiledMemory->m_pTilesMap + row*io_pTiledMemory->m_iTilesPerRow;
                    for (col = 0; col < io_pTiledMemory->m_iTilesPerCol-1; col++){
                        tmMoveTile(start_tile[col+1],start_tile[col],in_iOffset,in_eFlag);
                    }
                    tmMoveTile(start_tile[io_pTiledMemory->m_iTilesPerCol-1],start_tile[0],in_iOffset,in_eFlag);
                }
                row = 0;
                while(row < total_tiles){
                    tmCleanTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += io_pTiledMemory->m_iTilesPerRow;
                }
            }
            case tmMoveDirectionFlagUP:{
                if (tile_to_move!=0){
                    int element_to_move = tile_to_move*io_pTiledMemory->m_iTilesPerRow;
                    int element_n_to_move = io_pTiledMemory->m_iTilesPerCol * io_pTiledMemory->m_iTilesPerRow-element_to_move;
                    memcpy(gAuxTiledMemory,io_pTiledMemory->m_pTilesMap, element_to_move);
                    memmove(io_pTiledMemory->m_pTilesMap, io_pTiledMemory->m_pTilesMap+element_to_move, element_n_to_move);
                    memcpy(io_pTiledMemory->m_pTilesMap+element_to_move, gAuxTiledMemory, element_to_move);
                }
                int i = 0;
                in_iOffset %= TILE_SIZE;
                int to_index = 0;
                for (i = 0; i < total_tiles; i++){
                    to_index = (i + total_tiles-io_pTiledMemory->m_iTilesPerRow)%total_tiles;
                    tmMoveTile(io_pTiledMemory->m_pTilesMap[i], io_pTiledMemory->m_pTilesMap[i+to_index],in_iOffset,in_eFlag);
                }
                row = total_tiles-io_pTiledMemory->m_iTilesPerRow;
                while(row < total_tiles){
                    tmCleanTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += 1;
                }
            }
            case tmMoveDirectionFlagDown:{
                if (tile_to_move!=0){
                    int element_to_move = tile_to_move*io_pTiledMemory->m_iTilesPerRow;
                    int element_n_to_move = io_pTiledMemory->m_iTilesPerCol * io_pTiledMemory->m_iTilesPerRow-element_to_move;
                    memcpy(gAuxTiledMemory,io_pTiledMemory+element_n_to_move, element_to_move);
                    memmove(io_pTiledMemory->m_pTilesMap+element_to_move, io_pTiledMemory->m_pTilesMap, element_n_to_move);
                    memcpy(io_pTiledMemory->m_pTilesMap, gAuxTiledMemory, element_to_move);
                }
                int i = 0;
                in_iOffset %= TILE_SIZE;
                int to_index = 0;
                for (i = 0; i < total_tiles; i++){
                    to_index = (i + total_tiles-io_pTiledMemory->m_iTilesPerRow)%total_tiles;
                    tmMoveTile(io_pTiledMemory->m_pTilesMap[i+to_index],io_pTiledMemory->m_pTilesMap[i],in_iOffset,in_eFlag);
                }
                row = 0;
                while(row < io_pTiledMemory->m_iTilesPerRow){
                    tmCleanTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += 1;
                }

            }
        }
    }
}
void tmTransformTiledMemory(tmTiledMemory* io_pTile, tmMat4i* in_pMat){
    int x_movement,y_movement;
    x_movement = in_pMat[MATRIX_INDEX_TRANSFORM_X];
    y_movement = in_pMat[MATRIX_INDEX_TRANSFORM_Y];
    in_pMat[MATRIX_INDEX_TRANSFORM_Y] = 0;
    in_pMat[MATRIX_INDEX_TRANSFORM_X] = 0;
    tmLoadIndexMapFromTransFormMatrix(in_pMat,gInterTileIndexMap);
    tmLoadIndexMapFromTransFormMatrix(in_pMat,gIntraTileIndexMap);
    int tile_dimenson = io_pTile->m_iTiledDimension;
    if(x_movement!=0 || y_movement!=0){
        // Inter-tile movement
        if(x_movement!=0){
            tmMoveTiledMemory(io_pTile, x_movement, tmMirrorDirectionX);
        }
        if(y_movement!=0){
            tmMoveTiledMemory(io_pTile, y_movement, tmMirrorDirectionX);
        }
    }

    int i,total_tiles;
    total_tiles = io_pTile->m_iTilesPerRow * io_pTile->m_iTilesPerCol;
    for (i = 0 ; total_tiles; i++){
        tmMapTile(io_pTile->m_pTilesMap[i],gInterTileIndexMap);
    }
    tmMapTiledMemory(io_pTile,gIntraTileIndexMap);
}


//void tmRotateTiledMemory(tmTiledMemory* io_pTiledMemory, tmRotionDirectionFlag in_eFlag);
//void tmMoveTiledMemory(tmTiledMemory* io_pTiledMemory,int in_iOffset,tmMirrorDirectionFlag in_eFlag);
//void tmMirrorTiledMemory(tmTiledMemory* io_pTiledMemory,int in_iOffset, tmMirrorDirectionFlag in_eFlag);
//void tmMirrorTile(tmTile* io_pTile, tmMirrorDirectionFlag in_eFlag){
//    if (in_eFlag == tmMirrorDirectionX) {
//        char *temp_ptr;
//        int tile_row;
//        int half_tile = TILE_SIZE >> 1;
//        for (tile_row=0; tile_row<half_tile; tile_row++) {
//            temp_ptr = io_pTile->m_pRows[tile_row];
//            io_pTile->m_pRows[tile_row] = io_pTile->m_pRows[TILE_SIZE - tile_row];
//            io_pTile->m_pRows[tile_row - tile_row] = temp_ptr;
//        }      
//    } else {
//        int tile_row;
//        int tile_col;
//        int half_tile = TILE_SIZE >> 1;
//        char temp_r;
//        char temp_g;
//        char temp_b;
//        for (tile_row=0; tile_row<TILE_SIZE; tile_row++){
//            for (tile_col=0; tile_col< half_tile; tile_col++){ 
//                temp_r = io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE];
//                temp_g = io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 1];
//                temp_b = io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 2];
//                io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE] = io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE];
//                io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 1] = io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 1];
//                io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 2] = io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 2];
//                io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE] = temp_r;
//                io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 1] = temp_g;
//                io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 2] = temp_b;
//            }       
//        }       
//    }
//}
//void tmFreeTiledMemory(tmTiledMemory* in_pTiledMemory){
//    // Free total buffer
//    free(in_pTiledMemory->m_pBuffer);
//    int i;
//    // Free tile's row-mapping buffer
//    for (i = 0; i < TILE_SIZE; i++){
//        free(in_pTiledMemory->m_pTiles[i].m_pRows);
//    }
//    // Free tile-mapping buffer
//    free(in_pTiledMemory->m_pTiles);
//}

//void tmRotateTiledMemory(tmTiledMemory* io_pTiledMemory, tmRotionDirectionFlag in_eFlag){
//    
//}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image up
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveUp(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {
#ifndef SPEED_UP
    return processMoveUpReference(buffer_frame, width, height, offset);
#else
    gGlobalTransform[MATRIX_INDEX_TRANSFORM_Y] += offset;
    return NULL;
#endif
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image left
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveRight(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {
    return processMoveRightReference(buffer_frame, width, height, offset);
#ifndef SPEED_UP
    return processMoveRightReference()Reference(buffer_frame, width, height, offset);
#else
    gGlobalTransform[MATRIX_INDEX_TRANSFORM_X] += offset;
    return NULL;
#endif
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image up
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveDown(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {
#ifndef SPEED_UP
    return processMoveDownReference(buffer_frame, width, height, offset);
#else
    gGlobalTransform[MATRIX_INDEX_TRANSFORM_Y] -= offset;
    return NULL;
#endif
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image right
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveLeft(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {
#ifndef SPEED_UP
    return processMoveLeftReference()Reference(buffer_frame, width, height, offset);
#else
    gGlobalTransform[MATRIX_INDEX_TRANSFORM_X] -= offset;
    return NULL;
#endif
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param rotate_iteration - rotate object inside frame buffer clockwise by 90 degrees, <iteration> times
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note: You can assume the frame will always be square and you will be rotating the entire image
 **********************************************************************************************************************/
unsigned char *processRotateCW(unsigned char *buffer_frame, unsigned width, unsigned height,
                               int rotate_iteration) {
#ifndef SPEED_UP
    return processRotateCWReference(buffer_frame, width, height, rotate_iteration);
#else
    switch(rotate_iteration%4){
        case(0):{
            break;
        }
        case(1):{
            tmMatMulVecInplace(gGlobalCW, gGlobalTransform);
            break;
        }
        case(2):{
            tmMatMulVecInplace(gGlobalR180, gGlobalTransform);
            break;
        }
        case(3):{
            tmMatMulVecInplace(gGlobalCCW, gGlobalTransform);
            break;
        }
        default:{
            break;
        }
    }
    return NULL;
#endif
    
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param rotate_iteration - rotate object inside frame buffer counter clockwise by 90 degrees, <iteration> times
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note: You can assume the frame will always be square and you will be rotating the entire image
 **********************************************************************************************************************/
unsigned char *processRotateCCW(unsigned char *buffer_frame, unsigned width, unsigned height,
                                int rotate_iteration) {
#ifndef SPEED_UP
    return processRotateCCWReference(buffer_frame, width, height, rotate_iteration);
#else
    switch(rotate_iteration%4){
        case(0):{
            break;
        }
        case(1):{
            tmMatMulVecInplace(gGlobalCCW, gGlobalTransform);
            break;
        }
        case(2):{
            tmMatMulVecInplace(gGlobalR180, gGlobalTransform);
            break;
        }
        case(3):{
            tmMatMulVecInplace(gGlobalCW, gGlobalTransform);
            break;
        }
        default:{
            break;
        }
    }
    return NULL;
#endif
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorX(unsigned char *buffer_frame, unsigned int width, unsigned int height, int _unused) {
#ifndef SPEED_UP
    return processMirrorXReference(buffer_frame, width, height, _unused);
#else
    tmMatMulVecInplace(gGlobalMirrorXMatrix, gGlobalTransform);
    return NULL;
#endif
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorY(unsigned char *buffer_frame, unsigned width, unsigned height, int _unused) {
#ifndef SPEED_UP
    return processMirrorYReference(buffer_frame, width, height, _unused);
#else
    tmMatMulVecInplace(gGlobalMirrorYMatrix, gGlobalTransform);
    return NULL;
#endif
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "XXX";

    // Please fill in your information
    char student1_first_name[] = "Zhuang";
    char student1_last_name[] = "Li";
    char student1_student_number[] = "1000311628";

    // Please fill in your partner's information
    // If yon't have partner, do not modify this
    char student2_first_name[] = "Gujiang";
    char student2_last_name[] = "Lin";
    char student2_student_number[] = "1000268239";

    // Printing out team information
    printf("*******************************************************************************************************\n");
    printf("Team Information:\n");
    printf("\tteam_name: %s\n", team_name);
    printf("\tstudent1_first_name: %s\n", student1_first_name);
    printf("\tstudent1_last_name: %s\n", student1_last_name);
    printf("\tstudent1_student_number: %s\n", student1_student_number);
    printf("\tstudent2_first_name: %s\n", student2_first_name);
    printf("\tstudent2_last_name: %s\n", student2_last_name);
    printf("\tstudent2_student_number: %s\n", student2_student_number);
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          You can modify anything else in this file
 ***********************************************************************************************************************
 * @param sensor_values - structure stores parsed key value pairs of program instructions
 * @param sensor_values_count - number of valid sensor values parsed from sensor log file or commandline console
 * @param frame_buffer - pointer pointing to a buffer storing the imported  24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param grading_mode - turns off verification and turn on instrumentation
 ***********************************************************************************************************************
 *
 **********************************************************************************************************************/
void implementation_driver(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                           unsigned int width, unsigned int height, bool grading_mode) {
    int processed_frames = 0;
    tmInit(frame_buffer, width*height);
    // Allocate global frame
    
    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
//        printf("Processing sensor value #%d: %s, %d\n", sensorValueIdx, sensor_values[sensorValueIdx].key,
//               sensor_values[sensorValueIdx].value);
        if (!strcmp(sensor_values[sensorValueIdx].key, "W")) {
            frame_buffer = processMoveUp(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "A")) {
            frame_buffer = processMoveLeft(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "S")) {
            frame_buffer = processMoveDown(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "D")) {
            frame_buffer = processMoveRight(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "CW")) {
            frame_buffer = processRotateCW(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "CCW")) {
            frame_buffer = processRotateCCW(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MX")) {
            frame_buffer = processMirrorX(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MY")) {
            frame_buffer = processMirrorY(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        }
        processed_frames += 1;
        if (processed_frames % 25 == 0) {
            tmTransformTiledMemory(gGlobalTiledMemory, gGlobalTransform);
            tmUpdateVertex(gGlobalTransform);
            tmTiledMemoryToFrame(frame_buffer,width*height,gGlobalTiledMemory);
            verifyFrame(frame_buffer, width, height, grading_mode);
        }
    }
    tmCleanUp();
    return;
}
