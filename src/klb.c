#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <netdb.h>

#include "server.h"
#include "io_utils.h"

struct sockaddr_in * get_server_address(char hostname[], char port[]) {
  struct addrinfo * result;
  struct addrinfo hints;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int return_value = getaddrinfo(hostname, port, &hints, &result);
  if (return_value != 0) {
    printf("Failed to get address info with error code %d", return_value);
    perror("Error:");
    exit(-1);
  }
  return (struct sockaddr_in *) result->ai_addr;
}

int main(int argc, char *argv[]) {
  int port = atoi(argv[1]);

  struct sockaddr_in *backends;
  backends = malloc(sizeof(struct sockaddr_in) * (argc - 2));

  int current_backend = 0;
  int backend_count = 0;
  while (current_backend < (argc - 2)) {
    // Each argument contains a host and port pair
    backends[current_backend] = *get_server_address(argv[current_backend + 2], argv[current_backend + 3]);
    current_backend +=2;
    backend_count +=1;
  }

  run_server(port, backends, backend_count);
  return 0;
}
