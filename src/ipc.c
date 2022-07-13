#include <include/ipc.h>
#include <include/bookkeeper.h>

#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>

#include <include/simulation_stats.h>

/**
 * Crea le ipc necessarie al funzionamento del programma
 * @param ipcWrapper    *ipc    puntatore alla struttura nella quale salvare i dati delle ipc
 * @return
 */
void createIPC(ipc_wrapper *ipc, parameters params)
{

    
    int i = 0;
    for (i = 0; i < params.SO_NODES_NUM; i++)
    {
        ipc->node_queues[i] = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    }

    ipc->user_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    ipc->shm_bookkeeper = shmget(IPC_PRIVATE, sizeof(bookkeeper), 0666 | IPC_CREAT);
    ipc->shm_sim_stats = shmget(IPC_PRIVATE, sizeof(simulation_stats), 0666 | IPC_CREAT);

    ipc->sem_book_update = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    init_sem_available(ipc->sem_book_update, 0);

    ipc->sem_wrt = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    init_sem_available(ipc->sem_wrt, 0);

    ipc->sem_mutex_rd = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    init_sem_available(ipc->sem_mutex_rd, 0);
}

/**
 * Pulisce le ipcs utilizzate dal programma
 * @param ipcWrapper    ipc    struttura che contiene le ipc da eliminare
 * @return
 */
void delete_ipc(ipc_wrapper *ipc, parameters params)
{
    int i = 0;
    shmctl(ipc->shm_bookkeeper, IPC_RMID, NULL);
    shmctl(ipc->shm_sim_stats, IPC_RMID, NULL);

    for (i = 0; i < params.SO_NODES_NUM; i++)
    {
        msgctl(ipc->node_queues[i], IPC_RMID, NULL);
    }

    msgctl(ipc->user_queue, IPC_RMID, NULL);

    semctl(ipc->sem_wrt, 0, IPC_RMID);
    semctl(ipc->sem_book_update, 0, IPC_RMID);
    semctl(ipc->sem_mutex_rd, 0, IPC_RMID);
}

/**
 * Invia un messaggio alla coda di messaggi specificata
 * @param struct msgbuf *m          struttura nella quale verrà salvato il messaggio ricevuto
 * @param int           qID         id della coda di messaggi
 * @param int           msgType     tipo del messaggio
 * @param char          *text       messaggio che verrà inviato
 * @param int           len         lunghezza del messaggio che si vuole inviare
 * @return
 */

void send_function(struct msgbuf m, int qID, int msgType, transaction *trans)
{
    int num_bytes = sprintf(m.mtext, "%d %d %d %d %d", trans->timestamp, trans->sender, trans->receiver, trans->money_sum, trans->paid_reward);
    m.mtext[num_bytes] = '\0';
    m.mtype = msgType;
    msgsnd(qID, &m, num_bytes + 1, IPC_NOWAIT);
}

void send_status_back(struct msgbuf m, int qID, int msgType, transaction *trans)
{
    int num_bytes = sprintf(m.mtext, "%d,%d,%d", trans->timestamp, trans->receiver, trans->sender);
    m.mtype = msgType;

    msgsnd(qID, &m, strlen(m.mtext), IPC_NOWAIT);
}

/**
 * Riceve un messaggio dalla coda di messaggi specificata
 * @param struct msgbuf *m          struttura nella quale verrà salvato il messaggio ricevuto
 * @param int           qID         id della coda di messaggi
 * @param int           rcvType     mtype che verrà letto
 * @return
 */
int receive_function(struct msgbuf *m, int qID, int rcvType)
{
    int num_bytes;
    num_bytes = msgrcv(qID, m, MSG_LEN, rcvType, IPC_NOWAIT);
    /*   printf("block\n");
       if (num_bytes >= 0 || errno == 22) {
           break;
       }
    } */
    return num_bytes;
}

/**
 * Inizializza un semaforo come "disponibile"
 * @param int    semId      id ipc semaforo
 * @param int    semNum     numero del semaforo da inizializzare
 * @return
 */
int init_sem_available(int sem_id, int sem_num)
{
    union semun arg;
    arg.val = 1;
    return semctl(sem_id, sem_num, SETVAL, arg);
}

/**
 * Richiedi un semaforo (-1)
 * @param int    sem_id      id ipc semaforo
 * @param int    sem_num     numero del semaforo da decrementare
 * @param int    flag       Se vale 0, la chiamata è fatta con IPC_NOWAIT
 * @return
 */
int reserve_sem(int sem_id, int sem_num, int flag)
{
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = -1;
    sops.sem_flg = flag;
    return semop(sem_id, &sops, 1);
}

/**
 * Rilascia un semaforo (+1)
 * @param int    semId      id ipc semaforo
 * @param int    semNum     numero del semaforo da incrementare
 * @param int    flag       Se vale 0, la chiamata è fatta con IPC_NOWAIT
 *
 * @return
 */
int release_sem(int semId, int semNum, int flag)
{
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(semId, &sops, 1);
}