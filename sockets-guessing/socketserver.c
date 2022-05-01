#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define BUF_SIZE 1024
#define MAX_BACKLOG 10

int main(int argc, char *argv[]) {

    int server_fd = 0;
    struct sockaddr_in serv_addr; 
    socklen_t sock_len = sizeof(serv_addr);
    
    // connect to a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Connecting to socket failed");
        exit(1);
    }
    
    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // connect to ports until we find one to bind to
    int port = 4999;
	  do {
		  port++;
    	serv_addr.sin_port = htons(port); 
    } while (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0);
    
	  printf("Listening on port #%d\n", port);
     
    // listen on the socket with a maximum backlog
    if (listen(server_fd, MAX_BACKLOG) < 0) {
        fprintf(stderr, "Listening to socket failed");
        exit(1);
    }
    
    printf("Started server loop\n");
    
    // server loop to wait for incoming requests
    int i = 0;
    while (1) {
        printf("Game number: %d\n", i);
        
        // accept a pending request from the socket's queue
        int conn_fd = accept(server_fd, (struct sockaddr*) NULL, NULL);
        
        // create a child process to handle the guessing game
        if (fork() == 0) {
            char recv_buffer[BUF_SIZE];
            char send_buffer[BUF_SIZE];
        
            // generate a random number to be guessed
            srand(getpid() + time(NULL) + getuid());
  		      int num = (rand() % 100) + 1;
                  
            printf("Start game with random number: %d\n", num);
            
            int guesses = 0;
            int n;
            // each read is a number guess, send back to user result of guess
        		while ((n = read(conn_fd, recv_buffer, sizeof(recv_buffer))) > 0) {
          			int guess = atoi(recv_buffer);
          			guesses++;
                
                memset(send_buffer, '\0', sizeof(send_buffer));
          			
          			if (guess < num) {
                    sprintf(send_buffer, "> Try a larger number.");
                } else if (guess > num) {
                    sprintf(send_buffer, "< Try a smaller number.");
                } else {
                    sprintf(send_buffer, "You guessed the number in %d tries!", guesses);
                }
          				
          			write(conn_fd, send_buffer, strlen(send_buffer));
        		}
           
            printf("Finished with: %d\n", i);
             
            exit(0);
        } else {
            i++;
        }
        
        close(conn_fd);
        
    }
    
}