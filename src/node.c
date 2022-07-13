#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <include/models.h>
#include <include/node.h>
#include <include/user.h>
#include <time.h>
#include <errno.h>
#include <include/ipc.h>
#include <include/transaction_pool.h>
#include <include/config.h>
#include <include/bookkeeper.h>
#include <include/utils.h>
#include <include/simulation_stats.h>
parameters params;

void create_reward_transaction(transaction_block *block, int node_id)
{
    struct timespec current_time;
    transaction t;
    int i;
    t.money_sum = 0;

    clock_gettime(CLOCK_REALTIME, &current_time);
    t.timestamp = current_time.tv_sec;

    t.sender = SENDER_TRANSACTION_REWARD;
    t.receiver = node_id; /* NODE_ID*/

    for (i = 0; i < SO_BLOCK_SIZE - 1; i++)
    {
        t.money_sum += block->_transaction_array[i].paid_reward;
    }
    t.paid_reward = 0;
    block->_transaction_array[SO_BLOCK_SIZE - 1] = t;
}

void start_simulation(int queueId, transaction_pool *pool, int user_queue, int shm_book, int sem_id, int node_id, simulation_stats* stats, int sem_wrt, int sem_mutex_rd)
{
    struct msgbuf message;
    struct timespec s;
    transaction deserialized_transaction;
    int insert_result, block_id;
    int i, j;

    bookkeeper *book;
    book = shmat(shm_book, NULL, 0);

    while (is_simulation_running_read(stats, sem_wrt, sem_mutex_rd))
    {
        /* If there is a message to read */
        if (receive_function(&message, queueId, 1) != -1)
        {
            
            /* deserialize a transaction */
            sscanf(message.mtext, "%u %u %u %u %u", &deserialized_transaction.timestamp,
                   &deserialized_transaction.sender, &deserialized_transaction.receiver,
                   &deserialized_transaction.money_sum, &deserialized_transaction.paid_reward);

            /*
            Prendo l'id del blocco che sto modificando così se devo spostarlo nel
            libro mastro, so quale blocco prendere
            */
            block_id = P_CURRENT_INDEX(pool);/* pool->size;*/

            /* Insert a transaction inside pool */
            insert_result = transaction_pool_insert(pool, deserialized_transaction);

            /* full block into pool */
            if (insert_result == -1)
            {

                create_reward_transaction(&pool->pool_blocks[block_id], node_id);


                /* sleep for random time */
                s.tv_sec = 0;
                s.tv_nsec = my_random(params.SO_MIN_TRANS_PROC_NSEC, params.SO_MAX_TRANS_PROC_NSEC);
                nanosleep(&s, NULL);


                /* Move block in bookkeeper in mutual esclusion */
                reserve_sem(sem_id, 0, 1);
                /*
                    no one will update book size in this moment because the only point of 
                    update in all code is here, and for accessing it, it is needed
                    a semaphore
                */
                if (book->size < book->capacity)
                {
                    pool->pool_blocks[block_id].block_id = book->size;
                    book->_block_array[book->size] = pool->pool_blocks[block_id];
                    book->size++;
                }
                else
                {
                
                    release_sem(sem_id, 0, 1);
                    break;
                }
                release_sem(sem_id, 0, 1);

                /*
                    Notify the user that send of the transaction is completed
                */
                for(i = 0; i < SO_BLOCK_SIZE-1; i++) {
                    transaction tr = pool->pool_blocks[block_id]._transaction_array[i];
                    transaction t;
                    t.timestamp = tr.timestamp;
                    t.receiver = tr.money_sum;
                    t.sender = 0;

                    send_status_back(message, user_queue, tr.sender + 1, &t);

                }

                /*
                    Empty the block
                */
                pool->pool_blocks[block_id].dirty_bit = 0;
                pool->pool_blocks[block_id].size = 0;
                

            }

            /* Notify the user that pool is full and can't receive transactions */
            else if (insert_result == -2)
            {
                transaction t;
                t.timestamp = deserialized_transaction.timestamp;
                t.receiver = deserialized_transaction.receiver;
                t.sender = -1;
                send_status_back(message, user_queue, deserialized_transaction.sender + 1, &t);
            }
        }
    }

    shmdt(book);
}

int main(int argc, char *argv[])
{
    transaction_pool private_pool;
    ipc_wrapper *ipcs;
    int ipcs_id, node_id;
    int i, j;
    simulation_stats *statistics;
    int remaining_transactions;
    if (argc <= 1) return -1;


    /* Initialization */
    getParameters(&params);
    create_transaction_pool(&private_pool, params.SO_TP_SIZE, SO_BLOCK_SIZE, params.SO_TP_SIZE);

    ipcs_id = atoi(argv[0]);
    node_id = atoi(argv[1]);

    set_seed();

    ipcs = (ipc_wrapper *)shmat(ipcs_id, NULL, 0);
    statistics = shmat(ipcs->shm_sim_stats, NULL, 0);
    start_simulation(ipcs->node_queues[node_id], &private_pool, ipcs->user_queue, ipcs->shm_bookkeeper, ipcs->sem_book_update, node_id, statistics, ipcs->sem_wrt, ipcs->sem_mutex_rd);
    
    for(i = 0; i < private_pool.capacity; i++) {
        /* Add size */
        remaining_transactions+= (private_pool.pool_blocks[i].size > 0? private_pool.pool_blocks[i].size +1 : 0);
    }

    /* Comunicate to master how much transaction reamin in his pool. */
    statistics->node_status[node_id] = remaining_transactions;

    /* Detatch IPCs */
    shmdt(statistics);
    shmdt(ipcs);
    return 0;
}
