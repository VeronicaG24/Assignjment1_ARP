/*===========================================================================
-----------------------------------------------------------------------------
  	command_console.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
  	The Command Console manage the interface for the velocities 
    of the two motors. It has 6 button to increase/decrease the velocities 
    along x-axis and z-axis and two buttons to stop the two motors. 

=============================================================================*/

#include "./../include/command_utilities.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

//fifos
#define rwX "/tmp/fifoCX"
#define rwZ "/tmp/fifoCZ"

int fd_X, fd_Z;
int v[] = {0.0, 0.0};
char * fd[2]= {"/tmp/fifoCX","/tmp/fifoCZ"};
bool reset=FALSE;

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
  Write new velocity on the pipe
  to comunicate to the motor.
  INPUT:
  act
    -(+1) to increment
    -0 to stop
    -(-1)to decrement
  index
    -0 motorX
    -1 motorZ
  RETURN:
    null
=====================================*/
int write_vel(int act, int index) {
    if(act == 0) {
        v[index] = 0.0;
    }
    else {
        v[index] += act;
    }

    int fd2= open(fd[index], O_WRONLY); 

    if(write(fd2, &v[index], sizeof(int))<sizeof(int)){
        perror("Command: error in write");
    }
    close(fd2);
}

/*=====================================
  Manage signals received
  INPUT:
  SIGINT
    -close pipes
  SIGUSR1
    -reset routine
  SIGUSR2
    -stop routine
  RETURN:
    null
=====================================*/
void sig_handler(int signo) {
    //signal SIGINT
    if(signo==SIGINT) {
        printf("Command: received SIGINT, closing the pipes and exit\n");
        if(close(fd_X)!=0) {
            perror("Command: Can't close the read pipe");
            exit(-1);
        }
        if(close(fd_Z)!= 0) {
            perror("Command Can't close the write pipe");
            exit(-1);
        }             
        exit(0);
    }

    //signal SIGUSR1 (RESET)
    else if(signo==SIGUSR1){
        //set motors velocity to 0
        write_vel(0, 1);
        write_vel(0, 0);
        
        reset=TRUE;
        while(reset){
            //blocked until hoist reach (0,0) position
        }   
    }
    
    //signal SIGUSR2 (STOP)
    else if(signo ==SIGUSR2){
        //set motors velocity to 0
        write_vel(0, 1);
        write_vel(0, 0);
        reset=FALSE;
    }
    
    //manage errors in handling signals
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        perror("Command:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        perror("Command:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        perror("Command:Can't set the signal handler for SIGUSR2(STOP)\n");
    }
}

/*=====================================
  Manage the interface for the 
  velocities, generate buttons
  RETURN:
    null
=====================================*/
int main(int argc, char const *argv[]) {
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    //send pid to inspection console
    printf("synchonization command-inspection\n");


    if(mkfifo("/tmp/fifoCI", 0666) < 0){
        perror("create CI: ");
        fflush(stdout);
    }
    else
        printf("create CI!");

    int fw=open("/tmp/fifoCI", O_RDWR);
    if(fw){
        perror("open pipe CI:");
        fflush(stdout);
    }
    pid_t mypid= getpid();
    int res=0;
    int res2;
    printf("%d", mypid);
    fflush(stdout);
    
    if(write(fw, &mypid, sizeof(pid_t))){
        perror("write:");
        fflush(stdout);
    }
    sleep(2);

    if(close(fw)){
        perror("close CI:");
        fflush(stdout);
    }

    if(unlink("/tmp/fifoCI")){
        perror("unlink CI");
        fflush(stdout);
    }

    // Initialize User Interface 
    init_console_ui();

    //initilize signal handling
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("Command:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("Command:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("Command:Can't set the signal handler for SIGUSR2(STOP)\n");
    }

    while(TRUE) {	
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // Vx-- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                    //update Vx+ on motor X
                    //send message to the pipe
                    write_vel(-1, 0);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+"); //a+ fa append 
                    if (flog == NULL) {
                            perror("Command Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< COMM_CONSOLE > pressed Vx-- button at time: %s \n", curr_time);
                    }
                    fclose(flog);

                }
                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    //update Vx- on motor X 
                    //send message to the pipe
                    write_vel( 1, 0);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+"); //a+ fa append 
                    if (flog == NULL) {
                            perror("Command Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< COMM_CONSOLE > pressed Vx++ button at time: %s \n", curr_time);
                    }
                    fclose(flog);

                }
                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    //update Vx=0 on motor X 
                    //send message to the pipe
                    write_vel(0, 0);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+"); //a+ fa append 
                    if (flog == NULL) {
                            perror("Command Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< COMM_CONSOLE > pressed Vx stop button at time: %s \n", curr_time);
                    }
                    fclose(flog);
                   
                }
                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    //update Vz- on motor X 
                    //send message to the pipe
                    write_vel(-1, 1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+");
                    if (flog == NULL) {
                            perror("Command Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< COMM_CONSOLE > pressed Vz-- button at time: %s \n", curr_time);
                    }
                    fclose(flog);
                    
                }
                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    //update Vz- on motor z
                    //send message to the pipe
                    write_vel(1, 1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+");
                    if (flog == NULL) {
                            perror("Command Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< COMM_CONSOLE > pressed Vz++ button at time: %s \n", curr_time);
                    }
                    fclose(flog);

                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    //update Vz=0 on motor z
                    //send message to the pipe
                    write_vel(0, 1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+"); //a+ fa append 
                    if (flog == NULL) {
                            perror("Command Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< COMM_CONSOLE > pressed Vz stop button at time: %s \n", curr_time);
                    }
                    fclose(flog);
                    
                }               
            }
        }

        refresh();
	}

    // Terminate
    endwin();
    return 0;
}
