#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <pthread.h>
#include "client.h"
#include "utils.h"

typedef struct sockaddr_in sockaddr_in;

static WSADATA wsa;
static SOCKET ServerSocket;
static sockaddr_in server;

static dataReceived* firstDataCollected = NULL;
static dataReceived* lastDataCollected = NULL;
int threadState = ACTIVE;

bool sendData(char* message) {
	char temp[2500];

	// Copy the message in a array with fixed length
	for (int i = 0; i < strlen(message) + 1; i++) {
		temp[i] = message[i];
	}

	// Send the message
	if (send(ServerSocket, temp, 2500, 0) < 0) {
		printf("\nFailed sending the message to the server!\n");
		return FALSE;
	}
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
		dataReceived dataRequested = {NULL, 0};
		return dataRequested;
	}

	// Get the data from the element
	char* dataContainer = firstDataCollected -> data;
	int dataLen = firstDataCollected -> length;

	// If the first element is the last element reset both
	if (firstDataCollected == lastDataCollected) {
		free(firstDataCollected);
		firstDataCollected = NULL;
		lastDataCollected = NULL;
		// Return the data requested
		dataReceived dataRequested = {dataContainer, dataLen};
		return dataRequested;
	}

	// Copy the address of the first address
	dataReceived* temp = firstDataCollected;

	// Make the second element the first
	firstDataCollected = firstDataCollected -> next;

    // Deallocate the first element
    free(temp);

	// Return the data requested
	dataReceived dataRequested = {dataContainer, dataLen};
	return dataRequested;
}

static void saveDataReceived(char* dataRecv, int dataLen) {
	// Create a new element
	dataReceived* newData = (dataReceived*) malloc(sizeof(dataReceived));

	// Set the len of the message
	newData -> length = dataLen + 1;

	// Set the given string in the data collection
	newData -> data = dataRecv; 

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

void* receiveData() {
	// Set the exit for the recursion
	if (!threadState) {
		return NULL;
	}

	int recv_size;
	char* response = (char*) calloc(2500, 1);

	if ((recv_size = recv(ServerSocket, response, 2500, 0)) == SOCKET_ERROR) {
		printf("\nFailed receiving the data!\n");
		pthread_exit(NULL);
		return receiveData();
	} else {	
		response = (char*) realloc(response, strlen(response) + 1);

		// Save the data received
		saveDataReceived(response, strlen(response));

		return receiveData();
	}
	return receiveData();
}

static SOCKET connectSocket(sockaddr_in server, char* ip, u_short port) {
	SOCKET temp;

	// Create the socket 
	if ((temp = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("\nCould not create socket : %d\n" , WSAGetLastError());
		return (SOCKET) NULL;
	}

	// Set the host info for connection (ip, family, port)
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// Connect to the server
	if (connect(temp, (struct sockaddr*) &server, sizeof(server)) < 0) {
		printf("\nFailed connecting to the server!\n");
		return (SOCKET) NULL;
	}

	return temp;
}

bool initClient() {
	// Init winsock2 and check if fails initializing it
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
		printf("\nFailed. Error Code : %d\n", WSAGetLastError());
		return FALSE;
	}

	// Regex to clear the terminal.
    printf("\e[1;1H\e[2J");

	// Request the ip address
	char ip_addrs[255];
	printf("\nInsert the ip address: ");
	fgets(ip_addrs, 255, stdin);
	char* temp = strchr(ip_addrs, '\n');
	if (temp != NULL)
		*temp = '\0';

	printf("\nTrying to connect to the server at ip address: %s!\n", ip_addrs);

	// Create the socket and connect it, also check for errors
	if ((ServerSocket = connectSocket(server, ip_addrs, 8080)) == ((SOCKET) NULL)) {
		return FALSE;
	}

	// Regex to clear the terminal.
    printf("\e[1;1H\e[2J");
	
	printf("\nWaiting the game master to select the game settings!");

	return TRUE;
}

void closeClient() {
	closesocket(ServerSocket);
	WSACleanup();
	return;
}