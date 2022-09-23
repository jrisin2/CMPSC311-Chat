#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define SERV_TCP_PORT 23 /* server's port */
#define MAX_SIZE 150

int len;
int sockfd;
char string[MAX_SIZE];

void str_trim(char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

static void * recieveMessage (void *arg)
{
	while(1) {
		len = read(sockfd, string, MAX_SIZE);
	 if (len < 0) {
		 printf("Error reading");
		 exit(1);
	 }		 /* make sure it's a proper string */
	 
     string[len] = 0;
     printf("%s\n", string);
	}
}


int main(int argc, char *argv[])
{
  pthread_t t1;
  int s;
  //int sockfd;
  char buf[100];
  struct sockaddr_in serv_addr;
  char *serv_host = "localhost";
  struct hostent *host_ptr;
  int port;
  int buff_size = 0;
  char intro[100]; //*name* has entered the chat
  char clientName[30];
  char message[150]; //includes client name and message

  /* command line: client [host [port]]*/
  if(argc >= 2)
     serv_host = argv[1]; /* read the host if provided */
  if(argc == 3)
     sscanf(argv[2], "%d", &port); /* read the port if provided */
  else
     port = SERV_TCP_PORT;

  /* get the address of the host */
  if((host_ptr = gethostbyname(serv_host)) == NULL) {
     perror("gethostbyname error");
     exit(1);
  }

  if(host_ptr->h_addrtype !=  AF_INET) {
     perror("unknown address type");
     exit(1);
  }


  printf("Enter your name: ");
  fgets(clientName, 30, stdin);
  str_trim(clientName, strlen(clientName));
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr =
     ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
  serv_addr.sin_port = htons(port);


  /* open a TCP socket */
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
     perror("can't open stream socket");
     exit(1);
  }

  /* connect to the server */
  if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     perror("can't connect to server");
     exit(1);
  }

  
  sprintf(intro, "<%s has entered the chat>", clientName);
  /* write a message to the server */
  send(sockfd, intro, 150,0);


	s = pthread_create(&t1, NULL, recieveMessage, NULL);
    if ( s != 0 ) //if creating the new thread is successful, then s will be zero.
    {
        perror("Thread create error");
    }
  
  while(1) {
	  //printf("%s: ", clientName);
      //fgets(buf,100,stdin);
	  
	  fgets(buf,100,stdin); //gets message
	  str_trim(buf, strlen(buf));
	  if (strncmp(buf, "end", 3) == 0) {
		  sprintf(message, "<%s has left>", clientName); //user name and message
		  send(sockfd, message, sizeof(message), 0);
		  exit(1);
	  }
	  sprintf(message, "%s: %s", clientName, buf); //user name and message
	  send(sockfd, message, sizeof(message), 0);
	  
	 
  }

  close(sockfd);
}

/*
TO RUN THIS CODE:
	gcc client.c -o client
	./client localhost 7777
	OR
	./client 127.0.0.1 7777

*/
