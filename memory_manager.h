//
//  memory_manager.h
//  
//
//  Created by XueFei Yang on 2015-03-25.
//
//

#define LEFT_SIDE 1
#define RIGHT_SIDE 2

typedef struct{
    void* pointer;
    unsigned int size;
    unsigned int used;
    unsigned int side;
}Block;


void *get_memory(int size);
void release_memory(void *p);
void *grow_memory(int size, void *p);
void *pregrow_memory(int size, void *p);
int start_memory(int size);
void end_memory(void);
void print_memory();
