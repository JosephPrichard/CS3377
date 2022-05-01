#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_QUOTES 500
#define MAX_COMMANDS 100
#define MAX_ARGS 20
#define MAX_LINE 1000

#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1

char *quotes[MAX_QUOTES];
int quoteLen = 0;

int fds[MAX_COMMANDS][2];

int random(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

void removeNewline(char* line) {
    for (int i = 0; line[i] != 0; i++) {
        if (line[i] == '\n') {
            line[i] = 0;
        }
    }
}

void loadQuotes(FILE* file) {
    char line[MAX_LINE];

    while(fgets(line, MAX_LINE, file) != NULL && quoteLen < MAX_QUOTES) {
        quotes[quoteLen] = malloc(strlen(line) + 1);
        strcpy(quotes[quoteLen], line);
        quoteLen++;
    }
}

void cleanUpQuotes() {
    for(int i = 0; i < quoteLen; i++) {
        free(quotes[i]);
    }
}

void printQuote() {
    int r = random(0, quoteLen - 1);
    fputs(quotes[r], stdout);
}

void createPipes(int numPipes) {
    for (int i = 0; i < numPipes; i++) {
        pipe(fds[i]);
    }
}

void closePipes(int numPipes) {
    for (int i = 0; i < numPipes; i++) {
        close(fds[i][0]);
        close(fds[i][1]);
    }
}

int tokDoublePipe(char* line, char** commands) {
    int numCommands = 0;
    char* token = strtok(line, "=");
    while (token != NULL && numCommands < 2) {
        commands[numCommands++] = token;
        token = strtok(NULL, "=");
    }
    return numCommands;
}

int tokCommands(char* line, char** commands) {
    int numCommands = 0;
    char* token = strtok(line, "|");
    while(token != NULL && numCommands < MAX_COMMANDS) {
        commands[numCommands++] = token;
        token = strtok(NULL, "|");
    }
    return numCommands;
}

int tokArgs(char* command, char** argv) {
    int numArgs = 0;
    char* token = strtok(command, " ");
    while(token != NULL && numArgs < MAX_ARGS) {
        if (strcmp(token, "") != 0) {
            argv[numArgs++] = token;
            token = strtok(NULL, " ");
        }
    }
    argv[numArgs] = NULL;
    return numArgs;
}

void pipeChainChild(int index, char** argv, int numCommands, int numPipes) {
    // only connect the pipes if there is at least one command
    if (numCommands > 1) {
        // connect the pipes to each other
        if (index == 0) {
            // first process in pipeline
            dup2(fds[index][WRITE], STDOUT);
        } else if (index == numCommands - 1) {
            // last process in pipeline
            dup2(fds[index-1][READ], STDIN);
        } else {
            // middle process in pipeline
            dup2(fds[index][WRITE], STDOUT);
            dup2(fds[index-1][READ], STDIN);
        }
    }

    // close pipes to prevent zombie processes
    closePipes(numPipes);

    execvp(argv[0], argv);
    exit(1);
}

void doublePipeChild1(char** argv) {
    dup2(fds[0][WRITE], STDOUT);
    dup2(fds[1][READ], STDIN);
    
    // close pipes to prevent zombie processes
    closePipes(2);

    execvp(argv[0], argv);
    exit(1);
}

void doublePipeChild2(char** argv) {
    dup2(fds[0][READ], STDIN);
    dup2(fds[1][WRITE], STDOUT);
    
    // close pipes to prevent zombie processes
    closePipes(2);

    execvp(argv[0], argv);
    exit(1);
}

int main() {
    srand(getpid() + time(NULL));

    FILE* file = fopen("quotes.txt", "r");
    loadQuotes(file);
    fclose(file);

    char line[MAX_LINE];

    int pid, wpid, status;

    printQuote();
    while(fgets(line, MAX_LINE, stdin) != NULL) {
        removeNewline(line);

        char* commands[MAX_COMMANDS];

        // determine if the line uses a double pipe or not
        int numCommands = tokDoublePipe(line, commands);

        if (numCommands == 2) {
            // double pipe
            
            // create a pipe for each command
            createPipes(2);
            
            char* command1 = commands[0];
            char* command2 = commands[1];
            
            char* argv[MAX_ARGS];

            // tokenize command1 and its flags
            int numArgs = tokArgs(command1, argv);
            
            pid = fork();
            if(pid == 0) {
                doublePipeChild1(argv);
            }
            
            // tokenize command2 and its flags
            numArgs = tokArgs(command2, argv);
            
            pid = fork();
            if(pid == 0) {
                doublePipeChild2(argv);
            }
            
            closePipes(2);
        } else {
            // pipe chain

            // tokenize the commands and add them to our command list
            numCommands = tokCommands(line, commands);

            int numPipes = numCommands - 1;

            // create a pipe for each command
            createPipes(numPipes);

            // fork a process for each command
            for (int i = 0; i < numCommands; i++) {
                char* command = commands[i];

                char* argv[MAX_ARGS];

                // tokenize the command and its flags
                int numArgs = tokArgs(command, argv);

                pid = fork();
                if (pid == 0) {
                    // child process
                    pipeChainChild(i, argv, numCommands, numPipes);
                }
            }

            closePipes(numPipes);
        }
        
        while ((wpid = wait(&status)) > 0);

        printf("\n");
        printQuote();
    }
    
    cleanUpQuotes();
}


