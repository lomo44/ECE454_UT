#ifndef _lifemt_h
#define _lifemt_h
#include <pthread.h>
#include "ggTypes.h"

struct __ggWorkerContext;


typedef struct __ggWorkerContext{
    // Pthread handle
    pthread_t m_thread;
    // Global barrier than each thread has to meet before each iterations
    pthread_barrier_t* m_pBarrier;
    // Starting position of the thread
    int m_iStartRow;
    // Working range of the thread
    int m_iRange;
    // Board dimensions
    int m_iBoardDimension;
    // Board swap chains
    ggBoard m_pInboard;
    ggBoard m_pOutboard;
    // Maximum amout of iterations
    int m_iMaxIteration;
    char* m_pCache;
} ggWorkerContext;

void* ggWorkerThread(void* in_pContext);

char* mt_game_of_lie(char* outboard, char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max);

#endif
