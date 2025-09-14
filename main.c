#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <signal.h>

#include <parsing.h>
#include <commands.h>


enum IO_FDS {
    STDIN_FD = 0,
    STDOUT_FD = 1,
    STDERR_FD = 2
};


typedef struct JobStruct {

} Job;
Job jobs[20];


/**
 * 
 * 
 * File creation: use open with S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
 */

// parent process
int main() {
    // forkLearn();
    // execLearn();
    // printf("DONE\n");
    // execvp is used to find and execute binary executables

    while (1) {
        char* result = readline("# ");

        // types of commands: job-related, piped, normal
        ParsedCmd pcmd = parseCmd(result);
        // printParsedCmd(&pcmd);
        // printf("\n");

        if (pcmd.size == 0) {
            // user pressed 'enter'
            // TODO: do we quit the shell or continue as normal?
            continue;
        }

        
        CommandLine cl = buildCommandLine(&pcmd);
        // printCommandLine(&cl);
        
        /*
        Command processing steps:
        - set stdin, stdout, and stderr
        - fork
        - execvp
        - 
        */
        if (cl.one) {
            if (cl.isBackground) {
                // background task
                
            }
            else {
                if (cl.two) {
                    // piping
                    int pipeFileDescriptors[2];
                    int* readEndOfPipe = &pipeFileDescriptors[0];
                    int* writeEndOfPipe = &pipeFileDescriptors[1];

                    int pipeSuccess = pipe(pipeFileDescriptors);
                    if (pipeSuccess == -1) {
                        perror("Error constructing pipe");
                    }

                    pid_t pidOne = fork();
                    if (pidOne == -1) {
                        perror("Error creating fork");
                    }
                    else if (pidOne == 0) {
                        close(*readEndOfPipe);
                        dup2(*writeEndOfPipe, STDOUT_FD);
                        execvp(cl.one->cmd[0], cl.one->cmd);
                    }

                    pid_t pidTwo = fork();
                    if (pidTwo == -1) {
                        perror("Error creating fork");
                    }
                    else if (pidTwo == 0) {
                        close(*writeEndOfPipe);
                        dup2(*readEndOfPipe, STDIN_FD);
                        execvp(cl.two->cmd[1], cl.two->cmd);
                    }
                    
                    // parent process
                    close(*readEndOfPipe);
                    close(*writeEndOfPipe);
                    waitpid(pidOne, NULL, 0);
                    waitpid(pidTwo, NULL, 0);
                }
                else {
                    char* commandName = cl.one->cmd[0];
                    if (strcmp(commandName, "jobs") == 0) {
                        
                    }
                    else if (strcmp(commandName, "bg") == 0) {
                        
                    }
                    else if (strcmp(commandName, "fg") == 0) {
                        
                    }
                    else {
                        // exec vp
                    }
                }
            }
        }


        freeCommandLineProcesses(cl);
        freeParsedCmd(pcmd);
        free(result);
    }
    return 0;
}