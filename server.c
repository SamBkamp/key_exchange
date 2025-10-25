#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 6060
#define CLIENTS_MAX 2

typedef struct{
  int sockfd;
  struct pollfd fdtab;
}client;

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


void init_fdtabs(client *in_data){

  in_data->fdtab.fd = in_data->sockfd;
  in_data->fdtab.events = POLLIN;
  in_data->fdtab.revents = 0;
}

int main(){
  client clients[CLIENTS_MAX];
  uint8_t clients_connected = 0;
  int sockfd;
  struct sockaddr_in host_addr;
  struct sockaddr_in peer_addr;
  socklen_t peer_addr_size = sizeof(peer_addr);

  //open binded listening port
  if(open_connection(PORT, &sockfd, &host_addr) == -1)
    return 1;

  while(clients_connected < CLIENTS_MAX){
    int inbd_sock = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_size);
    clients[clients_connected].sockfd = inbd_sock;
    init_fdtabs(&clients[clients_connected]);
    clients_connected++;
  }
  printf("all clients connected!\n");
  //main event loop
  uint8_t do_work = 1;
  while(do_work){
    char buffer[1028];
    for(int i = 0; i < CLIENTS_MAX; i++){
      int retpoll = poll(&clients[i].fdtab, 1, 100);
      if(retpoll < 0){
	perror("poll");
	return 1;
      }
      if(clients[i].fdtab.revents & POLLIN){
	ssize_t read_amt = read(clients[i].sockfd, buffer, 1028);
	if(read_amt == 0)
	  do_work = 0;
	else{
	  buffer[read_amt] = 0;
	  printf("[%d] %s", i, buffer);
	  write(clients[i^1].sockfd, buffer, strlen(buffer));
	}
      }
      //polled but no POLLIN
    }
  }
  fflush(stdout);
  close(clients[0].sockfd);
  close(clients[1].sockfd);
  printf("closed. Good bye\n");
  return 0;
}
