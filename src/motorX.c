#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define dt 1/30
#define r "/tmp/fifoCX"
#define w "/tmp/fifoXW"
#define X_MAX 40
#define X_MIN 0

int fd_read;
int fd_write;
int nbytes = sizeof(float);

void sig_handler(int signo) {
    
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
    signal(SIGINT, sig_handler);
} 

int main(){
    //definire gestione SIGNIT
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGINT\n");
    }
    
    //aprire la pipe in letteura(CX) e contrallare non dia errore
    if((fd_read=open(r, O_RDONLY| O_NONBLOCK)) ==0 ) {
        perror("MotorX:Can't open /tmp/fifoCX");
        exit(-1);
    }
    
    //aprire pipe in scritture(XW)
    if((fd_write=open(w, O_WRONLY))==0) {
        perror("MotorX:can't open  tmp/fifoXW");
        exit(-1);
    }

    float v = 0, v_read = 0;
    float X=X_MIN, xOld = 0;
    int read_byteV;
    
    while(1) {
        //leggere CX e controllare che non dia errore
        read_byteV = read(fd_read, &v_read, nbytes);
        
        if(read_byteV == -1 && errno != EAGAIN) 
            perror("Motorx: error in reading");
        else if(read_byteV < nbytes) {
            //printf("MotorX: nothing to read");
        }
        else {
            v = v_read;
        }

        //aggiornare X
        float dx = (v*dt);
        if(X< (X_MAX - dx) || X > (X_MIN + dx)) { 
            X+=dx;
        }
        else 
            X=X_MAX;
        
        if((X + dx) > X_MAX) {
            X=X_MAX;
        }
        else if( (X + dx) < X_MIN) {
            X=X_MIN;
        }
        else {
            X+=dx;
        }
        
        //scrivere in XW solo se x è cambiata
        if (X != xOld) {
            if(write(fd_write, &X, nbytes) == -1)
                perror("MotorX: error in writing");
            
            xOld = X;
        }
        
        //sleep
        sleep(1);
    }

    close(fd_read);
    close(fd_write);
}
