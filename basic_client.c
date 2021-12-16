#include "pipe_networking.h"



int main() {
  int to_server;
  int from_server;

  from_server = client_handshake( &to_server );

  while (1) {
    char input[1024];
    printf("> ");
    fgets(input,1024,stdin);
    *strrchr(input,'\n') = '\0';
    
    printf("Input: '%s'\n",input);
    if (!strcmp("!quit",input)) {exit(0);}
    
    int data_length = strlen(input)+1;
    write(to_server,&data_length,sizeof(int));
    write(to_server,input,data_length);

    char *processed_data;
    
    data_length = 0;
    read(from_server,&data_length,sizeof(int));

    processed_data = malloc(data_length);

    read(from_server,processed_data,data_length);
    printf("Processed text: %s\n",processed_data);

    free(processed_data);
    
  }
}
