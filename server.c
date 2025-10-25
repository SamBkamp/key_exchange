#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 6060
#define CLIENT_MAX 2

int open_connection(uint16_t port, int *sockfd, struct sockaddr_in *host_addr){
  *sockfd = socket(AF_INET, SOCK_STREAM, 0); //open a socket
  if(*sockfd == -1){
    perror("socket error");
    return -1;
  }

  if(setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) != 0)
    perror("[NON-FATAL] couldn't set sockopt\n");

  memset(host_addr, 0, sizeof(*host_addr)); //reset our socket address structs
  host_addr->sin_family = AF_INET; //were using IP
  host_addr->sin_port = htons(port); //turn from host byte order (little endian) to network byte (big endian)
  host_addr->sin_addr = (struct in_addr){INADDR_ANY}; //accept connections from anywhere (0.0.0.0)
  //only accepts the in_addr struct which takes one value, for some reason.

  if(bind(*sockfd, (struct sockaddr *)host_addr, sizeof(*host_addr)) != 0){ //bind our socket to our address
    perror("bind error");
    return -1;
  }
  if(listen(*sockfd, 5) == -1){ //set our socket as listening
    perror("listen error");
    return -1;
  }
  return 0;
}



int main(int argc, char* argv[]){

  int sockfd;
  struct sockaddr_in host_addr;
  struct sockaddr_in peer_addr;
  socklen_t peer_addr_size = sizeof(peer_addr);
  if(open_connection(PORT, &sockfd, &host_addr) == -1)
    return 1;

  while(1){
    int inbd_sock = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_size);
    char buffer[2048];
    ssize_t read_len = read(inbd_sock, buffer, 2048);

    while(read_len > 0){
      buffer[read_len] = 0;
      printf("%s", buffer);
      read_len = read(inbd_sock, buffer, 2048);
    }
    if(read_len < 0)
      perror("read");
  }
}
