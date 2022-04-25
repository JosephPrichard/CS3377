#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LINESIZE 16

//use one command line argument
int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: diagonal2 <textstring>\n");
		return -1;
	}
	
	//create a file so that 16 rows of empty will appear with od -c command
	int fd = open("diagonal2.out", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	
	char space = ' ';
	for(int arg=0; arg<argc-1; arg++)
		for(int line=0; line<LINESIZE; line++)
			for(int column=0; column<LINESIZE; column++)
				write(fd, &space, 1);

	//Each line of od outputs 16 characters 
	//So, to make the output diagonal, we will use 0, 17, 34, ....
	//Write at character 0, then skip 16 forward, write at 17, so on and so forth
	
	int charCount = 0;
	for (int i=1; i<argc; i++) {
		int n = strlen(argv[i]);
		for(int j=0; j<n; j++) {
			int seekPos = LINESIZE * LINESIZE * (i-1);
			if (i % 2 != 0) {
				seekPos += (LINESIZE+1)*j;
			}
			else {
				seekPos += LINESIZE*j + (LINESIZE-1-j);
			}
			lseek(fd, seekPos, SEEK_SET);
			write(fd, &argv[i][j], 1);
			charCount++;
		}
	}
	close(fd);
	puts("diagonal2.out has been created. Use od -c diagonal2.out to see the contents.");
}
