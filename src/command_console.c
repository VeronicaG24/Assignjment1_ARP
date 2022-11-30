#include "./../include/command_utilities.h"
#include <fcntl.h>

#define rwX "/tmp/fifoCX"
#define rwZ "/tmp/fifoCZ"

int fd_X, fd_Z;
float v[2] = {0, 0};

int write_vel(int fd, int act, int index){
    /*give the file descriptor fd and a integer to say how velocity need to change act
    write on the pipe associated with fd the new velocity
    act will be:
    -(+1) to increment
    -0 to stop 
    -(-1)to decrement
    */

    //decide the new 
    if(act == 0) {
        v[index] = 0;
    }
    else {
        v[index] += act;
    }
    
    if(write(fd, &v[index], sizeof(float))){
        perror("Can't write ");
    }

}

int main(int argc, char const *argv[])
{
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();
    
    //Open pipes
     if(fd_X = open(rwX, O_RDONLY) == 0 ) {
        perror("Can't open /tmp/fifoCZ");
        exit(-1);
    }
    
    //aprire pipe in scritture(ZW)
    if(fd_Z= open(rwZ, O_WRONLY) == 0 ) {
        perror("can't open  tmp/fifoZW");
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

                // Vx++ button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //update Vx+ on motor X 
                        //inviare messaggio nella pipe
                        write_vel(fd_X, -1, 0);
                }

                // Vx-- button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //update Vx- on motor X 
                        //inviare messaggio nella pipe
                        write_vel(fd_X, 1, 0);
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    ////update Vx=0 on motor X 
                        //inviare messaggio nella pipe
                        write_vel(fd_X, 0, 0);
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //update Vz+ on motor z
                        //inviare messaggio nella pipe
                        write_vel(fd_Z, -1, 1);
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //update Vz- on motor z
                        //inviare messaggio nella pipe
                        write_vel(fd_Z, 1, 1);
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //update Vz=0 on motor z
                        //inviare messaggio nella pipe
                        write_vel(fd_Z, 0, 1);
                }               
            }
        }

        refresh();
	}

    // Terminate
    endwin();
    return 0;
}
