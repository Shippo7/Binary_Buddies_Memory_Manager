//
//  memory_manager.c
//  
//
//  Created by XueFei Yang on 2015-03-25.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "memory_manager.h"

/* Global variables */
static int memory_size = 0;
static int free_space = 0;
static void* memory;
static int partition = 0;

/* To calculate the binary buddies partition size */
int BB_size (int size){
    int i;
    int result = 0;
    for(i=0; pow(2, i+1)<=size; i++);
    result = pow(2, i);
    
    return result;
}

int BB_size2 (int size){
    int i;
    int result = 0;
    for(i=0; pow(2, i)<size; i++);
    result = pow(2,i);
    
    return result;
}

/* To find a best fit memory partition */
Block* find_hole(int size){
    Block* best_block = NULL;
    Block* current_block = memory;
    
    /* first, we have to find a un-used memory partition */
    while(current_block->used != 0 || current_block->size < size+sizeof(Block)){
        if(((void *)current_block+(current_block->size)) >= memory+memory_size){
            return NULL;
        }
        current_block = (void *)current_block+(current_block->size);
    }
    best_block = current_block;
    
    /* find the next memory partition */
    while(((void *)current_block+(current_block->size))<memory+memory_size){
        current_block = (void *)current_block+(current_block->size);
        /* if the next memory partition exists, it not being used and the size is appropriate, it is the best_block so far*/
        if(current_block->size < best_block->size && current_block->size >= size+sizeof(Block)&& current_block->used == 0){
            best_block = current_block;
        }
    }
    
    return best_block;
}

/* split a memory partition */
void split_block(Block* current) {
    Block* new_block = (void *)current+((current->size)/2);
    new_block->pointer = (void *)new_block+sizeof(Block);
    new_block->size = (current->size)/2;
    new_block->used = 0;
    new_block->side = RIGHT_SIDE;
    
    current->size = (current->size)/2;
    current->side = LEFT_SIDE;
}

void *get_memory(int size){
    
    /* if the memory not initialized or get memory size is invalid, return NULL */
    if(free_space==0||size<=0){
        printf("invalid size!\n");
        return NULL;
    }
    
    /* if the new memory partition size is bigger than free memory size, return NULL */
    if(size>free_space){
        printf("No free space!");
        return NULL;
    }
    
    /* Thread safe: no more than one thread can find the hole or add new block at the same time */
    Block* fit = find_hole(size);
    
    /* if there is no hole fit the new memory partition, return NULL */
    if(fit == NULL){
        printf("No fit!\n");
        return NULL;
    }
    
    /* find the best fit free space */
    int fit_size = fit->size;
    
    while((size+sizeof(Block))*2<=fit_size){
        /* if size of the free block is bigger than the new memory partition size *2, split it */
        split_block(fit);
        fit_size = fit->size;
    }
    
    free_space -= size+sizeof(Block);
    
    /* mark the memory space have been used */
    partition++; //invoke a new memory partition ID
    
    fit->used = partition;
    
    
    return fit->pointer;
}

/* To find the memory partition in the binary tree with given pointer */
Block* find_block_list(void *p){
    Block *current_block = memory;
    
    while((void *)current_block < memory+memory_size){
        if(current_block->pointer == p){
            return current_block;
        }
        
        current_block = (void *)current_block+(current_block->size);
    }
    
    return NULL;
}

/* Try to combine the memory partition */
void *combine_block (Block *current){
    if(current->side == LEFT_SIDE){
        current->size = (current->size)*2;
        
        return current;
    }
    else{
        Block *previous_block = current-(current->size);
        previous_block->size = (previous_block->size)*2;
        previous_block->side = LEFT_SIDE;
        
        return previous_block;
    }
}

