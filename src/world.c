#include <fcntl.h>

#define rX "/tmp/fifoXW"
#define rZ "/tmp/fifoZW"
#define w "/tmp/fifoWI"

int fd_readX, fd_readZ;
int fd_write;
struct position {
        float x;
        float z;
};

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

        float Xold=0;
        float Zold=0;
        float Xr=0, Zr=0;
        struct position p = {0, 0};
        int read_byteX, read_byteZ;
        //deve mandare la x e la z solo quando viene modificata!
        while(1){
                //read X from pipe
                read_byteX = read(fd_readX, &Xr, sizeof(float));
                read_byteZ = read(fd_readZ, &Zr, sizeof(float));
                
                if(read_byteX == -1) {
                        perror("can't read X");
                }
                else if(read_byteX < sizeof(float)) {
                        printf("nothing to readX");
                        Xr = Xold;
                }
                else {
                        if(Xr!= Xold) {
                                p.x = Xr;
                        }
                }
                
                //readZ from pipe 
                if(read_byteZ == -1) {
                        perror("can't read Z");
                }
                else if(read_byteZ < sizeof(float)) {
                        printf("nothing to readZ");
                        Zr = Zold;
                }
                else {
                        if(Zr!= Zold) {
                                p.z = Zr;
                        }
                }
                
                //if X and Z are change write on the pipe and update their value
                if(Xr!= Xold || Zr != Zold) {
                        
                        //write the new value on the pipe.
                        if(write(fd_write, &p, sizeof(struct position)) != -1) {
                                //update the value if the write go well
                                Xold = Xr;
                                Zold = Zr;
                        }
                        else
                                perror("can't write position");

                        
                        //update log file
                }


        }
}