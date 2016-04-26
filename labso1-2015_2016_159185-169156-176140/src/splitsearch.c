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
void splitsearch(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end, char *target, int *count);

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

  printf("TOTAL LINES: %02d.\n", n_lines);
  printf("Searching for '%s' in: \n", target);
  print_array(lines, 0, n_lines);

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
  int count = 0;
  int pid = getpid();
  splitsearch(lines, 0, n_lines, target, &count);

  if(count < 1 && (pid == getpid()))
    printf("NO MATCH found.\n");

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

// SplitSearch forking function
void splitsearch(char array[MAX_ENTRY_SIZE][MAX_STRING_LENGTH], int start, int end, char *target, int *count)
{
  if(start == end)
  {
    if(strcmp(target, array[start]) == 0)
    {
      (*count)++;

      printf("---Count at(%X): %d.\n", &count, *count);
      printf("FOUND at line %02d.\n", start + 1);
    }
  }
  else
  {
    int mid = (start + end) / 2;

    if(fork() == 0)
    {
      splitsearch(array, start, mid, target, count);
    }
    else
    {
      splitsearch(array, mid + 1, end, target, count);
    }
  }
}
