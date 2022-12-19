/*===========================================================================
-----------------------------------------------------------------------------
  	inspection_console.c
-----------------------------------------------------------------------------

AUTHOR: Written by Francesca Corrao and Veronica Gavagna.

-----------------------------------------------------------------------------

DESCRIPTION
  	The Inspection Console manage the interface of the hoist, 
    showing how it moves, and the stop and reset buttons.

=============================================================================*/

#include "./../include/inspection_utilities.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

//fifo
#define r "/tmp/fifoWI"
//position structure (x,z)
struct position {
        float x;
        float z;
};
int fd_read;
//boolean to define if reset mode is on
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
  Manage signals received
  INPUT:
  SIGINT
    -close pipes
  RETURN:
    null
=====================================*/
void sig_handler(int signo) {
    //signal SIGINT
    if(signo==SIGINT) {
        printf("Inspection: received SIGINT, closing the pipes and exit\n");
        if(close(fd_read)!=0) {
            perror("Inspection: Can't close the read pipe");
            exit(-1);
        }           
        exit(0);
    }

    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        perror("Inspection:Can't set the signal handler for SIGINT\n");
        exit(-1);
    }
}

/*=====================================
  Manage the interface of the hoist,
  and the stop and reset buttons
  RETURN:
    null
=====================================*/
int main(int argc, char const *argv[]) {
    
    //motors' pid
    pid_t pid_motorX, pid_motorZ;
    pid_motorX = atoi(argv[1]);
    pid_motorZ = atoi(argv[2]);

    //get command window pid
    sleep(1);
    //file descriptor for command-inspection consoles
    int fd2=open("/tmp/fifoCI", O_RDONLY|O_NONBLOCK);
    if(fd2<0){
        perror("open pipe CI:");
        fflush(stdout);
        
        if(unlink("/tmp/fifoCI")) {
            perror("unlink CI");
            fflush(stdout);
        }
        sleep(5);
        exit(-1);
    }

    pid_t pid_c=0;
    pid_t p1;
    while(pid_c == 0){
        if(read(fd2, &p1, sizeof(pid_t))<sizeof(pid_t)){
            perror("read error:");
        }
        if(p1!=0){
            printf("read %d", p1);
            pid_c=p1;
        }
    }
    close(fd2);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    struct position p = {0, 0};

    // End-effector coordinates
    float ee_x, ee_z;

    // Initialize User Interface 
    init_console_ui();
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        perror("Inspection:Can't set the signal handler for SIGINT\n");
        exit(-1);
    }

    int read_byte;

    //open pipe World-inspection in writing mode
    if((fd_read = open(r, O_RDONLY | O_NONBLOCK)) == 0) {
            perror("Inspection: Can't open /tmp/fifoWI");
            exit(-1);
    }

    // Infinite loop
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

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "STP button pressed");
                    refresh();
                    //send SIGUSR2 to motorX
                    if(kill(pid_motorX, SIGUSR2) == -1) {
                        perror("Inspection: failed to stop motorX");
                    }
                    //send SIGUSR2 to motorZ
                    if(kill(pid_motorZ, SIGUSR2) == -1)  {
                        perror("Inspection: failed to stop motorZ");
                    }
                    //send SIGUSR2 to commando console
                    if(kill(pid_c, SIGUSR2) == -1)  {
                        perror("Inspection: failed to stop command");
                    }
                    //set boolean reset to false
                    reset=FALSE;
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+");
                    if (flog == NULL) {
                        perror("Inspection Console: cannot open log file");
                    }
                    else {
                        char * curr_time = current_time();
                        fprintf(flog, "< INSP_CONSOLE > stop signal at time: %s \n", curr_time);
                    }
                    fclose(flog);

                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    mvprintw(LINES - 1, 1, "RST button pressed");
                    refresh();
                    //send SIGUSR1 to motorX
                    if(kill(pid_motorX, SIGUSR1) == -1)  {
                        perror("Inspection: failed to reset motorX");
                    }
                    //send SIGUSR1 to motorZ
                    if(kill(pid_motorZ, SIGUSR1) == -1)  {
                        perror("Inspection: failed to reset motorZ");
                    }
                    //send SIGUSR1 to command console
                    if(kill(pid_c, SIGUSR1) == -1)  {
                        perror("Inspection: failed to reset command");
                    }

                    //set boolean reset to true
                    reset=TRUE;
                    
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    //update log file
                    FILE *flog;
                    flog = fopen("logFile.log", "a+");
                    if (flog == NULL) {
                            perror("Inspection Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< INSP_CONSOLE > reset signal at time: %s \n", curr_time);
                    }
                    fclose(flog);
                }
            }
        }
        
        //read new position on the pipe
        read_byte = read(fd_read, &p, sizeof(struct position));
        
        if(read_byte == -1 && errno != EAGAIN) {
            perror("can't read position");
        }
        else if(read_byte < sizeof(struct position)) {
            
        }
        //update values for the interface
        else {
            ee_x = p.x;
            ee_z = p.z;
            // Update UI
            update_console_ui(&ee_x, &ee_z);
            if(ee_x<=0.05 && ee_z<=0.05 && reset ){
                if(kill(pid_c, SIGUSR2) == -1)  {
                    perror("Inspection: failed to resume command");
                    sleep(1);
                    exit(-1);
                }
                reset=FALSE;
            }
        }

	}

    // Terminate
    if(close(fd_read)!=0) {
        perror("Inspection: Can't close the read pipe");
        endwin();
        return -1;  
    }
    else{
        endwin();
        return 0;
    }

}
