#include "./../include/command_utilities.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

#define rwX "/tmp/fifoCX"
#define rwZ "/tmp/fifoCZ"

int fd_X, fd_Z;
float v[] = {0.0, 0.0};
char * fd[2]= { "/tmp/fifoCX","/tmp/fifoCZ"};

int write_vel(int act, int index) {
    /*give the file descriptor fd and a integer to say how velocity need to change act
    write on the pipe associated with fd the new velocity
    act will be:
    -(+1) to increment
    -0 to stop 
    -(-1)to decrement
    */

    //decide the new 
    if(act == 0) {
        v[index] = 0.0;
    }
    else {
        v[index] += act;
    }

    int fd2= open(fd[index], O_WRONLY); 

    if(write(fd2, &v[index], sizeof(float))<sizeof(float)){
        perror("Command: error in write");
    }
    close(fd2);

}

int main(int argc, char const *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();
    
    //Open pipe CX in srittura
     if(fd_X = open(rwX, O_WRONLY) == 0 ) {
        perror("Command: Can't open /tmp/fifoCX");
        exit(-1);
    }
    
    //aprire pipe in scritture(CZ)
    if(fd_Z= open(rwZ, O_WRONLY) == 0 ) {
        perror("Command: can't open  tmp/ffoCZ");
        exit(-1);
    }

    // Infinite loop
    while(TRUE)
	{	
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
                    //inviare messaggio nella pipe
                    write_vel(-1, 0);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    //update Vx- on motor X 
                    //inviare messaggio nella pipe
                    write_vel( 1, 0);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    ////update Vx=0 on motor X 
                    //inviare messaggio nella pipe
                    write_vel(0, 0);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                   
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    //inviare messaggio nella pipe
                    write_vel(-1, 1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //update Vz+ on motor z
                    
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    //update Vz- on motor z
                    //inviare messaggio nella pipe
                    write_vel(1, 1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    //update Vz=0 on motor z
                    //inviare messaggio nella pipe
                    write_vel(0, 1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                }               
            }
        }

        refresh();
	}

    // Terminate
    endwin();
    return 0;
}
