#include "mastColl.h"

/*
Funzione main: prende in ingrsso path e n_worker controlla che gli argomenti ricevuti siano giiusti
Crea la struct condivisa che conterrà le informazioni per la connessione, crea poi un nuovo processo,
il processo figlio sarà il collector, il padre il master
*/

int main(int argc, char** argv){
    check_arg(argc, argv);

    struct sockaddr_un sa;
    strncpy(sa.sun_path, socketName, maxPathUnix);
    sa.sun_family=AF_UNIX;

    pid_t pidC;
    SYSCALL_ERROR((pidC = fork()), -1, "Errore nella creazione del processo collector", 0);

    if(pidC == 0){
        collector(&sa);
        return 0;
    }else{
        master(argv, &sa);
        SYSCALL_ERROR((waitpid(pidC, NULL, 0)), -1, "Errore nell'attesa del processo collector", 0);
        return 0;
    }
}