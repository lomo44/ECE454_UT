#include "lifemt.h"
#include <stdio.h>
#include <assert.h>

#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

void* ggWorkerThread(void* in_pContext){
	// Casting the in_pcontext to the correct context
	ggWorkerContext* context = (ggWorkerContext*)in_pContext;
	int gens_max = context->m_iMaxIteration;
	int curgen,inorth,isouth,jwest,jeast,nrows,ncols,i,j,endrows;
	char* inboard = context->m_pInboard;
	char* outboard = context->m_pOutboard;
	nrows = context->m_iBoardDimension;
	ncols = context->m_iBoardDimension;
	int LDA = nrows;
	endrows = context->m_iStartRow + context->m_iRange;
	for (curgen = 0; curgen < gens_max; curgen++)
    {
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        inorth = nrows-1;
        isouth = 1;
        for (i = context->m_iStartRow; i < endrows; i++)
        {
            jwest = ncols-1;
            jeast = 1;
            for (j = 0; j < ncols; j++)
            {
				int a = BOARD (inboard, inorth, jwest);
				int index = (BOARD (inboard, inorth, jwest) << 8) |
                    (BOARD (inboard, inorth, j) << 7) |
                    (BOARD (inboard, inorth, jeast) << 6) | 
                    (BOARD (inboard, i, jwest) << 5) |
                    (BOARD (inboard, i, jeast) << 4) |
                    (BOARD (inboard, isouth, jwest) << 3) |
                    (BOARD (inboard, isouth, j) << 2) | 
                    (BOARD (inboard, isouth, jeast) << 1) |
                    BOARD (inboard, i,j); 
                BOARD(outboard, i, j) = context->m_pCache[
                    (BOARD (inboard, inorth, jwest) << 8) |
                    (BOARD (inboard, inorth, j) << 7) |
                    (BOARD (inboard, inorth, jeast) << 6) | 
                    (BOARD (inboard, i, jwest) << 5) |
                    (BOARD (inboard, i, jeast) << 4) |
                    (BOARD (inboard, isouth, jwest) << 3) |
                    (BOARD (inboard, isouth, j) << 2) | 
                    (BOARD (inboard, isouth, jeast) << 1) |
                    BOARD (inboard, i,j)
                ];
                jwest = j;
                jeast = (j+2) & (ncols -1);
            }
            inorth = i;
            isouth = (i+2)&(nrows-1);
        }
		pthread_barrier_wait(context->m_pBarrier);
		SWAP_BOARDS( outboard, inboard );
    }
}

char* mt_game_of_lie(char* outboard, char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max){
    // Getting the concurrency of the system
    int t_count = 2;
	printf("Threads: %d\n",t_count);
	// Create global barrier
	pthread_barrier_t gBarrier;
	pthread_barrier_init(&gBarrier,NULL,t_count);
	// Create worker thread array
	ggWorkerContext* workers = ggAllocArray(ggWorkerContext,t_count);
	// Calculate worker range
	int worker_range = nrows/t_count;
	// 9* cache for mapping
	char cache[512] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	// Create workers
	for(int i = 0; i < t_count;i++){
		workers[i].m_iBoardDimension = nrows;
		workers[i].m_iStartRow = i*worker_range;
		workers[i].m_iRange = worker_range;
		workers[i].m_pBarrier = &gBarrier;
		workers[i].m_pInboard = inboard;
		workers[i].m_pOutboard = outboard;
		workers[i].m_iMaxIteration = gens_max;
		workers[i].m_pCache = cache;
		pthread_create(&workers[i].m_thread,NULL,ggWorkerThread,&workers[i]);
	}
	// Join all of the thread
	for(int i = 0; i < t_count;i++){
		pthread_join(workers[i].m_thread,NULL);
	}
	return gens_max % 2 == 1? outboard : inboard;
}