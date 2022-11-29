#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>



char * fifoXW = "/tmp/fifoXW";
char * fifoCX = "/tmp/fifoCX";

//function to spawn konsole
int spawn(const char * program, char * arg_list[]) {
  printf("forking\n");
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
  printf("Master\n");
  fflush(stdout);
  char * arg_motorX[]={"./motorX", NULL};
  //signal(SIGINT, sig_handler);
  //create pipes
  /*if(unlink(fifoXW)==-1){
    perror("Can't unlik pipe");
  }
  if(unlink(fifoCX)==-1){
    perror("can't umlink pipe");
  }*/
  //pipe X-world 
  if (mkfifo(fifoXW, 0666 ) != 0)
    perror("Cannot create fifo. Already existing?");
  //pipe C-World
  if (mkfifo(fifoCX, 0666 ) != 0)
    perror("Cannot create fifo. Already existing?");  
  printf("pipe created\n");
  //generate two motor process
  pid_t pid_motorX=spawn("./motorX", arg_motorX);
  //printf("%d\n",getpid());
  printf("Master\n");
  fflush(stdout);
  int fd_read, fd_write;
  float value=1;
  //aprire la pipe in letteura(CX) e contrallare non dia errore
  fd_read=open(fifoXW, O_RDWR);
  if(fd_read ==0 ){
        perror("Can't open /tmp/fifoCX");
    }
    //aprire pipe in scritture(XW)
  fd_write=open(fifoCX, O_RDWR);
    if(fd_write == 0){
        perror("can't open  tmp/fifoXW");
    }
    printf("inserire velocità: ");
    scanf("%f", &value);
    if(write(fd_write, &value, sizeof(float))!= sizeof(float)){
      perror("error in writing");
    }
    //sleep(5);
    read(fd_read, &value, sizeof(float));
    printf("read value %f\n", value);
    fflush(stdout);
    kill(pid_motorX, SIGINT);
    close(fd_read);
    close(fd_write);
    unlink(fifoCX);
    unlink(fifoXW);
  int status;
  waitpid(pid_motorX, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}