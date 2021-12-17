#include "pipe_networking.h"


static void sighandler(int signo);
void reverse(char *str);


int main() {
  signal(SIGINT, sighandler);

  while (1) {
    printf("\n[server] setting up for new connection\n");
    int from_client = server_setup();
    printf("[server] got a connection, forking\n");

    int pid = fork();
    if (pid == 0) {
      printf("[subserver] forked! finishing connection with client\n");
      int to_client = server_connect(from_client);
      printf("[subserver] connected to client!\n");
      
      char data[500];
    
      while (1) {
        int r = read(from_client, data, sizeof(data));
        if (r == 0) {
          printf("[subserver] disconnected from client\n");
          close(from_client);
          close(to_client);

          exit(EXIT_SUCCESS);
        }
        
        data[r] = 0;
        reverse(data);

        write(to_client, data, strlen(data));
      }
    }
    else if (pid == -1) {
      printf("[server] Couldn't fork (%s, %d)\n", strerror(errno), errno);
      exit(EXIT_FAILURE);
    }
    else {
      printf("[server] forked!\n");
      continue;
    }
  }

  return 0;
}


static void sighandler(int signo) {
  if (signo == SIGINT) {
    printf("[server] SIGINT recieved, exiting!\n");
    
    remove(WKP);
    printf("\n");
    exit(EXIT_SUCCESS);
  }
}


void reverse(char *str) {
  int len = strlen(str);

  int i;
  for (i = 0; i < len/2; i++) {
    int temp = str[len-i-1];
    str[len-i-1] = str[i];
    str[i] = temp;
  }
}
