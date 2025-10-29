//code for the host (ie. the server in the client/server relationship. Not to be confused with server.c which is the window peeker thing)

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
  uint8_t do_work = 1;
  while(do_work){
    ssize_t bytes_read = read(sockfd, buffer, BUFFER_SIZE);
    if(bytes_read == 0)
      break;
    buffer[bytes_read] = 0;
    if(strncmp(buffer, "ClientHello\n", 13) == 0){
      sprintf(buffer, "%s\n", "ServerHello");
      write(sockfd, buffer, strlen(buffer));
      break;
    }
    printf("handshake corrupted. Aborting.\n");
    close(sockfd);
  }
}
