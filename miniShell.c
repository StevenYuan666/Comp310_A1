#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
//#include <sys/wait.h>
pid_t jobs[150];

// Insert a process id to the jobs list
void insert(pid_t id, pid_t *ids)
{
    for (int i = 0; i < 150; i++)
    {
        if (ids[i] == 0)
        {
            ids[i] = id;
            break;
        }
    }
}
// Remove a process id from the jobs list
void pop(pid_t id, pid_t *ids)
{
    for (int i = 0; i < 150; i++)
    {
        if (ids[i] == id)
        {
            ids[i] = 0;
            break;
        }
    }
}
// Display the jobs id in the jobs list
void job(pid_t *ids)
{
    for (int i = 0; i < 150; i++)
    {
        if (ids[i] != 0)
        {
            printf("%d \n", ids[i]);
        }
    }
}

// Read command from the standard input
int getcmd(char *prompt, char *args[], int *background)
{
    int length, i = 0;
    char *token, *loc;
    char *line = NULL;
    size_t linecap = 0;
    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);
    if (length <= 0)
    {
        exit(-1);
    }
    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL)
    {
        *background = 1;
        *loc = ' ';
    }
    else
        *background = 0;
    while ((token = strsep(&line, " \t\n")) != NULL)
    {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
        {
            args[i++] = token;
        }
    }
    // Add the null terminator in the end
    args[i] = NULL;
    return i;
}

// Define a signal handler to remove a process id when a background process has ended
void sigchld(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) != -1)
    {
        pop(pid, jobs);
        break;
    }
}

// The main function to mimic the behavior of shell
int main(void)
{
    char *args[20];
    int bg;
    pid_t pid;
    // Initialize the value of jobs list
    for (int i = 0; i < 150; i++)
    {
        jobs[i] = 0;
    }
    // The main loop
    while (1)
    {
        // Close the default behavior of control c and control z
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        // Listen to all child processes and detect when child processes finished
        signal(SIGCHLD, sigchld);
        bg = 0;
        int cnt = getcmd(">> ", args, &bg);
        // Handle the empty input
        if (cnt == 0)
        {
            continue;
        }

        //Change directory
        if (strcmp(args[0], "cd") == 0)
        {
            if (args[1] == NULL)
            {
                char cwd[100];
                getcwd(cwd, 100);
                printf("%s\n", cwd);
            }
            else
            {
                chdir(args[1]);
            }
            continue;
        }

        //Exit the shell
        if (strcmp(args[0], "exit") == 0)
        {
            exit(0);
        }

        // Display all running jobs
        if (strcmp(args[0], "jobs") == 0)
        {
            job(jobs);
            continue;
        }

        // foreground a background process
        if (strcmp(args[0], "fg") == 0)
        {
            if (args[1] != NULL)
            {
                int num = atoi(args[1]);
                int count = 0;
                int i;
                for (i = 0; i < 150; i++)
                {
                    if (jobs[i] != 0)
                    {
                        count++;
                    }
                    if (count == num)
                    {
                        break;
                    }
                }
                pid_t id = jobs[i];
                jobs[i] = 0;
                waitpid(id, NULL, 0);
                continue;
            }
            continue;
        }

        // Handle the simpe pipeline
        int j;
        int ifPipe = 0;
        char *command1[20];
        char *command2[20];

        for (j = 0; j < 20; j++)
        {
            if (args[j] != NULL)
            {
                if (strcmp(args[j], "|") == 0)
                {
                    command1[j] = NULL;
                    ifPipe = 1;
                    break;
                }
                command1[j] = args[j];
            }
            else
            {
                break;
            }
        }
        // If have a pipeline, create two child processes and run concurrently
        if (ifPipe)
        {
            int k;
            int count = 0;
            for (k = j + 1; k < 20; k++)
            {
                if (args[k] != NULL)
                {
                    command2[count] = args[k];
                    count++;
                }
                else
                {
                    break;
                }
            }
            command2[count] = NULL;
            int pipes[2];
            if (pipe(pipes) == -1)
            {
                perror("Pipe failed");
                exit(1);
            }
            //first child process
            if (fork() == 0)
            {
                // Restore the default behavior of control c
                signal(SIGINT, SIG_DFL);
                //closing stdout
                close(STDOUT_FILENO);
                //replacing stdout with pipe write
                dup(pipes[1]);
                //closing pipe read
                close(pipes[0]);
                // closing pipe write
                close(pipes[1]);
                // execute the command
                execvp(command1[0], command1);
            }
            //creating 2nd child
            if (fork() == 0)
            {
                // Restore the default behavior of control c
                signal(SIGINT, SIG_DFL);
                //closing stdin
                close(STDIN_FILENO);
                //replacing stdin with pipe read
                dup(pipes[0]);
                //closing pipe write
                close(pipes[1]);
                //closing pipe read
                close(pipes[0]);
                // execute the second command
                execvp(command2[0], command2);
            }
            // closing pipe read and pipe write
            close(pipes[0]);
            close(pipes[1]);
            // wait for the two child processes
            wait(0);
            wait(0);
            continue;
        }

        // Child process
        pid = fork();
        if (pid == 0)
        {
            int processId = (int)getpid();
            signal(SIGINT, SIG_DFL);
            int result;
            char *slice[20];
            int i;
            char *file = NULL;

            // handle the output redirection
            for (i = 0; i < 20; i++)
            {
                if (args[i] != NULL)
                {
                    if (strcmp(args[i], ">") == 0)
                    {
                        file = args[i + 1];
                        break;
                    }
                    slice[i] = args[i];
                }
                else
                {
                    break;
                }
            }
            // Add the null terminator at the end
            slice[i] = NULL;
            if (file != NULL)
            {
                int fd2;
                // Try to open the file
                if ((fd2 = open(file, O_WRONLY | O_CREAT, 0644)) < 0)
                {
                    perror("couldn't open output file.");
                    exit(0);
                }
                // replace the standard output with the file specified
                dup2(fd2, STDOUT_FILENO);
                close(fd2);
            }
            // execute the command
            result = execvp(slice[0], slice);
            if (result == -1)
            {
                printf("Command Not Found\n");
                pop(processId, jobs);
                exit(1);
            }
            pop(processId, jobs);
            exit(0);
        }
        // Parent process
        else if (pid > 0)
        {
            insert(pid, jobs);
            if (bg == 0)
            {
                waitpid(pid, NULL, 0);
                pop(pid, jobs);
            }
        }
        else
        {
            printf("Error during forking");
        }
    }
}