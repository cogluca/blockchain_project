
#include <include/simulation_stats.h>
#include <include/ipc.h>
int is_simulation_running_read(simulation_stats *statistics, ipc_wrapper* ipcs)
{
    int ret = 0;
    reserve_sem(ipcs->sem_in_sim,0, 1);
    ipcs->child_in_sim++;
    release_sem(ipcs->sem_in_sim,0, 1);

    ret = statistics->is_simulation_running;

    reserve_sem(ipcs->sem_out_sim,0, 1);
    ipcs->child_out_sim++;
    if((ipcs->master_wait_sim==1)&& (ipcs->child_in_sim == ipcs->child_out_sim)){
        release_sem(ipcs->sem_wrt, 0 ,1);
    }
    release_sem(ipcs->sem_out_sim,0, 1);
    

    return statistics->is_simulation_running;

}