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

const int OPEN_WRITE = O_WRONLY|O_CREAT|O_TRUNC;
const int OPEN_READ = O_RDONLY;
const int OPEN_MODE = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;


enum IO_FDS {
    STDIN_FD = 0,
    STDOUT_FD = 1,
    STDERR_FD = 2
};

typedef struct JobStruct {
    char* cmd;
    int jobId;
    pid_t jobPid;
    int status; // 0 for running, 1 for stopped, 2 for done
    struct JobStruct* nextJob;
    struct JobStruct* previousJob;
} Job;


Job* head;
Job* tail;



void freeJobsLinkedList(Job* head) {
    if (!head)
        return;
    freeJobsLinkedList(head->nextJob);
    free(head->cmd);
    free(head);
}

void addJob(Job* job) {
    Job* jbHead = head;
    if (!jbHead)
        return;
    while(jbHead->nextJob) {
        jbHead = jbHead->nextJob;
    }
    jbHead->nextJob = job;
    job->previousJob = jbHead;
    job->nextJob = NULL;
    tail = job;
}

void closeOperation(CommandLine* cl, ParsedCmd* pcmd, char* cmdString) {
    if (cl)
        freeCommandLineProcesses(*cl);
    if (pcmd)
        freeParsedCmd(*pcmd);
    if (cmdString)
        free(cmdString);
}




int debugMode = 0;

void checkAndReapJobs() {
    Job* current = head->nextJob;
    Job* prev = head;

    while (current != NULL) {
        if (current->status == 2) {
            printf("[%d]+ Done\t\t%s\n", current->jobId, current->cmd);
            prev->nextJob = current->nextJob;
            if (current->nextJob) {
                current->nextJob->previousJob = prev;
            } else {
                tail = prev;
            }

            Job* to_free = current;
            current = current->nextJob;

            free(to_free->cmd);
            free(to_free);
        } else {
            prev = current;
            current = current->nextJob;
        }
    }
}

void sigStpHandler(int signal) {
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WUNTRACED))) {

    }
}

void sigChildHandler(int signal) {
    fprintf(stderr, "!!! SIGCHLD HANDLER FIRED !!!\n"); 

    int status;
    pid_t pid;
    

    // Use WNOHANG | WUNTRACED to reap any child that has stopped or terminated.
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Job* jb = head; // Assume 'head' is your global job list head
        while (jb) {
            if (jb->jobPid == pid) {
                if (WIFSTOPPED(status)) {
                    // Job was stopped by a signal (Ctrl-Z)
                    jb->status = 1; // 1 for stopped
                } else {
                    // Job was terminated (normally or by a signal)
                    jb->status = 2; // 2 for done
                }
                break;
            }
            jb = jb->nextJob;
        }
    }
}

