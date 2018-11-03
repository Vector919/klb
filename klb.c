#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

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

int main() {
  int server_socket = initialize_server(9002);
  int client_socket;
  struct sockaddr_in cli_addr;
  char buffer[256];
  char upstream_buffer[256];
  socklen_t address_length = sizeof(cli_addr);
  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
  client_addr.sin_port = htons(8080);

  int upstream_socket;
  int br;
  char response_buffer[255];
  while (1) {
    client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &address_length);
    read(client_socket, buffer, 255);
    printf("%s", buffer);
    // buffer now contains client request
    upstream_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect(upstream_socket, (struct sockaddr *) &client_addr, sizeof(client_addr));
    write(upstream_socket, buffer, strlen(buffer));

    // now recive response
    bzero(upstream_buffer, 256);
    bzero(response_buffer, 256);
    br = read(upstream_socket, upstream_buffer, 255);
    while (br != 0) {
      strncat(response_buffer, upstream_buffer, br);
      br = read(upstream_socket, upstream_buffer, 255);
    }
    printf("%s\n", response_buffer);

    close(upstream_socket);
    write(client_socket, response_buffer, 255);
    close(client_socket);
  }

  return 0;
}
