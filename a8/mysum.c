#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int fds[100][2];

int child() {
	//rewrite the pipes for fds 0, 1 & 3 before invoking add/subtract/multiply/divide
	//setup FD 0 for reading 1st parameter
	close(0);     
	dup(fds[0][0]);
	//setup FD 3 for reading 2nd parameter
	close(3);
	dup(fds[1][0]);
	//setup FD 1 for outputing the result
	close(1);
	dup(fds[2][1]);

	execl("./add", "add", "-v", NULL);
	fputs("I hope you do not see me!", stderr);
	exit(1);
}

int main() {
	//create 3 pipes: 2 for sending values & 1 for receiving result
	for(int i=0; i<3; i++)
		pipe(fds[i]);

	if (fork() == 0) 
		child();
	
	while (1) {
		printf("Enter 2 numbers:\n");
		int x, y, z;
		scanf("%d%d", &x, &y);
		if (write(fds[0][1], &x, sizeof(int)) == 0)
			exit(1);
		if (write(fds[1][1], &y, sizeof(int)) == 0)
			exit(2);
		if (read(fds[2][0], &z, sizeof(int)) == 0)
			exit(3);
		printf("result from add: %d\n\n", z);
	}
}
