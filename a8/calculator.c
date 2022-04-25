#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE_LEN 1000
#define MAX_OPERATORS 100

const char* OPERATORS = "+-*/";

int fds[2 * MAX_OPERATORS + 1][2];
int operatorCount = 0;
int numPipes = 0;
int debug = 0;

// executes the right executable for a given operator
int execute(short op) {
    switch (op) {
        case '+': {
            if (debug) {
                execl("./add", "add", "-v", NULL);
            } else {
                execl("./add", "add", NULL);
            }
            break;
        }
        case 0x2013:
        case '-': {
            if (debug) {
                execl("./subtract", "subtract", "-v", NULL);
            } else {
                execl("./subtract", "subtract", NULL);
            }
            break;
        }
        case '*': {
            if (debug) {
                execl("./multiply", "multiply", "-v", NULL);
            } else {
                execl("./multiply", "multiply", NULL);
            }
            break;
        }
        case '/': {
            if (debug) {
                execl("./divide", "divide", "-v", NULL);
            } else {
                execl("./divide", "divide", NULL);
            }
            break;
        }
        default: {
            fprintf(stderr, "Unknown operator: %d", op);
            exit(1);
        }
    }
}

void createPipes() {
    // create pipes in the fds array
    for (int i = 0; i < numPipes; i++) {
        pipe(fds[i]);
    }
}

void closePipes() {
    // close pipes to get rid of zombie processes
    for (int i = 0; i < numPipes; i++) {
        close(fds[i][0]);
        close(fds[i][1]);
    }
}

void setupChild(char op, int index) {
    close(0);
    if(index == 0) {
        dup(fds[0][0]);
    } else {
        dup(fds[operatorCount + index][0]);
    }
    
    close(3);
    dup(fds[index + 1][0]);
    
    close(1);
    dup(fds[operatorCount + index + 1][1]);
    
    closePipes();  
    
    // execute the right executable depending on the operator
    execute(op);
    fprintf(stderr, "Failed to execute, I shouldn't be here!\n");
    exit(1);
}

// sets up the pipe structure to compute the formula
void setupPipes(const char* operators) {
    // setup the pipes such that output goes to the next input
    for (int i = 0; i < operatorCount; i++) {
        if (fork() == 0) {
            char op = operators[i];
            setupChild(op, i); 
        }
    }

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: calculator <file_path>\n");
        exit(1);
    }
    
    if (argc > 2) {
        // debug flag
        debug = 1;
    }

    char line[MAX_LINE_LEN];
    char* token;

    FILE* file = fopen(argv[1], "r");

    // store the operators in our array
    char operators[100];

    // first we extract the operators from the first line and put them in an array
    fgets(line, MAX_LINE_LEN, file);

    // iterate through each token split by space, add the tokens to the array
    operatorCount = 0;
    token = strtok(line, " ");
    while (token != NULL) {
        if (strchr(OPERATORS, *token) != NULL) {
            operators[operatorCount] = *token;
            operatorCount++;
        }
        token = strtok(NULL, " ");
    }
    
    if(operatorCount > MAX_OPERATORS) {
        printf("Formula may not contain more than %d operators\n", MAX_OPERATORS);
        exit(1);
    }
    
    numPipes =  2 * operatorCount + 1;
    createPipes();
    
    // setup the pipe structure for the formula
    setupPipes(operators);

    // iterate through each line, and do the computation for each line
    while (fgets(line, MAX_LINE_LEN, file) != NULL) {
        int operands[MAX_OPERATORS + 1];
        
        // parse the line and add all the operands to an array
        int operandCount = 0;
        token = strtok(line, " ");
        while(token != NULL && operandCount < operatorCount + 1) {
            int number = atoi(token);
            operands[operandCount] = number;
            token = strtok(NULL, " ");
            operandCount++;
        }
        
        if (operandCount <= 1) {
            break;
        }
        
        write(fds[0][1], &operands[0], sizeof(int));
        
        for (int i = 1; i < operandCount; i++) {
            write(fds[i][1], &operands[i], sizeof(int));
        }
        
        int result;
        read(fds[2 * operatorCount][0], &result, sizeof(int));
        printf("%d\n", result);
    }
    
    closePipes();
}

