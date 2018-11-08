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

char* read_all_bytes(int file_descriptor, int force_read) {
  int bytes_read;
  int current_buffer_size;

  char buffer[256];
  char* data = NULL;
  char* temp = NULL;

  bzero(buffer, BUFFER_LENGTH);

  bytes_read = read(file_descriptor, buffer, BUFFER_LENGTH - 1);
  current_buffer_size = BUFFER_LENGTH - 1;
  while (bytes_read > 0) {
      temp = malloc(sizeof(char) * (current_buffer_size + bytes_read));
      bzero(temp, strlen(temp));
      if (data != NULL) {
        strcat(temp, data);
      }
      strcat(temp, buffer);
      data = temp;

      bzero(buffer, BUFFER_LENGTH);
      current_buffer_size = current_buffer_size + bytes_read;
      if (bytes_read >= BUFFER_LENGTH - 1 || force_read == 1) {
        bytes_read = read(file_descriptor, buffer, BUFFER_LENGTH - 1);
      } else {
        bytes_read = 0;
      }
    }
    return data;
}

int main() {
  int server_socket = initialize_server(9002);
  int client_socket;
  int upstream_socket;

  struct sockaddr_in cli_addr;
  socklen_t address_length = sizeof(cli_addr);

  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
  client_addr.sin_port = htons(8080);

  char* request = NULL;
  char* response = NULL;

  while (1) {
    client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &address_length);

    request = read_all_bytes(client_socket, 0);
    printf("Request: === \n %s \n", request);

    // buffer now contains client request
    upstream_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect(upstream_socket, (struct sockaddr *) &client_addr, sizeof(client_addr));
    write(upstream_socket, request, strlen(request));

    // now recive response
    response = read_all_bytes(upstream_socket, 1);
    printf("Response: === \n %s \n", response);

    close(upstream_socket);

    write(client_socket, response, strlen(response));
    close(client_socket);

    free(response);
    free(request);
  }

  return 0;
}
