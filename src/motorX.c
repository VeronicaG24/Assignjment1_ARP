#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define dt 1/30
#define nbytes sizeof(float)
#define r "/tmp/fifoCX"
#define w "/tmp/fifoXW"
#define X_MAX 40
#define X_MIN 0
int fd_read;
int fd_write;
void sig_handler(int signo){
    if(signo==SIGINT){
        print("received SIGINT, closing the pipes and exit\n");
        if(close(fd_read)!=0){
            perror("Can't close the reading pipe");
            exit(-1);
        }
        if(close(fd_write)!=0){
            perror("Can't close the write pipe");
            exit(-1);
        }
        exit(0);
    }
    signal(SIGINT, sig_handler);
} 

int main(){
    float v=0;
    float X=X_MIN;
    //definire gestione SIGNIT
    if(signal(SIGINT, sig_handler)==SIG_ERR){
        printf("Can't set the signal handler for SIGINT\n");
    }
    //aprire la pipe in letteura(CX) e contrallare non dia errore
        if((fd_read=open(r, O_RDONLY)) !=0 ){
            perror("Can't open /tmp/fifoCX");
            exit(-1);
        }
        //aprire pipe in scritture(XW)
        if((fd_write=open(w, O_WRONLY))!=0){
            perror("can't open  tmp/fifoXW");
            exit(-1);
        }
    while(1){
        //leggere CX e controllare che non dia errore
        if(read(fd_read,&v, nbytes)==-1)
            perror("error in reading");
        //aggiornare X
        float dx = (v*dt);
        if(X<X_MAX-dx){
            X+=dx;
        }
        else 
            X=X_MAX;
        //scrivere in XW
        if(write(fd_write,&X, nbytes)==-1)
            perror("error in writing");
        //sleep
        sleep(1);
    }

    
}
