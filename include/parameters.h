#pragma once


typedef struct _parameters {
    int SO_USERS_NUM;
    int SO_NODES_NUM;
    int SO_BUDGET_INIT;     
    int SO_REWARD;    
    int SO_MIN_TRANS_GEN_NSEC;     
    int SO_MAX_TRANS_GEN_NSEC; 
    int SO_RETRY;       
    int SO_TP_SIZE;  
    int SO_MIN_TRANS_PROC_NSEC;
    int SO_MAX_TRANS_PROC_NSEC;
    int SO_SIM_SEC;

} parameters;