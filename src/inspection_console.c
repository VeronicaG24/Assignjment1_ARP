#include "./../include/inspection_utilities.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define r "/tmp/fifoWI"

struct position {
        float x;
        float z;
};
int fd_read;

int main(int argc, char const *argv[]) {
    
    //const char * pid_cmd_c = argv[3];
    //char * pid_mX_c = argv[4];
    //char * pid_mZ_c = argv[5];


    pid_t pid_cmd, pid_motorX, pid_motorZ;
    pid_cmd = atoi(argv[1]);
    pid_motorX = atoi(argv[2]);
    pid_motorZ = atoi(argv[3]);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    struct position p = {0, 0};

    // End-effector coordinates
    float ee_x, ee_z;

    // Initialize User Interface 
    init_console_ui();

    /*printf("3)%s\n", argv[1]);
    printf("4)%s\n", argv[2]);
    printf("5)%s\n", argv[3]);
    fflush(stdout);*/

    int read_byte;

    //aprire pipe WI in lettura
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
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //signal to STOP everything send to every proccess
                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    mvprintw(LINES - 1, 1, "RST button pressed");
                    refresh();
                    sleep(1);
                    
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    //comand console unable to use untill reached the original position
                    //motor X and motor Z V=0 then Vx Vz negative untill reach 0,0
                    kill(pid_motorX, SIGUSR1);
                    kill(pid_motorZ, SIGUSR1);
                    kill(pid_cmd, SIGUSR1);
                }
            }
        }
        
        read_byte = read(fd_read, &p, sizeof(struct position));
        
        if(read_byte == -1 && errno != EAGAIN) {
            perror("can't read position");
        }
        else if(read_byte < sizeof(struct position)) {
            //printf("nothing to read");
        }
        else {
            ee_x = p.x;
            ee_z = p.z;
            // Update UI
            update_console_ui(&ee_x, &ee_z);
        }

        // To be commented in final version...
        /*switch (cmd)
        {
            case KEY_LEFT:
                ee_x--;
                break;
            case KEY_RIGHT:
                ee_x++;
                break;
            case KEY_UP:
                ee_z--;
                break;
            case KEY_DOWN:
                ee_z++;
                break;
            default:
                break;
        }
        */

	}

    // Terminate
    endwin();
    return 0;
}
