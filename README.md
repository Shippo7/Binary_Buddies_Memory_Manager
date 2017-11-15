# Description
It is a memory management system using “binary buddies” strategy.
This program includes two files “memory_manager.c” and “memory_manager.h”. I also provided a test program “Test.c” for debug and to test how is it works. A makefile for compiler is also provided.

This program has following functions:

### int start memory( int size )
This function will initialize a memory space with “size” bytes. The memory space includes a header and the available free space. The header storages the information of the free space. As we using binary buddies, the available free space will be size of (2^i). To keep this size, there may some space be wasted.<br /><br />
For example, if the user allocates a memory space with 3000 bytes:<br />
![start_memory](http://i.v2ex.co/8r3SPtn7.png)

### void *get memory( int size )
To allocate a memory partition with “size” bytes in it. Each memory partition has a header to storage the memory information, so the actual size of memory partition is (2^i) that includes “user required size” + “header size”.<br />
![get_memory](http://i.v2ex.co/0E8dQ6pA.png)

### void release memory( void *p )
To release memory partition referenced by pointer “p” back to free space and combine the free space partitions if possible (with binary buddies rule).

### void *grow memory( int size, void *p )
First, the program will find the memory partition with the pointer “p”. Then, the program will backup current memory status. After that, the memory partition will be released and the program will try to find a new place for the partition with the new size. If the program finds a suitable space, it will copy the memory content to the new space. Otherwise, it will recover the old memory status from the backup.

### void *pregrow memory( int size, void *p )
Similar with the grow_memory(). But any extra space is allocated at the front of the partition.

### void end memory( void )
Release the memory space and then exit the program.

# Test Case
I provided a test program to show how this memory manager program works. In each step, this program will show the memory status graphically. 
## Usage:
function: command<br />
get_memory: add &lt;size><br />
grow_memory: grow &lt;new size> <pointer><br />
pregrow_memory: pregrow &lt;new size> <pointer><br />
release_memory: release &lt;pointer><br />
end_memory: end<br />

# License
This program is released under the MIT License  
http://opensource.org/licenses/MIT
