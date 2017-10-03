#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

#define ENABLE_SIMD 0
#define SPEED_UP 1

#define TILE_SIZE 160
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
#define COLOR_WHITE 255

///////////// Matrix Operation ////////////////////
unsigned char *processRotateCCW(unsigned char *buffer_frame, unsigned width, unsigned height,
                                int rotate_iteration);
typedef int tmMat4i;
typedef int tmVec4i;
typedef unsigned char* tmBuffer;

typedef enum _tmMatrixType{
    eMatrixType_Identity,
    eMatrixType_CCW90,
    eMatrixType_CW90,
    eMatrixType_R180,
    eMatrixType_MirroX,
    eMatrixType_MirroY
} tmMatFlag;
typedef struct _tmBoundingCache{
    tmBuffer m_pBuffer;
    int m_iWidth;
    int m_iLength;
} tmFrameCache;

int gInited = 0;

#define tmMin(a,b) (((a)>(b))?(b):(a))
#define tmMax(a,b) (((a)>(b))?(a):(b))
#define tmMin4(a,b,c,d) tmMin(tmMin(tmMin(a,b),c),d)
#define tmMax4(a,b,c,d) tmMax(tmMax(tmMax(a,b),c),d)

#define tmCopyPixel(src,dst,src_i,dst_i) (*(unsigned int*)(src+src_i)) |= (*((unsigned int*)(dst+dst_i)) & 0xffffff00)


tmMat4i* gTempMul = NULL;
tmMat4i* gGlobalCW = NULL;
tmMat4i* gGlobalCCW = NULL;
tmMat4i* gGlobalMirrorYMatrix = NULL;
tmMat4i* gGlobalMirrorXMatrix = NULL;
tmMat4i* gGlobalR180 = NULL;
tmMat4i* gGlobalTranslate = NULL;
tmMat4i* gGlobalTransform = NULL;


tmVec4i* gTL= NULL;
tmVec4i* gTR= NULL;
tmVec4i* gBL = NULL;
tmVec4i* gBR = NULL;
tmVec4i* gTL_clean= NULL;
tmVec4i* gTR_clean= NULL;
tmVec4i* gBL_clean = NULL;
tmVec4i* gBR_clean = NULL;

