#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <pthread.h>
#include "server.h"
#include "utils.h"

typedef struct sockaddr_in sockaddr_in;

/* -------------------- GLOBAL VARIABLES ----------------------------- */

static WSADATA wsa;
static SOCKET ServerSocket, clientSockets[3];
static sockaddr_in server_addr;
static char* ip_addrs[3];
static int clientsCount = 0;
static utilsStatus searchConnectionsStatus = SEARCHING;
static dataReceived* firstDataCollected = NULL;
static dataReceived* lastDataCollected = NULL;
static int threadState = ACTIVE;

/* ------------------------------------------------------------------- */

/// @brief Initialize the server.
/// @param server 
/// @param port 
/// @param backlog 
/// @return Return the status of the initialization.
static SOCKET initServer(sockaddr_in server, u_short port, int backlog) {
	SOCKET s;

	// Create a socket and check if fails creating it
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d\n", WSAGetLastError());
		return (SOCKET) NULL;
	}

	// Set the server info (family, address, port)
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);
	
	// Bind the socket and check if fails binding it
	if (bind(s, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR) {
		printf("\nBind failed with error code : %d\n", WSAGetLastError());
		return (SOCKET) NULL;
	}

	// Listen to incoming connections, with backlog (queue) limit of n connections
	listen(s, backlog);

	return s;
}

/// @brief Ask the user if he wants to end the search of players.
static void askToClose() {
	char confirm;
	printf("\n\nDo you want to end the search? (Y/N): ");
	scanf("%c", &confirm);

	{	
		// Clean the stdin
		unsigned char c;
		while((c = getc(stdin)) != EOF) {
			if(c == '\n') {
				break;          
			}
		}  
	}

	if (confirm == 'Y') {
		searchConnectionsStatus = END;
	} else if (confirm != 'N') {
		return askToClose();
	}

	return;
}

bool sendData(int clientIndex, char* message) {
	clientIndex--;
	char* temp = (char*) calloc(2500, 1);

	// Copy the message in a array with fixed length
	sprintf(temp, "%s", message);

	// Send the message
	if (send(clientSockets[clientIndex], temp, 2500, 0) < 0) {
		free(temp);
		printf("\nFailed sending the message to the client %d!\n", clientIndex + 1);
		return FALSE;
	}

	free(temp);

	return TRUE;
}

int getDataReceivedLen() {
	dataReceived* scan = firstDataCollected;
	int dataCollectedNum = 0;

	// Scan all the element in the list and count them
	while (scan != NULL) {
		scan = scan -> next;
		dataCollectedNum++;
	}

	return dataCollectedNum;
}

dataReceived getDataReceived() {
	// Check if there's something to retrive
	if (firstDataCollected == NULL) {
		// Return the data requested
		dataReceived dataRequested = {NULL, 0, -1};
		return dataRequested;
	}

	// Get the data from the element
	char* dataContainer = firstDataCollected -> data;
	int dataLen = firstDataCollected -> length;
	int clientId = firstDataCollected -> clientId;

	// If the first element is the last element reset both
	if (firstDataCollected == lastDataCollected) {
		free(firstDataCollected);
		firstDataCollected = NULL;
		lastDataCollected = NULL;
		// Return the data requested
		dataReceived dataRequested = {dataContainer, dataLen, clientId};
		return dataRequested;
	}

	// Copy the address of the first address
	dataReceived* temp = firstDataCollected;

	// Make the second element the first
	firstDataCollected = firstDataCollected -> next;

    // Deallocate the first element
    free(temp);

	// Return the data requested
	dataReceived dataRequested = {dataContainer, dataLen, clientId};
	return dataRequested;
}

static void saveDataReceived(char* dataRecv, int dataLen, int clientId) {
	// Create a new element
	dataReceived* newData = (dataReceived*) malloc(sizeof(dataReceived));

	// Set the len of the message
	newData -> length = dataLen + 1;

	// Set the given string in the data collection
	newData -> data = dataRecv; 

	newData -> clientId = clientId;

	// Set the next pointer as the last element
	newData -> next = NULL;

	// If the first element hasn't been created, create it
	if (firstDataCollected == NULL) {
		// Set the first element and the last element
		firstDataCollected = newData;

		// Set the last element equal to the first element as the list has only one element
		lastDataCollected = firstDataCollected;

		// Set the last element as the next one
		firstDataCollected -> next = lastDataCollected;
		
		return;
	}

	// The old last element points to the new
	lastDataCollected -> next = newData;

	// Set the last element as this new element
	lastDataCollected = newData;

	return;
}

void* receiveData(void* vargp) {
	// Store the value argument passed to this thread
    int clientId = *(int*) vargp;

	// Set the exit for the recursion
	if (!threadState) {
		return NULL;
	}

	int recv_size;
	char* response = (char*) calloc(2500, 1);

	if ((recv_size = recv(clientSockets[clientId], response, 2500, 0)) == SOCKET_ERROR) {
		printf("\nFailed receiving the data from the client %d!\n", clientId + 1);
		pthread_exit(NULL);
		return receiveData(vargp);
	} else {	
		response = (char*) realloc(response, strlen(response) + 1);

		// Save the data received
		saveDataReceived(response, strlen(response), clientId);

		return receiveData(vargp);
	}
	return receiveData(vargp);
}

int loadServer() {
	// Init winsock2 and check if fails initializing it
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
		printf("\nFailed. Error Code : %d\n", WSAGetLastError());
		return FALSE;
	}

	// Initialize the server
	if ((ServerSocket = initServer(server_addr, 8080, 3)) == ((SOCKET) NULL)) {
		printf("\nError: failed initializing the server!");
		return FALSE;
	}

	return TRUE;

} 

int createServerList() {
	int c = sizeof(sockaddr_in);
	SOCKET client;
	sockaddr_in client_addr;

	// Regex to clear the terminal.
    printf("\e[1;1H\e[2J");
	printf("\nWaiting the first user to connect...\n");
	
	// Wait till the number of the user connected is reached
	do {	
		// Check if the connection is made by an invalid socket.
		while ((client = accept(ServerSocket, (struct sockaddr*) &client_addr, &c)) != INVALID_SOCKET) {
			// Add the client to the list
			clientSockets[clientsCount] = client;

			// Add the new user to the list
			ip_addrs[clientsCount] = inet_ntoa(client_addr.sin_addr);
			clientsCount++;
			break;
		}

		// Regex to clear the terminal.
		printf("\e[1;1H\e[2J");

		printf("\n-------------------- USERS CONNECTED --------------------\n");

		// Print the users currently connected
		for (int i = 0; i < clientsCount; i++) {
			printf("\n%d) Ip: %s ;", i + 1, ip_addrs[i]);
		}

		askToClose();

	} while ((clientsCount < 3) && (searchConnectionsStatus != END));

	return clientsCount;
}

void closeServer() {
	printf("\nClosing the server!");
	closesocket(ServerSocket);
	WSACleanup();
	return;
}
