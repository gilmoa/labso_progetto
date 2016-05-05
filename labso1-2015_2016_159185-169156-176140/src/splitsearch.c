#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MSL 512
#define MES 2048

void print_array(char array[MES][MSL], int start, int end);
void chomp(char *s);
int get_strings_in_file(FILE *fp, char entries[MES][MSL]);
void splitsearch(char array[MES][MSL], int start, int end, char *target, int f[2], int c[2], int max, int n);
void pipe_add(int x, int c[2]);

void print_var(char testo[24], int *n);

// Usage: splitsearch <string> <dictionary_file>

int main(int argc, char *argv[])
{
  // todo: check arguments

	char *target;
	int targetF = 0;

  char *input_file;
	int input_fileF = 0;

	char *output_file;
	int output_fileF = 0;

	int max = MES;
	int i;

	for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-i") == 0)
					{
						i++;
						input_fileF = 1;
            input_file = argv[i];
					}
      else if (strcmp(argv[i], "-t") == 0)
          {
						i++;
						targetF = 1;
          	target = argv[i];
					}
			else if (strcmp(argv[i], "-m") == 0)
          {
						i++;
          	max = atoi(argv[i]);
					}
			else if (strcmp(argv[i], "-o") == 0)
          {
						i++;
						output_fileF = 1;
          	output_file = argv[i];
					}
      else
					{
						printf("usage: %s <-t stringa_di_ricerca> <-i input> [-o output]\n\t[-m risultati_max]\n", argv[0]);	
						exit(1);
					}
    }

		if((input_fileF == 0) || (targetF == 0))
			{
				printf("usage: %s <-t stringa_di_ricerca> <-i input> [-o output]\n\t[-m risultati_max]\n", argv[0]);	
				exit(1);
			}

  

  // parse file for values
  FILE *fp;
  if((fp = fopen(input_file, "r")) == NULL)
  {
    perror(input_file);
    exit(1);
  }

  int n_lines;
  char lines[MES][MSL];



  n_lines = get_strings_in_file(fp, lines);

  fclose(fp);

  // splitsearch

  int pid = getpid();

  int fd[2];
  int cp[2];

  if(pipe(fd) == -1)
  {
      perror("Pipe");
      exit(1);
  }

  if(pipe(cp) == -1)
  {
      perror("Pipe");
      exit(1);
  }

  int count = 0;

  write(cp[1], &count, sizeof(count));
	
	int n = 0;
  
	splitsearch(lines, 0, n_lines, target, fd, cp, max, n);

  read(cp[0], &count, sizeof(count));

  print_var("COUNT", &count);

  if(count == 0 && pid == getpid())
  {
    printf("0\n");
    exit(0);
  }

  while(count > 0)
  {
    int r=0;
    read(fd[0], &r, sizeof(r));

    printf("%d\n", r);
    count--;
  }

  exit(0);
}

// Print array size and indexed values
void print_array(char array[MES][MSL], int start, int end)
{
  int i;
  printf("SIZE = %02d.\n------\n", (end - start));
  for(i = start; i < end; i++)
  {
    printf("(%02d) - [%s]\n", i + 1, array[i]);
  }
  printf("------\n");
}

// Switch trailing newline char to endofstring
void chomp(char *s)
{
  while(*s != '\n')
    s++;

  *s = '\0';
}

// Copy valid lines to array and return number of lines
int get_strings_in_file(FILE *fp, char entries[MES][MSL])
{
  char line[MSL];
  int lines = 0;

  while(fgets(line, sizeof(line), fp) && lines < MES)
  {
    chomp(line);
    if(strlen(line) > 0)
    {
      strcpy(entries[lines], line);
      lines++;
    }
  }

  return lines;
}

// Debug function
void print_var(char testo[24], int *n)
{
        printf("[d]%24s: %p - %i\n", testo, n, *n);
}

// SplitSearch forking function
void splitsearch(char array[MES][MSL], int start, int end, char *target, int f[2], int c[2], int max, int n)
{
	printf("[%02d] Ricerca: <%i - %i>.\n", n + 1, start, end);
  int tmp = 0;
  read(c[0], &tmp, sizeof(tmp));

  if(tmp >= max)
  {
	   write(c[1], &tmp, sizeof(tmp));
  }
  else
  {
    if(start == end)
    {
      if(strcmp(target, array[start]) == 0)
      {
        int found = start + 1;
        write(f[1], &found, sizeof(found));

		    tmp += 1;
		    write(c[1], &tmp, sizeof(tmp));
				printf("[%02d] Ricerca: <%s> - riga %i.\n", n + 1, target, end);
		  }
			else
			{
			     write(c[1], &tmp, sizeof(tmp));
			}
		}
		else
		{
			write(c[1], &tmp, sizeof(tmp));

		  int mid = (start + end) / 2;

			n++;

		  int pid_figlio = fork();

		  if(pid_figlio < 0)
		  {
		      perror("Unable to fork");
		      exit(1);
		  }
		  else if(pid_figlio == 0)
		  {
		    splitsearch(array, start, mid, target, f, c, max, n);
		    exit(0);
		  }
		  else
		  {
		    splitsearch(array, mid + 1, end, target, f, c, max, n);
		    int status;
		    waitpid(pid_figlio, &status, 0);
		  }
		}
	}
}
