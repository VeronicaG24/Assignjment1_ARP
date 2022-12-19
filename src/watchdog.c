/*===========================================================================
-----------------------------------------------------------------------------
  	watchdog.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
  	The Warchdog process checks the inactivity of the programs 
    though the log file. If any process is inactive for more than 60 seconds
    it kills all the process including itself.

=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

/*=====================================
  Get current time
  RETURN:
    time and date
=====================================*/
char* current_time(){
    time_t rawtime;
    struct tm * timeinfo;
    char* timedate;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    timedate = asctime(timeinfo);
    return timedate;
}

/*=====================================
  Checks the inactivity of the programs
  RETURN:
    null
=====================================*/
int main(int argc, char const *argv[]) {
  
    //pids of all the processes
    pid_t pid_motorX, pid_motorZ,pid_cmd, pid_insp, pid_world, pid_master;
    pid_motorX = atoi(argv[1]);
    pid_motorZ = atoi(argv[2]);
    pid_cmd=atoi(argv[3]);
    pid_insp=atoi(argv[4]);
    pid_world=atoi(argv[5]);
    pid_master=atoi(argv[6]);

    //varibles for log file
    FILE *flog;
    long bytesWritten = 0;
    long bytesWritten_old = 0;
    time_t startTimer;
    sleep(1);

    //infinite loop
    while(1) { 
      
      //open log file
      flog = fopen("./logFile.log", "r");
      //file pointer at 0 bytes from the end
      fseek(flog, 0L, SEEK_END);
      //count number of bytes written before the file pointer
      bytesWritten_old = ftell(flog);
      //close log file
      fclose(flog);
      //set timer
      startTimer = time(NULL);
      //set number of bytes written new to the one read before
      bytesWritten = bytesWritten_old;

      //loop until the number of bytes before and after are equal AND the timer of 60 seconds is not over
      while((bytesWritten==bytesWritten_old) && (difftime(time(NULL), startTimer) < 60)) {
        //open log fine
        flog = fopen("./logFile.log", "r");
        if(flog==NULL){
          perror("Watchdog: can not open log file");

        }
        else{
          //file pointer at 0 bytes from the end
          fseek(flog, 0L, SEEK_END); 
          //update old number of bytes
          bytesWritten_old = ftell(flog);
        }
        //close the file
        fclose(flog);
      }

      //if timer is over, so the number of bytes is not changed, kill everything 
      if(difftime(time(NULL), startTimer) >= 60) {
        //write on the log file
        char * currTime = current_time();
        flog = fopen("./logFile.log", "a+");
        if(flog==NULL){
          perror("Watchdog: can not open log file");

        }
        else
          fprintf(flog, "< WATCHDOG > inactivity detected, resetting the system at: %s \n", currTime);
        fclose(flog);

        //kill motorX process
        sleep(1);
        if(kill(pid_motorX,SIGINT) == -1) {
          perror("Watchdog: failed to kill motorX");
        }
        //kill motorZ process
        sleep(1);
        if(kill(pid_motorZ,SIGINT) == -1) { 
          perror("Watchdog: failed to kill motorZ");
        }
        //kill world process
        sleep(1);
        if(kill(pid_world,SIGINT) == -1) {
          perror("Watchdog: failed to kill world");
        }
        //kill command console process
        sleep(1);
        if(kill(pid_cmd,SIGINT) == -1) {
          perror("Watchdog: failed to kill motorX");
        }
        //kill inspection console process
        sleep(1);
        if(kill(pid_insp,SIGINT) == -1) {
          perror("Watchdog: failed to kill motorX");
        }
        sleep(1);
        //kill master process
        if(kill(pid_master,SIGINT) == -1) {
          perror("Watchdog: failed to kill master");
        }
        sleep(1);

        //kill itself
        printf ("Watchdog: exiting with status %d\n", 0);
        exit(0);
      }
    }
}
