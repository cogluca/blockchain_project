#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <strings.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <include/models.h>
#include <include/ipc.h>
#include <include/parameters.h>
#include <include/config.h>
#include <include/bookkeeper.h>
#include <include/simulation_stats.h>
#include <include/utils.h>


parameters params;
bookkeeper *book;
simulation_stats *statistics;
ipc_wrapper *ipcs;

void calculate_simulation_budgets(int *users_budget, int *nodes_budget)
{
    int i, j;
    int size;

    int sum = 0, sum_nodes = 0;
    for (i = 0; i < params.SO_USERS_NUM; i++)
        users_budget[i] = params.SO_BUDGET_INIT;

    for (i = 0; i < params.SO_NODES_NUM; i++)
        nodes_budget[i] = 0;

    size = book->size;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < SO_BLOCK_SIZE; j++)
        {
            int sender = book->_block_array[i]._transaction_array[j].sender;
            int receiver = book->_block_array[i]._transaction_array[j].receiver;
            int money_sum = book->_block_array[i]._transaction_array[j].money_sum;
            transaction t = book->_block_array[i]._transaction_array[j];

            /* Se transazione di reward, aggiungi all'array di budget nodi ed stima i buget degli utenti ... */
            if (sender == SENDER_TRANSACTION_REWARD)
            {
                nodes_budget[receiver] += money_sum;
            }
            else
            {
                users_budget[sender] -= money_sum;
                users_budget[receiver] += money_sum;
            }
        }
    }
}
int cmpfunc(const void *a, const void *b)
{
    return (*(int *)b - *(int *)a);
}
void handler()
{
    int i;
    int sum = 0;
    int *users_budget, *nodes_budget;

    int count_user = 0;

    params.SO_SIM_SEC--;
    printf("%d seconds to simulation end.\n", params.SO_SIM_SEC);

    nodes_budget = (int*) malloc(sizeof(int)*params.SO_NODES_NUM);
    users_budget = (int*) malloc(sizeof(int)*params.SO_USERS_NUM);


    calculate_simulation_budgets(users_budget, nodes_budget);

    /*
    Stampa numero processi utente e nodo attivi
    */
    qsort(users_budget, params.SO_USERS_NUM, sizeof(int), cmpfunc);
    printf("------ Printing Users budget ------\n");
    if (params.SO_USERS_NUM > 10)
    {
        printf("------ Users with max budget ------\n");

        for (i = 0; i < 5; i++)
        {
            printf("%d)User: %d\n", i + 1, users_budget[i]);
        }
        printf("------ Users with few budget ------\n");

        for (i = params.SO_USERS_NUM - 1; i > params.SO_USERS_NUM - 6; i--)
        {
            printf("%d)User: %d\n", i + 1, users_budget[i]);
        }
    }
    else
    {
        printf("------ All users ------\n");

        for (i = 0; i < params.SO_USERS_NUM; i++)
        {
            printf("%d)User: %d\n", i + 1, users_budget[i]);
        }
    }
    printf("\n------ Printing Nodes budget ------\n");
    for (i = 0; i < params.SO_NODES_NUM; i++)
    {
        printf("%d)Node: %d\n", i + 1, nodes_budget[i]);
    }
    
    printf("\n------Number of users process active ------\n");
    for (i = 0; i < params.SO_USERS_NUM; i++)
    {
        if (statistics->user_status[i] == 1)
            count_user++;
    }
    printf("There are %d user process(es) active.\n", count_user);

    /* nodes can't exit, so just use parameters value*/
    printf("There are %d node process(es) active.\n\n", params.SO_NODES_NUM);

    signal(SIGALRM, handler);
    /*
    Controllo condizioni di terminazione!
        1. Timeout
        2. Libro mastro completo
        3. 0 utenti attivi
    Se una di queste è vera, esco, aspetto che tutti i processi finiscano e distruggo tutte le IPCs
    */
    if (params.SO_SIM_SEC == 0 || book->size >= book->capacity - 1 || count_user == 0)
    {
        /** Critical section */
        reserve_sem(ipcs->sem_in_sim,0,1);
        reserve_sem(ipcs->sem_out_sim, 0, 1);
        if(ipcs->child_in_sim == ipcs->child_out_sim){
            release_sem(ipcs->sem_out_sim, 0, 1);
        }
        else{
            ipcs->master_wait_sim = 1;
            release_sem(ipcs->sem_out_sim, 0, 1);
            reserve_sem(ipcs->sem_wrt, 0, 1);
            ipcs->master_wait_sim= 0;
        }

    
        statistics->is_simulation_running = 0;

        release_sem(ipcs->sem_in_sim, 0, 1);

        alarm(0);
    }
    
    else
    {
        alarm(1);
    }
    free(nodes_budget);
    free(users_budget);
}

