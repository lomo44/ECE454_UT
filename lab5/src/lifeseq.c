/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include "load.h"
#include <string.h>
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])


    char*
sequential_game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
    const int LDA = nrows;
    int curgen, i, j;

    for (curgen = 0; curgen < gens_max; curgen++)
    {
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        for (i = 0; i < nrows; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);

                const char neighbor_count = 
                    BOARD (inboard, inorth, jwest) + 
                    BOARD (inboard, inorth, j) + 
                    BOARD (inboard, inorth, jeast) + 
                    BOARD (inboard, i, jwest) +
                    BOARD (inboard, i, jeast) + 
                    BOARD (inboard, isouth, jwest) +
                    BOARD (inboard, isouth, j) + 
                    BOARD (inboard, isouth, jeast);

                BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));

            }
        }
        SWAP_BOARDS( outboard, inboard );

    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return inboard;
}

    char*
sequential_game_of_life_opt (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
    const int LDA = nrows;
    int curgen, i, j;
    char* inboard_count = NULL;
    char* outboard_count = NULL;
    inboard_count = make_board (nrows, ncols);
    outboard_count = make_board (nrows, ncols);
    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++) {
         BOARD (outboard_count, i, j) = 0;
        }
    }
    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++) {
            const int inorth = mod (i-1, nrows);
            const int isouth = mod (i+1, nrows);
            const int jwest = mod (j-1, ncols);
            const int jeast = mod (j+1, ncols);
            if (BOARD(inboard, i, j) == 1) {
                BOARD (outboard_count, inorth, jwest) ++; 
                BOARD (outboard_count, inorth, j) ++; 
                BOARD (outboard_count, inorth, jeast) ++; 
                BOARD (outboard_count, i, jwest) ++;
                BOARD (outboard_count, i, jeast) ++; 
                BOARD (outboard_count, isouth, jwest) ++;
                BOARD (outboard_count, isouth, j) ++; 
                BOARD (outboard_count, isouth, jeast) ++;
            }
        }
    }
    int over = 0;
    for (curgen = 0; curgen < gens_max; curgen++)
    {
        memcpy (inboard_count, outboard_count, nrows * ncols * sizeof (char));        
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        for (i = 0; i < nrows; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
                const char neighbour_count = BOARD (inboard_count, i, j);
                if ( neighbour_count == (char) 3) {
                    if ( BOARD (inboard, i, j) ==0){
                        BOARD (inboard, i, j) = 1;
                        BOARD (outboard_count, inorth, jwest) ++;
                        BOARD (outboard_count, inorth, j) ++;
                        BOARD (outboard_count, inorth, jeast) ++;
                        BOARD (outboard_count, i, jwest) ++;
                        BOARD (outboard_count, i, jeast) ++;
                        BOARD (outboard_count, isouth, jwest) ++;
                        BOARD (outboard_count, isouth, j) ++;
                        BOARD (outboard_count, isouth, jeast) ++;
                        over++;
                    } 
                } else if ( neighbour_count != 2) {
                    if (BOARD (inboard, i, j)==1) {
                        BOARD (inboard, i, j) = 0;
                        BOARD (outboard_count, inorth, jwest) --;
                        BOARD (outboard_count, inorth, j) --;
                        BOARD (outboard_count, inorth, jeast) --;
                        BOARD (outboard_count, i, jwest) --;
                        BOARD (outboard_count, i, jeast) --;
                        BOARD (outboard_count, isouth, jwest) --;
                        BOARD (outboard_count, isouth, j) --;
                        BOARD (outboard_count, isouth, jeast) --;
                        over++;
                    } 
                }
            }
        }
    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return inboard;
}


