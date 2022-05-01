#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {

    if(argc != 3) {
        printf("Usage: %s <ip of server> <port #> \n", argv[0]);
        return 1;
    }
    
    char recv_buffer[BUF_SIZE];
    
    int sock_fd = 0;
    
    struct sockaddr_in serv_addr; 
    
    memset(recv_buffer, '\0', sizeof(recv_buffer));
    
    // create socket
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: Could not create socket \n");
        return 1;
    } 
    
    memset(&serv_addr, '\0', sizeof(serv_addr)); 
    
    serv_addr.sin_family = AF_INET;
    int port = atoi(argv[2]);
    serv_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("inet_pton error occured\n");
        return 1;
    } 

    // conect to socket
    if(connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
       printf("Error : Connect Failed \n");
       return 1;
    }
    
    puts("Guess a number between 0 and 100.");
    
    int n;
  	// get each line from user and send it to server
  	while (fgets(recv_buffer, sizeof(recv_buffer), stdin) > 0) {
   
        write(sock_fd, recv_buffer, strlen(recv_buffer) + 1);
        
        if ((n = read(sock_fd, recv_buffer, sizeof(recv_buffer) - 1)) > 0) {
 		        recv_buffer[n] = 0;
                    
            puts(recv_buffer);
  		
 			      if (recv_buffer[0] != '<' && recv_buffer[0] != '>') {
                break;  
            }
  		  } else {
            break;
        }
 	  }
    
}