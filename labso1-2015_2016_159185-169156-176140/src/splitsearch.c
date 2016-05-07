#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

// Stampa usage generico.
void print_usage(char *ex_name);

// Rimuove un eventuale carattere newline dalla stringa e si assicura sia
// '\0' terminata.
void chomp(char *s);

// Ritorna un array contenente le linee del file fp e imposta length al numero
// di righe
char **get_strings_from_file(FILE *fp, int *length);

// Effettua una ricerca ricorsiva di target in array dalla posizione start
// alla posizione end.
// Se un risultato viene trovato la sua posizione in array viene aggiunta
// alla pipe r e viene incrementato il valore nella pipe c.
// Termina se il contenuto della pipe c e' maggiore o uguale a max.
//
// n e' indice della profondita' della ricorsione.
// soutput indica dove stampare una rappresentazione del progresso delle azioni.
void splitsearch(char **array, int start, int end, char *target, int r[2],
                 int c[2], int max, int n, FILE *soutput);

// entry point del programma
int main(int argc, char *argv[])
{
    //
    // Controllo degli argomenti al programma
    //
    // Inizializzazione delle variabili per il controllo degli argomenti
    char *target;               // obbiettivo di ricerca
    char *input_file;           // file di input
    char *output_file;          // file di output
    int max;                    // numero massimo di risultati
    // FLAGs per argomenti obbligatori
    int targetF = 0;
    int input_fileF = 0;
    // FLAGs per argomenti opzionali
    int maxF = 0;
    // Descrittore per l'output a file o a schermo
    FILE *soutput_file = stdout;                    // default a <stdout>
    FILE *soutput;
    if((soutput = fopen("/dev/null", "w")) == NULL) // default a </dev/null>
    {
        perror("/dev/null");
        exit(EXIT_FAILURE);
    }
    // Verifica degli argomenti del programma
    int i;
    for (i = 1; i < argc; i++) {
        // <-i input file>
        if (strcmp(argv[i], "-i") == 0)
        {
            input_fileF++;              // FLAG attiva
            input_file = argv[++i];     // Percorso file di input
        }
        // <-t target>
        else if (strcmp(argv[i], "-t") == 0)
        {
            targetF++;                  // FLAG attiva
            target = argv[++i];         // Obbiettivo di ricerca
        }
        // [-m max_risultati]
        else if (strcmp(argv[i], "-m") == 0)
        {
            maxF++;                     // FLAG attiva
            max = atoi(argv[++i]);      // Valore massimo opzionale
        }
        // [-o output_file]
        else if (strcmp(argv[i], "-o") == 0)
        {
            output_file = argv[++i];    // Percorso file di output opzionale
            // Stream di output sul file non a schermo
            if((soutput_file = fopen(output_file, "w")) == NULL)
            {
                perror(output_file);
                exit(EXIT_FAILURE);
            }
        }
        // [-v]
        else if (strcmp(argv[i], "-v") == 0)
        {
            soutput = stdout;           // Stream di progresso su <stdout>
        }
        // Gestione argomenti errati. Stampa usage.
        else
        {
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }
    // Gestione mancanza argomenti obbligatori. Stampa usage.
    if(!input_fileF || !targetF)
    {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }
    //
    // Inizio programma vero e proprio
    //
    // Inizializzazione variabili
    char **lines;                       // array per le stringhe lette dal file
    FILE *fp;                           // descrittore file di input
    // Inizializzazione descrittori pipe
    int rp[2];                          // pipe per i risultati
    int cp[2];                          // pipe per il conteggio
    // Inizializzazione pipe dei risultati
    if(pipe(rp) == -1)
    {
        perror("Pipe");
        exit(EXIT_FAILURE);
    }
    // Inizializzazione pipe del conteggio
    if(pipe(cp) == -1)
    {
        perror("Pipe");
        exit(EXIT_FAILURE);
    }
    // Apertura file di input
    if((fp = fopen(input_file, "r")) == NULL)
    {
        perror(input_file);
        exit(EXIT_FAILURE);
    }
    // Lettura file di input nell'array lines
    int n_lines;
    lines = get_strings_from_file(fp, &n_lines);
    fclose(fp); // Chiusura file di input
    if(!maxF)
        max = n_lines;                // Valore di default di max
    // Scrittura in pipe del count iniziale
    int count = 0;
    write(cp[WRITE], &count, sizeof(count));
    // Definizione formato della modalita' descrittiva
    fprintf(soutput, "--------------------\n[PID] Ricerca: <inizio - fine>\n--------------------\n");
    int n = 0;                              // primo livello di profondita'
    // Inizio ricerca ricorsiva
    splitsearch(lines, 0, n_lines - 1, target, rp, cp, max, n, soutput);
    // Aggiornamento numero di risultati dopo la ricerca
    read(cp[READ], &count, sizeof(count));
    // Solo per modalita' descrittiva (MD)
    // Stampa fine del processo
    fprintf(soutput, "\n===");
    if(count >= max)                            // se e' stato raggiunto il
        fprintf(soutput, "LIMITE RAGGIUNTO. "); // limite imposto si notifica
    fprintf(soutput, "HO TROVATO %d RISULTATI===\n\n", count);
    // Se non sono stati trovati risultati stampo 0
    if(count < 1)
    {
        fprintf(soutput_file, "0\n");
        exit(EXIT_SUCCESS);
    }
    // Altrimenti leggo e scrivo ogni riga trovata nella pipe dei risultati
    while(count > 0)
    {
        int n_line = 0;
        read(rp[READ], &n_line, sizeof(n_line));
        fprintf(soutput_file, "%d\n", n_line);
        count--;
    }
    exit(EXIT_SUCCESS);
}

void print_usage(char *ex_name)
{
    printf("%-10s%s <-t stringa_di_ricerca> <-i input_file> \n%-10s[-o output_file] [-m max_risultati] [-v]\n\n", "usage:", ex_name, " ");
    printf("Parametri\n\n");
    printf("   %-9s La stringa da cercare\n", "-t");
    printf("   %-9s Il file sul quale si vuole eseguire la ricerca\n", "-i");
    printf("Opzionali\n");
    printf("   %-9s Il file dove salvare l' output\n", "-o");
    printf("   %-9s Il numero massimo di risultati da trovare\n", "-m");
    printf("   %-9s Stampa a schermo una descrizione del processo di ricerca\n", "-v");
}

void chomp(char *s)
{
    // Il puntatore alla stringa si muove alla fine
    while(*s != '\n' && *s != '\0')
        s++;
    // '\0' termine stringa
    *s = '\0';
}

char **get_strings_from_file(FILE *fp, int *length)
{
    // Inizializzazione variabili utili
    static char **array;

    int c_line = 0;                 // Conteggio di riga
    int line_step = 128;            // Incremento grandezza array a step di 128
    int line_expansion = 0;         // Grandezza dell' array

    size_t buffer_length = 512;     // Grandezza iniziale del buffer
    int line_length;                // Lunghezza della linea

    // Inizializzazione buffer
    char *buffer = malloc(buffer_length * sizeof(char));
    if(buffer == NULL)
    {
        fprintf(stderr, "Impossibile allocare nuova memoria.\n");
        exit(EXIT_FAILURE);
    }

    // Lettura file riga per riga
    while((line_length = getline(&buffer, &buffer_length, fp)) > 0)
    {
        // Se serve piu' memoria viene riallocata con step di line_step
        if(c_line >= line_expansion)
        {
            line_expansion += line_step;
            array = realloc(array, line_expansion * sizeof(char*));
            if(array == NULL)
            {
                fprintf(stderr, "Impossibile riallocare la memoria.\n");
                exit(EXIT_FAILURE);
            }
        }
        // Inizializzazione memoria per riga nell' array
        array[c_line] = malloc(line_length    );
        if(array[c_line] == NULL)
        {
            fprintf(stderr, "Impossibile allocare nuova memoria.\n");
            exit(EXIT_FAILURE);
        }

        // La riga viene copiata nell' array
        chomp(buffer);
        strcpy(array[c_line], buffer);
        c_line++;
    }

    // length contiene il numero totale di righe
    *length = c_line;
    return array;
}

void splitsearch(char **array, int start, int end, char *target, int r[2],
                 int c[2], int max, int n, FILE *soutput)
{
    // Lettura conteggio dei risultati
    int c_count = 0;
    read(c[READ], &c_count, sizeof(c_count));

    // Se e' stato raggiunto il limite di risultati
    // non faccio altro
    if(c_count >= max)
    {
        write(c[WRITE], &c_count, sizeof(c_count));
    }
    else
    {
        // Solo per modalita' descrittiva (MD)
        // Indentazione di profondita' del processo
        int i;
        for(i = 0; i < n; i++)
        fprintf(soutput, "-");

        // Se va verificato un solo elemento
        if(start == end)
        {
            fprintf(soutput, "[%5d] Ricerca: <%i>.", getpid(),
                    start + 1);                                     // (MD)

            // Se l'elemento corrisponde all' obbiettivo
            if(strcmp(target, array[start]) == 0)
            {
                // Inserimento del numero di riga nella pipe dei risultati
                int found = start + 1;
                write(r[WRITE], &found, sizeof(found));

                // Aggiornamento del contatore nella pipe del conteggio
                c_count += 1;
                write(c[WRITE], &c_count, sizeof(c_count));
                fprintf(soutput, "%10s.\n", " TROVATO");            // (MD)
            }
            else
            {
                fprintf(soutput, "\n");                             // (MD)
                write(c[WRITE], &c_count, sizeof(c_count));
            }
        }
        else
        {
            // Aumento indice di profondita'
            n++;

            fprintf(soutput, "[%5d] Ricerca: <%i - %i>.\n", getpid(),
                    start + 1, end + 1);                            // (MD)
            write(c[WRITE], &c_count, sizeof(c_count));

            // Calcolo punto medio del gruppo di analisi
            int mid = (start + end) / 2;

            // Fork del processo
            int pid_figlio = fork();

            // Test del fallimento del fork()
            // int pid_figlio;
            // if(n < 2)
                // pid_figlio = fork();
            // else
                // pid_figlio = -1;

            // Se il fork fallisce
            if(pid_figlio < 0)
            {
                // Passo a una ricerca iterativa
                fprintf(soutput, "[%5d]*** Fork fallito, continuo con la ricerca iterativa ***\n",
                        getpid());

                int j;
                for(j = start; j <= end; j++)
                {
                    // Lettura conteggio dei risultati
                    read(c[READ], &c_count, sizeof(c_count));
                    // Se e' stato raggiunto il limite dei risultati
                    // termino il ciclo di ricerca
                    if(c_count >= max)
                    {
                        write(c[WRITE], &c_count, sizeof(c_count));
                        j = end;
                    }
                    else
                    {
                        // Solo per modalita' descrittiva (MD)
                        // Indentazione di profondita' del processo
                        int i;
                        for(i = 0; i < n; i++)
                        fprintf(soutput, "-");

                        fprintf(soutput, "[%5d] Ricerca: <%i>.", getpid(),
                                j + 1);                     // (MD)
                        // Se l'elemento corrisponde all'obbiettivo
                        if(strcmp(target, array[j]) == 0)
                        {
                            // Inserimento del numero di riga nella pipe
                            // dei risultati
                            int found = j + 1;
                            write(r[WRITE], &found, sizeof(found));

                            // Aggiornamento del contatore nella pipe
                            // del conteggio
                            c_count += 1;
                            write(c[WRITE], &c_count, sizeof(c_count));
                            fprintf(soutput, "%10s.\n", " TROVATO");    // (MD)
                        }
                        else
                        {
                            fprintf(soutput, "\n");                     // (MD)
                            write(c[WRITE], &c_count, sizeof(c_count));
                        }
                    }
                }
            }
            // Se il nuovo processo e' figlio
            else if(pid_figlio == 0)
            {
                splitsearch(array, start, mid, target, r, c, max, n, soutput);
                exit(EXIT_SUCCESS);
            }
            // Se il processo e' lo stesso
            else
            {
                splitsearch(array, mid + 1, end, target, r, c, max, n, soutput);
                int status;
                waitpid(pid_figlio, &status, 0);
            }
        }
    }
}
