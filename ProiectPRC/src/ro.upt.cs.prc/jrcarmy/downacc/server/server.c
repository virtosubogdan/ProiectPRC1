#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
/**
 * @since 28/02/2012
 * @author bufu und ady bicepsosu
 * @bla uatever...
 */

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5

int main(int argc, char* argv[]) {
	int hSocket, hServerSocket; /* handle to socket */
	//struct hostent* pHostInfo; /* holds info about a machine */
	struct sockaddr_in Address; /* Internet socket address stuct */
	int nAddressSize = sizeof(struct sockaddr_in);

	int nHostPort;

	if (argc < 2) {
		printf("\nUsage: server host-port\n");
		return 0;
	} else {
		nHostPort = atoi(argv[1]);
	}

	printf("\nStarting server");

	printf("\nMaking socket");
	/* make a socket */
	hServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (hServerSocket == SOCKET_ERROR) {
		printf("\nCould not make a socket\n");
		return 0;
	}

	/* fill address struct */
	Address.sin_addr.s_addr = INADDR_ANY;
	Address.sin_port = htons(nHostPort);
	Address.sin_family = AF_INET;

	printf("\nBinding to port %d", nHostPort);

	/* bind to a port */
	if (bind(hServerSocket, (struct sockaddr*) &Address,
			sizeof(Address)) == SOCKET_ERROR) {
		printf("\nCould not connect to host\n");
		return 0;
	}
	/*  get port number */
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

	printf("\nMaking a listen queue of %d elements", QUEUE_SIZE);
	/* establish listen queue */
	if (listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR) {
		printf("\nCould not listen\n");
		return 0;
	}

	for (;;) {
		printf("\nWaiting for a connection\n");
		/* get the connected socket */
		hSocket = accept(hServerSocket, (struct sockaddr*) &Address,
				(socklen_t *) &nAddressSize);

		printf("\nGot a connection");

		int req;
		int filenameSize;
		int resp;

		read(hSocket, &req, 4);
		read(hSocket, &filenameSize, 4);

		char filename[filenameSize];
		read(hSocket, &filename, filenameSize);


		strcat(argv[2],filename);

		struct stat fs;
		if(stat(argv[2],&fs) < 0)resp=-1;
		else resp=fs.st_size;

		if(req==0)// iterogare existenta fisier
			write(hSocket, resp, 8);


		printf("\nClosing the socket");
		/* close socket */
		if (close(hSocket) == SOCKET_ERROR) {
			printf("\nCould not close socket\n");
			return 0;
		}
	}
}

