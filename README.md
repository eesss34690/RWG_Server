# NPShell
A NetProgramming Implementation of shell environment

## Structure
- For ```main.cpp```, the function keeps getting the new line command.
- The command is wrapped by ```Command``` object, which separate the command by pipe and categorize each functionality.
- The ```Pipe_IO``` contains each small file IO for the small task
- The ```pipe_block``` process the line by forking and repiping.
- ```Pipeline``` is the whole structure for all shell. It has a table that can use maximum capacity to store all the forks in avoidance of handling fork error.

## Usage
1. ```make```
2. ```./NPShell```

## Functionality check
- [x] ```printenv```
- [x] ```setenv```
- [x] ```cat/ ls```
- [x] ```./bin```
- [ ] ```file IO```
- [ ] ```|N```

----
1. child sometimes hanging forever
2. ```|N```: segmentation fault
3. ```file IO```: no output
----
