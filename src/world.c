/*===========================================================================
-----------------------------------------------------------------------------
     world.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
     World manages the position of the hoist w.r.t the velocity imposed.

=============================================================================*/

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>

//fifos
#define rX "/tmp/fifoXW"
#define rZ "/tmp/fifoZW"
#define w "/tmp/fifoWI"
#define boundErr 0.05

int fd_readX, fd_readZ;
int fd_write;
float err = 0;

//position structure (x,z)
struct position {
        float x;
        float z;
};

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
  Manage signals received
  INPUT:
  SIGINT
    -close pipes
  RETURN:
    null
=====================================*/
void sig_handler(int signo) {
        if(signo==SIGINT) {
                printf("World: received SIGINT, closing the pipes and exit\n");

                //close pipes
                if(close(fd_readX)!=0){
                        perror("World: Can't close the reading pipe X");
                        exit(-1);
                }
                if(close(fd_readZ)!=0){
                        perror("World: Can't close the reading pipe Z");
                        exit(-1);
                }
                if(close(fd_write)!=0){
                        perror("World: Can't close the write pipe");
                        exit(-1);
                }

                exit(0);
        }
}

/*=====================================
  Manage the position of the hoist
  RETURN:
    null
=====================================*/
int main() {

        if(signal(SIGINT, sig_handler)==SIG_ERR) {
                printf("World:Can't set the signal handler for SIGINT\n");
        }
        if((fd_readX = open(rX, O_RDONLY|O_NONBLOCK)) == 0 ) {
                perror("World: Can't open /tmp/fifoXW");
                exit(-1);
        }
        
        if((fd_readZ = open(rZ, O_RDONLY|O_NONBLOCK)) == 0 ) {
                perror("World: Can't open /tmp/fifoZW");
                exit(-1);
        }

        if((fd_write = open(w, O_WRONLY)) == 0) {
                perror("World: can't open  tmp/fifoWI");
                exit(-1);
        }

        float Xold = 0;
        float Zold = 0;
        float Xr = 0, Zr = 0;
        struct position p = {0, 0};
        int read_byteX, read_byteZ;

        //infinite loop
        while(1) {
                //read X and Z from the pipes 
                read_byteX = read(fd_readX, &Xr, sizeof(float));
                read_byteZ = read(fd_readZ, &Zr, sizeof(float));
                
                if(read_byteX == -1 && errno != EAGAIN) {
                        perror("World: can't read X");
                }
                else if(read_byteX < sizeof(float) || errno != EAGAIN) {
                        //if there is nothing to read, set X equal to the previous value
                        Xr = Xold;
                }
                else {
                        //if the X value is changed
                        if(Xr != Xold) {
                                //get a random error between -boundErr and +boundError
                                err = (float)rand()/(float)(RAND_MAX/(boundErr*2));
                                err = err - boundErr;
                                //set the new X position summing the error
                                p.x = Xr + err;
                        }
                }
                
                
                if(read_byteZ == -1 && errno != EAGAIN) {
                        perror("World: can't read Z");
                }
                else if(read_byteZ < sizeof(float) || errno != EAGAIN) {
                        //if there is nothing to read, set Z equal to the previous value
                        Zr = Zold;
                }
                else {
                        //if the Z value is changed
                        if(Zr != Zold) {
                                //get a random error between -boundErr and +boundError
                                err = (float)rand()/(float)(RAND_MAX/(boundErr*2));
                                err = err - boundErr;
                                //set the new Z position summing the error
                                p.z = Zr + err;
                        }
                }
                
                //if X and Z are change write on the pipe and update their value
                if(Xr != Xold || Zr != Zold) {
                        
                        //write the new value on the pipe
                        if(write(fd_write, &p, sizeof(struct position)) != -1) {
                                //update the value if the write go well
                                Xold = Xr;
                                Zold = Zr;

                                //update log file
                                FILE *flog;
                                flog = fopen("logFile.log", "a+");
                                if (flog == NULL) {
                                        perror("World: cannot open log file");
                                }
                                else {
                                        char * curr_time = current_time();
                                        fprintf(flog, "< WORLD > updated (x,z) position (%f,%f) cm at time: %s \n", Xr, Zr, curr_time);
                                }
                                fclose(flog);
                        }
                        else
                                perror("World: can't write position");
                }


        }
}