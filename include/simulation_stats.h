#pragma once

typedef struct _sim_stats {
    
    /* is user[i] still alive? */
    int user_status[1000];

    /* How much transaction has each node inside his transaction pool at termination */
    int node_status[10];
    

    /* 0 when simulation is finished. flag for notify children processes */
    int is_simulation_running;
    
    /* process number reading is_simulation_running flag */
    int num_reader;


} simulation_stats;


int is_simulation_running_read(simulation_stats *statistics, int wrt_id, int rd_id);