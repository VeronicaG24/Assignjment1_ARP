#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define dt 1/30
#define nbytes sizeof(float)
#define r "/tmp/fifoCZ"
#define w "/tmp/fifoZW"
#define zMax 10
#define zMin 0

int fd_read, fd_write;
float z =zMin, v = 0, zOld =0;

char* current_time(){
    time_t rawtime;
    struct tm * timeinfo;
    char* timedate;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    timedate = asctime(timeinfo);
    return timedate;
}

void update_z(float v){
    if((z + v*dt) > zMax) {
            z = zMax;
        }
    else if ((z + v*dt) < zMin) {
        z = zMin;
        }
    else
        z += v*dt;

    //write the new value on the pipe if it is different
    if (z != zOld) {
            if(write(fd_write, &z, nbytes) == -1)
                perror("MotorZ: error in writing");
            
            FILE *flog;
            flog = fopen("logFile.log", "a+"); //a+ fa append 
            if (flog == NULL) {
                perror("MotorZ: cannot open log file");
            }
            else {
                char * curr_time = current_time();
                fprintf(flog, "< MOTOR Z > reached position %f cm at time: %s \n", z, curr_time);
            }
            fclose(flog);
            
            zOld = z;
        }
}

void sig_handler(int signo) {
    
    if(signo == SIGINT) {
        printf("MotorZ:received SIGINT, closing pipes and exit\n");
        
        //chiudere le pipe
        if(close(fd_read) != 0){
            perror("MotorZ:can't close tmp/fifoCZ");
            exit(-1);
        }

        if(close(fd_write) != 0) {
            perror("MotorZ:an't close tmp/fifoZW");
            exit(-1);
        }
        
        exit(0);
    }
    //code to execute when receive SIGUSR1(RESET)
    
    else if(signo==SIGUSR1){
        printf("MotorZ:received SIGUSR1, reset routine starting\n");
        //RESET INSTRUCTION ROUTINE
        //stop 
        update_z(0);
        sleep(0.5);
        v=0;
        while(z!=0){
            //update Z
            update_z(-2);
            sleep(1);
        }
    }
    
    //code to execute when receive SIGUSR2(STOP)
    
    else if(signo ==SIGUSR2){
        printf("MotorZ: received SIGUSR2- STOP routine starting\n");
        //STOP INSTRUCTION ROUTINE
        //set v=0
        //update X
        update_z(0);
        v = 0;
    }

   
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR2(STOP)\n");
    }
}

int main() {

    //gestione segnale SIGINT
    if(signal(SIGINT, sig_handler) == SIG_ERR)
        printf("MotorZ: can't set the signal hendler for SIGINT\n");
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorZ:Can't set the signal handler for SIGUSR2(STOP)\n");
    }

    //aprire la pipe in letteura(CZ) e contrallare non dia errore
    if((fd_read = open(r, O_RDONLY|O_NONBLOCK)) == 0 ) {
        perror("MotorZ: Can't open /tmp/fifoCZ");
        exit(-1);
    }
    
    //aprire pipe in scritture(ZW)
    if((fd_write = open(w, O_WRONLY)) == 0 ) {
        perror("MotorZ: can't open  tmp/fifoZW");
        exit(-1);
    }

    float v_read = 0;
    float zOld = 0;
    int read_byteV;

    while(1) {
        //leggere ZX e controllare che non dia errore
        read_byteV = read(fd_read, &v_read, nbytes);
        
        if(read_byteV == -1 && errno != EAGAIN) 
            perror("MotorZ: error in reading");
        else if(read_byteV < nbytes) {
            //printf("nothing to read");
        }
        else {
            v = v_read;
        }

        update_z(v);
        
        //sleep
        sleep(1);
    }

}
