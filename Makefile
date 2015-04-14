CFLAGS = -lm

all: test

test: memory_manager.o
	gcc $(CFLAGS) -o test memory_manager.o Test.c

memory_manager.o: memory_manager.c memory_manager.h
	gcc $(CFLAGS) -c memory_manager.c

clean:
	-rm memory_manager.o test

