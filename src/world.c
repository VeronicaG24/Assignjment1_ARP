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
        float Xr=0, Zr=0;
        //deve mandare la x e la z solo quando viene modificata!
        while(1){
                //read X from pipe
                if(read(fd_readX, &Xr, sizeof(float))==-1){
                        perror("can't read X");
                }
                
                //readZ from pipe 
                if(read(fd_readZ, &Zr, sizeof(float))==-1){
                        perror("can't read Z");
                }
                
                //if X and Z are change write on the pipe and update their value
                if(Xr!= X || Zr != Z){
                        //write the new value on the pipe.
                        //update the value if the write go well
                        X=Xr;
                        Z=Zr;
                        //update log file
                }


        }
}