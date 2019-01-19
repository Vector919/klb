#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include "io_utils.h"
#include "server.h"

int initialize_server(int port) {
  int socket_fd;
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd < 0) {
    printf("Failed to open socket");
    exit(-1);
  }

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  int bind_return = bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (bind_return < 0) {
    printf("Failed to bind socket\n");
    exit(-1);
  }

  int listen_return = listen(socket_fd, 5);
  if (listen_return < 0) {
    printf("Failed to listen with socket\n");
    exit(-1);
  }

  printf("Started server on port %d!\n", port);
  return socket_fd;
}

void run_server(int port, struct sockaddr_in *backends, int backend_count) {
  int server_socket = initialize_server(port);
  int client_socket;
  int upstream_socket;

  struct read_response request;
  struct read_response response;
  struct sockaddr_in cli_addr;
  socklen_t address_length = sizeof(cli_addr);

  int request_number = 0;
  while (1) {
    client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &address_length);

    request = read_all_bytes(client_socket, 0);
    printf("Request: === \n %s \n", request.data);

    // buffer now contains client request
    upstream_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect(upstream_socket, (struct sockaddr_in *) &backends[request_number % backend_count], sizeof(backends[request_number % backend_count]));
    write(upstream_socket, request.data, request.length);

    // now recive response
    response = read_all_bytes(upstream_socket, 1);

    close(upstream_socket);

    write(client_socket, response.data, response.length);
    close(client_socket);

    free(response.data);
    free(request.data);

    request_number +=1;
  }
}