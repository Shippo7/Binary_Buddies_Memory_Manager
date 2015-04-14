//
//  Test.c
//  
//
//  Created by XueFei Yang on 2014-07-26.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"

int main(){
    int start = 0;
    printf("start_memory: ");
    scanf("%i", &start);
    
    if(start_memory(start)==0){
        printf("Start Memory Error\n");
        return 1;
    }
    print_memory();
    
    int number;
    char command[10];
    void *pointer = NULL;
    while(1){
        printf("> ");
        scanf("%s", command);
        
        if(strcmp(command, "get")==0){
            scanf("%d", &number);
            void* newPointer = get_memory(number);
            
            if(newPointer==NULL){
                printf("Not enough Memory Space\n");
            }
            else{
                print_memory();
                printf("Placed new memory partition successful\nAddress: %p\n", newPointer);
            }
        }
        else if(strcmp(command, "release")==0){
            scanf("%p", &pointer);
            release_memory(pointer);
            print_memory();
        }
        else if(strcmp(command, "grow")==0){
            scanf("%d", &number);
            scanf("%p", &pointer);
            grow_memory(number, pointer);
            print_memory();
        }
        else if(strcmp(command, "pregrow")==0){
            scanf("%d", &number);
            scanf("%p", &pointer);
            pregrow_memory(number, pointer);
            print_memory();
        }
        else if(strcmp(command, "end")==0){
            end_memory();
            break;
        }
        else{
            printf("Invalid Command\n");
        }
    }
    
    return 0;
}
