#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

#define ENABLE_SIMD 0

#define TILE_SIZE 1000
#define PIXEL_SIZE 3
#define tmAlloc(type,size) (type*)malloc(sizeof(type)*size)

#define MATRIX_00 0
#define MATRIX_01 1
#define MATRIX_02 2
#define MATRIX_10 3
#define MATRIX_11 4
#define MATRIX_12 5
#define MATRIX_20 6
#define MATRIX_21 7
#define MATRIX_22 8
#define VECTOR_X 0
#define VECTOR_Y 1
#define VECTOR_Z 2


///////////// Matrix Operation ////////////////////

typedef int tmMat4i;
typedef int tmVec4i;

typedef enum _tmMatrixType{
    eMatrixType_Identity,
    eMatrixType_CCW90,
    eMatrixType_CW90,
    eMatrixType_MirroX,
    eMatrixType_MirroY
} tmMatFlag;

// MAT A * MAT B
void tmMatMulVec(tmMat4i* in_pA, tmVec4i* in_pB, tmVec4i* io_pC){
#if ENABLE_SIMD
    
#else
    io_pC[VECTOR_X] = in_pA[MATRIX_00]*in_pB[VECTOR_X] + in_pA[MATRIX_01]*in_pB[VECTOR_Y] + in_pA[MATRIX_02]*in_pB[VECTOR_Z];
    io_pC[VECTOR_Y] = in_pA[MATRIX_10]*in_pB[VECTOR_X] + in_pA[MATRIX_11]*in_pB[VECTOR_Y] + in_pA[MATRIX_12]*in_pB[VECTOR_Z];
    io_pC[VECTOR_Z] = in_pA[MATRIX_20]*in_pB[VECTOR_X] + in_pA[MATRIX_21]*in_pB[VECTOR_Y] + in_pA[MATRIX_22]*in_pB[VECTOR_Z];
#endif
}
void tmMatMulMat(tmMat4i* in_pA, tmMat4i* in_pB, tmMat4i* io_pC){
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
void tmLoadMatRotation(tmMat4i* in_pA, tmMatFlag in_eFlag){
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
void tmLoadMatMirror(tmMat4i* in_pA, tmMatFlag in_eFlag){
    switch(in_eFlag){
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
void tmLoadMatTranslate(tmMat4i* in_pA, int in_iOffsetX,int in_iOffsetY){
    in_pA[MATRIX_02] = in_iOffsetX;
    in_pA[MATRIX_02] = in_iOffsetY;
}
void tmLoadMatIdentity(tmMat4i* in_pA){
    in_pA[MATRIX_00] = 1;
    in_pA[MATRIX_01] = 0;
    in_pA[MATRIX_10] = 0;
    in_pA[MATRIX_11] = 1;
    in_pA[MATRIX_22] = 1;
}
tmMat4i* tmAllocMat(){
    tmMat4i* ret = tmAlloc(int, 16);
    tmLoadMatIdentity(ret);
    return ret;
}
void tmFreeMat(tmMat4i* in_pA){
    free(in_pA);
}
tmVec4i* tmAllocVec(){
    tmVec4i* ret = tmAlloc(int, 4);
    ret[VECTOR_Z] = 1;
    return ret;
}
void tmFreeVec(tmVec4i* in_pA){
    free(in_pA);
}
void tmMatrixIndexToVec(int in_iIndex,int in_iStrideSize, tmVec4i* io_pVec){
    io_pVec[VECTOR_X] = in_iIndex % in_iStrideSize;
    io_pVec[VECTOR_Y] = in_iIndex / in_iStrideSize;
    io_pVec[VECTOR_Z] = 1;
}
int tmVecToMatrixIndex(tmVec4i* in_pVec,int in_iStrideSize){
    return in_pVec[VECTOR_X] + in_pVec[VECTOR_Y]*in_iStrideSize;
}


typedef struct _tmTile{
    unsigned char* m_pBuffer;
    unsigned char** m_pRows;
} tmTile;

// Global While tile

tmTile** gTempTiles = NULL;

typedef struct _tmTiledBuffer{
    unsigned char* m_pBuffer;
    tmTile* m_pTiles;
    tmTile** m_pTilesMap;
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

void tmRotateTile(tmTile* io_pTile, tmRotionDirectionFlag in_eFlag);
void tmMirrorTile(tmTile* io_pTile, tmMirrorDirectionFlag in_eFlag);
void tmMoveTile(tmTile* io_pFrom, tmTile* io_pTo, int in_iOffset, tmMoveDirectionFlag in_eFlag);
void tmWhiteTile(tmTile* io_pTile, int in_iOffset, tmMoveDirectionFlag in_eFlag);
void tmSwapTile(tmTile* io_pTileA, tmTile* io_pTileB);

tmTiledMemory* tmAllocTiledMemory(size_t in_iTileSize, size_t in_iTilesPerRow, size_t in_iTilesPerCol);
void tmFreeTiledMemory(tmTiledMemory* in_pTiledMemory);
void tmMakeTile(tmTile* io_pTile, unsigned char* in_pBuffer);

void tmFrameToTiledMemory(unsigned char* in_pBuffer, int in_iSize, tmTiledMemory* io_pOutputTiled);
void tmTiledMemoryToFrame(unsigned char* io_pBuffer, int in_iSize, tmTiledMemory* in_pOutputTiled);

void tmRotateTiledMemory(tmTiledMemory* io_pTiledMemory, tmRotionDirectionFlag in_eFlag);
void tmMoveTiledMemory(tmTiledMemory* io_pTiledMemory,int in_iOffset,tmMirrorDirectionFlag in_eFlag);
void tmMirrorTiledMemory(tmTiledMemory* io_pTiledMemory,int in_iOffset, tmMirrorDirectionFlag in_eFlag);

void tmMakeTile(tmTile* io_pTile, unsigned char* in_pBuffer){
    io_pTile->m_pBuffer = in_pBuffer;
    io_pTile->m_pRows = tmAlloc(unsigned char*, TILE_SIZE);
    int i;
    for (int i = 0 ; i < TILE_SIZE; i++){
        io_pTile->m_pRows[i] = io_pTile->m_pBuffer + i*TILE_SIZE;
    }
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

void tmWhiteTile(tmTile* io_pTile, int in_iOffset, tmMoveDirectionFlag in_eFlag) {
   
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
void tmMirrorTile(tmTile* io_pTile, tmMirrorDirectionFlag in_eFlag){
    if (in_eFlag == tmMirrorDirectionX) {
        char *temp_ptr;
        int tile_row;
        int half_tile = TILE_SIZE >> 1;
        for (tile_row=0; tile_row<half_tile; tile_row++) {
            temp_ptr = io_pTile->m_pRows[tile_row];
            io_pTile->m_pRows[tile_row] = io_pTile->m_pRows[TILE_SIZE - tile_row];
            io_pTile->m_pRows[tile_row - tile_row] = temp_ptr;
        }      
    } else {
        int tile_row;
        int tile_col;
        int half_tile = TILE_SIZE >> 1;
        char temp_r;
        char temp_g;
        char temp_b;
        for (tile_row=0; tile_row<TILE_SIZE; tile_row++){
            for (tile_col=0; tile_col< half_tile; tile_col++){ 
                temp_r = io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE];
                temp_g = io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 1];
                temp_b = io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 2];
                io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE] = io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE];
                io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 1] = io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 1];
                io_pTile->m_pRows[tile_row][tile_col*PIXEL_SIZE + 2] = io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 2];
                io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE] = temp_r;
                io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 1] = temp_g;
                io_pTile->m_pRows[tile_row][TILE_SIZE - (tile_col + 1)*PIXEL_SIZE + 2] = temp_b;
            }       
        }       
    }
}

