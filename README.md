# Simple Command Line Shell
## Overall Design
A C program that provides the basic operations of a command line shell. This program is composed of two functions: main() and getcmd(). The getcmd() function reads in the user’s next command, and then parses it into separate tokens that are used to fill the argument vector for the command to be executed. If the command is to be run in the background, it will end with ‘&’, and getcmd() will update the background parameter so the main() function can act accordingly. The program terminates when the user enters <Control><D> because getcmd() invokes exit().
  
The main() calls getcmd(), which waits for the user to enter a command. The contents of the command entered by the user are loaded into the args array. For example, if the user enters ls –l at the command prompt, args[0] is set equal to the string “ls” and args[1] is set to the string to “– l”. (By “string,” we mean a null-terminated C-style string variable.)
  
The function of main() is organized into three parts:
 
  (1) creating the child process and executing the command in the child
  
  (2) signal handling feature
  
  (3) additional features.

In the main() function, upon returning from getcmd(), a child process is forked and it executes the command specified by the user.
  
A signal handling feature is also provided.
  
  (a) kill a program running inside the shell when Ctrl+C (SIGINT) is pressed in the keyboard
  
  (b) ignore the Ctrl+Z (SIGTSTP) signal. That is for SIGINT you kill the process that is running within the shell. 
  
Supported built-in commands:
  
  cd: change the working directory.
  
  pwd: display the current working directory.
  
  exit: terminate the current running program.
  
  fg: bring the background job to the foreground.
  
  jobs: list all running programs
  
  *simple output redirection*: ls > out.txt
  
  *simple command piping*: ls | wc -l
  
