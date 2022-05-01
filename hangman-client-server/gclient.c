#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_LINE 500

char buffer[MAX_LINE];

void removeNewline(char* line) {
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '\n') {
            line[i] = '\0';
        }
    }
}

void makeFifo(char* fifoName) {
    sprintf(fifoName, "/tmp/%s-%d", getenv("USER"), getpid());
    mkfifo(fifoName, 0600);
    chmod(fifoName, 0622);
}

void checkFifo(FILE* fifo, char* fifoName) {
    if (!fifo) {
        printf("FIFO %s cannot be opened.\n", fifoName);
        exit(2);
    }
}

void writeToGServerFifo(char* gameName, char c) {
    FILE* gameFp = fopen(gameName, "w");
    checkFifo(gameFp, gameName);
    
    fprintf(gameFp, "%c\n", c);

    fclose(gameFp);
}

int readFromClientFifo(char* clientName) {
    FILE* clientFp = fopen(clientName, "r");
    checkFifo(clientFp, clientName);

    int gameOver = 0;
    while (fgets(buffer, MAX_LINE, clientFp) != NULL) {
        if (strstr(buffer, "Game Over") != NULL) {
            gameOver = 1;
        }
        printf(buffer);
    }

    fclose(clientFp);
    
    return gameOver;
}

void hangmanClient(char* clientName, char* gameName) {
    // wait for prompt from game server and output it
    readFromClientFifo(clientName);
        
    while (1) {
        fgets(buffer, MAX_LINE, stdin);
        
        if (strlen(buffer) < 1) {
            continue;
        }

        char c = buffer[0];
        
        // write guess to game server
        writeToGServerFifo(gameName, c);
        
        // wait for response from game server and output it
        int gameOver = readFromClientFifo(clientName);
        
        if (gameOver) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: gclient <server-fifo-name>");
        exit(1);
    }

    // open the server fifo for writing
    char* serverName = argv[1];
    FILE* serverFp = fopen(serverName, "w");
    checkFifo(serverFp, serverName);

    char clientName[MAX_LINE];
    makeFifo(clientName);

    // write a request to the server to start a hangman game by sending the client fifo
    fprintf(serverFp, "%s\n", clientName);
    fclose(serverFp);

    // open the client fifo for reading from the server
    FILE* clientFp = fopen(clientName, "r");
    checkFifo(clientFp, clientName);

    // process for a response from the server containing the name of the game fifo
    char gameName[MAX_LINE];
    if (fgets(gameName, MAX_LINE, clientFp) != NULL) {
        removeNewline(gameName);
    } else {
        printf("Couldn't get game server fifo from server.\n");
    }

    fclose(clientFp);

    printf("Connected to a game of hangman on fifo %s\n\n", gameName);

    hangmanClient(clientName, gameName);
    
    unlink(clientName);
}


