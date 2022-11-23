#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include<sys/stat.h>

//function to spawn konsole
int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}


int main() {

  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };

  //create pipes

  //pipe X-world 
  char * fifoXW = "/tmp/fifoXW";
  if (mkfifo(fifoXW, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?");
  //pipe Z-world
  char * fifoZW = "/tmp/fifoZW";
  if (mkfifo(fifoZW, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?");

  //world -inspection
  char * fifoWI = "/tmp/fifoWI";
  if (mkfifo(fifoWI, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?");

  //command-X
  char * fifoCX = "/tmp/fifoCX";
  if (mkfifo(fifoCX, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?");  
  //command-Z
   char * fifoCZ = "/tmp/fifoCZ";
  if (mkfifo(fifoCZ, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?");

  //spawn command window and inspection window 
  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  //generate two motor process

  //generate world process

  //change into watchdog
  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_insp, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

