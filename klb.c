#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <netdb.h>

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

struct sockaddr_in * get_server_address(char hostname[]) {
  struct addrinfo * result;
  struct addrinfo hints;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
  hints.ai_socktype = SOCK_STREAM;

  int return_value = getaddrinfo(hostname, "http", &hints, &result);
  if (return_value != 0) {
    printf("Failed to get address info with error code %d", return_value);
    perror("Error:");
    exit(-1);
  }
  return (struct sockaddr_in *) result->ai_addr;
}

int main(int argc, char *argv[]) {
  int server_socket = initialize_server(9002);
  int client_socket;
  int upstream_socket;

  struct sockaddr_in cli_addr;
  socklen_t address_length = sizeof(cli_addr);

  struct sockaddr_in *backends;
  backends = malloc(sizeof(struct sockaddr_in) * (argc - 1));

  int current_backend = 0;
  while (current_backend < argc) {
    backends[current_backend] = *get_server_address(argv[current_backend + 1]);
    current_backend +=1;
  }

  char* request = NULL;
  char* response = NULL;
  int request_number = 0;
  while (1) {
    client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &address_length);

    request = read_all_bytes(client_socket, 0);
    printf("Request: === \n %s \n", request);

    // buffer now contains client request
    upstream_socket = socket(AF_INET, SOCK_STREAM, 0);
    connect(upstream_socket, (struct sockaddr_in *) &backends[request_number % (argc - 1)], sizeof(backends[request_number % (argc - 1)]));
    write(upstream_socket, request, strlen(request));

    // now recive response
    response = read_all_bytes(upstream_socket, 1);
    printf("Response: === \n %s \n", response);

    close(upstream_socket);

    write(client_socket, response, strlen(response));
    close(client_socket);

    free(response);
    free(request);

    request_number +=1;
  }

  return 0;
}
