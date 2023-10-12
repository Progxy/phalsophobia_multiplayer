#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "utils.h"

typedef struct sockaddr_in sockaddr_in;

static int socket_desc;
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
	if (send(socket_desc, temp, 2500, 0) < 0) {
		printf("\nFailed sending the message to the server!\n");
		return FALSE;
	}
    
	return TRUE;
}

char* getDataReceived() {
	// Check if there's something to retrive
	if (firstDataCollected == NULL) {
		// Return the data requested
		return NULL;
	}

	// Get the data from the element
	char* dataContainer = firstDataCollected -> data;

	// If the first element is the last element reset both
	if (firstDataCollected == lastDataCollected) {
		free(firstDataCollected);
		firstDataCollected = NULL;
		lastDataCollected = NULL;
		// Return the data requested
		return dataContainer;
	}

	// Copy the address of the first address
	dataReceived* temp = firstDataCollected;

	// Make the second element the first
	firstDataCollected = firstDataCollected -> next;

    // Deallocate the first element
    free(temp);

	// Return the data requested
	return dataContainer;
}

static void saveDataReceived(char* dataRecv) {
	// Create a new element
	dataReceived* newData = (dataReceived*) malloc(sizeof(dataReceived));

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

	char* response = (char*) calloc(2500, 1);

	int total_received = 0, received = 0;
	do {
		received = recv(socket_desc, response + total_received, 2500 - total_received, 0);
		if (received == -1) {
			printf("\nrecv() failed.");
			return receiveData();
		}

		if (received > 0) {
			total_received += received;
		}

	} while (received != 0);

	// Save the data received
	response[total_received] = 0;
	saveDataReceived(response);
	
	return receiveData();
}

bool initClient() {
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

	// Create the socket 
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("\nCould not create socket\n");
		return FALSE;
	}

	// Set the host info for connection (ip, family, port)
	server.sin_addr.s_addr = inet_addr(ip_addrs);
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);

	// Connect to the server
	if (connect(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
		printf("\nFailed connecting to the server!\n");
		return FALSE;
	}
	
	// Regex to clear the terminal.
    printf("\e[1;1H\e[2J");

	printf("\x1b[1;33m\nWaiting the game master to select the game settings...\x1b[1;0m");

	return TRUE;
}

void closeClient() {
	close(socket_desc);
	return;
}
