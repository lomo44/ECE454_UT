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
#define R_OFFSET 0
#define G_OFFSET 1
#define B_OFFSET 2

///////////// Matrix Operation ////////////////////

typedef int tmMat4i;
typedef int tmVec4i;
typedef unsigned char* tmBuffer;

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

bool gInited = 0;

int tmMin(int x, int y){
    if(x>y){
        return x;
    }
    else
        return y;
}

tmVec4i* gTempVec1 = NULL;
tmVec4i* gTempVec2 = NULL;
//int      gVertex   = etop_left;

tmMat4i* gTempMul = NULL;
tmMat4i* gGlobalCW = NULL;
tmMat4i* gGlobalCCW = NULL;
tmMat4i* gGlobalMirrorYMatrix = NULL;
tmMat4i* gGlobalMirrorXMatrix = NULL;
tmMat4i* gGlobalR180 = NULL;
tmMat4i* gGlobalTransform = NULL;

tmVec4i* gVertex = NULL;
tmVec4i* gTL= NULL;
tmVec4i* gTR= NULL;
tmVec4i* gBL = NULL;
tmVec4i* gBR = NULL;

int      gbb_size = 0;
void        tmPrintMat(tmMat4i* in_pMat){
    printf("%d,%d,%d\n",in_pMat[MATRIX_00],in_pMat[MATRIX_01],in_pMat[MATRIX_02]);
    printf("%d,%d,%d\n",in_pMat[MATRIX_10],in_pMat[MATRIX_11],in_pMat[MATRIX_12]);
    printf("%d,%d,%d\n",in_pMat[MATRIX_20],in_pMat[MATRIX_21],in_pMat[MATRIX_22]);

}
void        tmUpdateBoundingBox (unsigned char* in_iBuffer, int size, int length){
    int pixel_index;
    int x_min=length-1;
    int x_max=0;
    int y_min=length-1;
    int y_max=0;
    int x_temp;
    int y_temp;
    int x_size;
    int y_size;
    for (pixel_index = 0; pixel_index < size; pixel_index+= 3) {
        if (in_iBuffer[pixel_index] !=0 ||in_iBuffer[pixel_index+1] !=0 ||in_iBuffer[pixel_index+2] !=0 ) {
            y_temp = (pixel_index/PIXEL_SIZE)/length;
            x_temp = (pixel_index/PIXEL_SIZE)%length;
            if (x_temp > x_max){
                x_max = x_temp;
            }
            if (x_temp < x_min){
                x_min = x_temp;
            }
            if (y_temp > y_max){
                y_max = y_temp;
            }
            if (y_temp < y_min){
                y_min = y_temp;
            }
        }
    }

    x_size = x_max - x_min + 1;
    y_size = y_max - y_min + 1;

    if (x_size < y_size)
        gbb_size = y_size;
    else
        gbb_size = x_size;

    if (y_min + gbb_size > length) {
        y_min= length - gbb_size ;
    }
    if (x_min + gbb_size > length) {
        x_min= length - gbb_size ;
    }

    x_max = x_min + gbb_size -1 ;
    y_max = y_min + gbb_size -1 ;

    gVertex[VECTOR_X]= x_min;
    gVertex[VECTOR_Y]= y_min;
    gTL[VECTOR_X]= x_min;
    gTL[VECTOR_Y]= y_min;
    gTR[VECTOR_X]= x_max;
    gTR[VECTOR_Y]= y_min;
    gBL[VECTOR_X]= x_min;
    gBL[VECTOR_Y]= y_max;
    gBR[VECTOR_X]= x_max;
    gBR[VECTOR_Y]= y_max;
    
}
void        tmCopyMat(tmMat4i* in_pFrom, tmMat4i* in_pTo){
    in_pTo[MATRIX_00] = in_pFrom[MATRIX_00];
    in_pTo[MATRIX_01] = in_pFrom[MATRIX_01];
    in_pTo[MATRIX_02] = in_pFrom[MATRIX_02];
    in_pTo[MATRIX_10] = in_pFrom[MATRIX_10];
    in_pTo[MATRIX_11] = in_pFrom[MATRIX_11];
    in_pTo[MATRIX_12] = in_pFrom[MATRIX_12];
    in_pTo[MATRIX_20] = in_pFrom[MATRIX_20];
    in_pTo[MATRIX_21] = in_pFrom[MATRIX_21];
    in_pTo[MATRIX_22] = in_pFrom[MATRIX_22];
}
void        tmCopyVec(tmVec4i* in_pA, tmVec4i* in_pB){
    in_pA[VECTOR_X] = in_pB[VECTOR_X];
    in_pA[VECTOR_Y] = in_pB[VECTOR_Y];
    in_pA[VECTOR_Z] = in_pB[VECTOR_Z];
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
    io_pC[MATRIX_02] = in_pA[MATRIX_00]*in_pB[MATRIX_02] + in_pA[MATRIX_01]*in_pB[MATRIX_12] + in_pA[MATRIX_02]*in_pB[MATRIX_22];
    io_pC[MATRIX_10] = in_pA[MATRIX_10]*in_pB[MATRIX_00] + in_pA[MATRIX_11]*in_pB[MATRIX_10] + in_pA[MATRIX_12]*in_pB[MATRIX_20];
    io_pC[MATRIX_11] = in_pA[MATRIX_10]*in_pB[MATRIX_01] + in_pA[MATRIX_11]*in_pB[MATRIX_11] + in_pA[MATRIX_12]*in_pB[MATRIX_21];
    io_pC[MATRIX_12] = in_pA[MATRIX_10]*in_pB[MATRIX_02] + in_pA[MATRIX_11]*in_pB[MATRIX_12] + in_pA[MATRIX_12]*in_pB[MATRIX_22];
    io_pC[MATRIX_20] = in_pA[MATRIX_20]*in_pB[MATRIX_00] + in_pA[MATRIX_21]*in_pB[MATRIX_10] + in_pA[MATRIX_22]*in_pB[MATRIX_20];
    io_pC[MATRIX_21] = in_pA[MATRIX_20]*in_pB[MATRIX_01] + in_pA[MATRIX_21]*in_pB[MATRIX_11] + in_pA[MATRIX_22]*in_pB[MATRIX_21];
    io_pC[MATRIX_22] = in_pA[MATRIX_20]*in_pB[MATRIX_02] + in_pA[MATRIX_21]*in_pB[MATRIX_12] + in_pA[MATRIX_22]*in_pB[MATRIX_22];
#endif
}
void        tmMatMulVecInplace(tmMat4i* in_pA, tmVec4i* io_pB){
    tmMatMulVec(in_pA, io_pB, gTempVec1);
    tmCopyVec(gTempVec1, io_pB);
}
void        tmMatMulMatInplace(tmMat4i* in_pA, tmMat4i* io_pB){
    tmMatMulMat(in_pA,io_pB,gTempMul);
    tmCopyMat(gTempMul,io_pB);
}
void        tmLoadMat(tmMat4i* in_pA, tmMatFlag in_eFlag){
    in_pA[MATRIX_INDEX_TRANSFORM_X] = 0;
    in_pA[MATRIX_INDEX_TRANSFORM_Y] = 0;
    in_pA[MATRIX_20] = 0;
    in_pA[MATRIX_21] = 0;
    switch(in_eFlag){
        case eMatrixType_CCW90:{
            in_pA[MATRIX_00] = 0;
            in_pA[MATRIX_01] = 1;
            in_pA[MATRIX_10] = -1;
            in_pA[MATRIX_11] = 0;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_CW90:{
            in_pA[MATRIX_00] = 0;
            in_pA[MATRIX_01] = -1;
            in_pA[MATRIX_10] = 1;
            in_pA[MATRIX_11] = 0;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_MirroX:{
            in_pA[MATRIX_00] = 1;
            in_pA[MATRIX_01] = 0;
            in_pA[MATRIX_10] = 0;
            in_pA[MATRIX_11] = -1;
            in_pA[MATRIX_22] = 1;
            break;
        }
        case eMatrixType_MirroY:{
            in_pA[MATRIX_00] = -1;
            in_pA[MATRIX_01] = 0;
            in_pA[MATRIX_10] = 0;
            in_pA[MATRIX_11] = 1;
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



typedef enum _tmOrientation {
    e_X_Y = 0,
    e_X_NY,
    e_NX_Y,
    e_NX_NY,
    e_F_X_Y,
    e_F_X_NY,
    e_F_NX_Y,
    e_F_NX_NY
} tmOrientation;

void tmUpdateVertex (tmMat4i* in_pMat, int length) {

    tmMatMulVecInplace(in_pMat, gTL);
    tmMatMulVecInplace(in_pMat, gTR);
    tmMatMulVecInplace(in_pMat, gBL);
    tmMatMulVecInplace(in_pMat, gBR);

    int x_min= gTL[VECTOR_X];
    if (gTR[VECTOR_X] < x_min)
        x_min = gTR[VECTOR_X];
    if (gBL[VECTOR_X] < x_min)
        x_min = gBL[VECTOR_X];
    if (gBR[VECTOR_X] < x_min)
        x_min = gBR[VECTOR_X];

    int y_min= gTL[VECTOR_Y];
    if (gTR[VECTOR_Y] < y_min)
        y_min = gTR[VECTOR_Y];
    if (gBL[VECTOR_Y] < y_min)
        y_min = gBL[VECTOR_Y];
    if (gBR[VECTOR_Y] < y_min)
        y_min = gBR[VECTOR_Y];
    if (x_min < 0) {
        gVertex [VECTOR_X] = x_min + length -1;
        gTL[VECTOR_X] =  gTL[VECTOR_X] + length -1;
        gTR[VECTOR_X] =  gTR[VECTOR_X] + length -1;
        gBL[VECTOR_X] =  gBL[VECTOR_X] + length -1;
        gBR[VECTOR_X] =  gBR[VECTOR_X] + length -1;
    } else {
        gVertex [VECTOR_X] = x_min;
    }
    if (y_min < 0) {
        gVertex [VECTOR_Y] = y_min + length -1;
        gTL[VECTOR_Y] =  gTL[VECTOR_Y] + length -1;
        gTR[VECTOR_Y] =  gTR[VECTOR_Y] + length -1;
        gBL[VECTOR_Y] =  gBL[VECTOR_Y] + length -1;
        gBR[VECTOR_Y] =  gBR[VECTOR_Y] + length -1;
    } else {
        gVertex [VECTOR_Y] = y_min;
    }



};
tmOrientation tmGetOrientationFromMat(tmMat4i* in_pMat){
    if(in_pMat[MATRIX_00] == 1){
        if(in_pMat[MATRIX_11] == 1) {
            return e_X_Y;
        }
        else{
            return e_X_NY;
        }
    } else if(in_pMat[MATRIX_00] == -1){
        if(in_pMat[MATRIX_11] == 1) {
            return e_NX_Y;
        }
        else{
            return e_NX_NY;
        }
    }

    if(in_pMat[MATRIX_01] == 1){
        if(in_pMat[MATRIX_10] == 1) {
            return e_F_X_Y;
        }
        else{
            return e_F_X_NY;
        }
    }
    else{
        if(in_pMat[MATRIX_10] == 1) {
            return e_F_NX_Y;
        }
        else{
            return e_F_NX_NY;
        }
    }
}

tmBuffer* gOrientationBuffer = NULL;

void        tmWhitePic (unsigned char* in_iBuffer, int length) {
    int row_count;
    int start_row = gVertex[VECTOR_Y];
    int start_col = gVertex[VECTOR_X];
    for (row_count = 0; row_count < gbb_size; row_count++) {
        memset(in_iBuffer + ( (start_row + row_count) * length + start_col)*PIXEL_SIZE,0,gbb_size);
    }
}

void tmWriteOrientationToBuffer(tmBuffer in_pBuffer,
                                int in_iDimension,
                                tmVec4i* in_pTopLeft,
                                int in_iBoundingBoxDimension,
                                tmOrientation in_eOrientation){
    int line_size_in_bytes = in_iDimension * PIXEL_SIZE;
    int total_offset = in_pTopLeft[VECTOR_Y]*line_size_in_bytes;
    int toal_x_offset = in_pTopLeft[VECTOR_X] * PIXEL_SIZE;
    int row = 0;
    for (row = 0; row < in_iBoundingBoxDimension; row++){
        memcpy(in_pBuffer+total_offset+row*line_size_in_bytes+toal_x_offset,
               gOrientationBuffer[in_eOrientation]+in_iBoundingBoxDimension*PIXEL_SIZE*row,
               in_iBoundingBoxDimension*PIXEL_SIZE);
    }
}

void tmBufferMirrorX(tmBuffer in_iInputBuffer, tmBuffer in_iOutputBuffer, int in_iSize){
    int row, col;
    int offset = 0;
    for (row = 0; row < in_iSize; row++){
        offset = row * in_iSize*PIXEL_SIZE;
        for (col = 0 ; col < in_iSize; col++){
            memcpy(in_iOutputBuffer+offset+(in_iSize-1-col)*PIXEL_SIZE,in_iInputBuffer+offset+col*PIXEL_SIZE, PIXEL_SIZE);
        }
    }
}

void tmBufferMirrorY(tmBuffer in_iInputBuffer, tmBuffer in_iOutputBuffer, int in_iSize){
    int start,end,line_size_in_Bytes;
    line_size_in_Bytes = in_iSize * PIXEL_SIZE;
    start = 0;
    end = (in_iSize-1)*in_iSize*PIXEL_SIZE;
    while(end == 0){
        memcpy(in_iOutputBuffer+end,in_iInputBuffer+start, line_size_in_Bytes);
        end -= line_size_in_Bytes;
        start += line_size_in_Bytes;
    }
}

void tmGenerateOrientationBuffer(tmBuffer in_pFrameBuffer,int in_iFrameDimension, int in_iBBDimension, tmOrientation in_eOrientation){

    if(gOrientationBuffer[in_eOrientation] == NULL){
        printf("Generate Buffer Orientation %d\n",in_eOrientation);
        gOrientationBuffer[in_eOrientation] = tmAlloc(unsigned char, in_iBBDimension*in_iBBDimension*PIXEL_SIZE);
    }
    else {
        return;
    }
    int line_size_in_bytes = in_iBBDimension * PIXEL_SIZE;
    tmBuffer src_buffer = gOrientationBuffer[e_X_Y];
    tmBuffer dst_buffer = gOrientationBuffer[in_eOrientation];
    switch(in_eOrientation){
        case e_X_Y: {
            int total_offset = gVertex[VECTOR_Y]*line_size_in_bytes;
            int toal_x_offset = gVertex[VECTOR_X] * PIXEL_SIZE;
            int row = 0;
            for (row = 0; row < in_iBBDimension; row++){
                memcpy(gOrientationBuffer[in_eOrientation]+in_iBBDimension*PIXEL_SIZE*row,
                       in_pFrameBuffer+total_offset+row*line_size_in_bytes+toal_x_offset,
                       in_iBBDimension*PIXEL_SIZE);
            }
            break;
        }
        case e_X_NY: {
            if(gOrientationBuffer[e_NX_NY]!=NULL){
                tmBufferMirrorX(gOrientationBuffer[e_NX_NY],gOrientationBuffer[e_X_NY],in_iBBDimension);
            }
            else{
                tmBufferMirrorY(gOrientationBuffer[e_X_Y],gOrientationBuffer[e_X_NY],in_iBBDimension);
            }
            break;
        }
        case e_NX_Y: {
            tmBufferMirrorX(gOrientationBuffer[e_X_Y],gOrientationBuffer[e_NX_Y],in_iBBDimension);
        }
        case e_NX_NY: {
            if (gOrientationBuffer[e_X_NY] != NULL) {
                tmBufferMirrorX(gOrientationBuffer[e_X_NY],gOrientationBuffer[e_NX_NY],in_iBBDimension);
            } else {
                int buffer_size = in_iBBDimension * in_iBBDimension*PIXEL_SIZE;
                int pixel_index;
                for (pixel_index = 0; pixel_index < buffer_size; pixel_index += 3) {
                    memcpy(gOrientationBuffer[in_eOrientation] + pixel_index, gOrientationBuffer[e_X_Y] + buffer_size -1 - pixel_index, PIXEL_SIZE);
                }
            }
            break;
        }
        case e_F_X_Y: {
            if(gOrientationBuffer[e_NX_Y]!=NULL){
                tmBufferMirrorX(gOrientationBuffer[e_NX_Y],gOrientationBuffer[e_X_Y],in_iBBDimension);
            }
            else{

                int f_row,f_col,t_row,t_col;
                int blk_col_size, blk_row_size;
                int src_x,src_y,src_index,dst_index;
                f_row = 0;
                f_col = 0;
                while(f_row < in_iBBDimension){
                    blk_row_size = tmMin(TILE_SIZE, in_iBBDimension-f_row);
                    while(f_col < in_iBBDimension){
                        blk_col_size = tmMin(TILE_SIZE, in_iBBDimension-f_col);
                        for(t_row = 0; t_row < blk_row_size; t_row++){
                            src_y = f_row+t_row;
                            for(t_col = 0 ; t_col < blk_col_size; t_col++){
                                src_x = f_col+t_col;
                                src_index = src_y*in_iBBDimension + src_x*PIXEL_SIZE;
                                dst_index = src_x*in_iBBDimension + src_y*PIXEL_SIZE;
                                dst_buffer[dst_index+R_OFFSET] = src_buffer[src_index+R_OFFSET];
                                dst_buffer[dst_index+G_OFFSET] = src_buffer[src_index+G_OFFSET];
                                dst_buffer[dst_index+B_OFFSET] = src_buffer[src_index+B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_row+=blk_row_size;
                }
            }
            break;
        }
        case e_F_X_NY: {
            if(gOrientationBuffer[e_F_X_Y]!=NULL){
                tmBufferMirrorY(gOrientationBuffer[e_F_X_Y],gOrientationBuffer[e_F_X_NY], in_iBBDimension);
            }
            else {
                int f_row, f_col, t_row, t_col;
                int blk_col_size, blk_row_size;
                int src_x, src_y, src_index, dst_index, dst_x, dst_y;
                f_row = 0;
                f_col = 0;
                while (f_row < in_iBBDimension) {
                    blk_row_size = tmMin(TILE_SIZE, in_iBBDimension - f_row);
                    while (f_col < in_iBBDimension) {
                        blk_col_size = tmMin(TILE_SIZE, in_iBBDimension - f_col);
                        for (t_row = 0; t_row < blk_row_size; t_row++) {
                            src_y = f_row + t_row;
                            for (t_col = 0; t_col < blk_col_size; t_col++) {
                                src_x = f_col + t_col;
                                dst_x = src_y;
                                dst_y = (in_iBBDimension - 1) - src_x;
                                src_index = src_y * in_iBBDimension + src_x * PIXEL_SIZE;
                                dst_index = dst_y * in_iBBDimension + dst_x * PIXEL_SIZE;
                                dst_buffer[dst_index + R_OFFSET] = src_buffer[src_index + R_OFFSET];
                                dst_buffer[dst_index + G_OFFSET] = src_buffer[src_index + G_OFFSET];
                                dst_buffer[dst_index + B_OFFSET] = src_buffer[src_index + B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_row += blk_row_size;
                }
            }
            break;
        }
        case e_F_NX_Y: {
            if(gOrientationBuffer[e_F_X_Y]!=NULL){
                tmBufferMirrorX(gOrientationBuffer[e_F_X_Y],gOrientationBuffer[e_F_NX_Y], in_iBBDimension);
            }
            else {
                int f_row, f_col, t_row, t_col;
                int blk_col_size, blk_row_size;
                int src_x, src_y, src_index, dst_index, dst_x, dst_y;
                f_row = 0;
                f_col = 0;
                while (f_row < in_iBBDimension) {
                    blk_row_size = tmMin(TILE_SIZE, in_iBBDimension - f_row);
                    while (f_col < in_iBBDimension) {
                        blk_col_size = tmMin(TILE_SIZE, in_iBBDimension - f_col);
                        for (t_row = 0; t_row < blk_row_size; t_row++) {
                            src_y = f_row + t_row;
                            for (t_col = 0; t_col < blk_col_size; t_col++) {
                                src_x = f_col + t_col;
                                dst_x = (in_iBBDimension - 1) - src_y;
                                dst_y = src_x;
                                src_index = src_y * in_iBBDimension + src_x * PIXEL_SIZE;
                                dst_index = dst_y * in_iBBDimension + dst_x * PIXEL_SIZE;
                                dst_buffer[dst_index + R_OFFSET] = src_buffer[src_index + R_OFFSET];
                                dst_buffer[dst_index + G_OFFSET] = src_buffer[src_index + G_OFFSET];
                                dst_buffer[dst_index + B_OFFSET] = src_buffer[src_index + B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_row += blk_row_size;
                }
            }
            break;
        }
        case e_F_NX_NY: {
            if(gOrientationBuffer[e_F_X_NY]!=NULL){
                tmBufferMirrorX(gOrientationBuffer[e_F_X_NY],gOrientationBuffer[e_F_NX_NY], in_iBBDimension);
            }
            else {
                int f_row, f_col, t_row, t_col;
                int blk_col_size, blk_row_size;
                int src_x, src_y, src_index, dst_index, dst_x, dst_y;
                f_row = 0;
                f_col = 0;
                while (f_row < in_iBBDimension) {
                    blk_row_size = tmMin(TILE_SIZE, in_iBBDimension - f_row);
                    while (f_col < in_iBBDimension) {
                        blk_col_size = tmMin(TILE_SIZE, in_iBBDimension - f_col);
                        for (t_row = 0; t_row < blk_row_size; t_row++) {
                            src_y = f_row + t_row;
                            for (t_col = 0; t_col < blk_col_size; t_col++) {
                                src_x = f_col + t_col;
                                dst_x = (in_iBBDimension - 1) - src_y;
                                dst_y = (in_iBBDimension - 1) - src_x;
                                src_index = src_y * in_iBBDimension + src_x * PIXEL_SIZE;
                                dst_index = dst_y * in_iBBDimension + dst_x * PIXEL_SIZE;
                                dst_buffer[dst_index + R_OFFSET] = src_buffer[src_index + R_OFFSET];
                                dst_buffer[dst_index + G_OFFSET] = src_buffer[src_index + G_OFFSET];
                                dst_buffer[dst_index + B_OFFSET] = src_buffer[src_index + B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_row += blk_row_size;
                }
            }
            break;
        }
        default:{
            break;
        }
    }
}


void tmInit(){
    if(gInited == 0) {
        // Initial global variable
        gGlobalCCW = tmAllocMat(eMatrixType_CCW90);
        gGlobalCW = tmAllocMat(eMatrixType_CW90);
        gGlobalMirrorXMatrix = tmAllocMat(eMatrixType_MirroX);
        gGlobalMirrorYMatrix = tmAllocMat(eMatrixType_MirroY);
        gGlobalR180 = tmAllocMat(eMatrixType_R180);
        gGlobalTransform = tmAllocMat(eMatrixType_Identity);
        gTempMul = tmAllocMat(eMatrixType_Identity);
        gTempVec1 = tmAllocVec();
        gTL = tmAlloc(tmVec4i, 1);
        gTR = tmAlloc(tmVec4i, 1);
        gBL = tmAlloc(tmVec4i, 1);
        gBR = tmAlloc(tmVec4i, 1);
        gVertex = tmAlloc(tmVec4i, 1);
        gOrientationBuffer = tmAlloc(tmBuffer, 8);
        memset(gOrientationBuffer, 0, sizeof(tmBuffer) * 8);
        gInited = 1;
    }
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
    tmFreeVec(gTL);
    tmFreeVec(gBL);
    tmFreeVec(gTR);
    tmFreeVec(gBR);
    tmFreeVec(gVertex);
    tmFreeVec(gTempVec1);
//    for(int i = 0; i < 8; i++){
//        if(gOrientationBuffer[i]!=NULL){
//            free(gOrientationBuffer[i]);
//            gOrientationBuffer[i] = NULL;
//        }
//
//    }
    free(gOrientationBuffer);

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
            tmMatMulMatInplace(gGlobalCW, gGlobalTransform);
            break;
        }
        case(2):{
            tmMatMulMatInplace(gGlobalR180, gGlobalTransform);
            break;
        }
        case(3):{
            tmMatMulMatInplace(gGlobalCCW, gGlobalTransform);
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
            tmMatMulMatInplace(gGlobalCCW, gGlobalTransform);
            break;
        }
        case(2):{
            tmMatMulMatInplace(gGlobalR180, gGlobalTransform);
            break;
        }
        case(3):{
            tmMatMulMatInplace(gGlobalCW, gGlobalTransform);
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
    tmMatMulMatInplace(gGlobalMirrorXMatrix, gGlobalTransform);
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
    tmMatMulMatInplace(gGlobalMirrorYMatrix, gGlobalTransform);
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
    tmUpdateBoundingBox(frame_buffer, height*height, height);
    tmGenerateOrientationBuffer(frame_buffer,width, gbb_size,e_X_Y);
    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
#ifdef SPEED_UP
        //        printf("Processing sensor value #%d: %s, %d\n", sensorValueIdx, sensor_values[sensorValueIdx].key,
//               sensor_values[sensorValueIdx].value);
        if (!strcmp(sensor_values[sensorValueIdx].key, "W")) {
            processMoveUp(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "A")) {
            processMoveLeft(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "S")) {
            processMoveDown(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "D")) {
            processMoveRight(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "CW")) {
            processRotateCW(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "CCW")) {
            processRotateCCW(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MX")) {
            processMirrorX(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MY")) {
            processMirrorY(frame_buffer, width, height, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        }
#else
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
#endif

        processed_frames += 1;
        if (processed_frames % 25 == 0) {
            tmPrintMat(gGlobalTransform);
            tmOrientation orientation  = tmGetOrientationFromMat(gGlobalTransform);
            if(gOrientationBuffer[orientation] == NULL){
                tmGenerateOrientationBuffer(NULL,width,gbb_size,orientation);
            }
            tmWhitePic(frame_buffer,width);
            tmUpdateVertex(gGlobalTransform,width);
            tmWriteOrientationToBuffer(frame_buffer,width,gVertex,gbb_size,orientation);
            //tmTransformTiledMemory(gGlobalTiledMemory, gGlobalTransform);
            //tmTiledMemoryToFrame(frame_buffer,width*height,gGlobalTiledMemory);
            verifyFrame(frame_buffer, width, height, grading_mode);
        }
    }
    return;
}
