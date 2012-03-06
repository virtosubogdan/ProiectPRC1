#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

/*
 * @since 28/02/2012
 * @author bufu
 */
#define ERROR        -1
#define ERROR_PARAMETRII 1
#define ERROR_SERVER 2
#define ERROR_CREATE 3

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
typedef struct downloadS {
	servInfo *serv;
	int file;
	int position;
	int size;
} down;
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
int download(down download, char *fisier) {
	printf("download cu %s,%d\n", download.serv->nume, download.serv->port);
	printf("%s\n", fisier);
	int socket = getSocket(*download.serv);
	int cerere = 1, lungimeNume = strlen(fisier) + 1;
	if (socket < 0)
		return 1;
	printf("wrote %d bytes\n", (int) write(socket, &cerere, 4));
	printf("wrote %d bytes\n", (int) write(socket, &lungimeNume, 4));
	printf("wrote %d bytes\n", (int) write(socket, fisier, lungimeNume));
	printf("cer dimensiunea %d\n", download.size);
	printf("wrote %d bytes\n", (int) write(socket, &download.size, 4));
	printf("incepand cu pozitia %d\n", download.position);
	printf("wrote %d bytes\n", (int) write(socket, &download.position, 4));
	char buffer[128];
	int index, nrCitiri = download.size / 128;
	for (index = 0; index < nrCitiri; index++) {
		int citit = read(socket, &buffer, 128);
		printf("citit %d\n", citit);
		//printf("am primit %s\n", buffer);
		if ((lseek(download.file, download.position + index * 128, SEEK_SET))
				== -1) {
			printf("Eroare deplasament(lseek)\n");
			return 2;
		}
		int scris = write(download.file, buffer, citit);
		printf("am scris in fisier %d bytes\n", scris);
	}
	return 0;
}
/**
 * Interogam serverele sa vedem daca au fisierul si dimensiunea corespunde
 * @return dimensiunea fisierului , -1 altfel
 */
int interogareSecventiala(int nrServere, servInfo servere[], char *fisier) {
	printf("Interogare %d servere\n", nrServere);
	int hSocket;
	int index, lungimeNume = strlen(fisier) + 1;
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
		printf("citit %d\n", (int) read(hSocket, &citit, 4));
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

int getServereActive(int nrServere, servInfo servere[]) {
	int index, nr = 0;
	for (index = 0; index < nrServere; index++)
		if (servere[index].stare == 1)
			nr++;
	return nr;
}
/**
 * Start client
 */
void startClient(int nrServere, int segmente, servInfo servere[], char *fisier) {
	printf("Starting Jurca's Army client\n");
	int index, indexSeg, fisierDownloadat = 0;
	int dimensiune, dimPerThread;
	int nrTrd = 0;
	int file;
	down task[100];
	if ((dimensiune = interogareSecventiala(nrServere, servere, fisier))
			== -1) {
		printf("Serverele returneaza dimensiuni eronate!\n");
		exit(ERROR_SERVER);
	}
	printf("Dimensiune fisier %s:%d\n", fisier, dimensiune);
	if ((file = open(fisier, O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		printf("Unable to create file:%s\n", fisier);
		exit(ERROR_CREATE);
	}
	//int servereActive;
	//while ((servereActive = getServereActive(nrServere, servere)) > 0
	//		&& !fisierDownloadat) {
	dimPerThread = dimensiune / (nrServere * segmente);
	for (index = 0; index < nrServere; index++) {
		if (servere[index].stare == 1) {
			for (indexSeg = 0; indexSeg < segmente; indexSeg++) {
				task[nrTrd].file = file;
				task[nrTrd].serv = &servere[index];
				task[nrTrd].position = nrTrd * dimPerThread;
				task[nrTrd].size = dimPerThread;
				if (fork() == 0) {
					exit(download(task[nrTrd], fisier));
				}
				nrTrd++;
			}
		}
	}
	fisierDownloadat = 1;
	int returnat, exitStatus;
	for (index = 0; index < nrTrd; index++) {
		wait(&returnat);
		exitStatus = WEXITSTATUS(returnat);
		if (exitStatus != 0)
			fisierDownloadat = 0;
		printf("Copilul a returnat %d\n", exitStatus);
	}
	//}
	if (fisierDownloadat)
		printf("Am downloadat fisierul!");
	else
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
	startClient(nrServere, nrSegmente, servere, strNumeFisier);

	return 0;
}
