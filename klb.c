#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

#define BUFFER_LENGTH 256

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
  int upstream_socket;

  struct sockaddr_in cli_addr;
  socklen_t address_length = sizeof(cli_addr);

  char buffer[BUFFER_LENGTH];
  char upstream_buffer[BUFFER_LENGTH];
  char response_buffer[BUFFER_LENGTH];

  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
  client_addr.sin_port = htons(8080);

  int upstream_bytes_read;
  int client_bytes_read;
  int current_buffer_size;

  char* temp = NULL;
  char* tc = NULL;
  while (1) {
    client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &address_length);

    client_bytes_read = read(client_socket, buffer, BUFFER_LENGTH - 1);
    current_buffer_size = BUFFER_LENGTH - 1;

    while (client_bytes_read > 0) {
      tc = malloc(sizeof(char) * (current_buffer_size * 2));
      if (temp != NULL) {
        strcat(tc, temp);
      }
      strcat(tc, buffer);
      temp = tc;
      bzero(buffer, BUFFER_LENGTH);
      current_buffer_size = current_buffer_size * 2;
      if (client_bytes_read >= BUFFER_LENGTH - 1) {
        client_bytes_read = read(client_socket, buffer, BUFFER_LENGTH - 1);
      } else {
        client_bytes_read = 0;
      }
    }

    printf("Request: === \n %s \n", temp);

    // buffer now contains client request
    upstream_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect(upstream_socket, (struct sockaddr *) &client_addr, sizeof(client_addr));
    write(upstream_socket, temp, strlen(temp));

    // now recive response
    bzero(upstream_buffer, BUFFER_LENGTH);
    bzero(response_buffer, BUFFER_LENGTH);

    upstream_bytes_read = read(upstream_socket, upstream_buffer, BUFFER_LENGTH - 1);
    while (upstream_bytes_read != 0) {
      strncat(response_buffer, upstream_buffer, upstream_bytes_read);
      upstream_bytes_read = read(upstream_socket, upstream_buffer, BUFFER_LENGTH - 1);
    }

    printf("Response: === \n %s \n", response_buffer);

    close(upstream_socket);
    write(client_socket, response_buffer, sizeof(response_buffer));
    close(client_socket);
  }

  return 0;
}
