#ifndef _lifemt_h
#define _lifemt_h
#include <pthread.h>
#include "ggTypes.h"

struct __ggWorkerContext;

// This is the strcture to include all the data for one thread
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
    int m_iWorkerID;
} ggWorkerContext;
/* Function: mt_game_of_lie
 *-------------------------------------------------------------------------------------
 * This is reall function calculated the result and is running on single thread
 *  * Given the initial board state in inboard and the board dimensions nrows by ncols, 
 * evolve the board state gens_max times.
 * 
 * in_pContext: pointer to all input data for each thread
 * 
 * Return: NULL
 */
void* ggWorkerThread(void* in_pContext);
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
			 const int gens_max);

#endif
