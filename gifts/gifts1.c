#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_NAME "gifts1.dat"
#define MAX_NAME_LEN 100

// a name is a string with at most 100 characters
// note this is the string itself and not a pointer to one
typedef char Name[MAX_NAME_LEN];

// names and amounts are pointers to parallel arrays of equal lengths
struct BalanceEntries {
    Name* names;
    float* amounts;
    unsigned int len;
};

// opens a file, reads the lengths and array contents and returns balance entries
struct BalanceEntries freadBalances() {
    int file = open(FILE_NAME, O_RDONLY);

    struct BalanceEntries balances;

    // read arrays length from file
    read(file, &balances.len, sizeof(unsigned int));

    // allocate arrays with the right length
    balances.names = calloc(balances.len, sizeof(Name));
    balances.amounts = calloc(balances.len, sizeof(float));

    // read array contents from file and put them in the arrays we just allocated
    read(file, balances.amounts, sizeof(float) * balances.len);
    read(file, balances.names, sizeof(Name) * balances.len);

    close(file);
    return balances;
}

// opens a file, and writes to it the length and array contents from the balance entries
void fwriteBalances(struct BalanceEntries balances) {
    int file = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC);

    // write the array length into file
    write(file, &balances.len, sizeof(unsigned int));

    // write array contents into file
    write(file, balances.amounts, sizeof(float) * balances.len);
    write(file, balances.names, sizeof(Name) * balances.len);

    close(file);
}

// iterate through the parallel arrays and print each element
void printBalances(struct BalanceEntries balances) {
    for (int i = 0; i < balances.len; i++) {
        printf("%10s: %5.2lf\n", balances.names[i], balances.amounts[i]);
    }
}

// finds the index of an array element by corresponding name
int findEntryIndexByName(struct BalanceEntries balances, char* name) {
    for (int i = 0; i < balances.len; i++) {
        if (strcmp(balances.names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// frees the contents of the balance entries
void freeBalances(struct BalanceEntries balances) {
    free(balances.names);
    free(balances.amounts);
}

int main(int argc, char* argv[]) {
    // detect if we are going to make a new set of balances
    if (strcmp(argv[1], "new") == 0) {
        // each pair of args after the first 2 are a corresponding name and amount
        int len = (argc - 2) / 2;

        // allocate the balance entries and the parallel arrays
        struct BalanceEntries balances;
        balances.len = len;

        balances.names = calloc(len, sizeof(Name));
        balances.amounts = calloc(len, sizeof(float));

        // iterate through each name and amount pair from the args and write to the arrays
        for (int i = 0; i < len; i++) {
            char* name = argv[2 + (i * 2)];
            float amount = atof(argv[2 + (i * 2) + 1]);
            
            strcpy(balances.names[i], name);
            balances.amounts[i] = amount;
        }

        fwriteBalances(balances);

        printBalances(balances);

        freeBalances(balances);
    } else {
        struct BalanceEntries balances = freadBalances();

        char* nameGiving = argv[1];
        float amountGiven = atof(argv[2]);

        // find the index of the name giving and subtract its balance
        int index = findEntryIndexByName(balances, nameGiving);
        if (index != -1) {
            balances.amounts[index] -= amountGiven;
        }

        // iterate through each recepient split the amount given evenly between them
        float takeCount = (float) argc - 3;
        for (int i = 3; i < argc; i++) {
            // find the index of the recepient and add to its balance
            index = findEntryIndexByName(balances, argv[i]);
            if (index != -1) {
                balances.amounts[index] += amountGiven / takeCount;
            }
        }

        fwriteBalances(balances);

        printBalances(balances);

        freeBalances(balances);
    }

    return 0;
}

