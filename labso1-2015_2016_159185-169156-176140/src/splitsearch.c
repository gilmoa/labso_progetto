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
void splitsearch(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end, char *target, int f[2], int c[2]);
void pipe_add(int x, int c[2]);

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

  write(cp[1], &count, 1);

  splitsearch(lines, 0, n_lines, target, fd, cp);

  read(cp[0], &count, 1);

  print_var("COUNT", &count);

  if(count < 1 && pid == getpid())
  {
    printf("NO MATCH found.\n");
    exit(0);
  }

  while(count > 0)
  {
    int r;
    read(fd[0], &r, 1);

    printf("%d\n", r);
    count--;
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

void pipe_add(int x, int c[2])
{
  int tmp;
  read(c[0], &tmp, 1);
  tmp += x;
  write(c[1], &tmp, 1);
  print_var("TMP", &tmp);
}

// SplitSearch forking function
void splitsearch(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end, char *target, int f[2], int c[2])
{
  int tmp;
  read(c[0], &tmp, 1);

  // print_var("TMP2", &tmp);
  if(tmp >= 2)
  {
    write(c[1], &tmp, 1);
    return;
  }

  write(c[1], &tmp, 1);

  if(start == end)
  {
    if(strcmp(target, array[start]) == 0)
    {
      int found = start + 1;
      write(f[1], &found, 1);

      int tmp;
      read(c[0], &tmp, 1);
      tmp += 1;
      write(c[1], &tmp, 1);

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
      splitsearch(array, start, mid, target, f, c);
      exit(0);
    }
    else
    {
      splitsearch(array, mid + 1, end, target, f, c);
      int status;
      waitpid(pid_figlio, &status, 0);
    }
  }
}
