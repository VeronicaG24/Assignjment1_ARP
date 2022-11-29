#include <fcntl.h>

#define rX "/tmp/fifoXW"
#define rZ "/tmp/fifoZW"
#define w "/tmp/fifoWI"

int fd_readX, fd_readZ;
int fd_write;

int main() {
        if((fd_readX = open(rX, O_RDONLY)) == 0 ) {
            perror("Can't open /tmp/fifoXW");
            exit(-1);
        }
    
        if((fd_readZ = open(rZ, O_RDONLY)) == 0 ) {
            perror("Can't open /tmp/fifoZW");
            exit(-1);
        }

        if((fd_write = open(w, O_WRONLY)) == 0) {
            perror("can't open  tmp/fifoWI");
            exit(-1);
        }
        float X=0;
        float Z=0;
    //deve mandare la x e la z solo quando viene modificata!
        while(1){
                float Xr, Zr;
                //read X from pipe
                if(read(fd_readX, &Xr, sizeof(float))){
                        perror("can't read X");
                }
                //readZ from pipe 
                if(read(fd_readZ, &Zr, sizeof(float))){
                        perro("can't read Z");
                }
                if(Xr!= X || Zr != Z){
                        //write the new value on the pipe.
                        X=Xr;
                        Z=Zr;
                        //update log file
                }


        }
}