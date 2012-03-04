#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/**
 * @since 28/02/2012
 * @author bufu
 */

#define ERROR        -1
#define QUEUE_SIZE          5
#define HOST_PORT			1025

void cerereInterogare(int nSocket) {
	printf("ajuns la cerere interogare");
}

void cerereDownload(int nSocket) {
	printf("ajuns la cerere download");
}

void trateazaSocket(int nSocket) {
	int nIdCerere;
	printf("read %d bytes\n", (int) read(nSocket, &nIdCerere, 4));
	printf("citit %d\n", nIdCerere);
	if (close(nSocket) == ERROR) {
		printf("\nCould not close socket in trateazaSocket\n");
	}
}

int main(int argc, char* argv[]) {
	int hClientSocket, hServerSocket; /* handle to socket */
	//struct hostent* pHostInfo; /* holds info about a machine */
	struct sockaddr_in Address; /* Internet socket address stuct */
	int nAddressSize = sizeof(struct sockaddr_in);

	printf("Starting Jurca's Army server\n");

	if ((hServerSocket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
		printf("Could not make a socket\n");
		return 1;
	}

	Address.sin_addr.s_addr = INADDR_ANY;
	Address.sin_port = htons(HOST_PORT);
	Address.sin_family = AF_INET;

	if (bind(hServerSocket, (struct sockaddr*) &Address,
			sizeof(Address)) == ERROR) {
		printf("Could not connect bind socket to port %d\n", HOST_PORT);
		return 0;
	}
	/*  get port number
	 getsockname(hServerSocket, (struct sockaddr *) &Address,
	 (socklen_t *) &nAddressSize);
	 printf("opened socket as fd (%d) on port (%d) for stream i/o\n",
	 hServerSocket, ntohs(Address.sin_port));

	 printf(
	 "Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n",
	 Address.sin_family, Address.sin_addr.s_addr,
	 ntohs(Address.sin_port));
	 */

	if (listen(hServerSocket, QUEUE_SIZE) == ERROR) {
		printf("Could not listen\n");
		return 0;
	}
	int nProcId;
	for (;;) {
		printf("Waiting for new connection\n");

		hClientSocket = accept(hServerSocket, (struct sockaddr*) &Address,
				(socklen_t *) &nAddressSize);
		if ((nProcId = fork()) == ERROR) {
			printf("Error on fork");
			if (close(hClientSocket) == ERROR)
				printf("Continued error on socked closing");
			break;
		}
		//write(hSocket, pBuffer, strlen(pBuffer) + 1);
		/* read from socket into buffer */
		//read(hSocket, pBuffer, BUFFER_SIZE);
		if (nProcId == 0) {
			if (close(hServerSocket) == ERROR) {
				printf("\nCould not close serverSocket in child\n");
				return 0;
			}
			trateazaSocket(hClientSocket);
			exit(0);
		}

		/*
		 * TODO: nu asteptam dupa fii, desi cred ca terminarea lor ar trebui prinsa cu
		 * semnale, adica instalam o rutina de pentru semnalul SIGCHLD ca sa afisam si sa preluam starea
		 */
		if (close(hClientSocket) == ERROR) {
			printf("\nCould not close socket in server\n");
			return 0;
		}
	}

	return 0;
}

