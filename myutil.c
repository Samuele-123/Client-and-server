#include "myutil.h"

/*
Funzione checkInt: Prende in ingresso una stringa e controlla se questa rappresenta un numero positivo,
il controllo viene fatto controllando se ogni carattere della stringa è riconosciuto come cifra
*/

int checkInt(char* number){
    int i = 0;
    while(number[i] != '\0'){
        if(!isdigit(number[i])) return 0;
        i++;
    }
    return 1;
}


/*
Funzione check_arg: Prende in ingresso il numero e gli argomenti ricevuti da linea di comando e procede a controllare che:
    1) Il numero di argomenti passato sia corretto
    2) Il nome della directory sia valido, in particolare che si possa accedere alle info e che sia effettivamente una directory
    3) Il valore passato come secondo parametro sia un intero positivo
*/

void check_arg(int argc, char** argv){
    if(argc < 3){
        printf("Errore nel passaggio dei parametri, riprovare con: %s directory_name worker_number\n", argv[0]);
        exit(1);
    }

    char* dir = argv[1];
    struct stat statbuf;

    if (stat(dir, &statbuf) == -1) {
        printf("Impossibile accedere alla directory %s\n", dir);
        exit(1);
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        printf("%s non è una directory\n", dir);
        exit(1);
    }

    if(checkInt(argv[2]) != 1){
        printf("Il secondo argomento non e' un intero positivo\n");
        exit(1);
    }
}

/*
Funzione isDirectory: Prende in ingresso una stringa controlla se è un file regolare accednedo alle
inforomazini e se è una directory
Ritorna 0 in caso di fallimento 1 in caso di successo
*/
int isDirectory(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) return 0;
    return S_ISDIR(path_stat.st_mode);
}

/*
Funzione leggi: Prende in input il nome della directory da leggere e la struct arg_t, apre la directory e
inizia a leggerla, il percorso assoluto viene allocato dinamicamente, controllo se questo percorso identifica
una directory, se sì ignoro le directory: ., .. e .vscode e chizmo ricorsivamente la funzione leggi sul path
della nuova directory, se invece il path non è una directory controllo se è un file che termina per .dat in questo
caso carico il path sulla coda condivisa
*/

void leggi(char* dirName, arg_t par){
    DIR *dir;
    struct dirent * entry;

    SYSCALL_ERROR((dir=opendir(dirName)), NULL, "Errore nell'apertura della directory\n", 1);

    errno = 0;
    while((entry = readdir(dir)) != NULL){

        char* path = (char*)malloc(sizeof(char)*(strlen(dirName)+strlen(entry->d_name)+2));
        MEMORY_ERROR(path);
        sprintf(path, "%s/%s", dirName, entry->d_name);

        if (isDirectory(path)) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".vscode") != 0){
                leggi(path, par);
            }
            free(path);
        } else if (strstr(entry->d_name, ".dat") != NULL) {
            SYSCALL_ERROR(push(par.q, path), -1, "Errore nella push del dato sulla coda", 1);
        }

        errno = 0;
    }

    if(errno != 0){
        perror("Errore nella lettura della directory\n");
        exit(EXIT_FAILURE);
    }

    SYSCALL_ERROR(closedir(dir), -1, "Errore nella chiusura della directory\n", 1);
}

/*
Funzione calcola: Prende in ingresso il percorso assoluto di un file, prova ad aprirlo in sola lettura e a leggere le linee contenute al
suo interno, le linee vengono trasformare in float con la strtof, viene allocato dinamicamente un array di float per allocarci valori che serviranno
nel calcolo della deviazione standard, viene poi calcolata la media e la deviazione standard.
La funzione calcola poi il numero di byte necessari per allocare la stringa che dovrà essere stampata in output tramite la snprintf,
alloca la memoria e crea la stringa, si libera la memoria allocata e si restituisce la stringa.
*/
char* calcola(const char *file_path) {

    FILE *fp;
    float readValue;
    int cont = 0;
    float somma = 0;
    float media = 0;
    float devStd = 0;
    float* arrayValue = NULL;
    char *linea = NULL;
    size_t length = 0;
    ssize_t read;
    int numChars;

    SYSCALL_ERROR((fp = fopen(file_path, "r")), NULL, "Errore nell'apertura del file\n", 1);

    errno = 0;
    while ((read = getline(&linea, &length, fp)) != -1){ 
        char *endptr;
        readValue = strtof(linea, &endptr);
        if (endptr == linea) {
            printf("Errore nella lettura dal file\n");
            exit(EXIT_FAILURE);
        }
        
        cont++;
        arrayValue = (float*)realloc(arrayValue, cont*sizeof(float));
        MEMORY_ERROR(arrayValue);
        arrayValue[cont-1] = readValue;
        somma+=readValue;
        errno = 0;
    }

    if(errno != 0){
        printf("Errore nella lettura del file\n");
        exit(EXIT_FAILURE);
    }

    SYSCALL_ERROR(fclose(fp), EOF, "Errore nella chiusura del file\n", 1);

    if(cont == 0){
        media = 0;
        devStd = 0;
    }else{
        media = somma/cont;
        somma = 0;

        for(int i = 0; i < cont; i++){
            somma+=pow((arrayValue[i]-media), 2);
        }

        devStd=sqrt(somma/cont);
    }


    numChars = snprintf(NULL, 0, "%d\t%.2f\t%.2f\t%s", cont, media, devStd, file_path);

    char* print = (char*)malloc(sizeof(char)*(numChars+1));
    MEMORY_ERROR(print);

    snprintf(print, numChars+1, "%d\t%.2f\t%.2f\t%s", cont, media, devStd, file_path);

    free(arrayValue);
    free(linea);

    return print;
}