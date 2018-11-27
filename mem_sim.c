/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Instructor: Boris Grot
 * *
 * *    TA: Siavash Katebzadeh
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
/* Do not add any more header files */

/*
 * Various structures
 */
typedef enum {FIFO, LRU, Random} replacement_p;

const char* get_replacement_policy(uint32_t p) {
    switch(p) {
    case FIFO: return "FIFO";
    case LRU: return "LRU";
    case Random: return "Random";
    default: assert(0); return "";
    };
    return "";
}

typedef struct {
    uint32_t address;
} mem_access_t;

// These are statistics for the cache and should be maintained by you.
typedef struct {
    uint32_t cache_hits;
    uint32_t cache_misses;
} result_t;


/*
 * Parameters for the cache that will be populated by the provided code skeleton.
 */

replacement_p replacement_policy = FIFO;
uint32_t associativity = 0;
uint32_t number_of_cache_blocks = 0;
uint32_t cache_block_size = 0;


/*
 * Each of the variables below must be populated by you.
 */
uint32_t g_num_cache_tag_bits = 0;
uint32_t g_cache_offset_bits= 0;
result_t g_result;


/* Reads a memory access from the trace file and returns
 * 32-bit physical memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1002];
    char* token = NULL;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file)!= NULL) {
        /* Get the address */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtoul(token, NULL, 16);
        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_cache_tag_bits, uint32_t cache_offset_bits, result_t* r) {
    /* Do Not Modify This Function */

    uint32_t cache_total_hits = r->cache_hits;
    uint32_t cache_total_misses = r->cache_misses;
    printf("CacheTagBits:%u\n", num_cache_tag_bits);
    printf("CacheOffsetBits:%u\n", cache_offset_bits);
    printf("Cache:hits:%u\n", r->cache_hits);
    printf("Cache:misses:%u\n", r->cache_misses);
    printf("Cache:hit-rate:%2.1f%%\n", cache_total_hits / (float)(cache_total_hits + cache_total_misses) * 100.0);
}

/*
 *
 * Add any global variables and/or functions here as needed.
 *
 */

/* Defines the structure of a cache block address */

typedef struct {
    uint32_t tag;
    uint32_t index;
    uint32_t offset;
} cache_block;




/* Sorting algorithm to keep track of least access block for fully associated LRU cache */


void sort_LRU(uint32_t index_associativity, uint32_t last_access_index, cache_block* accesses) {
    cache_block address = accesses[last_access_index];
    if (last_access_index != index_associativity - 1) {
        for(int i = last_access_index; i < index_associativity - 1; i++) {
            accesses[i] = accesses[i+1];
        }
        accesses[index_associativity - 1] = address;
    }

}


// Global variables and/or functions end


