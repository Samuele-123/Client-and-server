#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include "unboundedqueue.h"

#define REMOVE_ERROR(sc, ret, str)	\
    if (sc == ret) {				\
	    perror(str);				\
        exit(EXIT_FAILURE);         \
    }

#define SYSCALL_ERROR(sc, ret, str, flag)	\
    if (sc == ret) {				\
	    perror(str);				\
        if(flag == 1) REMOVE_ERROR(remove("sock"), -1, "Errore nella rimozione del file sock"); \
        exit(EXIT_FAILURE);         \
    }

#define MEMORY_ERROR(value) \
    if(value == NULL){      \
        printf("Errore nell'allocazione della memoria\n");  \
        REMOVE_ERROR(remove("sock"), -1, "Errore nella rimozione del file sock"); \
        exit(EXIT_FAILURE);     \
    }

#define PTHREAD_ERROR(sc, ret, str)	\
    if (sc != ret) {				\
	    printf("%s\n", str);		\
        REMOVE_ERROR(remove("sock"), -1, "Errore nella rimozione del file sock"); \
        exit(EXIT_FAILURE);         \
    }

#define maxPathUnix 108
#define socketName "sock"

typedef struct arg_t{
    Queue_t* q;
    int server;
    pthread_mutex_t mtx;
}arg_t;

void check_arg(int argc, char** argv);

int checkInt(char* number);

char* calcola(const char *);

void leggi(char *, arg_t);