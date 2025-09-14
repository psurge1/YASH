const char* PIPE = "|";
const char* SEND_BACKGROUND = "&";

typedef struct parsedCmd {
    char** arr;
    size_t size;
    int pipeLocation;
    int inputRedirectionOne;
    int outputRedirectionOne;
    int errRedirectionOne;
    int inputRedirectionTwo;
    int outputRedirectionTwo;
    int errRedirectionTwo;
} ParsedCmd;


typedef struct processStruct {
    char* input;
    char* output;
    char* err;
    char** cmd;
} Process;

typedef struct commandStruct {
    Process* one;
    Process* two;
    int isBackground;
} CommandLine;



ParsedCmd parseCmd(char* cmd);
int findEndOfCmd(const ParsedCmd* pcmd, int start, int end);
CommandLine buildCommandLine(const ParsedCmd* pcmd);

void freeParsedCmd(ParsedCmd pcmd);
void freeCommandLineProcesses(CommandLine cl);

void printParsedCmd(ParsedCmd* pc);
void printProcess(Process* p);
void printCommandLine(CommandLine* cl);