int main(int argc, char *argv[])
{
    int ipcs_id;

    pid_t node_child, user_child;
    int i;

    /* Lets suppose ipcs id and index of elements is never higher than 15 digits */
    char ipcs_id_into_string[15];
    char index_into_string[15];
    char argv_for_execv[3][15];



    int *node_children, *user_children;
    int *users_budget, *nodes_budget;
    int dead_user = 0;
    

    getParameters(&params);

    node_children = (int*) malloc(sizeof(int)*params.SO_NODES_NUM);
    user_children = (int*) malloc(sizeof(int)*params.SO_USERS_NUM);

    nodes_budget = (int*) malloc(sizeof(int)*params.SO_NODES_NUM);
    users_budget = (int*) malloc(sizeof(int)*params.SO_USERS_NUM);

    if (signal(SIGALRM, handler) == SIG_ERR)
    {
        fprintf(stderr, "Error\n");
        exit(-1);
    }

    ipcs_id = shmget(IPC_PRIVATE, sizeof(ipc_wrapper), IPC_CREAT | IPC_EXCL | 0666);
    ipcs = (ipc_wrapper *)shmat(ipcs_id, NULL, 0);

    sprintf(ipcs_id_into_string, "%d", ipcs_id);
    sprintf(argv_for_execv[0], "%d", ipcs_id);

    createIPC(ipcs, params);

    book = shmat(ipcs->shm_bookkeeper, NULL, 0);
    book->capacity = SO_REGISTRY_SIZE;

    statistics = shmat(ipcs->shm_sim_stats, NULL, 0);
    statistics->is_simulation_running = 1;

    /* utility variables */
    node_child = 1;
    user_child = 1;

    /* Create SO_NODES_NUM processess */
    for (i = 0; i < params.SO_NODES_NUM; i++)
    {
        sprintf(index_into_string, "%d", i);
        sprintf(argv_for_execv[1], "%d", i);


        switch (fork())
        {
        case -1:
            printf("Error occurred during node child forking");
            break;
        case 0:
            execv("./node", (char*[]) {ipcs_id_into_string, index_into_string, NULL});
            perror("node execve failed");
            break;

        default:
            node_children[i] = node_child;
            break;
        }
    }

    /* Create SO_NODES_NUM processess */
    for (i = 0; i < params.SO_USERS_NUM; i++)
    {
        sprintf(index_into_string, "%d", i);


        switch (fork())
        {
        case -1:
            printf("Error occurred during node child forking");
            break;
        case 0:
            execv("./user", (char*[]) {ipcs_id_into_string, index_into_string, NULL});
            perror("user execve failed");
            break;

        default:
            user_children[i] = node_child;
            break;
        }
    }

    alarm(1);

    while (statistics->is_simulation_running)
    {
        sleep(1);
    }


    /* Waiting for children exit, signaled by a -1 as a return from wait(NULL)*/
    while(wait(NULL) > 0) { };

    /* Calculating statistics */
    calculate_simulation_budgets(users_budget, nodes_budget);
    printf("------ Printing Users budget ------\n");
    printf("------ All users ------\n");

    for (i = 0; i < params.SO_USERS_NUM; i++)
    {
        printf("Users %d: %d\n", i, users_budget[i]);
    }

    printf("\n------ Printing Nodes budget ------\n");
    for (i = 0; i < params.SO_NODES_NUM; i++)
    {
        printf("Nodes %d: %d; Has %d transaction in pool\n", i, nodes_budget[i], statistics->node_status[i]);
    }


    printf("\n------Number of users process active ------\n");
    for (i = 0; i < params.SO_USERS_NUM; i++)
    {
        if (statistics->user_status[i] == 0)
            dead_user++;
    }
    printf("There are %d user process(es) exit before termination.\n", dead_user);

    /* FIX TERMINATION MOTIVATION */
    printf("Simulation terminated. ");
    if(params.SO_SIM_SEC == 0) {
        printf("Time elapsed");
    }
    else if(book->size == book->capacity) {
        printf("Capacity full");

    }
    else if(dead_user == params.SO_USERS_NUM) {
        printf("All user exited");

    }
    printf("\n%d of %d blocks used in bookkeeper\n", book->size, book->capacity);

    /*
     deallocazione memorie condiv
     chiusura e cancellazione semafori
    */

    shmdt(book);
    shmdt(statistics);
    delete_ipc(ipcs, params);
    shmctl(ipcs_id, IPC_RMID, NULL);


    free(node_children);
    free(user_children);
    free(nodes_budget);
    free(users_budget);

    return 0;
}
