#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>
#include "message_enc.h"

#define TRUE   1  
#define FALSE  0
#define PORT 8080

#define PID_SIZE 7
#define MAX_CLIENTS 20
#define BUFFER_SIZE 1024


/*		   MESSAGE FORMAT BEFORE FORMATTING
 *
 *          -----------------------------
 *          | MESSAGE (SIZE IS MULTIPLE |
 *          | OF AES_BLOCK_SIZE         |
 *          -----------------------------
 *	
 *		   MESSAGE FORMAT AFTER FORMATTING
 *
 *  <------------------------> (AES_BLOCK_SIZE)
 *  ------------------------------------------------------
 *  | '\0'    |  PROCESS PID | MESSAGE (SIZE IS MULTIPLE |
 *  | (1 bit) |  (15 bits)   | OF AES_BLOCK_SIZE         |
 *  ------------------------------------------------------
 */

int reformat_message(int client_pid, char* buffer, int valread, int cfd) {
	int i;
	char pid [AES_BLOCK_SIZE];
	int enc_size = AES_BLOCK_SIZE;
	memset(pid, 0x0, sizeof(pid));
	
	/* Compute the pid and append the character '\0'
	 * (used to identify the beginning of the message) */
	sprintf(pid, "%d", client_pid);	
	for (i = AES_BLOCK_SIZE - 2; i >= 0; i--)
		pid[i+1] = pid[i];
	pid[0] = '\0';

	/* We must encrypt the added header (with the same key) so that when
	 * decrypted from the the client, we get all the information from the
	 * message without any extra effort */
	#if (USE_ENC == 1)
	encrypt_decrypt_message (ENCRYPT, pid, &enc_size, cfd);
	#endif

	for (i = valread-1; i >= 0; i--)
		buffer[i + AES_BLOCK_SIZE] = buffer[i];

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		buffer[i] = pid[i];

	/* Return the new message size*/
	return valread + AES_BLOCK_SIZE;
		 
}

void accept_new_client (int master_socket,
			int client_socket[MAX_CLIENTS],
			pid_t client_pid [MAX_CLIENTS]) {

	char *pid, *end;
	int pid_size, result, i;
	struct sockaddr_in address;
	int addrlen;
	int new_socket;

	/* Accept the new connection */
	if ((new_socket = accept(master_socket,
				(struct sockaddr *) &address,
				(socklen_t*) &addrlen)) < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	pid = calloc(PID_SIZE, sizeof(char));
	pid_size = read(new_socket, pid, PID_SIZE);

	if (pid_size == 0) {
		printf("Failed to receive the client's PID, connection aborted\n");
		close(new_socket);
		return;
	}

	/* Send Acknowledgement */
	send(new_socket, "ACK", 3, 0);

	result = strtol(pid, &end, 10);  

	if (end == pid) {
		printf("Could not convert the received PID");
		close(new_socket);
		return;
	}

	//inform user of socket number - used in send and receive commands  
	printf("New connection: [Socket FD :: %d ] [IP :: %s] [PID :: %d]\n", 
		   new_socket, inet_ntoa(address.sin_addr), result);

	//add new socket to array of sockets
	for (i = 0; i < MAX_CLIENTS; i++) { 
		if( client_socket[i] == 0 ) {
			client_socket[i] = new_socket;
			client_pid[i] = (pid_t) result;
			printf("Adding to list of sockets as %d\n" , i);
			return;   
		}
	}

	close(new_socket);
	printf("Cannot accept any more clients [Limit: %d clients]\n", MAX_CLIENTS);
}

void handle_client (int index, int cfd,
		   int client_socket[MAX_CLIENTS],
		   pid_t client_pid [MAX_CLIENTS]) {

	int valread, j, sd_r;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in address;
	int addrlen;

	if ((valread = read(client_socket[index], buffer, BUFFER_SIZE)) == 0) {
		/* A client disconnected */
		/* We use getoeername() to get the client's inforamtion
		 * given his FD */
		getpeername(client_socket[index], (struct sockaddr*) &address, (socklen_t*) &addrlen);
		printf("Host disconnected: [IP :: %s] [PID :: %d]\n",
				inet_ntoa(address.sin_addr) , client_pid[index]);

		// Close the socket and mark as 0 in list for reuse  
		close(client_socket[index]);   
		client_socket[index] = 0;
		client_pid[index] = 0;
	} else {
		valread = reformat_message(client_pid[index], buffer, valread, cfd);
		for (j = 0 ; j < MAX_CLIENTS; j++) 
			if(client_socket[j] > 0) 
				send(client_socket[j], buffer , valread , 0);
	}
}

int main(int argc , char *argv[])   
{   
	int opt = TRUE;
	int master_socket , addrlen , new_socket , client_socket[MAX_CLIENTS],
		activity, i, j, sd, sd_r;
	pid_t client_pid [MAX_CLIENTS];
	int max_sd, cfd;
	struct sockaddr_in address;
	
	cfd = open("/dev/crypto", O_RDWR, 0);

	/* Set of socket descriptors */
	fd_set readfds;

	/* Initialise all the client sockets */
	for (i = 0; i < MAX_CLIENTS; i++)
		client_socket[i] = 0;

	/* Create a master socket (The one listening for new
	 * connections) */
	if((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	/* Set master socket to allow multiple connections */
	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
				 (char *)&opt, sizeof(opt)) < 0 ) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* We set a time limit for the read operations in case the pid
	 authentication fails */
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if( setsockopt(master_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,
				sizeof tv) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Type of socket created */  
	address.sin_family = AF_INET;   
	address.sin_addr.s_addr = INADDR_ANY;   
	address.sin_port = htons( PORT );   

	//bind the socket to localhost port 8888  
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
	{   
		perror("bind failed");   
		exit(EXIT_FAILURE);   
	}   
	printf("Listener on port %d \n", PORT);   

	/* Specify a maximum of 3 pending connections
	 * for the master socket */ 
	if (listen(master_socket, 3) < 0)   
	{   
		perror("listen");   
		exit(EXIT_FAILURE);   
	}   

	//accept the incoming connection  
	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	while(TRUE)   
	{   
		/* Clear the socket set */  
		FD_ZERO(&readfds);   

		/* Add master socket to set */ 
		FD_SET(master_socket, &readfds);   
		max_sd = master_socket;   
 
		/* Add all the bound client sockets */
		for ( i = 0 ; i < MAX_CLIENTS ; i++)   
		{
			sd = client_socket[i];   
 
			if(sd > 0)
				FD_SET(sd , &readfds);

			if(sd > max_sd)
				max_sd = sd;
		}   

		/* Wait for an activity on one of the sockets */ 
		/* Timeout is NULL, so we wait indefinitely */  
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   

		/* If there was an error not caused by a signal,
		 * report the incident */
		if ((activity < 0) && (errno!=EINTR))
			printf("select error");

		/* Check the master socket for incoming connections */
		if (FD_ISSET(master_socket, &readfds)) {
			accept_new_client(master_socket, client_socket, client_pid);
		}

		/* Else activity was completed beacuse of some operation
		 * regarding a client socket */
		for (i = 0; i < MAX_CLIENTS; i++)
			if (FD_ISSET(client_socket[i], &readfds))
				handle_client (i, cfd, client_socket, client_pid);
	}   

	close(master_socket);
	close(cfd);
	return 0;   
}   

