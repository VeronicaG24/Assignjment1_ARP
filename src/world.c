#include <fcntl.h>

#define rX "/tmp/fifoXW"
#define rZ "/tmp/fifoZW"
#define w "/tmp/fifoWI"

int fd_readX, fd_readZ;
int fd_write;

int main() {
    if((fd_readX = open(rX, O_RDONLY)) != 0 ) {
            perror("Can't open /tmp/fifoXW");
            exit(-1);
    }
    
    if((fd_readZ = open(rZ, O_RDONLY)) != 0 ) {
            perror("Can't open /tmp/fifoZW");
            exit(-1);
    }

    if((fd_write = open(w, O_WRONLY)) != 0) {
            perror("can't open  tmp/fifoWI");
            exit(-1);
    }

    //deve mandare la x e la z solo quando viene modificata!
}