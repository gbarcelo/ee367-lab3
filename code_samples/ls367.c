/*
 * This is an example program that will launch the 'ls' syhstem call
 * It shows how to call a program from within
 * a C program using "execl", which is a variation of exec.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void error(char *s);
char *data = "Some input data\n";

void main()
{
  int pid;
  int status;

  if ((pid=fork()) == 0) {

    /* This is the child process*/
    execl("/usr/bin/ls", "ls", (char *)NULL);

    /* a successful execl replaces this child process
     * such that nothing here will run
     */
    error("Could not exec 'ls'");
  }

  /*  The parent process */ 
  wait(&status);
  printf("Spawned 'ls' is a child process at pid %d\n", pid);

  /* This is the parent process */

  exit(0);
}

void error(char *s)
{
  perror(s);
  exit(1);
}
