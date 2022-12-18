/*===========================================================================
-----------------------------------------------------------------------------
  	motorZ.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
  	MotorZ manage the motion along z-axis.

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

#define dt 0.03 //30 Hz
#define nbytes sizeof(float)
#define r "/tmp/fifoCZ"
#define w "/tmp/fifoZW"
#define zMax 10
#define zMin 0

int fd_read, fd_write;
float z =zMin,  zOld =0;
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
  Update Z position
  INPUT:
    velocity
  RETURN:
    null
=====================================*/
void update_z(int v){
    if((z + v*dt) > zMax) {
            z = zMax;
        }
    else if ((z + v*dt) < zMin) {
        z = zMin;
        }
    else
        z += v*dt;

    //write the new value on the pipe if it is different
    if (z != zOld) {
            if(write(fd_write, &z, nbytes) == -1)
                perror("MotorZ: error in writing");
            
            //update log file
            FILE *flog;
            flog = fopen("logFile.log", "a+");
            if (flog == NULL) {
                perror("MotorZ: cannot open log file");
            }
            else {
                char * curr_time = current_time();
                fprintf(flog, "< MOTOR Z > reached position %f cm at time: %s \n", z, curr_time);
            }
            fclose(flog);
            
            //set Zold to the new value
            zOld = z;
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
    if(signo == SIGINT) {
        printf("MotorZ:received SIGINT, closing pipes and exit\n");
        
        if(close(fd_read) != 0){
            perror("MotorZ:can't close tmp/fifoCZ");
            exit(-1);
        }

        if(close(fd_write) != 0) {
            perror("MotorZ:an't close tmp/fifoZW");
            exit(-1);
        }
        
        exit(0);
    }

    //signal SIGUSR1 (RESET)
    else if(signo==SIGUSR1) {
        printf("MotorZ:received SIGUSR1, reset routine starting\n");
        //set boolean reset to true
        reset=true;
        //set z velocity to 0 (stop)
        update_z(0);
        v=0;
        //set velocity to -1 until z position is at 0
        while(z!=0 && reset){
            //update Z position
            update_z(-1);
            usleep(dt*1000000);
        }
        //set boolean reset to false when z is at 0
        reset=false;
    }
    
    //signal SIGUSR2 (STOP)
    else if(signo ==SIGUSR2){
        printf("MotorZ: received SIGUSR2- STOP routine starting\n");
        //set z velocity to 0 (stop)
        update_z(0);
        v = 0;
        //set boolean reset to false
        reset=false;
        usleep(dt*1000000);
    }

    //manage errors in handling signals
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR2(STOP)\n");
    }
}

/*=====================================
  Manage the motion along z-axis
  RETURN:
    null
=====================================*/
int main() {

    //manage signals
    if(signal(SIGINT, sig_handler) == SIG_ERR)
        printf("MotorZ: can't set the signal hendler for SIGINT\n");
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR2(STOP)\n");
    }

    //open pipe with the command in reading non-blocking mode
    if((fd_read = open(r, O_RDONLY|O_NONBLOCK)) == 0 ) {
        perror("MotorZ: Can't open /tmp/fifoCZ");
        exit(-1);
    }
    
    //open pipe with the world in writing mode
    if((fd_write = open(w, O_WRONLY)) == 0 ) {
        perror("MotorZ: can't open  tmp/fifoZW");
        exit(-1);
    }

    int v_read = 0;
    int read_byteV;

    //infinite loop
    while(1) {
        //read z velocity
        read_byteV = read(fd_read, &v_read, sizeof(int));
        
        if(read_byteV == -1 && errno != EAGAIN) 
            perror("MotorZ: error in reading");
        else if(read_byteV < sizeof(int)) {
            
        }
        else {
            v = v_read;
        }

        //update z position
        update_z(v);
        
        usleep(dt*1000000);
    }

}
