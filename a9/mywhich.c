#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char command[1000], pathname[1000];

	sprintf(command, "which %s", argv[1]);
	FILE *fp = popen(command, "r");
	fgets(pathname, 1000, fp);
	pclose(fp);
	puts(pathname);
}
