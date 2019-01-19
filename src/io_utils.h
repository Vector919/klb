struct read_response {
  char* data;
  int length;
};

struct read_response read_all_bytes(int file_descriptor, int force_read);

