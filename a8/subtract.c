#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int x, y, z;
	while (read(0, &x, sizeof(int)) > 0) {
		if (argc > 1)
			fprintf(stderr, "Read #1: %d\n", x);
		if (read(3, &y, sizeof(int)) > 0) {
			z = x - y;
			write(1, &z, sizeof(int));
			if (argc > 1)
				fprintf(stderr, "Read #2: %d\nResult: %d\n", y, z);
		} else
			exit(1);
	}
	exit(0);
}	
