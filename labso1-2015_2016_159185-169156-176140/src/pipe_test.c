#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int fd[2];
  int nbytes;
  int pid_figlio;

  int string = 10;
  int buffer;

  if(pipe(fd) == -1)
  {
    perror("Pipe");
    exit(1);
  }

  pid_figlio = fork();

  if(pid_figlio < 0)
  {
    perror("Unable to fork");
    exit(1);
  }
  else if(pid_figlio == 0)
  {
    close(fd[0]);
    write(fd[1], &string, 1);
    exit(0);
  }
  else
  {
    close(fd[1]);
    nbytes = read(fd[0], &buffer, 1);
    printf("Leggo: %d\n", buffer);
  }

  exit(0);
}
