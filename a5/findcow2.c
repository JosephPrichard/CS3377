#include <stdio.h>
#include <ctype.h>

#define MAXLEN 1000000

// intelligent: O(n)
int cowFind2(const char* string) {
    int possibleLocations = 0;

    int hindLegCount = 0;

    // search for front legs and hind legs in the array
    // store the number of hind legs we've found so far
    // whenever we find a front leg, it could pair with we increment locations
    for (int i = 0; string[i] != 0; i++) {
        if (string[i] == '(' && string[i+1] == '(') {
            hindLegCount++;
        }
        else if (string[i] == ')' && string[i+1] == ')') {
            possibleLocations += hindLegCount;
        }
    }

    return possibleLocations;
}


int main() {
	char str[MAXLEN];
	scanf("%s", str);
	
	int locations = cowFind2(str);

	printf("Possible Locations: %d\n", locations); 
}
