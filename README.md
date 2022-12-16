# ARP-Hoist-Assignment
Base project structure for the first *Advanced and Robot Programming* (ARP) assignment.
The project provides the basic functionalities for the **Command** and **Inspection processes**, both of which are implemented through the *ncurses library* as simple GUIs. In particular, the repository is organized as follows:
- The `src` folder contains the source code for the Command console, Inspection console and Master, MotorX, MotorZ, World and watchdog processes.
- The `include` folder contains all the data structures and methods used within the ncurses framework to build the two GUIs. 
- The `bin` folder is where the executable files are expected to be after compilation.
- The `logFile.log` file cointains what is hapening during the execution.
- The `compile.sh` and `run.sh` to copile and run the project.
- The `install.md` with the instruction for installing the necessary for running the code.

## How to run
To run the program it is necessary to download the repository:
```console
git clone https://github.com/VeronicaG24/Assignment1_ARP.git
```
Then, move into the folder and compile the code using:
```console
bash ./compile.sh
```
And run it:
```console
bash ./run.sh
```

## Description of the code
The code is divided into 7 processes: Command console, Inspection console and Master, MotorX, MotorZ, World and Watchdog. In each of the process, signal are manage through signal handler.

### Master
The master program manages the pipes, spawns the other processes, create the Log File and waits until one of the process closes to kill all the other process and unlink the pipes.

The pipes used are:
- MotorX-World
- MotorZ-World
- World-Inspection console
- Command console-MotorX
- Command console-MotorZ

### MotorX and MotorZ
MotorX and MotorZ manage the motion along x-axis and z-axis. These two processes are equivalent except for the direction of the motion: the pipe with the Command console is open in reading mode with non-blocking to avoind error if the pipe is not already opened in writing on the other side, and the pipe with the World is opened in writing mode.
After opening the pipes, an infinite loop starts to continuosly read from the pipe the new velocity and update the position.

### World
World manages the position of the ??. First, it opens the pipe with MotorX and MotorZ in reading mode with non-blocking, and with Inspection Console in writing mode. Then, an infinite loop starts to continuasly read from the pipe of the two motors the updated position of the ?? and, if it is different from the previous one (considering a random error bitween -0.05 and +0.05), it updates the new position and write it to the Inspection Console.

### Command Console
The Command Console manage the interface for the velocities of the two motors. !!add the one with the inspection and explain!!
It opens the pipe with the two motors in writing mode and then an infinite loop starts. In the loop are continusly checked if any button is pressed, when one of them is pressed the velocity is updated and written on pipe of the corresponding motor. Here are also manage the signal for stopping mode and resetting mode sent by the Inspection console: in case of stop signal the velocity of both motors is set to 0 until another button is pressed, in case of reset signal the velocity of both motors are first set to zero and then the command blocked until the reset routine is ended (so, until the ?? goes back to origin position).

### Inspection Console
The Inspection Console manage the interface of the ??, showing how it moves, and the stop and reset buttons. First, the pipes with the Command console and World are opened in non-blocking reading mode, !!aggiungi parte per leggere il pid della command". Then, an infinite loop starts to continuosly check if the reset or the stop button has been pressed and update the position on the interface.
If the stop or reset button is pressed, different signals are sent to MotorX, MotorZ and Command console.

### Watchdog 
The Warchdog process checks the inactivity of the program though the log file. It continuosly check the number of byte written on the log file and if it does not chenge for more than 60 second, it kills all the processes including itself.

The log file is updated each time occurs one of the followinf event:
- from MotorX and MotorZ: its position changes
- from World: the coordinates of the ?? are changed
- from Command Console: each time a button is pressed
- from Inspection Console: each time the stop button or the reset button is pressed
- from Watchdog: when an inactivity is detected


