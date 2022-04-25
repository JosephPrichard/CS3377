#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_WORDS 100000
#define MAX_LINE 500

char *words[MAX_WORDS];
int numWords = 0;

int random(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

void removeNewline(char* line) {
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '\n') {
            line[i] = '\0';
        }
    }
}

void loadWords(FILE* file) {
    char line[MAX_LINE];

    while(fgets(line, MAX_LINE, file) != NULL && numWords < MAX_WORDS) {
        words[numWords] = malloc(strlen(line) + 1);
        strcpy(words[numWords], line);
        removeNewline(words[numWords]);
        numWords++;
    }
}

void cleanUpWords() {
    for (int i = 0; i < numWords; i++) {
        free(words[i]);
    }
}

char* getRandomWord() {
    int r = random(0, numWords - 1);
    fprintf(stderr, "Generated random word: %s\n", words[r]);
    return words[r];
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

void writePromptToClientFifo(char* clientName, char* wordGuess) {
    FILE* clientFp = fopen(clientName, "w");
    checkFifo(clientFp, clientName);

    fprintf(clientFp, "(Guess) Enter a letter in word %s > \n", wordGuess);

    fclose(clientFp);
}

// sends a response and prompts for the next prompt
void writeResponseToClientFifo(char* clientName, char* response, char* wordGuess, int prompt) {
    FILE* clientFp = fopen(clientName, "w");
    checkFifo(clientFp, clientName);

    fputs(response, clientFp);
    if (prompt) {
        fprintf(clientFp, "(Guess) Enter a letter in word %s > \n", wordGuess);
    }

    fclose(clientFp);
}

int countUniqChar(char* word, int wordLen) {
    int freq[256] = {};
    
    int uniqCount = 0;
    
    for (int i = 0; i < wordLen; i++) {
        char c = word[i];
        if (!freq[c]) {
            freq[c] = 1;
            uniqCount++;
        }
    }
    return uniqCount;
}

// this function represents a hangman game server to be started inside a child process
void hangmanServer(char* word, char* clientName, char* gameName) {
    int wordLen = (int) strlen(word);
    int uniqCharCount = countUniqChar(word, wordLen);

    // the user has wordLen # of guesses
    int guessCount = 0;
    int missCount = 0;

    char* wordGuess = malloc(wordLen + 1);
    for (int i = 0; i < wordLen; i++) {
        wordGuess[i] = '*';
    }
    wordGuess[wordLen] = '\0';

    // each lineGuess is a hangman game letter wordGuess from the client
    char lineGuess[MAX_LINE];
    
    // send a prompt to the client to guess
    writePromptToClientFifo(clientName, wordGuess);

    while(1) {
        fprintf(stderr, "Word guess: %s\n", wordGuess);

        // wait for the guess
        FILE* gameFp = fopen(gameName, "r");
        checkFifo(gameFp, gameName);

        char response[MAX_LINE];

        // process the first line as the guess
        if (fgets(lineGuess, MAX_LINE, gameFp) != NULL) {
            removeNewline(lineGuess);

            char guess = lineGuess[0];
            sprintf(response, "\t%c is in the word\n", guess);

            // check if the character is already guessed
            char* found = strchr(wordGuess, guess);
            if (found != NULL) {
                // character was already guessed
                sprintf(response, "\t%c is already in the word\n", guess);
            } else {
                // character hasn't been guessed

                // check if the character is in the word
                int inWord = 0;
                for (int i = 0; i < wordLen; i++) {
                    if (word[i] == guess) {
                        wordGuess[i] = guess;
                        inWord = 1;
                    }
                }

                if (!inWord) {
                    missCount++;
                    sprintf(response, "\t%c is not in the word\n", guess);
                }

                guessCount++;
            }
        }

        fclose(gameFp);

        int isGameOver = 0;

        // check if the game is over, if so add additional response
        if (uniqCharCount == guessCount) {
            isGameOver = 1;
            sprintf(response, "%sGame Over. The word is %s. You missed %d time.\n", response, word, missCount);
        }

        // send a response to client
        writeResponseToClientFifo(clientName, response, wordGuess, !isGameOver);

        if (isGameOver) {
            break;
        }
    }
}

int main() {

    srand(getpid() + time(NULL) + getuid());
    
    FILE* dictionaryFp = fopen("dictionary.txt", "r");
    loadWords(dictionaryFp);
    fclose(dictionaryFp);

    //create a named pipe to read client's requests
    char serverName[MAX_LINE];
    makeFifo(serverName);
    printf("Send requests to start a hangman game to %s\n\n", serverName);

    char clientName[MAX_LINE];

    // starts a server loop to read requests from the server until termination
    while(1) {
        // fifo where clients make requests to start a hangman game
        FILE* serverFp = fopen(serverName, "r");
        checkFifo(serverFp, serverName);

        printf("Opened server fifo %s for reading.\n", serverName);

        // each clientName is a request from a client to start a hangman game
        while (fgets(clientName, MAX_LINE, serverFp) != NULL) {
            removeNewline(clientName);

            printf("Received hangman game request.\n");
            
            char* word = getRandomWord();

            //create a child to handle hangman game
            if (fork() == 0) {
                // the client fifo for the server to send responses to
                FILE* clientFp = fopen(clientName, "w");

                if (clientFp) {
                    char gameName[MAX_LINE];
                    makeFifo(gameName);

                    // send the game fifo to the client, so it can talk to the hangmanClient game
                    fprintf(clientFp, "%s\n", gameName);
                    fclose(clientFp);

                    fprintf(stderr, "Started a game of hangman on fifo %s\n", gameName);

                    hangmanServer(word, clientName, gameName);
                    
                    unlink(gameName);
                } else {
                    fprintf(stderr, "FIFO %s couldn't be opened.\n", clientName);
                }
            }
        }

        fclose(serverFp);
    }

    cleanUpWords();
    
    unlink(serverName);
}



