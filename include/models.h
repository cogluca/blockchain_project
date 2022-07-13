#pragma once
#define SO_REGISTRY_SIZE  1000
#define SO_BLOCK_SIZE  10
#define SENDER_TRANSACTION_REWARD   -1


typedef struct _transaction {

    int timestamp;
    int sender;
    int receiver;
    int money_sum;
    int paid_reward;

} transaction;
