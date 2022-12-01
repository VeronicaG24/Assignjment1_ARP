# compile .c files, from src folder, and put them into exc folder created

gcc ./src/master.c -o ./bin/master
gcc ./src/inspection_console.c -lncurses -lm -o ./bin/inspection
gcc ./src/command_console.c -lncurses -o ./bin/command
gcc ./src/motorX.c -o ./bin/motorX
gcc ./src/motorZ.c -o ./bin/motorZ
gcc ./src/world.c -o ./bin/world


