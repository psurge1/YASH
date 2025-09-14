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





typedef struct jobStruct {

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
        printParsedCmd(&pcmd);
        printf("\n");

        if (pcmd.size == 0) {
            // user pressed 'enter'
            // TODO: do we quit the shell or continue as normal?
            continue;
        }

        
        CommandLine cl = buildCommandLine(&pcmd);
        printCommandLine(&cl);
        
        if (pcmd.pipeLocation != -1) {
            // pipe command, will not be background process
            Process pLeft;
            Process pRight;
        }
        else {

            if (strcmp(pcmd.arr[pcmd.size - 1], SEND_BACKGROUND) == 0) {
                // background job
            }
            else {
                // normal job
                char* firstCmd = pcmd.arr[0];
                // TODO: Do we need to execute jobs commands if we have unecessary args?
                if (strcmp(firstCmd, "jobs") == 0) {
                    
                }
                else if (strcmp(firstCmd, "fg") == 0) {

                }   
                else if (strcmp(firstCmd, "bg") == 0) {

                }
                else {
                    // execvp used here
                    pid_t pid = fork();
                    if (pid == -1) {
                        perror("Fork failed!");
                    }
                    else if (pid == 0) {
                        // in child process
                        int inputFileIndex = pcmd.inputRedirectionOne;
                        int outputFileIndex = pcmd.outputRedirectionOne;
                        int errFileIndex = pcmd.errRedirectionOne;
                        int endOfCmd = minIndex(inputFileIndex, outputFileIndex, errFileIndex);

                        
                        if (endOfCmd == -1) {
                            int statusCode = execvp(firstCmd, pcmd.arr);
                        }
                        else {
                            char** command = (char**)malloc(sizeof(char*) * (endOfCmd + 1));
                            memcpy(command, pcmd.arr, sizeof(char**) * endOfCmd);
                            command[endOfCmd] = NULL;
                            
                            

                            int statusCode = execvp(firstCmd, command);

                            free(command);
                        }
                        _exit(EXIT_SUCCESS);
                    }
                    else {
                        // wait in parent process
                        int status;
                        waitpid(pid, &status, 0);
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