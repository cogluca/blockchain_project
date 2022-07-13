#pragma once


#include "transaction_pool.h"




/*
    Contiene SO_REGISTRY_SIZE blocks.
    Ogni block contiene SO_BLOCK_SIZE transazioni
*/
typedef struct bookkeeper {
    transaction_block _block_array[SO_REGISTRY_SIZE]; 

    int capacity;
    int size;
} bookkeeper;

