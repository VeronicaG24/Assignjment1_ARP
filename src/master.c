/*===========================================================================
-----------------------------------------------------------------------------
  	master.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
  	The master program manages the pipes, spawns the other processes, 
    create the Log File and waits until one of the process closes to kill 
    all the other process and unlink the pipes.

=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

//file descriptors
char * fifoXW = "/tmp/fifoXW";
char * fifoZW = "/tmp/fifoZW";
char * fifoWI = "/tmp/fifoWI";
char * fifoCX = "/tmp/fifoCX";
char * fifoCZ = "/tmp/fifoCZ";

//
int ret=0;

/*=====================================
  Unlink all the pipes
  RETURN:
    null
=====================================*/
void unlinkpipe() {
  if(unlink(fifoXW) != 0) {
    perror("Master: can't unlink tmp/fifoXW");
    ret=-1;
  }

  if(unlink(fifoZW) != 0) {
      perror("Master: can't unlink tmp/fifoZW");
      ret=-1;
  }

  if(unlink(fifoWI) != 0) {
      perror("Master: can't unlink tmp/fifoWI");
      ret=-1;
  }

  if(unlink(fifoCX) != 0) {
      perror("Master: can't unlink tmp/fifoCX");
      ret=-1;
  }

  if(unlink(fifoCZ) != 0) {
      perror("Master: can't unlink tmp/fifoCZ");
      ret=-1;
  }
}


/*=====================================
  Manage signals received
  INPUT:
  SIGINT
    -close pipes
  RETURN:
    null
=====================================*/
void sig_handler(int signo) {
  if(signo == SIGINT) {
    printf("Master: received SIGINT, unlink pipes and exit\n");

    unlinkpipe();  

    exit(ret);
  }
  
  if(signal(SIGINT, sig_handler)==SIG_ERR) {
      perror("Master:Can't set the signal handler for SIGINT\n");
      exit(-1);
  }
}


/*=====================================
  Spawn processes
  RETURN:
    1 if errors while forking happed
    1 if exec failed
    else child process pid
=====================================*/
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
    printf("%s", program);
    perror("Exec failed:");
    return 1;
  }

}

/*=====================================
  Manage processes, pipes and log file
  RETURN:
    null
=====================================*/
int main() {

  if(signal(SIGINT, sig_handler)==SIG_ERR) {
      perror("Command:Can't set the signal handler for SIGINT\n");
      exit(-1);
  }

  //pipe MotorX-world 
  if (mkfifo(fifoXW, 0666) != 0)
    perror("Cannot create fifoXW. Already existing?");
  
  //pipe MotorZ-world
  if (mkfifo(fifoZW, 0666) != 0)
    perror("Cannot create fifoZW. Already existing?");
  
  //pipe world-inspection console
  if (mkfifo(fifoWI, 0666) != 0)
    perror("Cannot create fifoWI. Already existing?");
  
  //pipe command-MotorX
  if (mkfifo(fifoCX, 0666) != 0)
    perror("Cannot create fifoCX. Already existing?");  
  
  //pipe command-MotorZ
  if (mkfifo(fifoCZ, 0666) != 0)
    perror("Cannot create fifoCZ. Already existing?");

  //reset log file if exists
  if(remove("./logFile.log")!=0){
    perror("Log file not deleted:");
  }
  //create new logfile
  fclose(fopen("./logFile.log", "w"));
  
  //generate two motors processes
  char * arg_motorX[]={"./bin/motorX", NULL};
  char * arg_motorZ[]={"./bin/motorZ", NULL};
  pid_t pid_motorX=spawn("./bin/motorX", arg_motorX);
  pid_t pid_motorZ=spawn("./bin/motorZ", arg_motorZ);
  //convert pid motors into char* to pass as arguments to the inspection console
  char * pid_mX_c = malloc(6);
  char * pid_mZ_c = malloc(6);
  sprintf(pid_mX_c, "%d", pid_motorX);
  sprintf(pid_mZ_c, "%d", pid_motorZ);

  //generate world process
  char * arg_world[]={"./bin/world", NULL};
  pid_t pid_world=spawn("./bin/world", arg_world);

  //spawn command window and inspection window 
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", pid_mX_c, pid_mZ_c, NULL};
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  //convert pid of the processes to pass as arguments to the watchdog
  char * pid_cmd_c = malloc(6);
  char * pid_insp_c = malloc(6);
  char * pid_world_c = malloc(6);
  char * pid_master_c = malloc(6);
  sprintf(pid_insp_c, "%d", pid_cmd);
  sprintf(pid_cmd_c, "%d", pid_insp);
  sprintf(pid_world_c, "%d", pid_world);
  pid_t pid_master=getpid();
  sprintf(pid_master_c, "%d", pid_master);
  
  //spawn watchdog
  char * arg_list_watchdog[] = { "./bin/watchdog", pid_mX_c, pid_mZ_c, pid_cmd_c, pid_insp_c, pid_world_c, pid_master_c, NULL};
  pid_t pid_watchdog = spawn("./bin/watchdog", arg_list_watchdog);
  
  //wait until one process ends
  int status;
  wait(&status);
  
  //kill motor X
  sleep(1);
  if(kill(pid_motorX,SIGINT) == -1) {
    perror("Master: failed to kill motorX");
  }
  else{
    waitpid(pid_motorX, &status,0);
    printf("MotorX exit with status %d", WEXITSTATUS(status));
  }

  //kill motor z
  if(kill(pid_motorZ,SIGINT) == -1) { 
    perror("Master: failed to kill motorZ");
  }
  else{
    waitpid(pid_motorZ, &status,0);
    printf("MotorZ exit with status %d", WEXITSTATUS(status));
  }
  
  //kill world
  if(kill(pid_world,SIGINT) == -1) {
    perror("Master: failed to kill world");
  }
  else{
    waitpid(pid_world, &status,0);
    printf("World exit with status %d", WEXITSTATUS(status));
  }

  //kill command console
  if(kill(pid_cmd,SIGINT) == -1) {
    perror("Master: failed to kill command");
  }
  else{
    waitpid(pid_cmd, &status,0);
    printf("Command exit with status %d", WEXITSTATUS(status));
  }

  //kill inspection console
  if(kill(pid_insp,SIGINT) == -1) {
    perror("Master: failed to kill inspection");
  }
  else{
    waitpid(pid_insp, &status,0);
    printf("Inspection exit with status %d", WEXITSTATUS(status));
  }

  //kill watchdog
  if(kill(pid_watchdog,SIGINT) == -1) {
    perror("Master: failed to kill watchdog");
  }
  else{
    waitpid(pid_watchdog, &status,0);
    printf("Watchdog exit with status %d", WEXITSTATUS(status));
  }

  //unlink pipes
  unlinkpipe();

  //exit program
  printf ("Main program exiting with status %d\n", 0);
  exit(0);
}

