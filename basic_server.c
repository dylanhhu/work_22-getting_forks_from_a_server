#include "pipe_networking.h"


static void sighandler(int signo);
void reverse(char *str);


int main() {
  signal(SIGINT, sighandler);

  while (1) {
    int from_client = server_setup();

    int pid = fork();
    if (pid) {
      int to_client = server_connect(from_client);
      
      char data[500];
    
      while (1) {
        int r = read(from_client, data, sizeof(data));
        if (r == 0) break;
        
        data[r] = 0;
        reverse(data);

        write(to_client, data, strlen(data));
      }
    }
  }

  return 0;
}


static void sighandler(int signo) {
  if (signo == SIGINT) {
    remove(WKP);
    printf("\n");
    exit(EXIT_SUCCESS);
  }
}


void reverse(char *str) {
  int len = strlen(str);

  int i;
  for (i = 0; i < len / 2; i++) {
    int temp = str[len-i-1];
    str[len-i-1] = str[i];
    str[i] = temp;
  }
}
