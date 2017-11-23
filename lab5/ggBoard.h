#ifndef _ggBoardUtil_h
#define _ggBoardUtil_h
#include "ggTypes.h"
// Some utilities function for manipulating the board

typedef struct _ggBoard {
    char* m_pBoardData;
    ggeQuadSize m_eBoardSize;
    struct __ggBoard* m_pFutureBoard;
} ggBoard;

int ggCompareBoard(ggBoard* in_pBoardA, ggBoard* in_pBaordB);
void ggGenerateFuture(ggBoard* in_pBoard);
void ggFillBoard(ggBoard* in_pSrcBoard,ggBoard* in_pTargetBoard, ggPosition in_pPos);
void ggCropBoard(ggBoard* in_pSrcBoard,ggBoard* in_pTargetBoard, ggPosition in_pPose);

#endif