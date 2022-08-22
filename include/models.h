#pragma once
#define SO_REGISTRY_SIZE  1000
#define SO_BLOCK_SIZE  10
#define SENDER_TRANSACTION_REWARD   -1


typedef struct _transaction {

    unsigned int timestamp;
    unsigned int sender;
    unsigned int receiver;
    unsigned int money_sum;
    unsigned int paid_reward;

} transaction;
