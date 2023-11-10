#include "myutil.h"

void * worker(void *);

void master(char** argv, struct sockaddr_un *);

void collector(struct sockaddr_un *);