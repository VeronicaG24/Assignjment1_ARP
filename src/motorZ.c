#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include <fcntl.h>

#define dt 1/30
#define nbytes sizeof(float)
#define r "/tmp/fifoCZ"
#define w "/tmp/fifoZW"
#define zMax 10
#define zMin 0


int main(){
    float v=0;
    float z = zMin;
    while(1){
        //aprire la pipe in letteura(CZ) e contrallare non dia errore
        int fd_read ;
        if(open(r, O_RDONLY) !=0 ){
            perror("Can't open /tmp/fifoCZ");
        }
        //aprire pipe in scritture(ZW)
        int fd_write;
        if(open(w, O_WRONLY)!=0)
            perror("can't open  tmp/fifoZW");
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
