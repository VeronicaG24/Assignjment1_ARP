#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

char * fifoXW = "/tmp/fifoXW";
char * fifoZW = "/tmp/fifoZW";
char * fifoWI = "/tmp/fifoWI";
char * fifoCX = "/tmp/fifoCX";
char * fifoCZ = "/tmp/fifoCZ";


void unlinkpipe(){
if(unlink(fifoXW) != 0) {
        perror("can't unlink tmp/fifoXW");
            //exit(-1);
        }
        
        if(unlink(fifoZW) != 0) {
            perror("can't unlink tmp/fifoZW");
            //exit(-1);
        }
        
        if(unlink(fifoWI) != 0) {
            perror("can't unlink tmp/fifoWI");
            //exit(-1);
        }
        
        if(unlink(fifoCX) != 0) {
            perror("can't unlink tmp/fifoCX");
            //exit(-1);
        }
        
        if(unlink(fifoCZ) != 0) {
            perror("can't unlink tmp/fifoCZ");
            //exit(-1);
        }
}
//function to manage SIGINT
void sig_handler(int signo) {
    if(signo == SIGINT) {
        printf("Master: received SIGINT, unlink pipes and exit\n");
        
        //unlink le pipe
        unlinkpipe();  
      
        exit(0);
    }
    
    signal(SIGINT, sig_handler);
}


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
    printf("%s", program);
    perror("Exec failed:");
    return 1;
  }

}


int main() {

  signal(SIGINT, sig_handler);
  //unlinkpipe();
  //create pipes
  //pipe X-world 
  if (mkfifo(fifoXW, 0666) != 0)
    perror("Cannot create fifo. Already existing?");
  
  //pipe Z-world
  if (mkfifo(fifoZW, 0666) != 0)
    perror("Cannot create fifo. Already existing?");
  
  //world -inspection
  if (mkfifo(fifoWI, 0666) != 0)
    perror("Cannot create fifo. Already existing?");
  
  //command-X
  if (mkfifo(fifoCX, 0666) != 0)
    perror("Cannot create fifo. Already existing?");  
  
  //command-Z
  if (mkfifo(fifoCZ, 0666) != 0)
    perror("Cannot create fifo. Already existing?");

  //log file
  if(remove("./logFile.log")!=0){
    perror("Log file not deleted:");
  }
  
  fclose(fopen("./logFile.log", "w"));
  
  
  //generate two motor process
  char * arg_motorX[]={"./bin/motorX", NULL};
  char * arg_motorZ[]={"./bin/motorZ", NULL};
  pid_t pid_motorX=spawn("./bin/motorX", arg_motorX);
  pid_t pid_motorZ=spawn("./bin/motorZ", arg_motorZ);

  //generate world process
  char * arg_world[]={"./bin/world", NULL};
  pid_t pid_world=spawn("./bin/world", arg_world);

  //spawn command window and inspection window 
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  char * pid_mX_c = malloc(6);
  char * pid_mZ_c = malloc(6);
  sprintf(pid_mX_c, "%d", pid_motorX);
  sprintf(pid_mZ_c, "%d", pid_motorZ);
  //error in compiling when passing the pid 
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", pid_mX_c, pid_mZ_c, NULL};
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);

  //spawn watchdog
  char * arg_list_inspection[] = { "./bin/watchdog", pid_mX_c, pid_mZ_c, NULL};
  pid_t pid_insp = spawn("./bin/watchdog", arg_list_inspection);
  //wait 
  wait(NULL);
  //kill motor X
  if(kill(pid_motorX,SIGINT) == -1) { //controlla sia -1
      perror("MotorX: failed to kill motorX");
    }
  //kill motor z
    sleep(1); //simone ha consigliato di fare le sleep se no non killa bene
    if(kill(pid_motorZ,SIGINT) == -1) { 
      perror("MotorZ: failed to kill motorZ");
    }
  
    sleep(1);
    if(kill(pid_world,SIGINT) == -1) {
      perror("World: failed to kill world");
    }
    sleep(1);
    if(kill(pid_cmd,SIGINT) == -1) { //controlla sia -1
      perror("Command: failed to kill motorX");
    }
    sleep(1);
    if(kill(pid_insp,SIGINT) == -1) { //controlla sia -1
      perror("nspection: failed to kill motorX");
    }
    //kill watchdog
    sleep(1);
    unlinkpipe();
    printf ("Main program exiting with status %d\n", -1);
    exit(0);
}

