#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MSL 512		// Lunghezza massima righe in lettura.
#define MES 8192  // Numero massimo di righe leggibili dal file.

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
void splitsearch(char **array, int start, int end, char *target, int r[2], int c[2], int max, int n, FILE *soutput);

// entry point del programma
int main(int argc, char *argv[])
{
	//
	// Controllo degli argomenti al programma
	//

	// Inizializzazione delle variabili per il controllo degli argomenti
	char *target;						// obbiettivo di ricerca
	char *input_file;				// file di input
	char *output_file;			// file di output

	int max = MES;					// numero massimo di risultati

	// FLAGs per argomenti obbligatori
	int targetF = 0;
	int input_fileF = 0;

	// Descrittore per l'output a file o a schermo
	FILE *soutput_file = stdout;										// default a <stdout>
	FILE *soutput;

	if((soutput = fopen("/dev/null", "w")) == NULL)	// default a </dev/null>
	{
		perror("/dev/null");
		exit(1);
	}

	// Verifica degli argomenti del programma
	int i;
	for (i = 1; i < argc; i++) {
		// <-i input file>
		if (strcmp(argv[i], "-i") == 0)
		{
			input_fileF = 1;					// Argomento necessario presente
			input_file = argv[++i];   // Assegnamento percorso file di input
		}
		// <-t target>
		else if (strcmp(argv[i], "-t") == 0)
		{
			targetF = 1;							// Argomento necessario presente
			target = argv[++i];				// Assegnamento obbiettivo di ricerca
		}
		// [-m risultati_max]
		else if (strcmp(argv[i], "-m") == 0)
		{
			max = atoi(argv[++i]);		// Assegnamento valore massimo opzionale
		}
		// [-o output_file]
		else if (strcmp(argv[i], "-o") == 0)
		{
			output_file = argv[++i];	// Assegnamento percorso file di output opzionale

			// Stream di output sul file non a schermo
			if((soutput_file = fopen(output_file, "w")) == NULL)
			{
				perror(output_file);
				exit(1);
			}
		}
		// [-v]
		else if (strcmp(argv[i], "-v") == 0)
		{
			soutput = stdout;					// Stream di progresso su <stdout>
		}
		// Gestione argomenti errati. Stampa usage.
		else
		{
			printf("usage: \t%s <-t stringa_di_ricerca> <-i input_file> [-o output_file]\n\t[-m risultati_max] [-v]\n", argv[0]);
			exit(1);
		}
	}

	// Gestione mancanza argomenti obbligatori. Stampa usage.
	if((input_fileF == 0) || (targetF == 0))
	{
		printf("usage: \t%s <-t stringa_di_ricerca> <-i input_file> [-o output_file]\n\t[-m risultati_max] [-v]\n", argv[0]);
		exit(1);
	}

	//
	// Inizio programma vero e proprio
	//

	// Inizializzazione variabili
	char **lines;						// array per le stringhe lette dal file
	FILE *fp;								// descrittore file di input

	// Inizzializzazione descrittori pipe
	int rp[2];							// pipe per i risultati
	int cp[2];							// pipe per il conteggio

	// Inizializzazione pipe dei risultati
	if(pipe(rp) == -1)
	{
		perror("Pipe");
		exit(1);
	}

	// Inizializzazione pipe del conteggio
	if(pipe(cp) == -1)
	{
		perror("Pipe");
		exit(1);
	}

	// Apertura file di input
	if((fp = fopen(input_file, "r")) == NULL)
	{
		perror(input_file);
		exit(1);
	}

	// Lettura file di input nell'array lines
	int n_lines;
	lines = get_strings_from_file(fp, &n_lines);
	fclose(fp);		// Chiusura file di input

	// Scrittura in pipe del count iniziale
	int count = 0;
	write(cp[1], &count, sizeof(count));

	// Definizione formato della modalita' descrittiva
	fprintf(soutput, "--------------------\n[PID] Ricerca: <inizio - fine>\n--------------------\n");
	int n = 0;		// primo livello di profondita'

	// Inizio ricerca ricorsiva
	splitsearch(lines, 0, n_lines - 1, target, rp, cp, max, n, soutput);

	// Aggiornamento numero di risultati dopo la ricerca
	read(cp[0], &count, sizeof(count));

	// Solo per modalita' descrittiva (MD)
	// Stampa fine del processo
	fprintf(soutput, "\n===");

	if(count >= max)													 	// se e' stato raggiunto il limite
		fprintf(soutput, "LIMITE RAGGIUNTO. ");  	// imposto si notifica

	fprintf(soutput, "FINITO===\n\n");

	// Se non sono stati trovati risultati stampo 0
	if(count < 1)
	{
		fprintf(soutput_file, "0\n");
		exit(0);
	}

	// Altrimenti leggo e scrivo ogni riga trovata nella pipe dei risultati
	while(count > 0)
	{
		int n_line = 0;
		read(rp[0], &n_line, sizeof(n_line));

		fprintf(soutput_file, "%d\n", n_line);
		count--;
	}

	exit(0);
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
  static char **array;

  int c_line = 0;
  int line_step = 2;
  int line_expansion = 0;

  size_t buffer_length = 512;
  int line_length;

  char *buffer = malloc(buffer_length * sizeof(char));
  if(buffer == NULL)
  {
    fprintf(stderr, "Impossibile allocare nuova memoria.\n");
    exit(EXIT_FAILURE);
  }

  while((line_length = getline(&buffer, &buffer_length, fp)) > 0)
  {
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
    array[c_line] = malloc(line_length    );
    if(array[c_line] == NULL)
    {
      fprintf(stderr, "Impossibile allocare nuova memoria.\n");
      exit(EXIT_FAILURE);
    }

    chomp(buffer);
    strcpy(array[c_line], buffer);
    c_line++;
  }

  *length = c_line;
  return array;
}

void splitsearch(char **array, int start, int end, char *target, int r[2], int c[2], int max, int n, FILE *soutput)
{
	// Lettura conteggio dei risultati
	int c_count = 0;
	read(c[0], &c_count, sizeof(c_count));

	// Se e' stato raggiunto il limite di risultati
	// non faccio altro
	if(c_count >= max)
	{
		write(c[1], &c_count, sizeof(c_count));
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
			fprintf(soutput, "[%5d] Ricerca: <%i>.", getpid(), start + 1);	// (MD)

			// Se l'elemento corrisponde all' obbiettivo
			if(strcmp(target, array[start]) == 0)
			{
				// Inserimento del numero di riga alla pipe dei risultati
				int found = start + 1;
				write(r[1], &found, sizeof(found));

				// Aggiornamento del contatore nella pipe del conteggio
				c_count += 1;
				write(c[1], &c_count, sizeof(c_count));
				fprintf(soutput, "%10s.\n", " TROVATO");			// (MD)
			}
			else
			{
				fprintf(soutput, "\n");				// (MD)
				write(c[1], &c_count, sizeof(c_count));
			}
		}
		else
		{
			// Aumento indice di profondita'
			n++;

			fprintf(soutput, "[%5d] Ricerca: <%i - %i>.\n", getpid(), start + 1,
			end + 1);									// (MD)
			write(c[1], &c_count, sizeof(c_count));

			// Calcolo punto medio del gruppo di analisi
			int mid = (start + end) / 2;

			// Fork del processo
			int pid_figlio = fork();

			// Se il fork fallisce
			if(pid_figlio < 0)
			{
				perror("Unable to fork");
				exit(1);
			}
			// Se il nuovo processo e' figlio
			else if(pid_figlio == 0)
			{
				splitsearch(array, start, mid, target, r, c, max, n, soutput);
				exit(0);
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