void release_memory(void *p){
    Block *current = find_block_list(p);
    Block *next_block = NULL;
    Block *previous_block = NULL;
    
    if(current == NULL){
        return;
    }
    
    /* release current memory partition */
    current->used = 0;
    
    free_space += current->size;
    
    /* check if the current memory partition can be combine with its neighbour, combine them */
    while(1){
        /* if current partition is on the LEFT_SIDE, the next partition exist, same size and not being used, combine them */
        if(current->side == LEFT_SIDE && ((void *)current+(current->size)) < memory+memory_size){
            next_block = (void *)current+(current->size);
            if(next_block->used == 0 && next_block->size == current->size){
                current = combine_block(current);
            }
            else{
                break;
            }
        }
        /* if current partition is on the RIGHT_SIDE, the previous partition exist, same size and not being used, combine them */
        else if(current->side == RIGHT_SIDE && ((void *)current-(current->size)) >= memory){
            previous_block = (void *)current-(current->size);
            if(previous_block->used == 0 && previous_block->side == LEFT_SIDE && previous_block->size == current->size){
                current = combine_block(previous_block);
            }
            else{
                break;
            }
        }
        else{
            break;
        }
    }
}


void *grow_memory(int size, void *p){
    Block *current = find_block_list(p);
    
    /* if we cannot found the memory partition by *p, return NULL */
    if(current == NULL){
        return NULL;
    }
    
    /* calculate the new binary buddies memory partition size (2^i) */
    int new_size = size+sizeof(Block);
    new_size = BB_size2(new_size);
    
    /* if the new size is bigger than total free space, return NULL */
    if(new_size > free_space){
        return NULL;
    }
    /* if the new memory partition size is bigger than old size, try to relocate the partition if the current partition cannot be grown in-place. */
    else if(new_size > (current->size)){
        /* first, we have to backup the current memory status. we will try to release current memory partition, combine the partition if it is possible, and then try to find a new location that fit the new partition size. If we cannot find an appoprate space, we will recover the old memory status from the backup. */
        /* to count how many memory partitions so far */
        int count = 0;
        Block *current_block = memory;
        while((void *)current_block < memory+memory_size){
            count++;
            current_block = (void *)current_block+(current_block->size);
        }
        /* create an array and backup the current memory information */
        Block backup[count];
        current_block = memory;
        int i;
        for(i=0; i<count; i++){
            backup[i].pointer = current_block->pointer;
            backup[i].size = current_block->size;
            backup[i].used = current_block->used;
            backup[i].side = current_block->side;
            current_block = (void *)current_block+(current_block->size);
        }
        /* release current memory partition and try to find a new fit space with the new size */
        release_memory(current->pointer);
        void *new_block = get_memory(size);
        /* if we relocate the memory partition successful, copy the contents of memory from the old partition to the new partition */
        if(new_block != NULL){
            memcpy(new_block, (current->pointer), (current->size));
            
            return new_block;
        }
        /* if we cannot relocate the partition, recover the memory from the backup */
        else{
            current_block = memory;
            for(i=0; i<count ; i++){
                current_block->pointer = backup[i].pointer;
                current_block->size = backup[i].size;
                current_block->used = backup[i].used;
                current_block->side = backup[i].side;
                current_block = (void *)current_block+(current_block->size);
            }
            free_space -= current->size;
            
            return NULL;
        }
    }
    /* If the new memory partition size is smaller than what is currently allocated, then space is truncated */
    else if(new_size < (current->size)){

        /* split the current partition until the partition size reached the new size */
        while(current->size > new_size){
            split_block(current);
        }
        
        return (void *)current;
    }
    /* if the new size is equal to the old size, return NULL */
    else{
        return NULL;
    }
}

