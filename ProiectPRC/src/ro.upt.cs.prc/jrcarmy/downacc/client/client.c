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
#define ERROR_SERVER 2

typedef struct serverInfoStruct {
	char nume[128];
	int port;
	/*
	 *0 -neintitalizat
	 *1 -are fisierul si e util
	 *2 -inutil
	 */
	int stare;
} servInfo;

/**
 * Returns a socket to the given server
 */
int getSocket(servInfo server) {
	int hSocket;
	struct hostent* pHostInfo;
	struct sockaddr_in Address;
	long nHostAddress;
	if ((hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == ERROR) {
		printf("Error creating socket socket\n");
		return -1;
	}
	pHostInfo = gethostbyname(server.nume); //will work for string IPs
	memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);
	Address.sin_addr.s_addr = nHostAddress;
	Address.sin_port = htons(server.port);
	Address.sin_family = AF_INET;
	if (connect(hSocket, (struct sockaddr*) &Address, sizeof(Address)) == ERROR) {
		printf("\nCould not connect to server\n");
		return -1;
	}
	return hSocket;
}

/**
 * Interogam serverele sa vedem daca au fisierul si dimensiunea corespunde
 * @return dimensiunea fisierului , -1 altfel
 */
int interogareSecventiala(int nrServere, servInfo servere[], char *fisier) {
	printf("Interogare %d servere\n", nrServere);
	int hSocket;
	int index, lungimeNume = strlen(fisier);
	int cerere = 0;
	int lungimeFisier = -1, citit;
	for (index = 0; index < nrServere; index++) {
		printf("Starting interrogation for %s\n", servere[index].nume);
		hSocket = getSocket(servere[index]);
		if (hSocket == -1)
			break;
		printf("wrote %d bytes\n", (int) write(hSocket, &cerere, 4));
		printf("wrote %d bytes\n", (int) write(hSocket, &lungimeNume, 4));
		printf("wrote %d bytes\n", (int) write(hSocket, fisier, lungimeNume));
		read(hSocket, &citit, 4);
		if (citit != -1) {
			if (lungimeFisier == -1) {
				lungimeFisier = citit;
				servere[index].stare = 1;
			} else if (citit != lungimeFisier) {
				if (close(hSocket) == ERROR) {
					printf("\nCould not close socket\n");
				}
				return -1;
			} else {
				servere[index].stare = 1;
			}
		} else {
			servere[index].stare = 2;
		}
		if (close(hSocket) == ERROR) {
			printf("\nCould not close socket\n");
		}
	}
	return lungimeFisier;
}

/**
 * Start client
 */
void startClient(int nrServere, servInfo servere[], char *fisier) {
	printf("Starting Jurca's Army client\n");
	int fisierDownloadat = 0;
	int servereActive = 1; //to get to the first iteration
	int dimensiune;
	if ((dimensiune = interogareSecventiala(nrServere, servere, fisier))
			== -1) {
		printf("Serverele returneaza dimensiuni eronate!\n");
		exit(ERROR_SERVER);
	}
	printf("Dimensiune fisier %s:%d\n", fisier, dimensiune);
	//TODO:aici facem fisierul in care scriem si care va fi pasat in fii
	while (servereActive > 0 && !fisierDownloadat) {
		//defapt pornim copii care sa downloadeze fisierul
		servereActive = 0;
	}
	if (servereActive == 0)
		printf("Nu exista servere de unde sa downloadam fisierul\n");
	printf("End client\n");
}

/**
 * @param argv[1] numeFisier
 * @param argv[2] nrSegmente
 * @param argv[3] numeserver:nrport
 * @optionali argv[4]startClient-argv[12] numeserver:port aditionali
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
	servInfo servere[nrServere];
	int index, cIndex, nameLength;
	char *buffer;
	for (index = 3; index < argc; index++) {
		cIndex = 0;
		nameLength = strlen(argv[index]);
		buffer = argv[index];
		while (buffer[cIndex] != ':' && cIndex < nameLength) {
			servere[index - 3].nume[cIndex] = buffer[cIndex];
			cIndex++;
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
	startClient(nrServere, servere, strNumeFisier);

	return 0;
}
