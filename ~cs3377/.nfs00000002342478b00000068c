#include <stdio.h>
#include <stdlib.h>

//most efficient mechanism to generate fibonacci numbers
//you can use this to compare the results with your programs.
//Sequence: 0, 1, 1, 2, 3, 5, 8, 13, 21, ...
//       n: 0  1  2  3  4  5  6   7   8  ...

int fib(int n) {
	if (n < 2)
		return n;

	int  prev = 1, prev2 = 0;
	n--;
	while (n--) {
		int current = prev + prev2;
		prev2 = prev;
		prev = current;
	}
	return prev;
}


int main(int argc, char *argv[]) {
	printf("%d\n", fib(atoi(argv[1])));
}
