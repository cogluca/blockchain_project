#include <include/config.h>
#include <stdio.h>

/**
 * @return
 */
void getParameters(parameters* params) {
    FILE *f = fopen(CONFIG_FILE,"r");
    fscanf(f,"SO_USERS_NUM=%d\n",&params->SO_USERS_NUM);
    fscanf(f,"SO_NODES_NUM=%d\n",&params->SO_NODES_NUM);
    fscanf(f,"SO_BUDGET_INIT=%d\n",&params->SO_BUDGET_INIT);
    fscanf(f,"SO_REWARD=%d\n",&params->SO_REWARD);
    fscanf(f,"SO_MIN_TRANS_GEN_NSEC=%d\n",&params->SO_MIN_TRANS_GEN_NSEC);
    fscanf(f,"SO_MAX_TRANS_GEN_NSEC=%d\n",&params->SO_MAX_TRANS_GEN_NSEC);
    fscanf(f,"SO_RETRY=%d\n",&params->SO_RETRY);
    fscanf(f,"SO_TP_SIZE=%d\n",&params->SO_TP_SIZE);
    fscanf(f,"SO_MIN_TRANS_PROC_NSEC=%d\n",&params->SO_MIN_TRANS_PROC_NSEC);
    fscanf(f,"SO_MAX_TRANS_PROC_NSEC=%d\n",&params->SO_MAX_TRANS_PROC_NSEC);
    fscanf(f,"SO_SIM_SEC=%d\n",&params->SO_SIM_SEC);
    fclose(f);

}