void        tmPrintVec(tmVec4i* in_pVec){
    printf("%d,%d,%d\n",in_pVec[VECTOR_X],in_pVec[VECTOR_Y],in_pVec[VECTOR_Z]);
}
void        tmPrintMat(tmMat4i* in_pMat){
    printf("%d,%d,%d\n",in_pMat[MATRIX_00],in_pMat[MATRIX_01],in_pMat[MATRIX_02]);
    printf("%d,%d,%d\n",in_pMat[MATRIX_10],in_pMat[MATRIX_11],in_pMat[MATRIX_12]);
    printf("%d,%d,%d\n",in_pMat[MATRIX_20],in_pMat[MATRIX_21],in_pMat[MATRIX_22]);

}
void        tmUpdateBoundingBox (unsigned char* in_iBuffer, int in_iFrameDimension){
    int pixel_index;
    int x_min=in_iFrameDimension-1;
    int x_max=0;
    int y_min=in_iFrameDimension-1;
    int y_max=0;
    int x_temp = 0;
    int y_temp = 0;
    int pixel_offset = 0;
    int total_pixel_count = in_iFrameDimension*in_iFrameDimension;
    for (pixel_index = 0; pixel_index < total_pixel_count; pixel_index++) {
        pixel_offset = pixel_index*PIXEL_SIZE;

        if (in_iBuffer[pixel_offset] != COLOR_WHITE || in_iBuffer[pixel_offset+1] != COLOR_WHITE || in_iBuffer[pixel_offset+2] != COLOR_WHITE ) {
        
            y_temp = pixel_index/in_iFrameDimension;
            x_temp = pixel_index%in_iFrameDimension;
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

    gTL[VECTOR_X]= x_min;
    gTL[VECTOR_Y]= y_min;
    gTR[VECTOR_X]= x_max;
    gTR[VECTOR_Y]= y_min;
    gBL[VECTOR_X]= x_min;
    gBL[VECTOR_Y]= y_max;
    gBR[VECTOR_X]= x_max;
    gBR[VECTOR_Y]= y_max;
    gTL_clean[VECTOR_X]= x_min;
    gTL_clean[VECTOR_Y]= y_min;
    gTR_clean[VECTOR_X]= x_max;
    gTR_clean[VECTOR_Y]= y_min;
    gBL_clean[VECTOR_X]= x_min;
    gBL_clean[VECTOR_Y]= y_max;
    gBR_clean[VECTOR_X]= x_max;
    gBR_clean[VECTOR_Y]= y_max;
}
void        tmCopyMat(tmMat4i* in_pFrom, tmMat4i* in_pTo){
    memcpy(in_pTo,in_pFrom,sizeof(int)*16);
}
void        tmCopyVec(tmVec4i* in_pFrom, tmVec4i* in_pTo){
    in_pTo[VECTOR_X] = in_pFrom[VECTOR_X];
    in_pTo[VECTOR_Y] = in_pFrom[VECTOR_Y];
    in_pTo[VECTOR_Z] = in_pFrom[VECTOR_Z];
}
void        tmCleanVec(tmVec4i* in_pA){
    in_pA[VECTOR_X] = 0;
    in_pA[VECTOR_Y] = 0;
    in_pA[VECTOR_Z] = 1;

}

void        tmMatMulVec(tmMat4i* in_pA, tmVec4i* in_pB, tmVec4i* io_pC){
#if ENABLE_SIMD

#else
    io_pC[VECTOR_X] = in_pA[MATRIX_00]*in_pB[VECTOR_X] + in_pA[MATRIX_01]*in_pB[VECTOR_Y] + in_pA[MATRIX_02]*in_pB[VECTOR_Z];
    io_pC[VECTOR_Y] = in_pA[MATRIX_10]*in_pB[VECTOR_X] + in_pA[MATRIX_11]*in_pB[VECTOR_Y] + in_pA[MATRIX_12]*in_pB[VECTOR_Z];
    io_pC[VECTOR_Z] = 1;
#endif

}
void        tmMatMulMat(tmMat4i* in_pA, tmMat4i* in_pB, tmMat4i* io_pC){
#if ENABLE_SIMD

#else
    io_pC[MATRIX_00] = in_pA[MATRIX_00]*in_pB[MATRIX_00] + in_pA[MATRIX_01]*in_pB[MATRIX_10];
    io_pC[MATRIX_01] = in_pA[MATRIX_00]*in_pB[MATRIX_01] + in_pA[MATRIX_01]*in_pB[MATRIX_11];
    io_pC[MATRIX_02] = in_pA[MATRIX_00]*in_pB[MATRIX_02] + in_pA[MATRIX_01]*in_pB[MATRIX_12] + in_pA[MATRIX_02];
    io_pC[MATRIX_10] = in_pA[MATRIX_10]*in_pB[MATRIX_00] + in_pA[MATRIX_11]*in_pB[MATRIX_10];
    io_pC[MATRIX_11] = in_pA[MATRIX_10]*in_pB[MATRIX_01] + in_pA[MATRIX_11]*in_pB[MATRIX_11];
    io_pC[MATRIX_12] = in_pA[MATRIX_10]*in_pB[MATRIX_02] + in_pA[MATRIX_11]*in_pB[MATRIX_12] + in_pA[MATRIX_12];
#endif
}
void        tmMatMulMatInplace2(tmMat4i* in_pA, tmMat4i** io_pB){
    tmMatMulMat(in_pA,*io_pB,gTempMul);
    // swap the matrix
    tmMat4i* currentGlobal = gTempMul;
    gTempMul = *io_pB;
    *io_pB = currentGlobal;
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
    ret[VECTOR_X] = 0;
    ret[VECTOR_Y] = 0;
    ret[VECTOR_Z] = 1;
    return ret;
}
void        tmFreeVec(tmVec4i* in_pA){
    free(in_pA);
}



typedef enum _tmOrientation {
    e_X_Y = 0,
    e_X_NY = 1,
    e_NX_Y = 2,
    e_NX_NY = 3,
    e_F_X_Y = 4,
    e_F_X_NY = 5,
    e_F_NX_Y = 6,
    e_F_NX_NY = 7,
    eOrientationCount = 8
} tmOrientation;

void tmUpdateVertex (tmMat4i* in_pMat, int length) {
    //tmPrintMat(in_pMat);
    //tmPrintVec(gTL);
//5
    tmMatMulVec(in_pMat, gTL, gTL_clean);
    tmMatMulVec(in_pMat, gTR, gTR_clean);
    tmMatMulVec(in_pMat, gBL, gBL_clean);
    tmMatMulVec(in_pMat, gBR, gBR_clean);
    //tmPrintVec(gTL_clean);
    if (in_pMat[MATRIX_00] < 0||in_pMat[MATRIX_01]< 0) {
        gTL_clean[VECTOR_X]+=length-1;
        gTR_clean[VECTOR_X]+=length-1;
        gBR_clean[VECTOR_X]+=length-1;
        gBL_clean[VECTOR_X]+=length-1;
    }
    if (in_pMat[MATRIX_10] < 0||in_pMat[MATRIX_11]< 0) {
        gTL_clean[VECTOR_Y]+=length-1;
        gTR_clean[VECTOR_Y]+=length-1;
        gBR_clean[VECTOR_Y]+=length-1;
        gBL_clean[VECTOR_Y]+=length-1;
    }
//
//    if (gTL_clean[VECTOR_X] < 0 || gTR_clean[VECTOR_X] < 0 || gBL_clean[VECTOR_X] < 0 || gBR_clean[VECTOR_X] <0){
//        gTL_clean[VECTOR_X]+=length-1;
//        gTR_clean[VECTOR_X]+=length-1;
//        gBR_clean[VECTOR_X]+=length-1;
//        gBL_clean[VECTOR_X]+=length-1;
//    }
//    if (gTL_clean[VECTOR_Y] < 0 || gTR_clean[VECTOR_Y] < 0 || gBL_clean[VECTOR_Y] < 0 || gBR_clean[VECTOR_Y] <0){
//        gTL_clean[VECTOR_Y]+=length-1;
//        gTR_clean[VECTOR_Y]+=length-1;
//        gBR_clean[VECTOR_Y]+=length-1;
//        gBL_clean[VECTOR_Y]+=length-1;
//    }
//    gTL_clean[VECTOR_X]+=shift_x;
//    gTR_clean[VECTOR_X]+=shift_x;
//    gBL_clean[VECTOR_X]+=shift_x;
//    gBR_clean[VECTOR_X]+=shift_x;
//    gTL_clean[VECTOR_Y]+=shift_y;
//    gTR_clean[VECTOR_Y]+=shift_y;
//    gBL_clean[VECTOR_Y]+=shift_y;
//    gBR_clean[VECTOR_Y]+=shift_y;

    int xmin,xmax,ymin,ymax;
    xmin = tmMin4(gTL_clean[VECTOR_X],gTR_clean[VECTOR_X],gBL_clean[VECTOR_X],gBR_clean[VECTOR_X]);
    xmax = tmMax4(gTL_clean[VECTOR_X],gTR_clean[VECTOR_X],gBL_clean[VECTOR_X],gBR_clean[VECTOR_X]);
    ymin = tmMin4(gTL_clean[VECTOR_Y],gTR_clean[VECTOR_Y],gBL_clean[VECTOR_Y],gBR_clean[VECTOR_Y]);
    ymax = tmMax4(gTL_clean[VECTOR_Y],gTR_clean[VECTOR_Y],gBL_clean[VECTOR_Y],gBR_clean[VECTOR_Y]);
    gTL_clean[VECTOR_X] = xmin;
    gTL_clean[VECTOR_Y] = ymin;
    gTR_clean[VECTOR_X] = xmax;
    gTR_clean[VECTOR_Y] = ymin;
    gBL_clean[VECTOR_X] = xmin;
    gBL_clean[VECTOR_Y] = ymax;
    gBR_clean[VECTOR_X] = xmax;
    gBR_clean[VECTOR_Y] = ymax;
//    in_pMat[MATRIX_INDEX_TRANSFORM_X] = shift_x;
//    in_pMat[MATRIX_INDEX_TRANSFORM_Y] = shift_y;

}
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


tmFrameCache* gFrameCache = NULL;

void tmPrintFrame(tmBuffer in_pBuffer, int in_iWidth, int in_iLength){
    printf("++++=====================================\n");
    int i,j;
    for(i = 0; i < in_iLength; i++){
        for(j = 0; j < in_iWidth*PIXEL_SIZE; j++){
            printf("%d ",in_pBuffer[(i*in_iWidth)*PIXEL_SIZE+j]);
        }
        printf("\n");
    }
    printf("\n");
}

void        tmClearFrame(unsigned char *in_iBuffer,int in_iFrameDimension, tmVec4i *in_pPoint, int in_iWidth, int in_iLength) {
    int row_count;
    // tmPrintVec(in_pPoint);
    for (row_count = 0; row_count < in_iLength; row_count++) {
        memset(in_iBuffer + ( (in_pPoint[VECTOR_Y] + row_count) * in_iFrameDimension + in_pPoint[VECTOR_X])*PIXEL_SIZE,COLOR_WHITE,in_iWidth*PIXEL_SIZE);
    }
}

void tmWriteBufferToFrame(tmBuffer io_dstBuffer,
                          int in_iDimension,
                          int in_idstX,int in_idstY,
                          tmBuffer in_srcBuffer,int in_iSrcWidth, int in_iSrcLength){
    int line_size_in_bytes = in_iDimension * PIXEL_SIZE;
    int total_offset = in_idstY*line_size_in_bytes;
    int toal_x_offset = in_idstX* PIXEL_SIZE;
    int row;
    for (row = 0; row < in_iSrcLength; row++){
        memcpy(io_dstBuffer+total_offset+row*line_size_in_bytes+toal_x_offset,
               in_srcBuffer+in_iSrcWidth*PIXEL_SIZE*row,
               in_iSrcWidth*PIXEL_SIZE);
    }
}

void tmWriteFrameFromCache(tmBuffer in_pBuffer,
                           int in_iDimension,
                           tmVec4i *in_pTopLeft,
                           tmOrientation in_eOrientation){
    tmWriteBufferToFrame(
            in_pBuffer,
            in_iDimension,
            in_pTopLeft[VECTOR_X],in_pTopLeft[VECTOR_Y],
            gFrameCache[in_eOrientation].m_pBuffer,
            gFrameCache[in_eOrientation].m_iWidth,
            gFrameCache[in_eOrientation].m_iLength);

}

void tmBufferMirrorY(tmBuffer in_iInputBuffer, tmBuffer in_iOutputBuffer, int in_iWidth, int in_iLength){
    int row, col;
    int offset = 0;
    for (row = 0; row < in_iLength; row++){
        offset = row * in_iWidth*PIXEL_SIZE;
        for (col = 0 ; col < in_iWidth; col++){
            memcpy(in_iOutputBuffer+offset+(in_iWidth-1-col)*PIXEL_SIZE,in_iInputBuffer+offset+col*PIXEL_SIZE, PIXEL_SIZE);
        }
    }
    //tmPrintFrame(in_iOutputBuffer,in_iWidth,in_iLength);
}

void tmBufferMirrorX(tmBuffer in_iInputBuffer, tmBuffer in_iOutputBuffer, int in_iWidth, int in_iLength){
    int start,end,line_size_in_Bytes;
    line_size_in_Bytes = in_iWidth * PIXEL_SIZE;
    start = 0;
    end = (in_iLength)*in_iWidth*PIXEL_SIZE;
    while(end != 0){
        end -= line_size_in_Bytes;
        memcpy(in_iOutputBuffer+end,in_iInputBuffer+start, line_size_in_Bytes);
        start += line_size_in_Bytes;
    }
}


int tmGetCurrentBoundingBoxWidth(){
    return gBR_clean[VECTOR_X] - gTL_clean[VECTOR_X]+1;
}

int tmGetCurrentBoundingBoxLength(){
    return gBR_clean[VECTOR_Y] - gTL_clean[VECTOR_Y]+1;
}

void tmGenerateBaseBuffer(tmBuffer in_pFrameBuffer, int in_iFrameDimension){
    if(gFrameCache[e_X_Y].m_pBuffer == NULL) {
        int base_Width = tmGetCurrentBoundingBoxWidth();
        int base_length = tmGetCurrentBoundingBoxLength();
        gFrameCache[e_X_Y].m_pBuffer = tmAlloc(unsigned char, base_length * base_Width * PIXEL_SIZE);

        //tmPrintFrame(in_pFrameBuffer,in_iFrameDimension,in_iFrameDimension);
        gFrameCache[e_X_Y].m_iWidth = base_Width;
        gFrameCache[e_X_Y].m_iLength = base_length;
        int line_size_in_bytes = in_iFrameDimension*PIXEL_SIZE;
        int row, total_y_offset,total_x_offset,total_offset;
        total_y_offset = gTL[VECTOR_Y] * line_size_in_bytes;
        total_x_offset = gTL[VECTOR_X] * PIXEL_SIZE;
        total_offset = total_x_offset+total_y_offset;
        base_Width*=PIXEL_SIZE;
        for(row = 0; row < base_length;row++){
            memcpy(gFrameCache[e_X_Y].m_pBuffer+row*base_Width,
                   in_pFrameBuffer+total_offset+row*line_size_in_bytes,
                   base_Width);
        }
        //tmPrintFrame(gFrameCache[e_X_Y].m_pBuffer,base_Width,base_length);
    }
}

void tmGenerateOrientationBuffer(tmOrientation in_eOrientation){
    if(gFrameCache[in_eOrientation].m_pBuffer == NULL){
        //printf("Generate Buffer Orientation %d\n",in_eOrientation);
        int size = gFrameCache[e_X_Y].m_iWidth * gFrameCache[e_X_Y].m_iLength;
        gFrameCache[in_eOrientation].m_pBuffer = tmAlloc(unsigned char, size*PIXEL_SIZE);
        memset(gFrameCache[in_eOrientation].m_pBuffer,0,size*PIXEL_SIZE);
    }
    else {
        return;
    }
    tmBuffer src_buffer,dst_buffer;
    int width,length;
    switch(in_eOrientation){
        case e_X_Y: {
            break;
        }
        case e_X_NY: {
            dst_buffer = gFrameCache[e_X_NY].m_pBuffer;
            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iWidth;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iLength;
            tmBufferMirrorX(gFrameCache[e_X_Y].m_pBuffer,dst_buffer,gFrameCache[e_X_NY].m_iWidth,gFrameCache[e_X_NY].m_iLength);
            break;
        }
        case e_NX_Y: {
            dst_buffer = gFrameCache[e_NX_Y].m_pBuffer;
            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iWidth;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iLength;
            if (gFrameCache[e_NX_NY].m_pBuffer!=NULL){
                tmBufferMirrorX(gFrameCache[e_NX_NY].m_pBuffer,dst_buffer,gFrameCache[e_NX_Y].m_iWidth,gFrameCache[e_NX_Y].m_iLength);
            } else {
                tmBufferMirrorY(gFrameCache[e_X_Y].m_pBuffer,dst_buffer,gFrameCache[e_NX_Y].m_iWidth,gFrameCache[e_NX_Y].m_iLength);
            }
            break;
        }
        case e_NX_NY: {
            dst_buffer = gFrameCache[e_NX_NY].m_pBuffer;

            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iWidth;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iLength;
//            if (gFrameCache[e_NX_Y].m_pBuffer != NULL) {
//                src_buffer = gFrameCache[e_NX_Y].m_pBuffer;
//                tmBufferMirrorX(src_buffer,dst_buffer,gFrameCache[e_NX_NY].m_iWidth,gFrameCache[e_NX_NY].m_iLength);
//            } else {
                src_buffer = gFrameCache[e_X_Y].m_pBuffer;
                int buffer_size = gFrameCache[e_X_Y].m_iLength*gFrameCache[e_X_Y].m_iWidth*PIXEL_SIZE;
                int pixel_index,offset;
                offset = buffer_size-PIXEL_SIZE;
                for (pixel_index = 0; pixel_index < buffer_size; pixel_index += 3) {
                    memcpy(dst_buffer + pixel_index, src_buffer+ (offset) - pixel_index, PIXEL_SIZE);
                }
//            }

            break;
        }
        case e_F_X_Y: {
            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iLength;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iWidth;
            width = gFrameCache[in_eOrientation].m_iWidth;
            length = gFrameCache[in_eOrientation].m_iLength;
            src_buffer = gFrameCache[e_F_X_NY].m_pBuffer;
            dst_buffer = gFrameCache[e_F_X_Y].m_pBuffer;
//            if(src_buffer!=NULL){
//                tmBufferMirrorX(src_buffer,dst_buffer,width,length);
//            }
//            else{
                src_buffer = gFrameCache[e_X_Y].m_pBuffer;
                int f_row,f_col,t_row,t_col;
                int blk_col_size, blk_row_size;
                int src_x,src_y,src_index,dst_index;
                f_row = 0;
                f_col = 0;
                width = gFrameCache[e_X_Y].m_iWidth;
                length = gFrameCache[e_X_Y].m_iLength;
                while(f_row < length){
                    blk_row_size = tmMin(TILE_SIZE, length-f_row);
                    while(f_col < width){
                        blk_col_size = tmMin(TILE_SIZE, width-f_col);
                        for(t_row = 0; t_row < blk_row_size; t_row++){
                            src_y = f_row+t_row;
                            for(t_col = 0 ; t_col < blk_col_size; t_col++){
                                src_x = f_col+t_col;
                                src_index = (src_y*width + src_x)*PIXEL_SIZE;
                                dst_index = (src_x*length+ src_y)*PIXEL_SIZE;
                                dst_buffer[dst_index+R_OFFSET] = src_buffer[src_index+R_OFFSET];
                                dst_buffer[dst_index+G_OFFSET] = src_buffer[src_index+G_OFFSET];
                                dst_buffer[dst_index+B_OFFSET] = src_buffer[src_index+B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_col = 0;
                    f_row+=blk_row_size;
                }
//            }
            break;
        }
        case e_F_X_NY: {
            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iLength;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iWidth;
            width = gFrameCache[in_eOrientation].m_iWidth;
            length = gFrameCache[in_eOrientation].m_iLength;
            src_buffer = gFrameCache[e_F_X_Y].m_pBuffer;
            dst_buffer = gFrameCache[in_eOrientation].m_pBuffer;
//            if(src_buffer!=NULL){
//                tmBufferMirrorX(src_buffer,dst_buffer, width,length);
//            }
//            else {
                src_buffer = gFrameCache[e_X_Y].m_pBuffer;
                int f_row, f_col, t_row, t_col;
                int blk_col_size, blk_row_size;
                int src_x, src_y, src_index, dst_index, dst_x, dst_y;
                f_row = 0;
                f_col = 0;
                width = gFrameCache[e_X_Y].m_iWidth;
                length = gFrameCache[e_X_Y].m_iLength;
                while (f_row < length) {
                    blk_row_size = tmMin(TILE_SIZE, length- f_row);
                    while (f_col < width) {
                        blk_col_size = tmMin(TILE_SIZE, width- f_col);
                        for (t_row = 0; t_row < blk_row_size; t_row++) {
                            src_y = f_row + t_row;
                            dst_x = src_y;
                            for (t_col = 0; t_col < blk_col_size; t_col++) {
                                src_x = f_col + t_col;
                                dst_y = (width - 1) - src_x;
                                src_index = (src_y * width+ src_x) * PIXEL_SIZE;
                                dst_index = (dst_y * length+ dst_x) * PIXEL_SIZE;

                                //tmCopyPixel(src_buffer,dst_buffer,src_index,dst_index);
                                dst_buffer[dst_index + R_OFFSET] = src_buffer[src_index + R_OFFSET];
                                dst_buffer[dst_index + G_OFFSET] = src_buffer[src_index + G_OFFSET];
                                dst_buffer[dst_index + B_OFFSET] = src_buffer[src_index + B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_col = 0;
                    f_row += blk_row_size;
//                }
            }
            break;
        }
        case e_F_NX_Y: {
            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iLength;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iWidth;
            width = gFrameCache[in_eOrientation].m_iWidth;
            length = gFrameCache[in_eOrientation].m_iLength;
            src_buffer = gFrameCache[e_F_NX_NY].m_pBuffer;
            dst_buffer = gFrameCache[e_F_NX_Y].m_pBuffer;
//            if(src_buffer!=NULL){
//                tmBufferMirrorX(src_buffer,dst_buffer, width,length);
//            }
//            else {
                src_buffer = gFrameCache[e_X_Y].m_pBuffer;
                int f_row, f_col, t_row, t_col;
                int blk_col_size, blk_row_size;
                int src_x, src_y, src_index, dst_index, dst_x, dst_y;
                f_row = 0;
                f_col = 0;
                width = gFrameCache[e_X_Y].m_iWidth;
                length = gFrameCache[e_X_Y].m_iLength;
                while (f_row < length) {
                    blk_row_size = tmMin(TILE_SIZE, length - f_row);
                    while (f_col < width) {
                        blk_col_size = tmMin(TILE_SIZE, width - f_col);
                        for (t_row = 0; t_row < blk_row_size; t_row++) {
                            src_y = f_row + t_row;
                            dst_x = (length - 1) - src_y;
                            for (t_col = 0; t_col < blk_col_size; t_col++) {
                                src_x = f_col + t_col;
                                dst_y = src_x;
                                src_index = (src_y * width + src_x) * PIXEL_SIZE;
                                dst_index = (dst_y * length + dst_x) * PIXEL_SIZE;
                                dst_buffer[dst_index + R_OFFSET] = src_buffer[src_index + R_OFFSET];
                                dst_buffer[dst_index + G_OFFSET] = src_buffer[src_index + G_OFFSET];
                                dst_buffer[dst_index + B_OFFSET] = src_buffer[src_index + B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_col = 0;
                    f_row += blk_row_size;
                }
            //}
            //tmPrintFrame(dst_buffer,gFrameCache[in_eOrientation].m_iWidth,gFrameCache[in_eOrientation].m_iLength);
            break;
        }
        case e_F_NX_NY: {
            gFrameCache[in_eOrientation].m_iWidth = gFrameCache[e_X_Y].m_iLength;
            gFrameCache[in_eOrientation].m_iLength = gFrameCache[e_X_Y].m_iWidth;
            width = gFrameCache[in_eOrientation].m_iWidth;
            length = gFrameCache[in_eOrientation].m_iLength;
            src_buffer = gFrameCache[e_F_NX_Y].m_pBuffer;
            dst_buffer = gFrameCache[e_F_NX_NY].m_pBuffer;
//            if(src_buffer!=NULL){
//                tmBufferMirrorX(src_buffer,dst_buffer, width,length);
//            }
//            else {
                src_buffer = gFrameCache[e_X_Y].m_pBuffer;
                int f_row, f_col, t_row, t_col;
                int blk_col_size, blk_row_size;
                int src_x, src_y, src_index, dst_index, dst_x, dst_y;
                f_row = 0;
                f_col = 0;
                width = gFrameCache[e_X_Y].m_iWidth;
                length = gFrameCache[e_X_Y].m_iLength;
                while (f_row < length) {
                    blk_row_size = tmMin(TILE_SIZE, length- f_row);
                    while (f_col < width) {
                        blk_col_size = tmMin(TILE_SIZE, width- f_col);
                        for (t_row = 0; t_row < blk_row_size; t_row++) {
                            src_y = f_row + t_row;
                            for (t_col = 0; t_col < blk_col_size; t_col++) {
                                src_x = f_col + t_col;
                                dst_x = (length - 1) - src_y;
                                dst_y = (width- 1) - src_x;
                                src_index = (src_y * width+ src_x) * PIXEL_SIZE;
                                dst_index = (dst_y * length+ dst_x) * PIXEL_SIZE;
                                dst_buffer[dst_index + R_OFFSET] = src_buffer[src_index + R_OFFSET];
                                dst_buffer[dst_index + G_OFFSET] = src_buffer[src_index + G_OFFSET];
                                dst_buffer[dst_index + B_OFFSET] = src_buffer[src_index + B_OFFSET];
                            }
                        }
                        f_col += blk_col_size;
                    }
                    f_col = 0;
                    f_row += blk_row_size;
                }
            //}
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
        gGlobalTranslate = tmAllocMat(eMatrixType_Identity);
        gTL = tmAllocVec();
        gTR = tmAllocVec();
        gBL = tmAllocVec();
        gBR = tmAllocVec();
        gTL_clean = tmAllocVec();
        gTR_clean = tmAllocVec();
        gBL_clean = tmAllocVec();
        gBR_clean = tmAllocVec();
        gFrameCache = tmAlloc(tmFrameCache,eOrientationCount);
        int i =0;
        for(i = 0; i < eOrientationCount; i++){
            gFrameCache[i].m_pBuffer = NULL;
            gFrameCache[i].m_iWidth = 0;
            gFrameCache[i].m_iLength = 0;
        }

    }
    else{
        tmLoadMat(gGlobalTransform,eMatrixType_Identity);
        tmLoadMat(gTempMul,eMatrixType_Identity);
        tmCopyVec(gTL,gTL_clean);
        tmCopyVec(gTR,gTR_clean);
        tmCopyVec(gBL,gBL_clean);
        tmCopyVec(gBR,gBR_clean);
    }
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
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_X] = 0;
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_Y] = -offset;
    tmMatMulMatInplace2(gGlobalTranslate,&gGlobalTransform);
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
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_X] = offset;
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_Y] = 0;
    tmMatMulMatInplace2(gGlobalTranslate,&gGlobalTransform);
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
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_X] = 0;
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_Y] = offset;
    tmMatMulMatInplace2(gGlobalTranslate,&gGlobalTransform);
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
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_X] = -offset;
    gGlobalTranslate[MATRIX_INDEX_TRANSFORM_Y] = 0;
    tmMatMulMatInplace2(gGlobalTranslate,&gGlobalTransform);
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
    if(rotate_iteration<0){
        processRotateCCW(buffer_frame,width,height,-rotate_iteration);
    }
    else{
        switch(rotate_iteration % 4){
            case(0):{
                break;
            }
            case(1):{
                tmMatMulMatInplace2(gGlobalCW, &gGlobalTransform);
                break;
            }
            case(2):{
                tmMatMulMatInplace2(gGlobalR180, &gGlobalTransform);
                break;
            }
            case(3):{
                tmMatMulMatInplace2(gGlobalCCW, &gGlobalTransform);
                break;
            }
            default:{
                break;
            }
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
    if(rotate_iteration < 0 ){
        processRotateCW(buffer_frame,width,height,-rotate_iteration);
    }
    else {
        switch (rotate_iteration % 4) {
            case (0): {
                break;
            }
            case (1): {
                tmMatMulMatInplace2(gGlobalCCW, &gGlobalTransform);
                break;
            }
            case (2): {
                tmMatMulMatInplace2(gGlobalR180, &gGlobalTransform);
                break;
            }
            case (3): {
                tmMatMulMatInplace2(gGlobalCW, &gGlobalTransform);
                break;
            }
            default: {
                break;
            }
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
    tmMatMulMatInplace2(gGlobalMirrorXMatrix, &gGlobalTransform);
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
    tmMatMulMatInplace2(gGlobalMirrorYMatrix, &gGlobalTransform);
    return NULL;
#endif
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "NSFW";

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
    int bb_width,bb_length;
    if(gInited!=1){
        tmInit(frame_buffer, width*height);
        tmUpdateBoundingBox(frame_buffer, height);
        gInited = 1;
    }
    else{
        tmInit(frame_buffer, width*height);
    }

    // Allocate global frame
    //tmUpdateBoundingBox(frame_buffer, height);
    tmGenerateBaseBuffer(frame_buffer,width);
    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
#ifdef SPEED_UP
//               printf("Processing sensor value #%d: %s, %d\n", sensorValueIdx, sensor_values[sensorValueIdx].key,
//              sensor_values[sensorValueIdx].value);
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
        //tmPrintMat(gGlobalTransform);
        processed_frames += 1;
        if (processed_frames % 25 == 0) {
            tmOrientation orientation  = tmGetOrientationFromMat(gGlobalTransform);
            //printf("Orientation: %d\n",orientation);
            if(gFrameCache[orientation].m_pBuffer == NULL){
                tmGenerateOrientationBuffer(orientation);
            }
            bb_width = tmGetCurrentBoundingBoxWidth();
            bb_length = tmGetCurrentBoundingBoxLength();

            tmClearFrame(frame_buffer,width,gTL_clean,bb_width,bb_length);

            tmUpdateVertex(gGlobalTransform,width);

            tmWriteFrameFromCache(frame_buffer, width, gTL_clean,orientation);
            //tmPrintFrame(frame_buffer,width,height);
            verifyFrame(frame_buffer, width, height, grading_mode);
        }
    }
    //printf("Terminate\n");
    return;
}
