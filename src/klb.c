#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>

#include "server.h"
#include "io_utils.h"

#define NEWLINE "\n"

struct server_configuration {
  int frontend_port; // The port that the KLB server will run on
  struct sockaddr_in *backends; // A list of server addresses we're load balancing between
  int backend_count;
};

/**
 * Takes a Hostname and port, and returns a network address
 * for use as a backend or server location
 */
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
/**
 * Parses a configuration file given it's filename.
 * Returns a server_configuration struct.
 */
struct server_configuration parse_file_configuration(char* filename) {
  struct server_configuration sc;
  int backend_count = 0;
  int file_descriptor = open(filename, O_RDONLY);
  struct read_response r = read_all_bytes(file_descriptor, 0);

  // Count the number of backends by looking for the ':' character
  // Should be present since the config file uses the form
  // <host>:<port>
  char *seperator = ":";
  struct sockaddr_in *backends;
  for (int i = 0; i < strlen(r.data); i++) {
    if(r.data[i] == *seperator) {
      backend_count +=1;
    }
  }
  backends = malloc(sizeof(struct sockaddr_in) * backend_count);

  char *line;
  char *token;
  char *linesave; // Needed for the multiple different calls to strtok

  char *hostname;
  char *port;

  int current_backend = 0;

  // Parse the file contents line by line
  line = strtok_r(r.data, NEWLINE, &linesave);
  while (line != NULL) {
    // Each line either consists of a variable (var=value)
    // Or just a backend (host:port)
    // right now, "port" is the onl
    token = strtok(line, "=");
    while (token != NULL) {
      if (strcmp(token, "port") == 0) {
        sc.frontend_port = atoi(strtok(NULL, "="));
      } else {
        hostname = strtok(line, ":");
        port = strtok(NULL, ":");
        if (hostname != NULL && port != NULL) {
          backends[current_backend] = *get_server_address(hostname, port);
          current_backend +=1;
        }
      }
      token = strtok(NULL, "=");
    }

    line = strtok_r(NULL, NEWLINE, &linesave);
  }
  sc.backends = backends;
  sc.backend_count = current_backend;
  return sc;
}

/**
 *  Converts the command line arguments into a struct containing all
 *  of the necessary information to run the server.
 *
 *  Params:
 *    char* arguments: List of command line arguments (usually argv)
 *    int argument_count: number of arguments (argc)
 */
struct server_configuration parse_commandline_configuration(char* arguments[], int argument_count) {
  struct server_configuration config;

  // Allow users to load the config from a file with the --file <filename> option
  if (strcmp(arguments[1], "--file") == 0) {
    return parse_file_configuration(arguments[2]);
  }

  config.frontend_port = atoi(arguments[1]); // the server port

  // only 1 argument means we have not passed any server information
  if (argument_count == 1) {
    printf("Must specify server port! \n");
    exit(-1);
  }

  if (argument_count < 4) {
    printf("Must specify at least one backend (host and port), to balance between\n");
    exit(-1);
  }



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
