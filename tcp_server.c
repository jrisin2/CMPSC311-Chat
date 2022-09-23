#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define SERV_TCP_PORT 23 /* server's port number */
#define MAX_SIZE 150

int len, maxSd;
char buf[100];
int newsockfd;
int clientSocket[5], maxClients = 5;

void str_trim(char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

static void * sendMessage (void *arg)
{
	char message[150];
	while(1) {
		
		fgets(buf,100,stdin);
		str_trim(buf, strlen(buf));
		sprintf(message, "Host: %s", buf);
        if(strncmp(message,"end",3)==0)
            break;

        //len=send(newsockfd, message, 150,0);
        /*if(len==-1)
        {
            printf("Error in sending");
            exit(1);
        }*/
		
		for (int j = 0; j < maxClients; j++) {
				send(clientSocket[j] , message , strlen(message) , 0 );
				
			}
	}
}

int main(int argc, char *argv[])
{
  pthread_t t1;
  int opt = 1;
  int s, sd, i, activity, valread;
  int sockfd, clilen;
  
  struct sockaddr_in cli_addr, serv_addr;
  int port;
  char string[MAX_SIZE];
  char databuf[1025];

  fd_set readfds; //set of socket descriptors
  
  for (i = 0; i < maxClients; i++) //initialize all clients to 0 
    {  
        clientSocket[i] = 0;  
    }  
  
  /* command line: server [port_number] */

  if(argc == 2)
     sscanf(argv[1], "%d", &port); /* read the port number if provided */
  else
     port = SERV_TCP_PORT;

  /* open a TCP socket (an Internet stream socket) */
  //sockfd is master socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
     perror("can't open stream socket");
     exit(1);
  }
  
  //allows master socket to allow multiple connections
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
  {
	  perror("setockopt");
	  exit(1);
  }

  /* bind the local address, so that the client can send to server */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     perror("can't bind local address");
     exit(1);
  }

  /* listen to the socket */
  if (listen(sockfd, 3) < 0) {
	  perror("listen failure");
	  exit(1);
  }
  
  clilen = sizeof(cli_addr);
  
  while(1) {
	  
	  FD_ZERO(&readfds); //clear socket set
	  
	  FD_SET(sockfd, &readfds); //add master to set
	  maxSd = sockfd;
	  
	  for (i = 0; i < maxClients; i++) {
		  
		  sd = clientSocket[i];
		  
		  if(sd > 0) {
			  FD_SET(sd, &readfds);
		  }
		  
		  if(sd > maxSd) {
			  maxSd = sd;
		  }
	  }
	  
	  //wait indefinitely for activity from a socket
	  activity = select(maxSd + 1, &readfds, NULL, NULL, NULL);
	  
	  if ((activity < 0) && (errno!=EINTR))
	  {
		  printf("select error\n");
	  }
	  
	  //
	  if (FD_ISSET(sockfd, &readfds))
	  {
		  if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen)) < 0)  //change?
		  {
			  perror("accept error");
			  exit(1);
		  }
		  
		  //printf("New connection\n"); 
		  
		  len=send(newsockfd,"***Welcome to the chatroom!***",50,0);
		  
		  for (i = 0; i < maxClients; i++)  
          {  
                //if position is empty 
              if( clientSocket[i] == 0 )  
              {  
                  clientSocket[i] = newsockfd;  
                  //printf("Adding to list of sockets as %d\n" , i);
                         
                  break;  
              }  
          }  
	  }
	  
	  s = pthread_create(&t1, NULL, sendMessage, NULL);
    if ( s != 0 ) //if creating the new thread is successful, then s will be zero.
    {
        perror("Thread create error");
    }
	
	  
	  for (i = 0; i < maxClients; i++)
	  {
		  
		  sd = clientSocket[i];
		  
		  if (FD_ISSET(sd, &readfds))
		  {
			  if ((valread = read( sd , databuf, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&cli_addr , \
                        (socklen_t*)&clilen);  
                    //printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(cli_addr.sin_addr) , ntohs(cli_addr.sin_port));  
                    //printf("A client disconnected");
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    clientSocket[i] = 0;  
                }  
                     
                //Echo back the message that came in 
                else 
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 

                    databuf[valread] = '\0';  
					
					//send to all but one who wrote it
					
					for (int j = 0; j < maxClients; j++) {
						if (clientSocket[j] != sd ) {
							send(clientSocket[j] , databuf , strlen(databuf) , 0 );
						}
					}
					
					 
					printf("%s\n", databuf);
					

					 
				  
                }  
		  }
	  }
	  
	  
	  
  }
  
  
  close(newsockfd);
}


//reflect code back to all clients
/*
TO RUN THIS CODE:
	gcc server.c -o server
	./server 7777



	NOTE: 7777 is the port number

*/