tmTiledMemory* tmAllocTiledMemory(size_t in_iTileSize, size_t in_iTilesPerRow, size_t in_iTilesPerCol){
    size_t in_iNumOfTile = in_iTilesPerCol * in_iTilesPerRow;
    unsigned char* fullBuffer = tmAlloc(unsigned char,in_iTileSize*in_iTileSize*in_iNumOfTile);
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
    // Free total buffer
    free(in_pTiledMemory->m_pBuffer);
    int i;
    // Free tile's row-mapping buffer
    for (i = 0; i < TILE_SIZE; i++){
        free(in_pTiledMemory->m_pTiles[i].m_pRows);
    }
    // Free tile-mapping buffer
    free(in_pTiledMemory->m_pTiles);
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
                        memcpy(start,gTempTiles, tile_to_move*sizeof(tmTile*));
                        memmove(start,start+tile_to_move, sizeof(tmTile*)*tile_umove_offset);
                        memcpy(start+tile_umove_offset, gTempTiles, tile_to_move*sizeof(tmTile*));
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
                    tmWhiteTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += io_pTiledMemory->m_iTilesPerRow;
                }
            }
            case tmMoveDirectionFlagRight:{
                if (tile_to_move!=0){
                    for(row = 0; row < io_pTiledMemory->m_iTilesPerRow; row++){
                        tmTile** start = io_pTiledMemory->m_pTilesMap+row*io_pTiledMemory->m_iTilesPerRow;
                        memcpy(start+tile_umove_offset,gTempTiles, tile_to_move*sizeof(tmTile*));
                        memmove(start+tile_to_move,start, sizeof(tmTile*)*tile_umove_offset);
                        memcpy(start, gTempTiles, tile_to_move*sizeof(tmTile*));
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
                    tmWhiteTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += io_pTiledMemory->m_iTilesPerRow;
                }
            }
            case tmMoveDirectionFlagUP:{
                if (tile_to_move!=0){
                    int element_to_move = tile_to_move*io_pTiledMemory->m_iTilesPerRow;
                    int element_n_to_move = io_pTiledMemory->m_iTilesPerCol * io_pTiledMemory->m_iTilesPerRow-element_to_move;
                    memcpy(gTempTiles,io_pTiledMemory->m_pTilesMap, element_to_move);
                    memmove(io_pTiledMemory->m_pTilesMap, io_pTiledMemory->m_pTilesMap+element_to_move, element_n_to_move);
                    memcpy(io_pTiledMemory->m_pTilesMap+element_to_move, gTempTiles, element_to_move);
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
                    tmWhiteTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += 1;
                }
            }
            case tmMoveDirectionFlagDown:{
                if (tile_to_move!=0){
                    int element_to_move = tile_to_move*io_pTiledMemory->m_iTilesPerRow;
                    int element_n_to_move = io_pTiledMemory->m_iTilesPerCol * io_pTiledMemory->m_iTilesPerRow-element_to_move;
                    memcpy(gTempTiles,io_pTiledMemory+element_n_to_move, element_to_move);
                    memmove(io_pTiledMemory->m_pTilesMap+element_to_move, io_pTiledMemory->m_pTilesMap, element_n_to_move);
                    memcpy(io_pTiledMemory->m_pTilesMap, gTempTiles, element_to_move);
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
                    tmWhiteTile(io_pTiledMemory->m_pTilesMap[row],in_iOffset,in_eFlag);
                    row += 1;
                }

            }
        }
    }
}
void tmRotateTiledMemory(tmTiledMemory* io_pTiledMemory, tmRotionDirectionFlag in_eFlag){
    
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
unsigned char *processMoveUp(unsigned char *buffer_frame, unsigned width, unsigned height, int offset) {
    return processMoveUpReference(buffer_frame, width, height, offset);
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
    return processMoveDownReference(buffer_frame, width, height, offset);
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
    return processMoveLeftReference(buffer_frame, width, height, offset);
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
    return processRotateCWReference(buffer_frame, width, height, rotate_iteration);
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
    return processRotateCCWReference(buffer_frame, width, height, rotate_iteration);
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorX(unsigned char *buffer_frame, unsigned int width, unsigned int height, int _unused) {
    return processMirrorXReference(buffer_frame, width, height, _unused);
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorY(unsigned char *buffer_frame, unsigned width, unsigned height, int _unused) {
    return processMirrorYReference(buffer_frame, width, height, _unused);
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
            verifyFrame(frame_buffer, width, height, grading_mode);
        }
    }
    return;
}
