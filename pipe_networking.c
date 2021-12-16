#include "pipe_networking.h"
#include <time.h>

int child_id = -1;

/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  int from_client;
  
  printf("[server]: Creating WKP\n");
  mkfifo(WKP,0600);
  if (errno) {
	  printf("mkfifo error: %s\n",strerror(errno));
  }
  printf("[server]: waiting for conn. on WKP\n");
  from_client = open(WKP, O_RDONLY);
  if (errno) {
    printf("[server]: error: %s\n",strerror(errno));
  }
  remove(WKP);
  return from_client;
}

/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
	int to_client, data_length;
	char *buffer;
	read(from_client,&data_length,sizeof(int));
  
	buffer = malloc(data_length);
	read(from_client,buffer,data_length);
	
	printf("[subserver %d]: opening FIFO '%s'\n",child_id,buffer);
	to_client = open(buffer, O_WRONLY);
	printf("[subserver %d]: connected to client private pipe\n",child_id);
	free(buffer);
  
	srand(time(NULL));
	int r = rand();
	int rrec;
	printf("[subserver %d]: send ack attempt %d\n",child_id,r);
	write(to_client,&r,sizeof(int));
	if (read(from_client,&rrec,sizeof(int)) < sizeof(int)) {
		printf("[subserver %d] error: fail on read: %s\n",child_id,strerror(errno));
	}
	// This wasn't working even though I got the right vals 
	//if (rrec == r + 1) {
	  printf("[subserver %d]: ack conf recieved (%d)\n",child_id,rrec);
	//} else {
	//  printf("[subserver %d]: ack went wrong, %d received\n",child_id,rrec);	
	//}  
	return to_client;
}

/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
	int from_server = 0;
	char pidstring[12];
	int dtlength;
  
	*to_server = open(WKP,O_WRONLY);
	snprintf(pidstring,12,"%d",getpid());

	mkfifo(pidstring,0600);

	printf("[client]: FIFO to send to server: %s\n",pidstring);

	dtlength = strlen(pidstring)+1;
	write(*to_server,&dtlength,sizeof(int));
	write(*to_server,pidstring,dtlength);

  
	from_server = open(pidstring,O_RDONLY);
	remove(pidstring);
  
	// Do acknowledge
	if (read(from_server,&dtlength,sizeof(int)) < sizeof(int)) { // use dtlength to hold random int from server
		printf("[client]: something went wrong, could not read from private pipe\n");
	} else {
		printf("[client]: recieved ack attempt %d\n",dtlength);
	}
	dtlength++;
	printf("[client]: send ack return %d\n",dtlength);
	if (write(*to_server,&dtlength,sizeof(int)) < sizeof(int)) {
		printf("[client]: error: could not write to former wkp ! \n");
	}
  
  
  return from_server;
}
