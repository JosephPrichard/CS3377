#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define FILE_NAME "gifts2.dat"
#define MAX_NAME_LEN 100

// entries is a pointer to array of length len
struct BalanceEntries {
    struct BalEntry* entries;
    unsigned int len;
};

// each balance entry has a name and amount pair
struct BalEntry {
    char name[MAX_NAME_LEN];
    float amount;
};

// opensthe file, reads length, array, and object contents and returns balance entries
struct BalanceEntries freadBalances() {
    FILE* file = fopen(FILE_NAME, "r+");

    fseek(file, 0, SEEK_SET);

    // read the array length from the file
    unsigned int len;
    fread(&len, sizeof(unsigned int), 1, file);

    // allocate balance entries array and write array of pairs to it
    struct BalEntry* entries = calloc(len, sizeof(struct BalEntry));
    fread(entries, sizeof(struct BalEntry), len, file);

    fclose(file);

    struct BalanceEntries balances = { entries, len };
    return balances;
}

// opens a file, writes to it the lengths and array contents from the balance entries
void fwriteBalances(struct BalanceEntries balances) {
    FILE* file = fopen(FILE_NAME, "w+");

    fseek(file, 0, SEEK_SET);

    // write array length to the file
    fwrite(&balances.len, sizeof(unsigned int), 1, file);

    // write array of objects to the file
    fwrite(balances.entries, sizeof(struct BalEntry), balances.len, file);

    fclose(file);
}

// iterate through parallel arrays and print each element
void printBalances(struct BalanceEntries balances) {
    for (int i = 0; i < balances.len; i++) {
        printf("%10s: %5.2lf\n", balances.entries[i].name, balances.entries[i].amount);
    }
}

// finds the array index of the element by corresponding name
int findEntryIndexByName(struct BalanceEntries balances, char* name) {
    for (int i = 0; i < balances.len; i++) {
        if (strcmp(balances.entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// frees the contents of the balance entries
void freeBalances(struct BalanceEntries balances) {
    free(balances.entries);
}

int main(int argc, char* argv[]) {
    // detect if we are going to make a new set of balances
    if (strcmp(argv[1], "new") == 0) {
        // each pair of args after the first 2 are a corresponding name and amount
        unsigned int len = (argc - 2) / 2;

        // allocate the balance entries array
        struct BalEntry* entries = calloc(len, sizeof(struct BalEntry));

        // iterate through each name and amount pair from the args and write to the array
        for (int i = 0; i < len; i++) {
            char* name = argv[2 + (i * 2)];
            float amount = atof(argv[2 + (i * 2) + 1]);

            strcpy(entries[i].name, name);
            entries[i].amount = amount;
        }

        struct BalanceEntries balances = { entries, len };
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
            balances.entries[index].amount -= amountGiven;
        }

        // iterate through each recepient split the amount given evenly between them
        float takeCount = (float) argc - 3;
        for (int i = 3; i < argc; i++) {
            index = findEntryIndexByName(balances, argv[i]);
            // find the index of the recepient and add to its balance
            if (index != -1) {
                balances.entries[index].amount += amountGiven / takeCount;
            }
        }

        fwriteBalances(balances);

        printBalances(balances);

        freeBalances(balances);
    }

    return 0;
}

