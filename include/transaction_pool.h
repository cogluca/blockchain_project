#pragma once
#include "models.h"
#include <math.h>


#define P_CURRENT_INDEX(pool)   pool->size
#define P_CURRENT_BLOCK(pool)   pool->pool_blocks[P_CURRENT_INDEX]

typedef struct _transaction_block {
    transaction _transaction_array[SO_BLOCK_SIZE]; /* SO_BLOCK_SIZE */
    int capacity; /* mantiene la dimensione massima dell'array di transazioni del blocco */
    int size; /* < mantiene la dimensione attuale dell'array di transazioni del singolo blocco */
    int dirty_bit;
    int block_id;

} transaction_block;



/*


Per trovare il numero di blocchi da inizializzare
             -> SO_TP_SIZE (numero massimo di transazioni) / SO_BLOCK_SIZE
    POOL
        {
         B1
         [ t1 t2 ... tn ]
         B2
         [          ]
         B3
         [          ]
        }
        
*/

typedef struct _transaction_pool {
    transaction_block* pool_blocks;
    int capacity; /* SO_TP_SIZE <- mantiene la dimensione massima della pool */
    int size; /* mantiene l'ultimo blocco disponibile non pieno in cui poter allocare dati */
} transaction_pool;

void create_transaction_pool(transaction_pool *, int, int, int);


/**
 * @brief 
 * 
 * @return int      0 if insert correctly and no index were shifted
 *                  -1 if insert correctly and block full
 *                  -2 if insert correctly and pool full
 */
int transaction_pool_insert(transaction_pool*, transaction);


/**
 * @brief 
 * 
 * @return int index of available block if exists, -1 otherwhise
 */
int transaction_pool_find_first_available_block(transaction_pool*);