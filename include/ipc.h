#ifndef __MY_IPC_H__
#define __MY_IPC_H__

#include "models.h"
#include "parameters.h"



#define MSG_LEN 100

struct msgbuf
{
	long mtype;
	char mtext[MSG_LEN];
};

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
#if defined(__linux__)
	struct seminfo *__buf;
#endif
};

typedef struct _ipc_wrapper
{
	int node_queues[10];
	int user_queue;

	int shm_bookkeeper;
	int shm_sim_stats;
	/* Used from nodes to write on bookkeeper in mutual exclusion */
	int sem_book_update;

	int sem_wrt;
	int sem_mutex_rd;

	int sem_in;
	int sem_reader_mutex;
	int reader_counter;
	int reader_out;

	int writer_wait;

} ipc_wrapper;

void createIPC(ipc_wrapper *ipc, parameters);
void delete_ipc(ipc_wrapper *ipc, parameters);
void send_function(struct msgbuf, int, int, transaction *);
void send_status_back(struct msgbuf, int, int, transaction *);
int receive_function(struct msgbuf *, int, int);

int init_sem_available(int, int);
int reserve_sem(int, int, int);
int release_sem(int, int, int);

#endif