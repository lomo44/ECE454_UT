#ifndef _ggTypes_h
#define _ggTypes_h
#include <stdlib.h>

typedef struct __ggPosition{
    int m_iX;
    int m_iY;
} ggPosition;

typedef enum __ggeQuadSize{
    eQuadSize_4,
    eQuadSize_8,
    eQuadSize_16,
    eQuadSize_32,
    eQuadSize_64,
    eQuadSize_128,
    eQuadSize_256,
    eQuadSize_512,
    eQuadSize_1024,
    eQuadSize_2048,
    eQuadSize_4096,
    eQuadSize_Count
} ggeQuadSize;


#define ggAllocArray(__type,__count) (__type*)(malloc(sizeof(__type)*__count))
#define ggAlloc(__type) ggAllocArray(__type,0)
#endif