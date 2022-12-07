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
//need to declare v and X outside the main otherwise can't update when RESET or STOP
float X=X_MIN;
float v = 0;

//function to update X
void update_X(float v){
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
}

void sig_handler(int signo) {
    //code to execute when arrive SIGINT
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
    //code to execute when receive SIGUSR1(RESET)
    
    else if(signo==SIGUSR1){
         printf("MotorX: received SIGUSR1- Reset routine starting\n");
        //RESET INSTRUCTION ROUTINE
        //stop 
        update_X(0);
        sleep(1);
        while(X!=0){
            //update X
            update_X(-1);
        }
    }
    

    //code to execute when receive SIGUSR2(STOP)
    
    else if(signo ==SIGUSR2){
        //STOP INSTRUCTION ROUTINE
        //update X
        update_X(0);
        v=0;

    }
    
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

int main(){
    //definire gestione SIGNIT
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR2(STOP)\n");
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

    float v_read = 0;
    float xOld = 0;
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

        update_X(v);
        
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
