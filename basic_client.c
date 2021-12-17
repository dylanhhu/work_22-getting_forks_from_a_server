#include "pipe_networking.h"


static void sighandler(int signo);


int main() {
  signal(SIGINT, sighandler);

  int to_server;
  int from_server;

  from_server = client_handshake(&to_server);

  char input[500];

  while (1) {
    printf(">>> ");

    fgets(input, sizeof(input), stdin);
    if (input[0] == '\n') continue;
    
    input[strlen(input) - 1] = 0;
    write(to_server, input, strlen(input));

    int r = read(from_server, input, sizeof(input));
    input[r] = 0;

    printf("%s\n", input);
  }
}


static void sighandler(int signo) {
  if (signo == SIGINT) {
    char pid[100];
    sprintf(pid, "%d", getpid());
    
    remove(pid);

    printf("\n");
    exit(EXIT_SUCCESS);
  }
}
