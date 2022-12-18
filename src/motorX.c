/*===========================================================================
-----------------------------------------------------------------------------
  	motorX.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
  	MotorX manage the motion along x-axis.

=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define dt 0.03 //30Hz
#define r "/tmp/fifoCX"
#define w "/tmp/fifoXW"
#define X_MAX 40
#define X_MIN 0

int fd_read;
int fd_write;
int nbytes = sizeof(float);
float X=X_MIN,xOld=0;
int v = 0;
bool reset = false;

/*=====================================
  Get current time
  RETURN:
    time and date
=====================================*/
char* current_time(){
    time_t rawtime;
    struct tm * timeinfo;
    char* timedate;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    timedate = asctime(timeinfo);
    return timedate;
}

/*=====================================
  Update X position
  INPUT:
    velocity
  RETURN:
    null
=====================================*/
void update_X(int v){
    float dx = (v*dt);    
    if((X + dx) > X_MAX) {
         X=X_MAX;
    }
    else if( (X + dx) < X_MIN) {
        X=X_MIN;
    }
    else {
        X+=dx;
    }

    //write the new value on the pipe if it is different
    if (X != xOld) {
            if(write(fd_write, &X, nbytes) == -1)
                perror("MotorX: error in writing");

            //update log file
            FILE *flog;
            flog = fopen("logFile.log", "a+");
            if (flog == NULL) {
                perror("MotorX: cannot open log file");
            }
            else {
                char * curr_time = current_time();
                fprintf(flog, "< MOTOR X > reached position %f cm at time: %s \n", X, curr_time);
            }
            fclose(flog);

            //set Zold to the new value
            xOld = X;
        }
}

/*=====================================
  Manage signals received
  INPUT:
  SIGINT
    -close pipes
  SIGUSR1
    -reset routine
  SIGUSR2
    -stop routine
  RETURN:
    null
=====================================*/
void sig_handler(int signo) {
    //signal SIGINT
    if(signo==SIGINT){
        printf("MotorX: received SIGINT, closing the pipes and exit\n");
        
        //chiusura pipe
        if(close(fd_read)!=0){
            perror("MotorX: Can't close the reading pipe");
            exit(-1);
        }
        if(close(fd_write)!=0){
            perror("MotorX: Can't close the write pipe");
            exit(-1);
        }
        
        exit(0);
    }
    
    //signal SIGUSR1 (RESET)
    else if(signo==SIGUSR1){
        printf("MotorX: received SIGUSR1- Reset routine starting\n");
        //set boolean reset to true
        reset = true;
        //set z velocity to 0 (stop)
        update_X(0);
        v=0;
        //set velocity to -1 until x position is at 0
        while(X!=0 && reset){
            //update X position
            update_X(-1);
            usleep(dt*1000000);
        }
        //set boolean reset to false when x is at 0
        reset=false;
    }
    
    //signal SIGUSR2 (STOP)
    else if(signo ==SIGUSR2){
        printf("MotorX: received SIGUSR2- STOP routine starting\n");
        //set x velocity to 0 (stop)
        update_X(0);
        v=0;
        //set boolean reset to false
        reset=false;
        usleep(dt*1000000);

    }
    
    //manage errors in handling signals
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR2(STOP)\n");
    }
} 

/*=====================================
  Manage the motion along z-axis
  RETURN:
    null
=====================================*/
int main(){
    //manage signals
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR2(STOP)\n");
    }
    
    //open pipe with the command in reading non-blocking mode
    if((fd_read=open(r, O_RDONLY| O_NONBLOCK)) == 0 ) {
        perror("MotorX:Can't open /tmp/fifoCX");
        exit(-1);
    }
    
    //open pipe with the world in writing mode
    if((fd_write=open(w, O_WRONLY))== 0) {
        perror("MotorX:can't open  tmp/fifoXW");
        exit(-1);
    }

    int v_read = 0;
    int read_byteV;
    
    //infinite loop
    while(1) {
        //read x velocity
        read_byteV = read(fd_read, &v_read, sizeof(int));
        
        if(read_byteV == -1 && errno != EAGAIN) 
            perror("Motorx: error in reading");
        else if(read_byteV < sizeof(int)) {
            
        }
        else {
            v = v_read;
        } 

        //update x position
        update_X(v);
        
        usleep(dt*1000000);
    }

}
