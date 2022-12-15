#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

// Retrieve current time procedure
char* current_time(){
    time_t rawtime;
    struct tm * timeinfo;
    char* timedate;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    timedate = asctime(timeinfo);
    return timedate;
}

int main(int argc, char const *argv[])
{
    pid_t pid_motorX, pid_motorZ,pid_cmd, pid_insp, pid_world, pid_master;
    pid_motorX = atoi(argv[1]);
    pid_motorZ = atoi(argv[2]);
    pid_cmd=atoi(argv[3]);
    pid_insp=atoi(argv[4]);
    pid_world=atoi(argv[5]);
    pid_master=atoi(argv[6]);

    //change into watchdog
    FILE *flog; //file pointer log file
    long bytesWritten = 0;
    long bytesWritten_old = 0;
    time_t startTimer;
    sleep(2);

    while(1) { 
    int status;
    //waitpid(pid_cmd, &status, 0);
    //waitpid(pid_insp, &status, 0);
    flog = fopen("./logFile.log", "r"); //decidere posizione
    fseek(flog, 0L, SEEK_END); //mi posizione alla fine del file (file pointer at 0 bytes from the end)
    bytesWritten_old = ftell(flog); //numero di byte dall'inizio alla fine (alla posizione del file pointer)
    fclose(flog);
    startTimer = time(NULL);

    bytesWritten = bytesWritten_old;
    while((bytesWritten==bytesWritten_old) && (difftime(time(NULL), startTimer) < 60)) {
        flog = fopen("./logFile.log", "r"); //decidere posizione
        fseek(flog, 0L, SEEK_END); 
        bytesWritten_old = ftell(flog); 
        fclose(flog);
    //sleep
  }


  if(difftime(time(NULL), startTimer) >= 60) {
    // Write on the LOG.log
    char * currTime = current_time();
    flog = fopen("./logFile.log", "a+"); //decidere posizione (controlla a o a+, per me a+ meglio)
    fprintf(flog, "< WATCHDOG > inactivity detected, resetting the system at: %s \n", currTime);
    fclose(flog);

    sleep(1);
    if(kill(pid_motorX,SIGINT) == -1) { //controlla sia -1
      perror("Watchdog: failed to kill motorX");
    }
    sleep(1); //simone ha consigliato di fare le sleep se no non killa bene
    if(kill(pid_motorZ,SIGINT) == -1) { 
      perror("Watchdog: failed to kill motorZ");
    }
    sleep(1);
    if(kill(pid_world,SIGINT) == -1) {
      perror("Watchdog: failed to kill world");
    }
    sleep(1);
    if(kill(pid_cmd,SIGINT) == -1) { //controlla sia -1
      perror("Watchdog: failed to kill motorX");
    }
    sleep(1);
    if(kill(pid_insp,SIGINT) == -1) { //controlla sia -1
      perror("Watchdog: failed to kill motorX");
    }
    sleep(1);
    if(kill(pid_master,SIGINT) == -1) { //controlla sia -1
      perror("Watchdog: failed to kill master");
    }
    sleep(1);
    
    //unlinkpipe();
    printf ("Main program exiting with status %d\n", status);
    exit(0);
  }
    
}
}
