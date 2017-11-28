#ifndef _lifemt_h
#define _lifemt_h
#include <pthread.h>
#include "ggTypes.h"

struct __ggWorkerContext;


typedef struct __ggWorkerContext{
    pthread_barrier_t* m_pBarrier;
    ggPosition m_StartPost;
    int m_iRange;
    ggBoard m_pInboard;
    ggBoard m_pOutBoard;
    int m_iMaxIteration;
} ggWorkerContext;

void ggWorkerThread(ggWorkerContext* in_pContext);
void ggInitWorkerContext(ggWorkerContext* in_pContext);

#endif
