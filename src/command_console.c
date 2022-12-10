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
char * fd[2]= {"/tmp/fifoCX","/tmp/fifoCZ"};
bool reset=FALSE;

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

void sig_handler(int signo) {
    //code to execute when arrive SIGINT
    if(signo==SIGINT){
        printf("Command: received SIGINT, closing the pipes and exit\n");
        //chiusura pipe
        if(close(fd_X)!=0){
            perror("Command: Can't close the re");
            exit(-1);
        }
        if(close(fd_Z)!= 0){
            perror("Command Can't close the write pipe");
            exit(-1);
        }             
        exit(0);
    }
    //code to execute when receive SIGUSR1(RESET)
    
    else if(signo==SIGUSR1){
        //RESET INSTRUCTION ROUTINE
        //printf("Command: received SIGUSR1- Reset routine starting\n");
        //vel motorZ 0
        write_vel(0, 1);
        //vel motorX 0
        write_vel(0, 0);
        reset=TRUE;
        //disattivare fino a quando non arriva a 0
        while(reset){
            //resta bloccato fino all'arrivo di segnale esterno
        }   
    }
    
    //code to execute when receive SIGUSR2(STOP)
    
    else if(signo ==SIGUSR2){
        //STOP INSTRUCTION ROUTINE
        //printf("Command: received SIGUSR2- STOP routine starting\n");
        //vel motorZ 0
        write_vel(0, 1);
        //vel motorX 0
        write_vel(0, 0);
        reset=FALSE;
    }
    
    if(signal(SIGINT, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGINT\n");
    }
    if(signal(SIGUSR1, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR1(RESET)\n");
    }
    if(signal(SIGUSR2, sig_handler)==SIG_ERR) {
        printf("MotorX:Can't set the signal handler for SIGUSR2(STOP)\n");
    }
} 

int main(int argc, char const *argv[]) {
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    //send pid to inspection console
    printf("synchonization command-inspection\n");
     
    //fifo from command to inspection to send pid
    /*if(unlink("/tmp/fifoCI")){
        perror("unlink CI");
        fflush(stdout);
    }
    if(unlink("/tmp/fifoIC")){
        perror("unlink IC:");
        fflush(stdout);
    }*/

    if(mkfifo("/tmp/fifoCI", 0666)<0){
        perror("create CI: ");
        fflush(stdout);
    }
    else
        printf("create CI!");
    //fifo from inspection to command to say receive pid
    if(mkfifo("/tmp/fifoIC", 0666)){
        perror("create IC: ");
        fflush(stdout);
    }
    //file descriptor
    int fw=open("/tmp/fifoCI", O_RDWR);
    if(fw){
        perror("open pipe CI:");
        fflush(stdout);
    }
    int fr=open("/tmp/fifoIC", O_RDONLY|O_NONBLOCK);
    if(fr){
        perror("open pipe IC:");
        fflush(stdout);
    }
    pid_t mypid= getpid();
    int res=0;
    int res2;
    printf("%d", mypid);
    fflush(stdout);
    //write pid
    if(write(fw, &mypid, sizeof(pid_t))){
        perror("write:");
        fflush(stdout);
    }

    /*while(res == 0){
        if(read(fr, &res2, sizeof(int))){
            perror("read:");
            fflush(stdout);
        }
        if(res2==1){
            res ==1;
        }
    }*/
    sleep(3);
    if(close(fw)){
        perror("close CI:");
        fflush(stdout);
    }
    if(close(fr)){
         perror("close IC:");
         fflush(stdout);
    }
    if(unlink("/tmp/fifoCI")){
        perror("unlink CI");
        fflush(stdout);
    }
    if(unlink("/tmp/fifoIC")){
        perror("unlink IC:");
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
