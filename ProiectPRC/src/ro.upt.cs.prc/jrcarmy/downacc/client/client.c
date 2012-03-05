#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * @since 28/02/2012
 * @author bufu
 */
#define ERROR        -1
#define SERVER_PORT   1025
#define ERROR_PARAMETRII 1

struct serverInfo {
	char nume[128];
	int port;
};
/**
 * @param argv[1] numeFisier
 * @param argv[2] nrSegmente
 * @param argv[3] numeserver:nrport
 * @optionali argv[4]-argv[12] numeserver:port aditionali
 */
int main(int argc, char* argv[]) {
	char strNumeFisier[512];
	int nrSegmente;
	if (argc < 4) {
		printf(
				"usage: numeFisier nrSegmente numeServer:port [..numeServer:port]");
		return ERROR_PARAMETRII;
	}
	if (strlen(argv[1]) > 511) {
		printf("Filename too long");
		return ERROR_PARAMETRII;
	}
	strcpy(strNumeFisier, argv[1]);
	nrSegmente = atoi(argv[2]);
	if (nrSegmente < 1 || nrSegmente > 100) {
		printf("Invalid segment number");
		return ERROR_PARAMETRII;
	}
	int nrServere = argc - 3;
	struct serverInfo servere[nrServere];
	int index, cIndex, nameLength;
	char *buffer;
	for (index = 3; index < argc; index++) {
		cIndex = 0;
		nameLength = strlen(argv[index]);
		buffer = argv[index];
		while (buffer[cIndex] != ':' && cIndex < nameLength) {
			servere[index - 3].nume[cIndex] = buffer[cIndex++];
		}
		servere[index - 3].nume[cIndex] = '\0';
		cIndex++;
		if (cIndex >= nameLength) {
			printf("Invalid server name");
			return ERROR_PARAMETRII;
		}
		servere[index - 3].port = atoi(buffer + cIndex);
		printf("server: %s cu port %d \n", servere[index - 3].nume,
				servere[index - 3].port);
	}

	printf("Starting Jurca's Army client\n");
	printf("nr servere:%d\n", nrServere);
	printf("de aici incolo cod de test cu trimitere int pe servere \n");
	int hSocket;
	struct hostent* pHostInfo;
	struct sockaddr_in Address;
	long nHostAddress;

	for (index = 0; index < nrServere; index++) {
		printf("starting send to %s\n", servere[index].nume);
		if ((hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR) {
			printf("Error creating socket socket\n");
			break;
		}

		pHostInfo = gethostbyname(servere[index].nume); //will work for string IPs
		memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);

		Address.sin_addr.s_addr = nHostAddress;
		Address.sin_port = htons(servere[index].port);
		Address.sin_family = AF_INET;

		if (connect(hSocket, (struct sockaddr*) &Address,
				sizeof(Address)) == ERROR) {
			printf("\nCould not connect to server\n");
			break;
		}
		int nr = index + 67;
		printf("write %d\n", (int) write(hSocket, &nr, 4));

		if (close(hSocket) == ERROR) {
			printf("\nCould not close socket\n");
			break;
		}
	}
	printf("end client\n");
	return 0;
}
