/*******************************************************************************
 * Name        : chatclient.c
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 05/07/21
 * Description : Chat client.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_client_socket() {
	int bytes_recvd;
	if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN + 1, 0)) < 0) {
		fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n",
				strerror(errno));
	} else if (bytes_recvd == 0) {
		fprintf(stderr, "\nConnection to server has been lost.\n");
		return 1;
	} 
	inbuf[bytes_recvd] = '\0';
	if (strcmp(inbuf, "bye") == 0) {
		printf("\nServer initiated shutdown.\n");
		return 1;
	}
	printf("\n%s", inbuf);
	fflush(stdout);
	return EXIT_SUCCESS;
}

int handle_stdin() {
	int gs_val;
	gs_val = get_string(outbuf, MAX_MSG_LEN);
		
	if (gs_val == 2) {
		printf("Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
		return 0;
	}
	if (strcmp(outbuf, "bye") == 0) {
		printf("Goodbye\n");
		return 1;
	}

	if (send(client_socket, outbuf, strlen(outbuf), 0) < 0) {
		fprintf(stderr, "Error: Failed to send message to server. %s.\n",
				strerror(errno));
		return 0;
	}
	
	return EXIT_SUCCESS;
}

int max(int x, int y) {
	if (x > y) {
		return x;
	} else {
		return y;
	}
}		

int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
		
	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int bytes_recvd, ip_conversion, retval = EXIT_SUCCESS;

	ip_conversion = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
	if (ip_conversion == 0) {
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
		return EXIT_FAILURE; 
	} else if (ip_conversion < 0) { 
		fprintf(stderr, "Error: Failed to convert IP address. %s.\n",
				strerror(errno));
		return EXIT_FAILURE;
	}
	
	int port = 0;
	char usage[] = "port number";
	if (!parse_int(argv[2], &port, usage)) {
		return EXIT_FAILURE;
	}
	 
	if (port < 1024 || port > 65535) {
		fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
		return EXIT_FAILURE;
	}
	printf("port: %d\n", port);
	
	int gs_val;
LOOP:
	printf("Enter your username: ");
	fflush(stdout);
	gs_val = get_string(username, MAX_NAME_LEN);
	if (gs_val == 1 || gs_val == 2) {
		printf("Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
		goto LOOP;
	}
	
	printf("Hello, %s. Let's try to connect to the server.\n\n", username);
	
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error: Failed to create socket. %s.\n",
				strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	memset(&server_addr, 0, addrlen);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	
	if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
		fprintf(stderr, "Error: Failed to connect to server. %s.\n",
				strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0) {
		fprintf(stderr, "Error: Failed to receive message from server. %s.\n",
				strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	} else if (bytes_recvd == 0) {
		fprintf(stderr, "All connections are busy. Try again later.\n");
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	inbuf[bytes_recvd] = '\0';
	printf("%s\n", inbuf);
	strcpy(outbuf, username);
	
	if (send(client_socket, outbuf, strlen(outbuf), 0) < 0) {
		fprintf(stderr, "Error: Failed to send username to server. %s.\n",
				strerror(errno));
		retval = EXIT_FAILURE;
		goto EXIT;
	}
	
	fd_set rset;
	printf("\n");
	while(1) {
		FD_ZERO(&rset);
		FD_SET(client_socket, &rset);
		FD_SET(STDIN_FILENO, &rset); 
		printf("[%s]: ", username);
		fflush(stdout);
		
		select(client_socket + 1, &rset, NULL, NULL, NULL);
		
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			if (handle_stdin() == 1){ 
				break;
			}
		}
		
		if (FD_ISSET(client_socket, &rset)) {
			if (handle_client_socket() == 1) {
				break;
			}	
			printf("\n");
		}
		fflush(stdout);
	}
EXIT:
	if (fcntl(client_socket, F_GETFD) >= 0) {
		close(client_socket);
	}
	return retval;
}
