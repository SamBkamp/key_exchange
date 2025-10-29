//code for the client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PORT "6060"
#define BUFFER_SIZE 1024

typedef struct{
  uint8_t STATUS; //lower 4 bits = SYN, upper 4 bits = hello
  uint8_t step;
  char *client_key;
  char *host_key;
}handshake_state;

int main(){
  handshake_state hs_state = {0};
  struct addrinfo hints, *res;
  int sockfd, gai;
  char buffer[BUFFER_SIZE];
  
  //hostname resolution
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  gai = getaddrinfo("fish", PORT, &hints, &res);
  if(gai != 0){
    perror("getaddrinfo");
    return 1;
  }
  if(res == NULL){
    printf("could not resolve hostname\n");
    freeaddrinfo(res);
    return 1;
  }    
    
  //construct socket
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(sockfd < 0){
    perror("socket");
    return 1;
  }

  //connect
  if(connect(sockfd, res->ai_addr, res->ai_addrlen) < 0){
    perror("connect");
    return 1;
  }
  freeaddrinfo(res);

  //main loop
  while(1){
    strncpy(buffer, "SYN\n", 5);
    write(sockfd, buffer, strlen(buffer));
    ssize_t bytes_read = read(sockfd, buffer, BUFFER_SIZE);
    
    if(bytes_read == 0)
      break;
    buffer[bytes_read] = 0;
    
    switch(hs_state.step){
    case 0:
      if(strncmp(buffer, "SYN ACK\n", 10)==0){
	hs_state.step ++;
	strncpy(buffer, "ACK\n", 5);
	write(sockfd, buffer, strlen(buffer));
	strncpy(buffer, "ClientHello\n", 13);
	write(sockfd, buffer, strlen(buffer));
	break;
      }
      goto fail; //FIGHT ME
    case 1:
      if(strncmp(buffer, "ServerHello\n", 13)!=0)
	goto fail;
      strncpy(buffer, "lol\n", 5);
      write(sockfd, buffer, strlen(buffer));
      hs_state.step++;
      break;
    fail:
    default:
      printf("handshake corrupted, exiting\n");
      strncpy(buffer, "goodbye\n", 9);
      write(sockfd, buffer, strlen(buffer));
      close(sockfd);
      return 1;
    }
  }
}
