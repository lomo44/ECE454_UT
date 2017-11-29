#include "lifemt.h"
#include <stdio.h>
#include <assert.h>
#include <sys/sysinfo.h>

#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__j) + LDA*(__i)])

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
	char data_nw,data_n,data_ne,data_w,data,data_e,data_sw,data_s,data_se;
	for (curgen = 0; curgen < gens_max; curgen++)
    {
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        inorth = context->m_iStartRow-1 < 0? nrows-1 : context->m_iStartRow-1;
        isouth = (context->m_iStartRow+1) & (nrows-1);
        for (i = context->m_iStartRow; i < endrows; i++)
        {
            jwest = ncols-1;
			jeast = 1;
			data_nw = BOARD (inboard, inorth, ncols-1);
			data_n  = BOARD (inboard, inorth, 0);
			data_w  = BOARD (inboard, i, ncols-1);
            data_sw = BOARD (inboard, isouth, ncols-1);
            data_s  = BOARD (inboard, isouth, 0);
            data    = BOARD (inboard, i,0);
            for (j = 0; j < ncols; j++)
            {
				data_ne = BOARD (inboard, inorth, jeast);
				data_e  = BOARD (inboard, i, jeast);
				data_se = BOARD (inboard, isouth, jeast);
                BOARD(outboard, i, j) = context->m_pCache[
                    (data_nw << 8) | (data_n  << 7) | (data_ne << 6) | 
                    (data_w << 5)  | (data_e  << 4) | (data_sw << 3) |
					(data_s << 2)  | (data_se << 1) |  data ];
				data_nw = data_n;
				data_w = data;
				data_sw = data_s;
				data_n = data_ne;
				data = data_e;
				data_s = data_se;

                jwest = j;
                jeast = (j+2) & (ncols -1);
            }
            inorth = i;
            isouth = (i+2)&(nrows-1);
        }
		SWAP_BOARDS( outboard, inboard );
		pthread_barrier_wait(context->m_pBarrier);
    }
}

char* mt_game_of_lie(char* outboard, char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max){
    // Getting the concurrency of the system
    int t_count = get_nprocs_conf();
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