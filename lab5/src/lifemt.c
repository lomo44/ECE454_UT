#define _GNU_SOURCE
#include "lifemt.h"
#include <stdio.h>
#include <assert.h>
#include <sys/sysinfo.h>

#include <sched.h>

#define SWAP_BOARDS( b1, b2 )  { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
}

#define BOARD( __board, __i, __j )  (__board[(__j) + LDA*(__i)])

/* Function: mt_game_of_lie
 *-------------------------------------------------------------------------------------
 * This is reall function calculated the result and is running on single thread
 * Given the initial board state in inboard and the board dimensions nrows by ncols, 
 * evolve the board state gens_max times. 
 * 
 * in_pContext: pointer to all input data for each thread
 * 
 * Return: NULL
 */
void* ggWorkerThread(void* in_pContext){
	// Casting the in_pcontext to the correct context
	ggWorkerContext* context = (ggWorkerContext*)in_pContext;
	//following code will lock each thread to one cpu 
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(context->m_iWorkerID,&set);
	pthread_setaffinity_np(context->m_thread,sizeof(cpu_set_t),&set);

	//load all data from the input 
	int gens_max = context->m_iMaxIteration;
	char* inboard = context->m_pInboard;
	char* outboard = context->m_pOutboard;
	int nrows = context->m_iBoardDimension;
	int ncols = context->m_iBoardDimension;
	int LDA = nrows;
	int endrows = context->m_iStartRow + context->m_iRange;

	int curgen,inorth,isouth,jeast,i,j;

    //create 9 register to store the neighbour value
	char data_nw,data_n,data_ne,data_w,data,data_e,data_sw,data_s,data_se;
	for (curgen = 0; curgen < gens_max; curgen++)
    {
        inorth = context->m_iStartRow-1 < 0? nrows-1 : context->m_iStartRow-1;
        isouth = (context->m_iStartRow+1) & (nrows-1);
        for (i = context->m_iStartRow; i < endrows; i++)
        {
			
			jeast = 1;
			data_nw = BOARD (inboard, inorth, ncols-1);
			data_n  = BOARD (inboard, inorth, 0);
			data_w  = BOARD (inboard, i, ncols-1);
            data_sw = BOARD (inboard, isouth, ncols-1);
            data_s  = BOARD (inboard, isouth, 0);
            data    = BOARD (inboard, i,0);
            for (j = 0; j < ncols; j++)
            {
				//only need to loac 3 new data for east most blocks 
				data_ne = BOARD (inboard, inorth, jeast);
				data_e  = BOARD (inboard, i, jeast);
				data_se = BOARD (inboard, isouth, jeast);

                BOARD(outboard, i, j) = context->m_pCache[
                    (data_nw << 8) | (data_n  << 7) | (data_ne << 6) | 
                    (data_w << 5)  | (data_e  << 4) | (data_sw << 3) |
					(data_s << 2)  | (data_se << 1) |  data ];
				
				//shift the east blocks to west 
				data_nw = data_n;
				data_w = data;
				data_sw = data_s;
				data_n = data_ne;
				data = data_e;
				data_s = data_se;

                jeast = (j+2) & (ncols -1);
            }
            inorth = i;
            isouth = (i+2)&(nrows-1);
        }
		SWAP_BOARDS( outboard, inboard );
		pthread_barrier_wait(context->m_pBarrier);
	}

	return NULL;
}

/* Function: mt_game_of_lie
 *-------------------------------------------------------------------------------------
 * Given the initial board state in inboard and the board dimensions nrows by ncols, 
 * evolve the board state gens_max times. This funciton will run with 8 threads 
 * 
 * inboard: initial board
 * outboard: extra board for store the new result and ping-pong with inboard
 * nrows: number of rows
 * ncols: number of cols
 * gens_max: the number of iterration should generate 
 * 
 * Return: a pointer to the final board; that pointer will be equal either to outboard or to
 * inboard depends odd/even of gens_max
 */
char* mt_game_of_life(char* outboard, char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max){
    // Getting the concurrency of the system
    int t_count = 8;
	printf("Threads: %d\n",t_count);
	// Create global barrier
	pthread_barrier_t gBarrier;
	pthread_barrier_init(&gBarrier,NULL,t_count);
	// Create worker thread array
	ggWorkerContext* workers = ggAllocArray(ggWorkerContext,t_count);
	// Calculate worker range
	int worker_range = nrows/t_count;
	// cache stores the result after 1 iteration for a 3*3 blocks
	// cache have 2^9 512 different entires
	// the index is [ north west block | north block      | north west block | 
	//				  west block       | east block       | south west block |
	//				  south blocks     | south east block |  center block    ]
	char cache[512] = {	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0,
						0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
						0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 
						0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 
						0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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
		workers[i].m_iWorkerID = i;
		pthread_create(&workers[i].m_thread,NULL,ggWorkerThread,&workers[i]);
	}
	// Join all of the thread
	for(int i = 0; i < t_count;i++){
		pthread_join(workers[i].m_thread,NULL);
	}
	return gens_max % 2 == 1? outboard : inboard;
}