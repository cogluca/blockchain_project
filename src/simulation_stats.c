
#include <include/simulation_stats.h>
#include <include/ipc.h>
int is_simulation_running_read(simulation_stats *statistics, int wrt_id, int rd_id)
{
   /* int ret = 0;
    
    reserve_sem(rd_id, 0, 1);

    statistics->num_reader++;
    if (statistics->num_reader == 1)
        reserve_sem(wrt_id, 0, 1);

    release_sem(rd_id, 0, 1);

    ret = statistics->is_simulation_running;

    reserve_sem(rd_id, 0, 1);
    statistics->num_reader--;
    if (statistics->num_reader == 0)
        release_sem(wrt_id, 0, 1);

    release_sem(rd_id, 0, 1);*/

    return statistics->is_simulation_running;

}