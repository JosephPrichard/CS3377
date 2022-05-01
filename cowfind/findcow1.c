#include <stdio.h>
#include <ctype.h>

#define MAXLEN 1000000

int strLen(const char* string) {
    int i;
    for(i = 0; string[i] != 0; i++) {}
    return i;
}

// brute force: O(n^2)
int cowFind1(const char* string) {
    int len = strLen(string);

    int possibleLocations = 0;

    // the leg arrays store the index of the first character in each leg,
    // arrays should be len - 1, in the worst case every pair of parenthesis is a leg

    int hindLegs[len - 1];
    int hindLegIndex = 0;

    int frontLegs[len - 1];
    int frontLegIndex = 0;

    // search for all possible legs in the string
    // add the hind legs to an array of all hind legs
    // add the front legs to an array of all front legs
    for (int i = 0; i < len-1; i++) {
        if (string[i] == '(' && string[i+1] == '(') {
            hindLegs[hindLegIndex] = i;
            hindLegIndex++;
        }
        else if (string[i] == ')' && string[i+1] == ')') {
            frontLegs[frontLegIndex] = i;
            frontLegIndex++;
        }
    }

    // perform a nested search between all possible combinations of front legs and hind legs
    // only count the leg pairs as possible location if the front leg index is larger than the hind leg index
    for (int i = 0; i < hindLegIndex; i++) {
        for (int j = 0; j < frontLegIndex; j++) {
            if (hindLegs[i] < frontLegs[j]) {
                possibleLocations++;
            }
        }
    }

    return possibleLocations;
}


int main() {
	char str[MAXLEN];
	scanf("%s", str);
	
	int locations = cowFind1(str);

	printf("Possible Locations: %d\n", locations); 
}
