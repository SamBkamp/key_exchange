//code for the client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PORT "6060"
#define BUFFER_SIZE 1024
#define PUBKEY_SIZE 460


typedef struct{
  uint8_t step;
  char *client_key;
  char *host_key;
}handshake_state;

int open_key(const char* path, char* buffer, ssize_t size){
  int public_key_fd;
  //file io
  public_key_fd = open(path, O_RDONLY);
  if(public_key_fd < 0)
    return -1;

  ssize_t public_key_bread = read(public_key_fd, buffer, size-1);
  if(public_key_bread < 0)
    return -1;

  buffer[public_key_bread] = 0;
  close(public_key_fd);
  return 0;
}

int main(){
  struct addrinfo hints, *res;
  handshake_state hs_state = {0};
  int sockfd, gai;
  char buffer[BUFFER_SIZE];
  char public_key_data[PUBKEY_SIZE];

  if(open_key("client_keys/public.pem", public_key_data, PUBKEY_SIZE)<0){
    perror("open_key");
    return 1;
  }

  //hostname resolution
  memset(&hints, 0, sizeof(hints)); //not including this creates WIERD fucking bugs
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  gai = getaddrinfo("fish", PORT, &hints, &res);
  if(gai != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
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
    ssize_t bytes_read = 1;
    if(hs_state.step > 0)
      bytes_read = read(sockfd, buffer, BUFFER_SIZE-1);
    if(bytes_read == 0)
      break;
    buffer[bytes_read] = 0;
    sprintf(buffer, "%s\n", "ClientHello");
    write(sockfd, buffer, strlen(buffer));
    hs_state.step++;
  }
}
