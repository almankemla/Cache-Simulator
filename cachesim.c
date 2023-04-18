/**
 * @author ECE 3058 TAs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cachesim.h"

// Statistics you will need to keep track. DO NOT CHANGE THESE.
counter_t accesses = 0;     // Total number of cache accesses
counter_t hits = 0;         // Total number of cache hits
counter_t misses = 0;       // Total number of cache misses
counter_t writebacks = 0;   // Total number of writebacks

/**
 * Function to perform a very basic log2. It is not a full log function, 
 * but it is all that is needed for this assignment. The <math.h> log
 * function causes issues for some people, so we are providing this. 
 * 
 * @param x is the number you want the log of.
 * @returns Techinically, floor(log_2(x)). But for this lab, x should always be a power of 2.
 */
int simple_log_2(int x) {
    int val = 0;
    while (x > 1) {
        x /= 2;
        val++;
    }
    return val; 
}

//  Here are some global variables you may find useful to get you started.
//      Feel free to add/change anyting here.
cache_set_t* cache;     // Data structure for the cache
int block_size;         // Block size.
int cache_size;         // Cache size.
int ways;               // Ways.
int num_sets;           // Number of sets.
int num_offset_bits;    // Number of offset bits.
int num_index_bits;     // Number of index bits.
int tag_length;         // Number of tag bits.

/**
 * Function to intialize your cache simulator with the given cache parameters. 
 * Note that we will only input valid parameters and all the inputs will always 
 * be a power of 2.
 * 
 * @param _block_size is the block size in bytes
 * @param _cache_size is the cache size in bytes
 * @param _ways is the associativity
 */
void cachesim_init(int _block_size, int _cache_size, int _ways) {
    // Set cache parameters to global variables
    block_size = _block_size;
    cache_size = _cache_size;
    ways = _ways;


    //Calculate number of sets
    num_sets = cache_size / (block_size * ways);

    //Calculate number of index bits
    num_index_bits = simple_log_2(num_sets);

    //Calculate number of offset bits
    num_offset_bits = simple_log_2(block_size);

    cache = malloc(num_sets * sizeof(cache_set_t));
    for (int i = 0; i < num_sets; i++)
    {
        cache[i].size = ways;
        cache[i].stack = init_lru_stack(ways);
        cache[i].taken = 0;
        cache[i].blocks = calloc(ways, sizeof(cache_block_t));
    }
}

/**
 * Function to perform a SINGLE memory access to your cache. In this function, 
 * you will need to update the required statistics (accesses, hits, misses, writebacks)
 * and update your cache data structure with any changes necessary.
 * 
 * @param physical_addr is the address to use for the memory access. 
 * @param access_type is the type of access - 0 (data read), 1 (data write) or 
 *      2 (instruction read). We have provided macros (MEMREAD, MEMWRITE, IFETCH)
 *      to reflect these values in cachesim.h so you can make your code more readable.
 */
void cachesim_access(addr_t physical_addr, int access_type) {
    accesses++;
    int count = 0;
    //Calculate number of tag bits
    tag_length = 64 - num_index_bits - num_offset_bits;

    int index_mask = ((1 << num_index_bits) - 1);
    int tag_mask = ((1 << tag_length) - 1);
    int offset_mask = (1 << num_offset_bits) - 1;

    int line_addr = (1 << (num_index_bits + tag_length)) - 1;

    int addr_offset = (physical_addr >> (1 - 1)) & offset_mask;
    int addr_index = (physical_addr >> (num_offset_bits + 1 - 1)) & index_mask;
    int addr_tag = (physical_addr >> (num_offset_bits + num_index_bits + 1 - 1)) & tag_mask;
    
    if (access_type == 0) //incase of memory read
    {
        read_from_cache(physical_addr, addr_offset, addr_index, addr_tag, access_type);
        return;
    } else if (access_type == 1) //incase of memory write
    {
        write_to_cache(physical_addr, addr_offset, addr_index, addr_tag, access_type);
        return;
    } else if (access_type == 2) //incase of instruction fetch
    {
        read_from_cache(physical_addr, addr_offset, addr_index, addr_tag, access_type);
        return;
    }

}
/**
 * Function to perform a write to memory 
 * 
 * @param physical_addr is the address to use for the memory access. 
 * @param addr_offset is the address offset
 * @param addr_index is the address index
 * @param addr_tag is the address tag
 * @param access is the type of access from mem in this case a write
 */