// parent process
int main() {
    // forkLearn();
    // execLearn();
    // printf("DONE\n");
    // execvp is used to find and execute binary executables
    // signal(SIGTSTP, sigStpHandler);
    signal(SIGCHLD, sigChildHandler);

    // signal(SIGINT, SIG_IGN);
    // signal(SIGTSTP, SIG_IGN);
    // signal(SIGTTOU, SIG_IGN);
    // signal(SIGCHLD, SIG_IGN);

    // linked list for jobs
    head = (Job*)malloc(sizeof(Job));
    head->cmd = NULL;
    head->jobId = -1;
    head->jobPid = -1;
    head->status = -1;
    head->nextJob = NULL;
    head->previousJob = NULL;
    tail = head->nextJob;

    int jobIdCounter = 0;


    while (1) {
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        checkAndReapJobs();

        char* result = readline("# ");

        // types of commands: job-related, piped, normal
        if (result == NULL){
            // end of file only (ctrl-d)
            closeOperation(NULL, NULL, NULL);
            break;
        }

        if (result[0] == '\0') {
            // blank enter
            closeOperation(NULL, NULL, result);
            continue;
        }
        // printf("%p, |%s|\n", (void*)result, result);
        char* resultDuplicate = (char*)malloc(sizeof(char) * strlen(result) + 1);
        strcpy(resultDuplicate, result);
        
        ParsedCmd pcmd = parseCmd(result);

        if (debugMode) {
            printParsedCmd(&pcmd);
            printf("\n");
        }

        if (pcmd.size == 0) {
            // equivalent to ctrl-d
            closeOperation(NULL, &pcmd, result);
            free(resultDuplicate);
            break;
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
                int inFdOne = -1;
                int outFdOne = -1;
                int errFdOne = -1;
                if (cl.one->input) {
                    inFdOne = open(cl.one->input, OPEN_READ, OPEN_MODE);
                    if (inFdOne == -1) {
                        perror("Error creating/opening input file!");
                        closeOperation(&cl, &pcmd, result);
                        free(resultDuplicate);
                        continue;
                    }
                }
                if (cl.one->output) {
                    outFdOne = open(cl.one->output, OPEN_WRITE, OPEN_MODE);
                    if (outFdOne == -1) {
                        perror("Error creating/opening output file!");
                        closeOperation(&cl, &pcmd, result);
                        free(resultDuplicate);
                        continue;
                    }
                }
                if (cl.one->err) {
                    int errFdOne = open(cl.one->err, OPEN_WRITE, OPEN_MODE);
                    if (errFdOne == -1) {
                        perror("Error creating/opening error file!");
                        closeOperation(&cl, &pcmd, result);
                        free(resultDuplicate);
                        continue;
                    }
                }

                Job* newJob = (Job*)malloc(sizeof(Job));
                newJob->jobId = ++jobIdCounter;
                newJob->status = 0;
                newJob->cmd = resultDuplicate;
                addJob(newJob);

                pid_t pid = fork();
                if (pid == -1) {
                    perror("Error creating fork");
                }
                else if (pid == 0) {
                    signal(SIGINT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    if (outFdOne != -1)
                        dup2(outFdOne, STDOUT_FD);
                    if (inFdOne != -1)
                        dup2(inFdOne, STDIN_FD);
                    if (errFdOne != -1)
                        dup2(errFdOne, STDERR_FD);
                    
                    execvp(cl.one->cmd[0], cl.one->cmd);
                    perror("Error executing process one!");
                    exit(EXIT_FAILURE);
                }
                else {
                    newJob->jobPid = pid;
                }
            }
            else {
                free(resultDuplicate);
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
                            closeOperation(&cl, &pcmd, result);
                            continue;
                        }
                    }
                    if (cl.one->output) {
                        outFdOne = open(cl.one->output, OPEN_WRITE, OPEN_MODE);
                        if (outFdOne == -1) {
                            perror("Error creating/opening output file!");
                            closeOperation(&cl, &pcmd, result);
                            continue;
                        }
                    }
                    if (cl.one->err) {
                        int errFdOne = open(cl.one->err, OPEN_WRITE, OPEN_MODE);
                        if (errFdOne == -1) {
                            perror("Error creating/opening error file!");
                            closeOperation(&cl, &pcmd, result);
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
                            closeOperation(&cl, &pcmd, result);
                            continue;
                        }
                    }
                    if (cl.two->output) {
                        outFdTwo = open(cl.two->output, OPEN_WRITE, OPEN_MODE);
                        if (outFdTwo == -1) {
                            perror("Error creating/opening output file!");
                            closeOperation(&cl, &pcmd, result);
                            continue;
                        }
                    }
                    if (cl.two->err) {
                        int errFdTwo = open(cl.two->err, OPEN_WRITE, OPEN_MODE);
                        if (errFdTwo == -1) {
                            perror("Error creating/opening error file!");
                            closeOperation(&cl, &pcmd, result);
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
                        setpgid(0, 0);
                        signal(SIGINT, SIG_DFL);
                        signal(SIGTSTP, SIG_DFL);
                        
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
                        setpgid(0, pidOne);
                        signal(SIGINT, SIG_DFL);
                        signal(SIGTSTP, SIG_DFL);
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
                        // jobs will print the job control table similar to bash.
                        // HOWEVER there are important differences between your yash shell's output and bash's output for the jobs command!
                        // Please see the FAQ.
                        Job* jb = head->nextJob;
                        while (jb && jb->nextJob) {
                            char* successString = '\0';
                            switch(jb->status) {
                                case 0: successString = "Running";
                                break;
                                
                                case 1: successString = "Stopped";
                                break;

                                case 2: successString = "Done";
                                break;
                            }
                            printf("[%d]- %s      %s\n", jb->jobId, successString, jb->cmd);
                            jb = jb->nextJob;
                        }
                        if (jb) {
                            char* successString = '\0';
                            switch(jb->status) {
                                case 0: successString = "Running";
                                break;
                                
                                case 1: successString = "Stopped";
                                break;

                                case 2: successString = "Done";
                                break;
                            }
                            printf("[%d]+ %s      %s\n", jb->jobId, successString, jb->cmd);
                        }
                    }
                    else if (strcmp(commandName, "bg") == 0) {
                        // bg must send SIGCONT to the most recent stopped process,
                        // print the process to stdout in the jobs format,
                        // and not wait for completion (as if &)
                    }
                    else if (strcmp(commandName, "fg") == 0) {
                        // fg must send SIGCONT to the most recent background or stopped process,
                        // print the process to stdout,
                        // and wait for completion
                        Job* jb = head->nextJob;
                        while (jb && jb->nextJob) {
                            jb = jb->nextJob;
                        }
                        if (jb) {
                            // bring the process to foreground
                        }
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
                                closeOperation(&cl, &pcmd, result);
                                continue;
                            }
                        }
                        if (cl.one->output) {
                            outFd = open(cl.one->output, OPEN_WRITE, OPEN_MODE);
                            if (outFd == -1) {
                                perror("Error creating/opening output file!");
                                closeOperation(&cl, &pcmd, result);
                                continue;
                            }
                        }
                        if (cl.one->err) {
                            errFd = open(cl.one->err, OPEN_WRITE, OPEN_MODE);
                            if (errFd == -1) {
                                perror("Error creating/opening error file!");
                                closeOperation(&cl, &pcmd, result);
                                continue;
                            }
                        }

                        pid_t pid = fork();
                        if (pid == -1) {
                            perror("Error creating fork!");
                        }
                        else if (pid == 0) {
                            signal(SIGINT, SIG_DFL);
                            signal(SIGTSTP, SIG_DFL);
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
                        else {
                            waitpid(pid, NULL, 0);
                        }
                    }
                }
            }
        }


        
        closeOperation(&cl, &pcmd, result);
    }

    freeJobsLinkedList(head);

    return 0;
}