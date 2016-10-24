#include <helpers/file.h>

/* read - close */
#include <unistd.h>

/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int fh_FileToBuffer(const char* name, void *buffer, int size) {
  int fd = open(name, O_RDONLY);
  int bytes_read = read(fd, buffer, size);
  close(fd);
  return bytes_read;
}

void fh_FileToStringBuffer(const char* name, char *buffer, int size) {
  buffer[fh_FileToBuffer(name, buffer, size)] = 0;
}
