/**
 * Each line will have at most two commands
 * - one command
 * - two with piping
 * 
 * Line
 * - start with a command
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <parsing.h>


ParsedCmd parseCmd(char* cmd) {
    char* commands[3000] = {NULL};
    int numCommands = 0;
    char* cmdToken = strtok(cmd, " ");
    while (cmdToken != NULL) {
        commands[numCommands] = cmdToken;
        cmdToken = strtok(NULL, " ");
        ++numCommands;
    }

    char** arr = (char**)malloc(sizeof(char*) * (numCommands + 1));
    memcpy(arr, commands, sizeof(char*) * (numCommands + 1));

    int pipeLocation = -1;
    
    int inputRedirectionOne = -1;
    int outputRedirectionOne = -1;
    int errRedirectionOne = -1;

    int inputRedirectionTwo = -1;
    int outputRedirectionTwo = -1;
    int errRedirectionTwo = -1;
    

    for (int i = 0; i < numCommands; ++i) {
        if (strcmp(arr[i], "|") == 0) {
            pipeLocation = i;
        }
        else if (strcmp(arr[i], ">") == 0) {
            if (outputRedirectionOne == -1) {
                outputRedirectionOne = i;
            }
            else if (outputRedirectionTwo == -1) {
                outputRedirectionTwo = i;
            }
        }
        else if (strcmp(arr[i], "<") == 0) {
            if (inputRedirectionOne == -1) {
                inputRedirectionOne = i;
            }
            else if (inputRedirectionTwo == -1) {
                inputRedirectionTwo = i;
            }
        }
        else if (strcmp(arr[i], "2>") == 0) {
            if (errRedirectionOne == -1) {
                errRedirectionOne = i;
            }
            else if (errRedirectionTwo == -1) {
                errRedirectionTwo = i;
            }
        }
    }
    ParsedCmd ck = {
        .arr=arr, .size=numCommands, .pipeLocation=pipeLocation,
        .inputRedirectionOne=inputRedirectionOne, .outputRedirectionOne=outputRedirectionOne, .errRedirectionOne=errRedirectionOne,
        .inputRedirectionTwo=inputRedirectionTwo, .outputRedirectionTwo=outputRedirectionTwo, .errRedirectionTwo=errRedirectionTwo
    };
    return ck;
}



int findEndOfCmd(const ParsedCmd* pcmd, int start, int end) {
    for (int i = start; i < end; ++i) {
        if (strcmp(pcmd->arr[i], "<") == 0 || strcmp(pcmd->arr[i], ">") == 0 || strcmp(pcmd->arr[i], "2>") == 0) {
            return i;
        }
    }
    return end;
}


CommandLine buildCommandLine(const ParsedCmd* pcmd) {
    CommandLine cmdLine = {
        .one = NULL,
        .two = NULL,
        .isBackground = 0
    };

    if (pcmd->size == 0) {
        return cmdLine;
    }

    int cmdSize = pcmd->size;
    if (pcmd->size > 0 && strcmp(pcmd->arr[cmdSize - 1], SEND_BACKGROUND) == 0) {
        cmdLine.isBackground = 1;
        cmdSize--;
    }
    if (pcmd->pipeLocation == -1) {
        // no piping
        cmdLine.one = (Process*)malloc(sizeof(Process));
        cmdLine.one->cmd = NULL;
        cmdLine.one->input = NULL;
        cmdLine.one->output = NULL;
        cmdLine.one->err = NULL;


        int endOfCmd = findEndOfCmd(pcmd, 0, cmdSize);

        cmdLine.one->cmd = (char**)malloc(sizeof(char*) * (endOfCmd + 1));
        memcpy(cmdLine.one->cmd, pcmd->arr, sizeof(char*) * endOfCmd);
        cmdLine.one->cmd[endOfCmd] = NULL;

        if (pcmd->inputRedirectionOne != -1)
            cmdLine.one->input = pcmd->arr[pcmd->inputRedirectionOne + 1];
        if (pcmd->outputRedirectionOne != -1)
            cmdLine.one->output = pcmd->arr[pcmd->outputRedirectionOne + 1];
        if (pcmd->errRedirectionOne != -1)
            cmdLine.one->err = pcmd->arr[pcmd->errRedirectionOne + 1];
    }
    else {
        cmdLine.one = (Process*)malloc(sizeof(Process));
        cmdLine.one->cmd = NULL;
        cmdLine.one->input = NULL;
        cmdLine.one->output = NULL;
        cmdLine.one->err = NULL;

        cmdLine.two = (Process*)malloc(sizeof(Process));
        cmdLine.two->cmd = NULL;
        cmdLine.two->input = NULL;
        cmdLine.two->output = NULL;
        cmdLine.two->err = NULL;


        int endOfCmdOne = find_command_end(pcmd, 0, pcmd->pipeLocation);
        cmdLine.one->cmd = (char**)malloc(sizeof(char*) * (endOfCmdOne + 1));
        memcpy(cmdLine.one->cmd, pcmd->arr, sizeof(char*) * endOfCmdOne);
        cmdLine.one->cmd[endOfCmdOne] = NULL;

        if (pcmd->inputRedirectionOne != -1)
            cmdLine.one->input = pcmd->arr[pcmd->inputRedirectionOne + 1];
        if (pcmd->outputRedirectionOne != -1)
            cmdLine.one->output = pcmd->arr[pcmd->outputRedirectionOne + 1];
        if (pcmd->errRedirectionOne != -1)
            cmdLine.one->err = pcmd->arr[pcmd->errRedirectionOne + 1];

        int startOfCmdTwo = pcmd->pipeLocation + 1;
        int endOfCmdTwo = find_command_end(pcmd, startOfCmdTwo, cmdSize);
        int cmdTwoWidth = endOfCmdTwo - startOfCmdTwo;

        cmdLine.two->cmd = (char**)malloc(sizeof(char*) * (cmdTwoWidth + 1));
        memcpy(cmdLine.two->cmd, &pcmd->arr[startOfCmdTwo], sizeof(char*) * (cmdTwoWidth));
        cmdLine.two->cmd[cmdTwoWidth] = NULL;

        if (pcmd->inputRedirectionTwo != -1)
            cmdLine.two->input = pcmd->arr[pcmd->inputRedirectionTwo + 1];
        if (pcmd->outputRedirectionTwo != -1)
            cmdLine.two->output = pcmd->arr[pcmd->outputRedirectionTwo + 1];
        if (pcmd->errRedirectionTwo != -1)
            cmdLine.two->err = pcmd->arr[pcmd->errRedirectionTwo + 1];
    }

    return cmdLine;
}



void freeParsedCmd(ParsedCmd pcmd) {
    free(pcmd.arr);
}

void freeCommandLineProcesses(CommandLine cl) {
    if (cl.one) {
        free(cl.one->cmd);
        free(cl.one);   
    }
    if (cl.two) {
        free(cl.two->cmd);
        free(cl.two);   
    }
}

void printParsedCmd(ParsedCmd* pc) {
    printf("arr: [");
    for (size_t i = 0; i < pc->size; ++i) {
        if (pc->arr[i] != NULL) {
            printf("\"%s\"", pc->arr[i]);
            if (i < pc->size - 1) {
                printf(", ");
            }
        }
    }
    printf("]\n");

    printf("size: %zu\n", pc->size);
    printf("pipeLocation: %d\n", pc->pipeLocation);
    printf("inputRedirection: [%d, %d]\n", pc->inputRedirectionOne, pc->inputRedirectionTwo);
    printf("outputRedirection: [%d, %d]\n", pc->outputRedirectionOne, pc->outputRedirectionTwo);
    printf("errRedirection: [%d, %d]\n", pc->errRedirectionOne, pc->errRedirectionTwo);
}


void printProcess(Process* p) {
    if (p == NULL) {
        return;
    }

    // Print the command and its arguments
    printf("   Command:   ");
    if (p->cmd) {
        for (int i = 0; p->cmd[i] != NULL; ++i) {
            printf("%s ", p->cmd[i]);
        }
    }
    printf("\n");

    printf("   Input Redirection: %s\n", p->input ? p->input : "(none)");
    printf("   Output Redirection: %s\n", p->output ? p->output : "(none)");
    printf("   Error Redirection: %s\n", p->err ? p->err : "(none)");
}


void printCommandLine(CommandLine* cl) {
    if (cl == NULL) {
        printf("CommandLine is NULL.\n");
        return;
    }
    printf("\n\nCOMMAND LINE");

    if (cl->one) {
        printf("PROCESS ONE\n");
        printProcess(cl->one);
    }
    else {
        printf("Process 1 is empty ?!?!?!!?\n");
    }

    if (cl->two) {
        printf("PROCESS TWO\n");
        printProcess(cl->two);
    }
    else {
        printf("No Process 2\n");
    }
    printf("Is Background Job: %s\n\n", cl->isBackground ? "Yes" : "No");
}