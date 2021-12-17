#define BASIC_SERVER_C
#include "pipe_networking.h"
#include <signal.h>

extern int child_id;

static void sighandler(int signum) {
	if (signum == SIGINT) {
		remove(WKP);
		exit(0);
	}
}

int strtoupper(char *string) {
	while (*string) {
		if (*string >= 'a' && *string <= 'z') {
		*string -= ('a' - 'A');
		}
		string++;
	}
}

int main() {
	int from_client, to_client = 0;
	
	signal(SIGINT,sighandler);
	
	while (1) {
		if (from_client != 0) {
			if (!fork()) {
				break;
			}
			close(from_client);
		}
		child_id++;
		
		from_client = server_setup();
	}
	// Subserver part 
	int data_length;
	char *data_to_process;
	
	to_client = server_connect(from_client);	
	while (1) {
		if (read(from_client,&data_length,sizeof(int)) == 0) {break;}
		data_to_process = malloc(data_length);
		if (read(from_client,data_to_process,data_length) == 0) {
			free(data_to_process);
			break;
		}
		printf("[subserver %d]: recieved data '%s'\n",child_id,data_to_process);
		strtoupper(data_to_process);
		printf("[subserver %d]: sending response '%s'\n",child_id,data_to_process);
		write(to_client,&data_length,sizeof(int));
		write(to_client,data_to_process,data_length);
	
		free(data_to_process);
	}
	printf("[subserver %d]: client terminated connection, exiting\n",child_id);
}
