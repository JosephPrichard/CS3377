#include <stdio.h>
#include <string.h>

#define MAXLEN 1000

char operators[100];

int main(int argc, char *argv[]) {
	char line[MAXLEN], *temp;

	FILE *dataFile = fopen(argv[1], "r");
	//read the first line - it contains the configuration
	fgets(line, MAXLEN, dataFile); 

	// sample content for line: a + b - c
	strtok(line, " \n"); //skip the symbol representing variable/parameter
	int operatorCount=0;
	while (temp = strtok(NULL, " \n")) {
		operators[operatorCount] = temp[0];
		printf("operator: %c\n", operators[operatorCount]);
		operatorCount++;
		strtok(NULL, " \n"); //skip the symbol representing variable/parameter
	}

	//setup the configuration with necessary # of children
	//continue to read the data from the file
	//you can use fscanf() to read the remaining data!
	//Here is some code to get started!
	int x;
	while (fscanf(dataFile, "%d", &x) > 0) {
		printf("%d ", x);
		//let us assume that line has that many pieces of data
		for(int i=1; i<=operatorCount; i++) {
			fscanf(dataFile, "%d", &x);
			printf("%d ", x);
		}
		putchar('\n');
	}
}

