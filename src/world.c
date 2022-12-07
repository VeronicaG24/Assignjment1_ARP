#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define rX "/tmp/fifoXW"
#define rZ "/tmp/fifoZW"
#define w "/tmp/fifoWI"
#define boundErr 0.05

int fd_readX, fd_readZ;
int fd_write;
float err = 0;

struct position {
        float x;
        float z;
};
void sig_handler(int signo){
        if(signo==SIGINT){
        printf("MotorX: received SIGINT, closing the pipes and exit\n");
        
        //chiusura pipe
        if(close(fd_readX)!=0){
            perror("World: Can't close the reading pipe X");
            exit(-1);
        }
        if(close(fd_readZ)!=0){
            perror("World: Can't close the reading pipe Z");
            exit(-1);
        }
        if(close(fd_write)!=0){
            perror("WOrld: Can't close the write pipe");
            exit(-1);
        }
        
        exit(0);
    }
}

int main() {
        printf("world \n");
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

        //deve mandare la x e la z solo quando viene modificata!
        while(1) {
                //read X from pipe 
                read_byteX = read(fd_readX, &Xr, sizeof(float));
                read_byteZ = read(fd_readZ, &Zr, sizeof(float));
                
                if(read_byteX == -1 && errno != EAGAIN) {
                        perror("World: can't read X");
                }
                else if(read_byteX < sizeof(float) || errno != EAGAIN) {
                        //printf("nothing to readX");
                        Xr = Xold;
                }
                else {
                        if(Xr != Xold) {
                                err = (float)rand()/(float)(RAND_MAX/(boundErr*2));
                                err = err - boundErr;
                                p.x = Xr + err;
                        }
                }
                
                //readZ from pipe 
                if(read_byteZ == -1 && errno != EAGAIN) {
                        perror("World: can't read Z");
                }
                else if(read_byteZ < sizeof(float) || errno != EAGAIN) {
                        //printf("nothing to readZ");
                        Zr = Zold;
                }
                else {
                        if(Zr != Zold) {
                                err = (float)rand()/(float)(RAND_MAX/(boundErr*2));
                                err = err - boundErr;
                                p.z = Zr + err;
                        }
                }
                
                //if X and Z are change write on the pipe and update their value
                if(Xr != Xold || Zr != Zold) {
                        
                        //write the new value on the pipe.
                        if(write(fd_write, &p, sizeof(struct position)) != -1) {
                                //update the value if the write go well
                                Xold = Xr;
                                Zold = Zr;
                        }
                        else
                                perror("World: can't write position");

                        //update log file
                }


        }
}