void write_to_cache(addr_t addr, int addr_offset, int addr_index, int addr_tag, int access) {
    bool hitted = false;
    int place;
    int count;
    for (int i = 0; i < ways; ++i)
    {
        //most straightforward situation
        if (cache[addr_index].blocks[i].tag == addr_tag && cache[addr_index].blocks[i].valid == 1)
        {
            hits++;
            hitted = true;
            count++;
            lru_stack_set_mru(cache[addr_index].stack, cache[addr_index].blocks[i].placement, hitted);
            if (cache[addr_index].blocks[place].dirty == 1) //if wb necessary
            {
                writebacks++;
                cache[addr_index].blocks[place].dirty = 1;
            }
            break; //If memory being read is in cache, do nothing and move on
        }
    }

    if (count != ways)
    {
        misses++;
        for (int i = 0; i < ways; ++i)
        {
            if (cache[addr_index].blocks[i].valid == 0)
            {
                lru_stack_set_mru(cache[addr_index].stack, i, false);
                cache[addr_index].blocks[place].tag = addr_tag;
                cache[addr_index].blocks[place].valid = 1;
                cache[addr_index].blocks[place].dirty = 1;
                cache[addr_index].blocks[place].placement = place;
                return;
            }
        }
    }
    if (hitted == false)
    {
        misses++;
        place = lru_stack_get_lru(cache[addr_index].stack);//get lru and override

        //if all places in index is taken, implement LRU
        if (cache[addr_index].taken != ways)
        {
            cache[addr_index].taken++;

        } //space within set
        if (cache[addr_index].blocks[place].dirty == 1) //if wb necessary
        {
            writebacks++;
        }
        lru_stack_set_mru(cache[addr_index].stack, cache[addr_index].blocks[place].placement, false);
        cache[addr_index].blocks[place].tag = addr_tag;
        cache[addr_index].blocks[place].valid = 1;
        cache[addr_index].blocks[place].dirty = 1;
        cache[addr_index].blocks[place].placement = place;
            
        //Update MRU
    }
    //update blocks


}

/**
 * Function to perform a read/instruction to memory 
 * 
 * @param physical_addr is the address to use for the memory access. 
 * @param addr_offset is the address offset
 * @param addr_index is the address index
 * @param addr_tag is the address tag
 * @param access is the type of access from mem in this case a write
 */
void read_from_cache(addr_t addr, int addr_offset, int addr_index, int addr_tag, int access) {
    bool hitted = false;
    int place;
    int count;
    for (int i = 0; i < ways; ++i)
    {
        //most straightforward situation
        if (cache[addr_index].blocks[i].valid == 1)
        {
            count++;
        }
        if (cache[addr_index].blocks[i].tag == addr_tag && cache[addr_index].blocks[i].valid == 1)
        {
            hits++;
            hitted = true;
            lru_stack_set_mru(cache[addr_index].stack, cache[addr_index].blocks[i].placement, hitted);
        return; //If memory being read is in cache, do nothing and move on
        }
    }

    if (count != ways)
    {
        misses++;
        for (int i = 0; i < ways; ++i)
        {
            if (cache[addr_index].blocks[i].valid == 0)
            {
                lru_stack_set_mru(cache[addr_index].stack, i, false);
                cache[addr_index].blocks[place].tag = addr_tag;
                cache[addr_index].blocks[place].valid = 1;
                cache[addr_index].blocks[place].dirty = 0;
                cache[addr_index].blocks[place].placement = place;
                return;
            }
        }
    }
    //If memory not in cache, fetch from mem and update parameters
    if (hitted == false)
    {
        misses++;
        place = lru_stack_get_lru(cache[addr_index].stack);//get lru and override

        //if all places in index is taken, implement LRU
        if (cache[addr_index].taken != ways)
        {
            cache[addr_index].taken++;

        } //space within set
        if (cache[addr_index].blocks[place].dirty == 1) //if wb necessary
        {
            writebacks++;
        }
        lru_stack_set_mru(cache[addr_index].stack, cache[addr_index].blocks[place].placement, false);
        cache[addr_index].blocks[place].tag = addr_tag;
        cache[addr_index].blocks[place].valid = 1;
        cache[addr_index].blocks[place].dirty = 0;
        cache[addr_index].blocks[place].placement = place;
            
        //Update MRU
        
    }
    
}
/**
 * Function to free up any dynamically allocated memory you allocated
 */
void cachesim_cleanup() {
    for (int i = 0; i < num_sets; i++)
    {
        free(cache[i].blocks);
    }

    free(cache);
    ////////////////////////////////////////////////////////////////////
    //  TODO: Write the code to do any heap allocation cleanup
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    //  End of your code   
    ////////////////////////////////////////////////////////////////////
}

/**
 * Function to print cache statistics
 * DO NOT update what this prints.
 */
void cachesim_print_stats() {
    printf("%llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);  
}

/**
 * Function to open the trace file
 * You do not need to update this function. 
 */
FILE *open_trace(const char *filename) {
    return fopen(filename, "r");
}

/**
 * Read in next line of the trace
 * 
 * @param trace is the file handler for the trace
 * @return 0 when error or EOF and 1 otherwise. 
 */
int next_line(FILE* trace) {
    if (feof(trace) || ferror(trace)) return 0;
    else {
        int t;
        unsigned long long address, instr;
        fscanf(trace, "%d %llx %llx\n", &t, &address, &instr);
        cachesim_access(address, t);
    }
    return 1;
}

/**
 * Main function. See error message for usage. 
 * 
 * @param argc number of arguments
 * @param argv Argument values
 * @returns 0 on success. 
 */
int main(int argc, char **argv) {
    FILE *input;

    if (argc != 5) {
        fprintf(stderr, "Usage:\n  %s <trace> <block size(bytes)>"
                        " <cache size(bytes)> <ways>\n", argv[0]);
        return 1;
    }
    
    input = open_trace(argv[1]);
    cachesim_init(atol(argv[2]), atol(argv[3]), atol(argv[4]));
    while (next_line(input));
    cachesim_print_stats();
    cachesim_cleanup();
    fclose(input);
    return 0;
}

