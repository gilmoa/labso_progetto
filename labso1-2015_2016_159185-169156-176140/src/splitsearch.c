#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_STRING_LENGTH 512
#define MAX_ENTRY_SIZE 2048

void print_array(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end);
void chomp(char *s);
int get_strings_in_file(FILE *fp, char entries[MAX_ENTRY_SIZE][MAX_STRING_LENGTH]);
void splitsearch(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end, char *target, int f[2]);

void print_var(char testo[24], int *n);

// Usage: splitsearch <string> <dictionary_file>

int main(int argc, char *argv[])
{
  // todo: check arguments

  char *target = argv[1];
  char *input_file = argv[2];

  // parse file for values
  FILE *fp;
  if((fp = fopen(input_file, "r")) == NULL)
  {
    perror(input_file);
    exit(1);
  }

  int n_lines;
  char lines[MAX_ENTRY_SIZE][MAX_STRING_LENGTH];

  n_lines = get_strings_in_file(fp, lines);

  fclose(fp);

  // printf("TOTAL LINES: %02d.\n", n_lines);
  // printf("Searching for '%s' in: \n", target);
  // print_array(lines, 0, n_lines);

  // for-loop search test
  // int i;
  // for(i = 0; i < n_lines; i++)
  // {
  //   if(strcmp(target, lines[i]) == 0)
  //   {
  //     printf("===FOUND! (%02d)===\n", i + 1);
  //   }
  // }
  //end test

  // splitsearch

  int pid = getpid();
  int fd[2];

  if(pipe(fd) == -1)
  {
      perror("Pipe");
      exit(1);
  }

  splitsearch(lines, 0, n_lines, target, fd);
  int term = 0;
  write(fd[1], &term, 1);

  int results[256];

  while(1)
  {
    int r;
    read(fd[0], &r, 1);
    if(r == 0)
      break;

    printf("%d\n", r);
  }

  if(pid == getpid())
  {
    printf("NO MATCH found.\n");
  }

  exit(0);
}

// Print array size and indexed values
void print_array(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end)
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
int get_strings_in_file(FILE *fp, char entries[MAX_ENTRY_SIZE][MAX_STRING_LENGTH])
{
  char line[MAX_STRING_LENGTH];
  int lines = 0;

  while(fgets(line, sizeof(line), fp) && lines < MAX_ENTRY_SIZE)
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
void splitsearch(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end, char *target, int f[2])
{
  if(start == end)
  {
    if(strcmp(target, array[start]) == 0)
    {
      int found = start + 1;
      write(f[1], &found, 1);

      // printf("FOUND at line %02d.\n", found);
    }
  }
  else
  {
    int mid = (start + end) / 2;

    int pid_figlio = fork();

    if(pid_figlio < 0)
    {
        perror("Unable to fork");
        exit(1);
    }
    else if(pid_figlio == 0)
    {
      splitsearch(array, start, mid, target, f);
      exit(0);
    }
    else
    {
      splitsearch(array, mid + 1, end, target, f);
      int status;
      waitpid(pid_figlio, &status, 0);
    }
  }
}
