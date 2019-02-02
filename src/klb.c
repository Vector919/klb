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

struct server_configuration {
  int frontend_port; // The port that the KLB server will run on
  struct sockaddr_in *backends; // A list of server addresses we're load balancing between
  int backend_count;
};

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

struct server_configuration parse_commandline_configuration(char* arguments[], int argument_count) {
  struct server_configuration config;

  config.frontend_port = atoi(arguments[1]); // the server port

  // don't count the first 2 arguments as backends (executable name and server port)
  // Every backend should have 2 associated arguments (host, port)
  config.backend_count = (argument_count - 2) / 2;

  struct sockaddr_in *backends;
  config.backends = malloc(sizeof(struct sockaddr_in) * config.backend_count);

  int current_backend = 0;
  int current_argument = 2;
  char* host;
  char* port;

  while (current_argument < argument_count) {
    host = arguments[current_argument];
    port = arguments[current_argument + 1];

    config.backends[current_backend] = *get_server_address(host, port);
    current_argument +=2;
    current_backend +=1;
  }

  return config;
}

int main(int argc, char *argv[]) {
  struct server_configuration config;
  config = parse_commandline_configuration(argv, argc);
  run_server(config.frontend_port, config.backends, config.backend_count);
  return 0;
}
