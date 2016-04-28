#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void dostuffs(int f[2]);

void dostuffs2(int f[2]);

int main()
{
  int fd[2];

  pipe(fd);

  dostuffs(fd);

  char r[50];

  close(fd[1]);
  read(fd[0], r, sizeof(r));

  printf("%s\n", r);

  exit(0);
}

void dostuffs(int f[2])
{
  int pid = fork();
  if(pid == 0)
  {
    // close(f[0]);
    char parola1[] = "SONO FIGLIO1";
    write(f[1], parola1, sizeof(parola1) - 1);

    dostuffs2(f);
    exit(0);
  }
  else
  {
    char parola1[] = "SONO PADRE1";
    write(f[1], parola1, sizeof(parola1) - 1);

    int status;
    waitpid(pid, &status, 0);
  }
}

void dostuffs2(int f[2])
{
  int pid = fork();
  if(pid == 0)
  {
    char parola1[] = "SONO FIGLIO2";
    write(f[1], parola1, sizeof(parola1)-1);
    exit(0);
  }
  else
  {
    char parola1[] = "SONO PADRE2";
    write(f[1], parola1, sizeof(parola1) - 1);
    exit(0);
  }
}
