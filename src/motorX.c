#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include <fcntl.h>

#define dt 1/30
#define nbytes sizeof(float)
#define r "/tmp/fifoCX"
#define w "/tmp/fifoXW"
#define X_MAX 40
#define X_MIN 0


int main(){
    float v=0;
    float X=X_MIN;
    while(1){
        //aprire la pipe in letteura(CX) e contrallare non dia errore
        int fd_read ;
        if(open(r, O_RDONLY) !=0 ){
            perror("Can't open /tmp/fifoCX");
        }
        //aprire pipe in scritture(XW)
        int fd_write;
        if(open(w, O_WRONLY)!=0)
            perror("can't open  tmp/fifoXW");
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
