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

#define r "/tmp/fifoWI"

struct position {
        float x;
        float z;
};
int fd_read;
bool reset=FALSE;

char* current_time(){
    time_t rawtime;
    struct tm * timeinfo;
    char* timedate;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    timedate = asctime(timeinfo);
    return timedate;
}

int main(int argc, char const *argv[]) {
    
    //const char * pid_cmd_c = argv[3];
    //char * pid_mX_c = argv[4];
    //char * pid_mZ_c = argv[5];

    pid_t pid_cmd, pid_motorX, pid_motorZ;
    pid_cmd = atoi(argv[1]);
    pid_motorX = atoi(argv[2]);
    pid_motorZ = atoi(argv[3]);

    //get command window pid
    sleep(2);
    //file descriptor
    int fd=open("/tmp/fifoIC", O_WRONLY);
    if(fd){
        perror("open pipe IC:");
        fflush(stdout);
    }
    int fd2=open("/tmp/fifoCI", O_RDONLY|O_NONBLOCK);
    if(fd2){
        perror("open pipe CI:");
        fflush(stdout);
    }
    pid_t pid_c=0;
    pid_t p1;
    while(pid_c == 0){
        if(read(fd2, &p1, sizeof(pid_t))){
            perror("read:");
        }
        if(p1!=0){
            printf("read %d", p1);
            pid_c=p1;
        }
    }
    int res=1;
    if(write(fd,&res, sizeof(int))){
        perror("write:");
    }
    close(fd);
    close(fd2);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    struct position p = {0, 0};

    // End-effector coordinates
    float ee_x, ee_z;

    // Initialize User Interface 
    init_console_ui();

    //get pid command window from command window

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
                    kill(pid_motorX, SIGUSR2);
                    kill(pid_motorZ, SIGUSR2);
                    kill(pid_c, SIGUSR2);
                    reset=FALSE;
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    //signal to STOP everything send to every proccess
                    FILE *flog;
                    flog = fopen("logFile.log", "a+"); //a+ fa append 
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
                    kill(pid_motorX, SIGUSR1);
                    kill(pid_motorZ, SIGUSR1);
                    kill(pid_c, SIGUSR1);
                    reset=TRUE;
                    sleep(1);
                    
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    FILE *flog;
                    flog = fopen("logFile.log", "a+"); //a+ fa append 
                    if (flog == NULL) {
                            perror("Inspection Console: cannot open log file");
                    }
                    else {
                            char * curr_time = current_time();
                            fprintf(flog, "< INSP_CONSOLE > reset signal at time: %s \n", curr_time);
                    }
                    fclose(flog);
                    //comand console unable to use untill reached the original position
                    //motor X and motor Z V=0 then Vx Vz negative untill reach 0,0
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
            if(ee_x<=0.05 && ee_z<=0.05 && reset ){
                kill(pid_c, SIGUSR2);
                reset=FALSE;
            }
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
