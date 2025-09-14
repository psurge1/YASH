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
        // printParsedCmd(&pcmd);
        // printf("\n");

        if (pcmd.size == 0) {
            // user pressed 'enter'
            // TODO: do we quit the shell or continue as normal?
            continue;
        }

        
        CommandLine cl = buildCommandLine(&pcmd);
        // printCommandLine(&cl);
        
        if (cl.one) {
            if (cl.isBackground) {
                
            }
            else {
                if (cl.two) {
                    // piping
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