int main(int argc, char** argv) {
    time_t t;
    /* Intializes random number generator */
    /* Important: *DO NOT* call this function anywhere else. */
    srand((unsigned) time(&t));
    /* ----------------------------------------------------- */
    /* ----------------------------------------------------- */

    /*
     *
     * Read command-line parameters and initialize configuration variables.
     *
     */
    int improper_args = 0;
    char file[10000];
    if (argc < 6) {
        improper_args = 1;
        printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */
        if (strcmp(argv[1], "FIFO") == 0) {
            replacement_policy = FIFO;
        } else if (strcmp(argv[1], "LRU") == 0) {
            replacement_policy = LRU;
        } else if (strcmp(argv[1], "Random") == 0) {
            replacement_policy = Random;
        } else {
            improper_args = 1;
            printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
        }
        associativity = atoi(argv[2]);
        number_of_cache_blocks = atoi(argv[3]);
        cache_block_size = atoi(argv[4]);
        strcpy(file, argv[5]);
    }
    if (improper_args) {
        exit(-1);
    }
    assert(number_of_cache_blocks == 16 || number_of_cache_blocks == 64 || number_of_cache_blocks == 256 || number_of_cache_blocks == 1024);
    assert(cache_block_size == 32 || cache_block_size == 64);
    assert(number_of_cache_blocks >= associativity);
    assert(associativity >= 1);

    printf("input:trace_file: %s\n", file);
    printf("input:replacement_policy: %s\n", get_replacement_policy(replacement_policy));
    printf("input:associativity: %u\n", associativity);
    printf("input:number_of_cache_blocks: %u\n", number_of_cache_blocks);
    printf("input:cache_block_size: %u\n", cache_block_size);
    printf("\n");

    

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file = fopen(file,"r");
    if (!ptr_file) {
        printf("Unable to open the trace file: %s\n", file);
        exit(-1);
    }

    /* result structure is initialized for you. */
    memset(&g_result, 0, sizeof(result_t));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task. */

    /* You may want to setup your Cache structure here. */
     


    uint32_t number_of_sets = number_of_cache_blocks / associativity;

    /* Defines the structure of a cache block address */

    /* Calculating the tag, index (where necessary) and byte offset of a physical memory address */

    if (associativity == 1) {
        uint32_t index_bits = (uint32_t)(log(number_of_cache_blocks)/log(2));
        g_cache_offset_bits = (uint32_t)(log(cache_block_size)/log(2));
        g_num_cache_tag_bits = 32 - g_cache_offset_bits - index_bits;
    }
    
    else if (associativity == number_of_cache_blocks)
    {
        g_cache_offset_bits = (uint32_t)(log(cache_block_size)/log(2));
        g_num_cache_tag_bits = 32 - g_cache_offset_bits;
    }

    else {
        uint32_t index_bits = (uint32_t)(log(number_of_sets)/log(2));
        g_cache_offset_bits = (uint32_t)(log(cache_block_size)/log(2));
        g_num_cache_tag_bits = 32 - g_cache_offset_bits - index_bits;
    }

    /* Calculating the tag, index (where necessary) and byte offset of a physical memory address */
    
    /* Initialize an array for the cache blocks */

    cache_block* cache = malloc ((number_of_cache_blocks + 100) * sizeof(cache_block));

    

    /* Initialize an array for each one of the (possible) block sets
       sets[i] = n if the ith set had the most recent hit/replacement in its nth block */

    uint32_t* sets = (uint32_t*) calloc (number_of_sets, sizeof(uint32_t));

    int i = 0;

    uint32_t* lru_trace = calloc (number_of_cache_blocks, sizeof(uint32_t));     // will keep evidence of the least recently used block from a set

   

    mem_access_t access;
    /* Loop until the whole trace file has been read. */
    while(1) {
        access = read_transaction(ptr_file);
        // If no transactions left, break out of loop.
        if (access.address == 0)
            break;

        /* Add your code here */

        uint32_t tag = access.address >> (32 - g_num_cache_tag_bits);
        uint32_t index;
        
        if (associativity == number_of_cache_blocks) {
            index = 0;
        }
        else { 
            index = (access.address << g_num_cache_tag_bits) >> (g_cache_offset_bits + g_num_cache_tag_bits);
        }

    
        int hit = 0;

        if (replacement_policy == FIFO)
        {
            if (sets[index/associativity] == associativity) {
                sets[index/associativity] = 0;                            // Resets the last accessed element of the set
            }
            int j = 0;
            for(int j = 0; j < associativity; j++){
                if(cache[j + index/associativity].tag == tag) {
                    g_result.cache_hits++;
                    hit = 1;
                    break;
                }
            }
            
            if (hit == 0) {
                g_result.cache_misses++;
                cache[index/associativity + sets[index/associativity]].tag = tag;
                cache[index/associativity + sets[index/associativity]].index = index;
                sets[index/associativity]++;
            }
            
        }

        int done = 0;
        

        if (replacement_policy == LRU) {
            for (int j = 0; j < associativity; j++) {
                lru_trace[j + index/associativity]++;
            }
            for(int j = 0; j < associativity; j++){
                if(cache[j + index/associativity].tag == tag) {
                    g_result.cache_hits++;
                    hit = 1;
                    lru_trace[j + index/associativity] = 0;
                    break;
                }
            }

            if (hit == 0) {
                g_result.cache_misses++;
                uint32_t max_val = 0;
                uint32_t max_val_index = 0;
                for(int j = 0; j < associativity; j++) {
                    // lru_trace[j + index/associativity]++;
                    if (lru_trace[j + index/associativity] > max_val) {
                        max_val = lru_trace[j + index/associativity];
                        max_val_index = j + index/associativity;
                    }
                }
                cache[max_val_index].tag = tag;
                cache[max_val_index].index = index;
                lru_trace[max_val_index] = 0;
            }
        }

        if (replacement_policy == Random) {

        }

        i++;      
                  
    }

    free(cache);
    free(sets);
    free(lru_trace);

    //g_result.cache_misses = i - g_result.cache_hits;

    /* Do not modify code below. */
    /* Make sure that all the parameters are appropriately populated. */
    print_statistics(g_num_cache_tag_bits, g_cache_offset_bits, &g_result);

    /* Close the trace file. */
    fclose(ptr_file);
    return 0;
}
