#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define LOG_MAX 2048
static char receive[LOG_MAX];

int main(int argc, char** argv){
  int fd = open("/dev/mykeylogger", O_RDONLY);
  int ret;
  int count = 0;

  if (fd < 0) {
    perror("Failed to open mykeylogger\n");
    return errno;
  }

  while(count < 10) {
    ret = read(fd, receive, LOG_MAX);
    if (ret > 0) {
      printf("%s", receive);
      memset(receive, 0, LOG_MAX);
    } else if (ret == 0){
      continue;
    } else {
      perror("Failed to read\n");
      return errno;
    }
  }
  
  return 0;
}
