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
char * fifoCZ = "/tmp/fifoCZ";
char * fifoZW = "/tmp/fifoZW";

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
  char * arg_motorZ[]={"./motorZ", NULL};
  //signal(SIGINT, sig_handler);
  //create pipes
  if(unlink(fifoXW)==-1){
    perror("Can't unlik pipe");
  }
  if(unlink(fifoCX)==-1){
    perror("can't umlink pipe");
  }
  if(unlink(fifoZW)==-1){
    perror("Can't unlik pipe");
  }
  if(unlink(fifoCZ)==-1){
    perror("can't umlink pipe");
  }
  //pipe X-world 
  if (mkfifo(fifoXW, 0666 ) != 0)
    perror("Cannot create fifo. Already existing?");
  //pipe C-World
  if (mkfifo(fifoCX, 0666 ) != 0)
    perror("Cannot create fifo. Already existing?");
  //pipe Z-world
  if (mkfifo(fifoZW, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?");
  //command-Z
  if (mkfifo(fifoCZ, S_IRUSR | S_IWUSR) != 0)
    perror("Cannot create fifo. Already existing?"); 
  printf("pipe created\n");
  //generate two motor process
  pid_t pid_motorX=spawn("./motorX", arg_motorX);
  pid_t pid_motorZ=spawn("./motorZ", arg_motorX);
  //printf("%d\n",getpid());
  printf("Master\n");
  fflush(stdout);
  int fd_readX, fd_writeX, fd_readZ, fd_writeZ;
  float value=1;
  //aprire la pipe in letteura(CX) e contrallare non dia errore
  fd_readX=open(fifoXW, O_RDWR);
  if(fd_readX ==0 ){
        perror("Can't open /tmp/fifoCX");
    }
    //aprire pipe in scritture(XW)
  fd_writeX=open(fifoCX, O_RDWR);
    if(fd_writeX == 0){
        perror("can't open  tmp/fifoXW");
    }
  fd_readZ=open(fifoZW, O_RDWR);
  if(fd_readX ==0 ){
        perror("Can't open /tmp/fifoCX");
    }
    //aprire pipe in scritture(XW)
  fd_writeZ=open(fifoCZ, O_RDWR);
    if(fd_writeX == 0){
        perror("can't open  tmp/fifoXW");
    }
    printf("inserire velocità X: ");
    scanf("%f", &value);
    if(write(fd_writeX, &value, sizeof(float))!= sizeof(float)){
      perror("error in writing");
    }
    printf("inserire velocità Z: ");
    scanf("%f", &value);
    if(write(fd_writeZ, &value, sizeof(float))!= sizeof(float)){
      perror("error in writing");
    }
    //sleep(5);
    read(fd_readX, &value, sizeof(float));
    printf("MotorX:read value %f\n", value);
    fflush(stdout);
    read(fd_readZ, &value, sizeof(float));
    printf("MotorZ:read value %f\n", value);
    fflush(stdout);
    kill(pid_motorX, SIGINT);
    kill(pid_motorZ, SIGINT);
    close(fd_readX);
    close(fd_writeX);
    close(fd_readZ);
    close(fd_writeZ);
    unlink(fifoCX);
    unlink(fifoXW);
    unlink(fifoCZ);
    unlink(fifoZW);
  int status;
  waitpid(pid_motorX, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}
