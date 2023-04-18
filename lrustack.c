/**
 * @author ECE 3058 TAs
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "lrustack.h"

/**
 * This file contains some starter code to get you started on your LRU implementation. 
 * You are free to implement it however you see fit. You can design it to emulate how this
 * would be implemented in hardware, or design a purely software stack. 
 * 
 * We have broken down the LRU stack's
 * job/interface into two parts:
 *  - get LRU: gets the current index of the LRU block
 *  - set MRU: sets a certain block's index as the MRU. 
 * If you implement it using these suggestions, you will be able to test your LRU implementation
 * using lrustacktest.c
 * 
 * NOTES: 
 *      - You are not required to use this LRU interface. Feel free to design it from scratch if 
 *      that is better for you.
 *      - This will not behave like your traditional LIFO stack  
 */

/**
 * Function to initialize an LRU stack for a cache set with a given <size>. This function
 * creates the LRU stack. 
 * 
 * @param size is the size of the LRU stack to initialize. 
 * @return the dynamically allocated stack. 
 */
lru_stack_t* init_lru_stack(int size) {
    //  Use malloc to dynamically allocate a lru_stack_t
	lru_stack_t* stack = (lru_stack_t*) malloc(sizeof(lru_stack_t));
    //  Set the stack size the caller passed in
    stack->size = size;
    stack->lru_queue = malloc(size * sizeof(lru_stack_t));
    for (int i = 0; i < size; i++)
    {
        stack->lru_queue[i] = i;
    }
    stack->mru = 0;
    stack->lru = size - 1;

	return stack;
}



/**
 * Function to get the index of the least recently used cache block, as indicated by <stack>.
 * This operation should not change/mutate your LRU stack. 
 * 
 * @param stack is the stack to run the operation on.
 * @return the index of the LRU cache block.
 */
int lru_stack_get_lru(lru_stack_t* stack) {

    
    return stack->lru;
}


/**
 * Function to mark the cache block with index <n> as MRU in <stack>. This operation should
 * change/mutate the LRU stack.
 * 
 * @param stack is the stack to run the operation on.
 * @param n the index to promote to MRU.  
 */
void lru_stack_set_mru(lru_stack_t* stack, int n, bool hitted) {
    if (hitted)
    {
        stack->mru = n;
        for (int i = n; i > 0; i--)
        {
            stack->lru_queue[i] = stack->lru_queue[i - 1];

        }
        stack->lru = stack->lru_queue[(stack->size) - 1];
        stack->lru_queue[0] = stack->mru;
        return;
    }
    //For loop useful to shift array of number downwards
    stack->mru = n;
    for (int i = stack->size - 1; i > 0; i--)
    {
        stack->lru_queue[i] = stack->lru_queue[i - 1];

    }
    stack->lru = stack->lru_queue[(stack->size) - 1];
    stack->lru_queue[0] = stack->mru;
    
}

/**
 * Function to free up any memory you dynamically allocated for <stack>
 * 
 * @param stack the stack to free
 */
void lru_stack_cleanup(lru_stack_t* stack) {

    free(stack->lru_queue);
    free(stack);        // Free the stack struct we malloc'd
}