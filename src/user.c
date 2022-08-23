
#include <include/config.h>
#include <include/ipc.h>
#include <include/models.h>
#include <include/bookkeeper.h>
#include <include/utils.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <include/simulation_stats.h>
#include <signal.h>
#include <unistd.h>

parameters params;
simulation_stats *statistics;
bookkeeper *book;
ipc_wrapper *ipcs;

int user_id;
int sent_transaction_money;

int calculate_budget(bookkeeper *book, int user_id, int sent_transaction_money, int reader_counter)
{
    int new_budget = params.SO_BUDGET_INIT;

    int i, j, size;
    
    size = book->size;

    reserve_sem(ipcs->sem_in, 0, 1);
    ipcs->reader_counter++;
    release_sem(ipcs->sem_in, 0, 1);
    

    for (i = 0; i < size; i++)
    {
        for (j = 0; j < SO_BLOCK_SIZE - 1; j++)
        {
            int sender = book->_block_array[i]._transaction_array[j].sender;
            int receiver = book->_block_array[i]._transaction_array[j].receiver;
            int money_sum = book->_block_array[i]._transaction_array[j].money_sum;
            if (sender == user_id)
            {
                new_budget -= money_sum;
            }
            else if (receiver == user_id)
            {
                new_budget += money_sum;
            }
        }
    }

    reserve_sem(ipcs->sem_reader_mutex, 0, 1);
    ipcs->reader_out++;
    if((ipcs->writer_wait == 1) && (ipcs->reader_counter == ipcs->reader_out)){
        release_sem(ipcs->sem_book_update, 0, 1);
    }
    release_sem(ipcs->sem_reader_mutex, 0, 1);

    new_budget -= sent_transaction_money;
    return new_budget;
}

transaction create_random_transaction(int sender, int sender_budget, int *sent_transaction_money)
{
    struct timespec current_time;

    transaction t;
    int receiver;
    int paid_reward;
    int money_sum;

    int random_from_budget = my_random(2, sender_budget);
    clock_gettime(CLOCK_REALTIME, &current_time);

    t.timestamp = current_time.tv_sec;
    t.sender = sender;

    do
    {
        receiver = my_random(0, params.SO_USERS_NUM - 1);
    } while (receiver == sender);

    t.receiver = receiver;
    t.paid_reward = random_from_budget * params.SO_REWARD / 100;
    t.paid_reward = t.paid_reward == 0 ? 1 : t.paid_reward;
    t.money_sum = random_from_budget - t.paid_reward;

    *sent_transaction_money += t.money_sum;

    return t;
}

transaction send_random_transaction(int user_id, int *node_queues, transaction t)
{
    struct timespec s;
    struct msgbuf message;
    int dest_node;

    s.tv_sec = 0;
    s.tv_nsec = my_random(params.SO_MIN_TRANS_GEN_NSEC, params.SO_MAX_TRANS_GEN_NSEC);

    dest_node = my_random(0, params.SO_NODES_NUM - 1);

    send_function(message, node_queues[dest_node], 1, &t);

    if(nanosleep(&s, NULL) < 0){
        printf("Failed to nanosleep\n");
    }

    return t;
}

void handler()
{
    printf("HANDLING SIGNAL --- Creating and sending a random transaction \n");
    transaction t;

    int budget = calculate_budget(book, user_id, sent_transaction_money, ipcs->reader_counter);

    if (budget > 2)
    {
        t = create_random_transaction(user_id, budget, &sent_transaction_money);
        send_random_transaction(user_id, ipcs->node_queues, t);
    }
}

int main(int argc, char *argv[])
{
    struct msgbuf message;
    int budget;
    int num_consecutive_attempt = 0;
    int transaction_money, timestamp, result;
    int ipcs_id;
    sent_transaction_money = 0;
    getParameters(&params);

    set_seed();

    if (argc <= 1)
        return -1;

    ipcs_id = atoi(argv[0]);
    user_id = atoi(argv[1]);

    ipcs = (ipc_wrapper *)shmat(ipcs_id, NULL, 0);
    book = shmat(ipcs->shm_bookkeeper, NULL, SHM_RDONLY);
    statistics = shmat(ipcs->shm_sim_stats, NULL, 0);
    statistics->user_status[user_id] = 1;

    if (signal(SIGUSR1, handler) == SIG_ERR)
    {
        fprintf(stderr, "Errore della disposizione dell'handler\n");
        exit(-1);
    }

    /*
        Una transazione può fallire se:
        1. La transaction pool del nodo scelto è piena
        2. Il saldo dell'utente è inferiore a 2
    */
    while (num_consecutive_attempt < params.SO_RETRY && is_simulation_running_read(statistics, ipcs->sem_wrt, ipcs->sem_mutex_rd))
    {
        int i = 0;
        transaction t;

        budget = calculate_budget(book, user_id, sent_transaction_money, ipcs->reader_counter);
        
        if (budget < 2)
        {
            num_consecutive_attempt++;
        }

        else
        {
            t = create_random_transaction(user_id, budget, &sent_transaction_money);
            send_random_transaction(user_id, ipcs->node_queues, t);
        }

        /* Controlla se qualche nodo ha notificato l'utente che qualche transazione è stata completata o rifiutata */
        while (receive_function(&message, ipcs->user_queue, user_id + 1) != -1)
        {

            sscanf(message.mtext, "%d,%d,%d", &timestamp, &transaction_money, &result);
            sent_transaction_money -= transaction_money;

            /*
            If result is equls to 0 (if node send a confirm for the specific transaction)
            set num_consecutive_attempt to 0
            */
            num_consecutive_attempt = result == 0 ? 0 : num_consecutive_attempt + 1;
        }
    }

    /* notify user im exiting */
    if (statistics->is_simulation_running)
        statistics->user_status[user_id] = 0;

    /*
        Clear all
    */
    shmdt(statistics);
    shmdt(ipcs);
    shmdt(book);
    return 0;
}