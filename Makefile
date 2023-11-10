.PHONY: test1, test2, test3, rimuovi, sort, exec

compile: unboundedqueue.h unboundedqueue.c myutil.h myutil.c mastColl.h mastColl.c main.c
		gcc -g -Wall -pedantic unboundedqueue.h myutil.h mastColl.h unboundedqueue.c myutil.c mastColl.c main.c -o exec -lpthread -lm

test1: exec
		./exec path 1

test2: exec
		./exec path 5

test3: exec
		valgrind --leak-check=full ./exec path 5