void *pregrow_memory(int size, void *p){
    Block *current = find_block_list(p);
    
    /* if we cannot found the memory partition by *p, return NULL */
    if(current == NULL){
        return NULL;
    }
    
    /* calculate the new binary buddies memory partition size (2^i) */
    int new_size = size+sizeof(Block);
    new_size = BB_size2(new_size);
    
    /* if the new size is bigger than total free space, return NULL */
    if(new_size > free_space){
        return NULL;
    }
    /* if the new memory partition size is bigger than old size, try to relocate the partition if the current partition cannot be grown in-place. */
    else if(new_size > (current->size)){

        /* first, we have to backup the current memory status. we will try to release current memory partition, combine the partition if it is possible, and then try to find a new location that fit the new partition size. If we cannot find an appoprate space, we will recover the old memory status from the backup. */
        /* to count how many memory partitions so far */
        int count = 0;
        Block *current_block = memory;
        while((void *)current_block < memory+memory_size){
            count++;
            current_block = (void *)current_block+(current_block->size);
        }
        /* create an array and backup the current memory information */
        Block backup[count];
        current_block = memory;
        int i;
        for(i=0; i<count; i++){
            backup[i].pointer = current_block->pointer;
            backup[i].size = current_block->size;
            backup[i].used = current_block->used;
            backup[i].side = current_block->side;
            current_block = (void *)current_block+(current_block->size);
        }
        /* release current memory partition and try to find a new fit space with the new size */
        release_memory(current->pointer);
        Block *new_block = get_memory(size);
        /* if we relocate the memory partition successful, copy the contents of memory from the old partition to the new partition */
        if(new_block != NULL){
            memcpy(new_block+((new_block->size) - (current->size)), (current->pointer), (current->size));
            
            return (void *)new_block;
        }
        /* if we cannot relocate the partition, recover the memory from the backup */
        else{
            current_block = memory;
            for(i=0; i<count ; i++){
                current_block->pointer = backup[i].pointer;
                current_block->size = backup[i].size;
                current_block->used = backup[i].used;
                current_block->side = backup[i].side;
                current_block = (void *)current_block+(current_block->size);
            }
            free_space -= current->size;
            
            return NULL;
        }
    }
    /* If the new memory partition size is smaller than what is currently allocated, then space is truncated */
    else if(new_size < (current->size)){

        /* truncate the memory content */
        memcpy((void *)current, (void *)current+((current->size)- new_size), new_size);
        /* split the current partition until the partition size reached the new size */
        while(current->size > new_size){
            split_block(current);
        }
        
        return (void *)current;
    }
    /* if the new size is equal to the old size, return NULL */
    else{
        return NULL;
    }

}


int start_memory(int size){
    printf("Memory partition header size: %lu\n", sizeof(Block));
    
    /* if the memory already initialized or the size is smaller than the memory header size, return false */
    if(memory_size!=0||size<sizeof(Block)){
        return 0;
    }
    
    /* calculate the free size without a header */
    int free_size = size-sizeof(Block);
    
    /* calculate the free size for the binary buddies memory (2^i) */
    free_size = BB_size(free_size);
    
    /* Allocate memory */
    memory = malloc(free_size);
    memory_size = free_size;
    
    if(memory == NULL){
        return 0;
    }
    
    Block *memory_list = memory;
    
    memory_list->pointer = (void *)memory+sizeof(Block);
    memory_list->size = free_size;
    memory_list->used = 0;
    memory_list->side = LEFT_SIDE;
    
    free_space = size-sizeof(Block);
    
    printf("memory address: %p\n", memory);
    
    return 1;
}

void end_memory(void){
    free(memory);
}

/* This method print the memory_list. It can be used for debug */
void print_memory(){
    printf("memory status:\n");
    printf("----------\n");
    
    Block *current = memory;
    
    while((void *)current<(memory+memory_size)){
        if(current->used == 0){
            printf("free size: %d\n", current->size);
            printf("header address: %p\n", current);
            printf("free space address: %p\n", current->pointer);
        }
        else{
            printf("P%d\n", current->used);
            printf("total size:%d\n", current->size);
            printf("header address: %p\n", current);
            printf("content address: %p\n", current->pointer);
        }
        printf("----------\n");
        current = (void *)current+(current->size);
    }
}
