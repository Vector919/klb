#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#define BUFFER_LENGTH 1024

struct read_response {
  char* data;
  int length;
};

struct read_response read_all_bytes(int file_descriptor, int force_read) {
  int bytes_read;
  int current_buffer_size;

  char buffer[BUFFER_LENGTH];
  char* data = NULL;
  char* temp = NULL;

  bzero(buffer, BUFFER_LENGTH);

  bytes_read = read(file_descriptor, buffer, BUFFER_LENGTH - 1);
  current_buffer_size = 0;
  while (bytes_read > 0) {
      temp = malloc(sizeof(char) * (current_buffer_size + bytes_read));
      if (data != NULL) {
        memcpy(temp, data, sizeof(char) * current_buffer_size);
      }
      memcpy(&temp[current_buffer_size], &buffer, bytes_read);

      data = temp;

      bzero(buffer, BUFFER_LENGTH);
      current_buffer_size = current_buffer_size + bytes_read;
      if (bytes_read >= BUFFER_LENGTH - 1 || force_read == 1) {
        bytes_read = read(file_descriptor, buffer, BUFFER_LENGTH - 1);
      } else {
        bytes_read = 0;
      }
    }
    struct read_response resp;
    resp.data = data;
    resp.length = current_buffer_size;
    return resp;
}