#include <include/transaction_pool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
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
void print_pool(transaction_pool *pool) {
    int i, j;
    return;
    for(i = 0; i < pool->capacity; i++) {
        printf("block [%d] => ", i);
        for(j = 0; j < pool->pool_blocks[i].size; j++) {
            printf(" [ %d ] ", pool->pool_blocks[i]._transaction_array[j].receiver);
        }

        for(; j < pool->pool_blocks[i].capacity; j++) {
            printf(" [ ] ");
        }
        printf("\n");
    }
}

void create_transaction_pool(transaction_pool *pool, int tp_size, int block_size, int so_tp_size) {

    double s = so_tp_size;

    int num_blocks = ceil(s/SO_BLOCK_SIZE);
    int i = 0;

    transaction_block * pool_blocks = (transaction_block*) malloc(sizeof(transaction_block) * num_blocks);
    
    for(i = 0; i < num_blocks;i++) {
        pool_blocks[i].capacity = SO_BLOCK_SIZE; 
        pool_blocks[i].size = 0;  
    }

    pool->pool_blocks = pool_blocks;
    pool->size = 0;
    pool->capacity = num_blocks;

}



/**
 * @brief 
 * 
 * @param pool 
 * @param block_number 
 * @param t 
 */
int transaction_pool_insert(transaction_pool* pool, transaction t) {
    int ret_val = 0;
    int available_block;
    int tr_array_index;
    int block_num;
    /*
    La pool era piena durante l'ultimo inserimento.
    controlla se si è liberato qualche spazio.
    */
    if(pool->size == -1) {
        available_block = transaction_pool_find_first_available_block(pool);
        if(available_block != -1) {
            
            /**/
            pool->size = available_block;
            pool->pool_blocks[available_block].size = 0;
        }
        else {
            /* Nessun blocco disponibile. notifica il sender che l'invio non è andato a buon fine. */
            /* ret_val = -2;*/
            return -2;
        }
    }

    tr_array_index = pool->pool_blocks[pool->size].size++;
    /*printf("Adding into block %d and transaction index %d\n", pool->size,  tr_array_index);*/
    
    pool->pool_blocks[pool->size]._transaction_array[tr_array_index] = t;
    
    /* Se il blocco si è saturato */
    if(pool->pool_blocks[pool->size].size >= pool->pool_blocks[pool->size].capacity -1) {

        /* Setta il blocco come sporco -> da spostare nel libro mastro */
        pool->pool_blocks[pool->size].dirty_bit = 1;


        /* Se ci troviamo sull'ultimo blocco -> dobbiamo trovare il primo indice libero */
        if(pool->size >= pool->capacity-1) {
            block_num = transaction_pool_find_first_available_block(pool);
            if(block_num != -1) {
                pool->size = block_num;
                ret_val = -1;
            }
            else {
                /* segnala che la pool è piena fino a quando non si libera un blocco */
                pool->size = -1;

                /* Nessun blocco disponibile. notifica il sender che l'invio non è andato a buon fine. */
                ret_val = -2;
            }
        }
        else {
            /* passa al prossimo blocco */
            pool->size++;
            ret_val = -1;
        }
    }
    print_pool(pool);
    return ret_val;
}



int transaction_pool_find_first_available_block(transaction_pool* pool) {
    int ret_val = -1;
    int i;

    for (i = 0; i < pool->capacity && ret_val == -1; i++) {
        ret_val = pool->pool_blocks[i].dirty_bit == 0? i : -1; 
    }

    return ret_val;
}