
struct MemoryRegion {
  void *startAddr;
  unsigned long int seg_size;
  int priv;
};

struct MemoryRegion processRegion(char* lineBuf);

void* strptr(char* address);

size_t get_file_size(const char* filename);

void print(const char* s);

void my_read(const char* filename, size_t readbyte, char* buf, int retry);

ssize_t read_exact(int fd, size_t nbyte_to_read, void* buf, int retry);

ssize_t my_write(int fd, const void* buf, size_t nbyte, int retry);
