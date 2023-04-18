/**
 * @author ECE 3058 TAs
 */

#ifndef __CACHESIM_H
#define __CACHESIM_H

// Use these in code to make mem read/mem write/inst read related code more readable
#define MEMREAD 0
#define MEMWRITE 1
#define IFETCH 2

#include "lrustack.h"

// Please DO NOT CHANGE the following two typedefs
typedef unsigned long long addr_t;		// Data type to hold addresses
typedef unsigned long long counter_t;	// Data type to hold cache statistic variables

/**
 * Struct for a cache block. Feel free to change any of this if you want. 
 */
typedef struct cache_block_t {
	int tag;
	int valid;
	int dirty;
	int placement; // recent here is used as a block position, 0 - way 0, n recent - num of ways

} cache_block_t;

/**
 * Struct for a cache set. Feel free to change any of this if you want. 
 */
typedef struct cache_set_t {
	int size;				// Number of blocks in this cache set
	int taken;
	lru_stack_t* stack;		// LRU Stack 
	cache_block_t* blocks;	// Array of cache block structs. You will need to
							// 	dynamically allocate based on number of blocks
							//	per set. 
} cache_set_t;

void cachesim_init(int block_size, int cache_size, int ways);
void read_from_cache(addr_t addr, int addr_offset, int addr_index, int addr_tag, int access);
void write_to_cache(addr_t addr, int addr_offset, int addr_index, int addr_tag, int access);
void cachesim_access(addr_t physical_add, int access_type);
void cachesim_cleanup(void);
void cachesim_print_stats(void);

#endif
