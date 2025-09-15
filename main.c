#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <signal.h>

#include <parsing.h>
#include <commands.h>

const int OPEN_WRITE = O_WRONLY|O_CREAT|O_TRUNC;
const int OPEN_READ = O_RDONLY;
const int OPEN_MODE = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;


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

int debugMode = 0;

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

        if (debugMode) {
            printParsedCmd(&pcmd);
            printf("\n");
        }

        if (pcmd.size == 0) {
            // user pressed 'enter'
            // TODO: do we quit the shell or continue as normal?
            continue;
        }

        
        CommandLine cl = buildCommandLine(&pcmd);
        if (debugMode) {
            printCommandLine(&cl);
        }
        
        // break;
        
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
                    // printf("PIPING\n");
                    
                    int inFdOne = -1;
                    int outFdOne = -1;
                    int errFdOne = -1;
                    if (cl.one->input) {
                        inFdOne = open(cl.one->input, OPEN_READ, OPEN_MODE);
                        if (inFdOne == -1) {
                            perror("Error creating/opening input file!");
                            continue;
                        }
                    }
                    if (cl.one->output) {
                        outFdOne = open(cl.one->output, OPEN_WRITE, OPEN_MODE);
                        if (outFdOne == -1) {
                            perror("Error creating/opening output file!");
                            continue;
                        }
                    }
                    if (cl.one->err) {
                        int errFdOne = open(cl.one->err, OPEN_WRITE, OPEN_MODE);
                        if (errFdOne == -1) {
                            perror("Error creating/opening error file!");
                            continue;
                        }
                    }

                    int inFdTwo = -1;
                    int outFdTwo = -1;
                    int errFdTwo = -1;
                    if (cl.two->input) {
                        inFdTwo = open(cl.two->input, OPEN_READ, OPEN_MODE);
                        if (inFdTwo == -1) {
                            perror("Error creating/opening input file!");
                            continue;
                        }
                    }
                    if (cl.two->output) {
                        outFdTwo = open(cl.two->output, OPEN_WRITE, OPEN_MODE);
                        if (outFdTwo == -1) {
                            perror("Error creating/opening output file!");
                            continue;
                        }
                    }
                    if (cl.two->err) {
                        int errFdTwo = open(cl.two->err, OPEN_WRITE, OPEN_MODE);
                        if (errFdTwo == -1) {
                            perror("Error creating/opening error file!");
                            continue;
                        }
                    }

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
                        // printf("INSIDE ONE\n");
                        // printf("INPUT FILE: |%s|\n", cl.one->input);
                        close(*readEndOfPipe);

                        if (outFdOne != -1)
                            dup2(outFdOne, STDOUT_FD);
                        else
                            dup2(*writeEndOfPipe, STDOUT_FD);
                        
                        if (inFdOne != -1)
                            dup2(inFdOne, STDIN_FD);
                        if (errFdOne != -1)
                            dup2(errFdOne, STDERR_FD);
                        // printf("DONE SETTING UP FDS\n");

                        execvp(cl.one->cmd[0], cl.one->cmd);
                        perror("Error executing process one!");
                        exit(EXIT_FAILURE);
                    }


                    pid_t pidTwo = fork();
                    if (pidTwo == -1) {
                        perror("Error creating fork");
                    }
                    else if (pidTwo == 0) {
                        // printf("INSIDE TWO\n");
                        close(*writeEndOfPipe);
                        
                        if (inFdTwo != -1)
                            dup2(inFdTwo, STDIN_FD);
                        else
                            dup2(*readEndOfPipe, STDIN_FD);

                        if (outFdTwo != -1) {
                            dup2(outFdTwo, STDOUT_FD);
                        }
                        if (errFdTwo != -1) {
                            dup2(errFdTwo, STDERR_FD);
                        }

                        execvp(cl.two->cmd[0], cl.two->cmd);
                        perror("Error executing process two!");
                        exit(EXIT_FAILURE);
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
                        // execvp
                        int inFd = -1;
                        int outFd = -1;
                        int errFd = -1;
                        if (cl.one->input) {
                            inFd = open(cl.one->input, OPEN_READ, OPEN_MODE);
                            if (inFd == -1) {
                                perror("Error creating/opening input file!");
                                continue;
                            }
                        }
                        if (cl.one->output) {
                            outFd = open(cl.one->output, OPEN_WRITE, OPEN_MODE);
                            if (outFd == -1) {
                                perror("Error creating/opening output file!");
                                continue;
                            }
                        }
                        if (cl.one->err) {
                            errFd = open(cl.one->err, OPEN_WRITE, OPEN_MODE);
                            if (errFd == -1) {
                                perror("Error creating/opening error file!");
                                continue;
                            }
                        }

                        pid_t pid = fork();
                        if (pid == -1) {
                            perror("Error creating fork!");
                        }
                        else if (pid == 0) {
                            // child
                            if (inFd != -1) {
                                dup2(inFd, STDIN_FD);
                            }
                            if (outFd != -1) {
                                dup2(outFd, STDOUT_FD);
                            }
                            if (errFd != -1) {
                                dup2(errFd, STDERR_FD);
                            }

                            execvp(cl.one->cmd[0], cl.one->cmd);
                            exit(EXIT_FAILURE);
                        }

                        waitpid(pid, NULL, 0);
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