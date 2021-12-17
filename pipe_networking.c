#include "pipe_networking.h"


/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  printf("[server] creating well known pipe\n");
  int wkp = mkfifo(WKP, 0600);
  if (wkp) {
    printf("[server] couldn't create well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("[server] created well known pipe\n");
  printf("[server] opening well known pipe for reading\n");
  wkp = open(WKP, O_RDONLY);
  if (wkp < 0) {
    printf("[server] couldn't open well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("[server] opened well known pipe\n");
  printf("[server] removing well known pipe\n");
  remove(WKP);

  return wkp;
}

/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  printf("[subserver] awaiting handshake\n");
  char handshake[HANDSHAKE_BUFFER_SIZE];
  int r = read(from_client, handshake, sizeof(handshake));
  handshake[r] = 0;

  printf("[subserver] recieved handshake\n");
  printf("[subserver] opening secret pipe named %s\n", handshake);

  int to_client = open(handshake, O_WRONLY);
  if (to_client < 0) {
    printf("[subserver] couldn't open secret pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("[subserver] opened secret pipe\n");
  printf("[subserver] sending SYN_ACK\n");
  srand(time(NULL));
  int syn_ack = rand() % HANDSHAKE_BUFFER_SIZE;
  sprintf(handshake, "%d", syn_ack);

  write(to_client, handshake, sizeof(handshake));

  printf("[subserver] sent SYN_ACK, awaiting ACK\n");
  read(from_client, handshake, sizeof(handshake));
  int recieved_ack = atoi(handshake);
  if (recieved_ack != syn_ack + 1) {
    printf("[subserver] handshake received bad ACK %s\n", handshake);
    exit(EXIT_FAILURE);
  }

  printf("[subserver] good ACK recieved\n");
  return to_client;
}


/*=========================
  server_handshake
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  int b, from_client;
  char buffer[HANDSHAKE_BUFFER_SIZE];

  printf("[server] handshake: making wkp\n");
  b = mkfifo(WKP, 0600);
  if ( b == -1 ) {
    printf("mkfifo error %d: %s\n", errno, strerror(errno));
    exit(-1);
  }
  //open & block
  from_client = open(WKP, O_RDONLY, 0);
  //remove WKP
  remove(WKP);

  printf("[server] handshake: removed wkp\n");
  //read initial message
  b = read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake received: -%s-\n", buffer);


  *to_client = open(buffer, O_WRONLY, 0);
  //create SYN_ACK message
  srand(time(NULL));
  int r = rand() % HANDSHAKE_BUFFER_SIZE;
  sprintf(buffer, "%d", r);

  write(*to_client, buffer, sizeof(buffer));
  //rad and check ACK
  read(from_client, buffer, sizeof(buffer));
  int ra = atoi(buffer);
  if (ra != r+1) {
    printf("[server] handshake received bad ACK: -%s-\n", buffer);
    exit(0);
  }//bad response
  printf("[server] handshake received: -%s-\n", buffer);

  return from_client;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  int from_server;
  char buffer[HANDSHAKE_BUFFER_SIZE];
  char ppname[HANDSHAKE_BUFFER_SIZE];

  //make private pipe
  printf("[client] handshake: making pp\n");
  sprintf(ppname, "%d", getpid() );
  mkfifo(ppname, 0600);

  //send pp name to server
  printf("[client] handshake: connecting to wkp\n");
  *to_server = open( WKP, O_WRONLY, 0);
  if ( *to_server == -1 ) {
    printf("open error %d: %s\n", errno, strerror(errno));
    exit(1);
  }

  write(*to_server, ppname, sizeof(buffer));
  //open and wait for connection
  from_server = open(ppname, O_RDONLY, 0);

  read(from_server, buffer, sizeof(buffer));
  /*validate buffer code goes here */
  printf("[client] handshake: received -%s-\n", buffer);

  //remove pp
  remove(ppname);
  printf("[client] handshake: removed pp\n");

  //send ACK to server
  int r = atoi(buffer) + 1;
  sprintf(buffer, "%d", r);
  write(*to_server, buffer, sizeof(buffer));

  return from_server;
}
