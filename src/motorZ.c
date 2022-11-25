#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define dt 1/30
#define nbytes sizeof(float)
#define r "/tmp/fifoCZ"
#define w "/tmp/fifoZW"
#define zMax 10
#define zMin 0

int fd_read, fd_write;

void sig_handler(int signo) {
    if(signo == SIGINT) {
        printf("received SIGINT, closing pipes and exit\n");
        //chiudere le pipe
        if(close(fd_read) != 0){
            perror("can't close tmp/fifoCZ");
            exit(-1);
        }
        if(close(fd_write) != 0) {
            perror("can't close tmp/fifoZW");
            exit(-1);
        }
        exit(0);
    }

    signal(SIGINT, sig_handler);
}

int main(){
    float v=0;
    float z = zMin;

    //gestione segnale SIGINT
    if(signal(SIGINT, sig_handler) == SIG_ERR)
        printf("can't set the signal hendler for SIGINT\n");
    
    //aprire la pipe in letteura(CZ) e contrallare non dia errore
    if(fd_read = open(r, O_RDONLY) == 0 ) {
        perror("Can't open /tmp/fifoCZ");
        exit(-1);
    }
    
    //aprire pipe in scritture(ZW)
    if(fd_write = open(w, O_WRONLY) == 0 ) {
        perror("can't open  tmp/fifoZW");
        exit(-1);
    }

    while(1){
        //leggere ZX e controllare che non dia errore
        if(read(fd_read, &v, nbytes)==-1)
            perror("error in reading");
        
        //aggiornare Z
        if(z < zMax - v*dt) {
            z += v*dt;
        }
        else
            z = zMax;    
        
        //scrivere in XW
        if(write(fd_write, &z, nbytes) == -1)
            perror("error in writing");

        //sleep
        sleep(1);
    }

}
