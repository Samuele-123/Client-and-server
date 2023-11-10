#include "mastColl.h"

/*
Funzione worker: Prende in input la struct arg_t che ha 3 campi, la coda dove verranno caricati/scaricati le stringhe di output
il fd del socket server, e un mutex per la scrittura sul socket in mutua esclusione.
La funzione fa un ciclo infinito in cui tentiamo di estrarre elementi dalla coda, se non sono presenti ci sospendiamo, se sono presenti
controllia che la stringa ottenuta non sia la stringa di terminazione, in tal caso uesce dal while e re-inserisce la stringa di temrinazione nella coda e terminiamo
Calcola poi la stringa di output chiamamndo la funzione calcola, infine accedendo in mutua escluzione al socket scrive la stringa calcolata
*/

void * worker(void* arg){
    arg_t* par = (arg_t*)arg;
    int dim;


    while(1){
        char* path = pop(par->q);
        if(strcmp(path, "end")==0) break;

        char* print = calcola(path);

        PTHREAD_ERROR(pthread_mutex_lock(&par->mtx), 0, "Errore nella lock");

        dim = strlen(print)+1;
        SYSCALL_ERROR(write(par->server, &dim, sizeof(int)), -1, "Errore nella scrittura della dimensione sul socket", 1);
        SYSCALL_ERROR(write(par->server, print, dim), -1, "Errore nella scrittura del messaggio sul socket", 1);

        PTHREAD_ERROR(pthread_mutex_unlock(&par->mtx), 0, "Errore nella unlock");

        free(print);

        free(path);
    }
    SYSCALL_ERROR(push(par->q, "end"), -1, "Errore nella push del dato di terminazione sulla coda", 1);

    return NULL;
}

/*
Funzione master: prende in ingresso gli argomenti, e la struct sockaddr per le informazioni sull'indirizzo, creiamo 
il socket lato client e tentiamo la connessione, una volta stabilita istanziamo la struct arg_t passandogli la
coda inizializzata e il fd del server, creiamo poi W thread e chiamiamo la funzione leggi, dopo aver finito la leggi
pushamo la stringa di terminazione end per far sapere ai worker che non verranno caricati altri path.
Dopo aver fatto la join dei thread scriviamo sul fd del server che le comunicazioni sono finite e chiudiamo il socket
*/

void master(char** argv, struct sockaddr_un * sa){
    int W = atoi(argv[2]);
    int dimEnd = 5;

    int server;
    SYSCALL_ERROR((server=socket(AF_UNIX,SOCK_STREAM,0)), -1, "Errore nella creazione del socket lato client", 0);

    while (connect(server,(struct sockaddr*)sa, sizeof(*sa)) == -1 ) {
        if ( errno == ENOENT ) sleep(1);
        else{
            perror("Errore nella connessione");
            exit(EXIT_FAILURE); 
        }
    }

    arg_t par;
    SYSCALL_ERROR((par.q = initQueue()), NULL, "Errore nell'allocazione della coda", 1);
    par.server = server;

    PTHREAD_ERROR(pthread_mutex_init(&par.mtx, NULL), 0, "Errore nella creazione del mutex");

    pthread_t workerVect[W];

    for(int i = 0; i < W; i++){
        PTHREAD_ERROR(pthread_create(&workerVect[i], NULL, worker, (void*)&par), 0, "Errore nella creazione del thread");
    }

    leggi(argv[1], par);

    SYSCALL_ERROR(push(par.q, "end"), -1, "Errore nella push del dato di terminazione sulla coda", 1);

    for(int i = 0; i < W; i++){
        PTHREAD_ERROR(pthread_join(workerVect[i], NULL), 0, "Errore nella join del thread");
    }

    SYSCALL_ERROR(write(server, &dimEnd, sizeof(int)), -1, "Errore nell'invio della dimensione del messaggio di terminazione al socket", 1);
    SYSCALL_ERROR(write(server, "fine", 5), -1, "Errore nell'invio del messaggio di terminazione al socket", 1);
    close(server);

    PTHREAD_ERROR(pthread_mutex_destroy(&par.mtx), 0, "Errore nella distruzione del thread");
    deleteQueue(par.q);
}

/*
Funzione collector: Prende in ingresso la struct sockaddr_un per l'indirizzo, viene creato il socket e il processo
accetta la connessione con i client, dopodichè iniziamo a fare la read fintantochè non leggiamo la stringa di terminazione
Infinre chiudiamo il fd del client e del socket e rimuoviamo il file sock creato.
*/

void collector(struct sockaddr_un * sa){
    char * buffer;
    int dim;

    int server;
    SYSCALL_ERROR((server=socket(AF_UNIX, SOCK_STREAM, 0)), -1, "Errore nella creazione del socket lato server", 0);
    SYSCALL_ERROR(bind(server, (struct sockaddr *)sa, sizeof(*sa)), -1, "Errore nella bind", 0);
    SYSCALL_ERROR(listen(server, 1), -1, "Errore nella listen", 1);

    int clientFd;
    SYSCALL_ERROR((clientFd = accept(server, NULL, NULL)), -1, "Errore nell'accept", 1);

    printf("n\tavg\tstd\tfile\n");
    printf("----------------------------------------------------------------------\n");

    while(1){
        SYSCALL_ERROR(read(clientFd, &dim, sizeof(int)), -1, "Errore nella lettura della dimensione del messaggio", 1);
        MEMORY_ERROR((buffer = (char*)malloc(sizeof(char)*dim)));
        SYSCALL_ERROR(read(clientFd, buffer, dim), -1, "Errore nella lettura del messaggio dal socket", 1);
        if(strcmp(buffer, "fine") == 0) break;
        printf("%s\n", buffer);
        
        free(buffer);
    }

    free(buffer);
    close(clientFd);
    close(server);
    REMOVE_ERROR(remove("sock"), -1, "Errore nella rimozione del file sock");
}
