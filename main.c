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
                    // printf("PIPING\n");
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
                        dup2(*writeEndOfPipe, STDOUT_FD);

                        if (cl.one->input) {
                            int inFd = open(cl.one->input, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                            if (inFd == -1) {
                                perror("Error creating/opening input file!");
                                exit(EXIT_FAILURE);
                            }
                            dup2(inFd, STDIN_FD);
                        }
                        if (cl.one->err) {
                            int errFd = open(cl.one->err, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                            if (errFd == -1) {
                                perror("Error creating/opening error file!");
                                exit(EXIT_FAILURE);
                            }
                            dup2(errFd, STDERR_FD);
                        }
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
                        dup2(*readEndOfPipe, STDIN_FD);

                        if (cl.two->output) {
                            int outFd = open(cl.two->output, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                            if (outFd == -1) {
                                perror("Error creating/opening output file!");
                                exit(EXIT_FAILURE);
                            }
                            dup2(outFd, STDOUT_FD);
                        }
                        if (cl.two->err) {
                            int errFd = open(cl.two->err, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                            if (errFd == -1) {
                                perror("Error creating/opening error file!");
                                exit(EXIT_FAILURE);
                            }
                            dup2(errFd, STDERR_FD);
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
                        pid_t pid = fork();
                        if (pid == -1) {
                            perror("Error creating fork!");
                        }
                        else if (pid == 0) {
                            // child
                            if (cl.one->input) {
                                int inFd = open(cl.one->input, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                                if (inFd == -1) {
                                    perror("Error creating/opening input file!");
                                    exit(EXIT_FAILURE);
                                }
                                dup2(inFd, STDIN_FD);
                            }
                            if (cl.one->output) {
                                int outFd = open(cl.two->output, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                                if (outFd == -1) {
                                    perror("Error creating/opening output file!");
                                    exit(EXIT_FAILURE);
                                }
                                dup2(outFd, STDIN_FD);
                            }
                            if (cl.one->err) {
                                int errFd = open(cl.two->err, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                                if (errFd == -1) {
                                    perror("Error creating/opening error file!");
                                    exit(EXIT_FAILURE);
                                }
                                dup2(errFd, STDIN_FD);
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