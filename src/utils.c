#include <include/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void set_seed(){

    unsigned int seed;
    FILE *urandom = fopen("/dev/urandom", "r");
    fread(&seed, sizeof(int), 1, urandom);
    fclose(urandom);    
    srand(seed);

}
int my_random(int min_val, int max_val)
{
    set_seed();
    return min_val + (int)((double)rand() / RAND_MAX * (max_val - min_val + 1));
}

