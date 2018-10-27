// Client side implementation of UDP client-server model 
#include <bits/stdc++.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "utility.cpp"
using namespace std;
#define PORT	 8080 
#define MAXLINE 1024 

struct sockaddr_in getServAddrClient (char *IPDestination, int port) {
	struct sockaddr_in	 servaddr; 
	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = inet_addr(IPDestination);

	return servaddr;
}


// Driver code 
int main(int argc, char* argv[]) { 
	//Input Processing
	char *fileName = argv[1];
	int windowSize = atoi(argv[2]); //frame
	int bufferSize = atoi(argv[3]); //byte
	char *IPDestination = argv[4];
	int portDestination = atoi(argv[5]);

	int sockfd; 
	char buffer[MAXLINE]; 
	string data = "Hello from client";
	const char *hello = data.c_str(); 

	struct sockaddr_in servaddr = getServAddrClient(IPDestination,portDestination); 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	}

	FILE* file = fopen(fileName, "r");
	if (file == NULL) {
		exit(0);
	}

	unsigned char buffer[bufferSize];
	int counterBuffer = 0;
	bool isSentAll = false;
	while (!isSentAll) {
		if (counterBuffer < bufferSize) {

		}
	}	







	// unsigned int n, len; 
	
	// sendto(sockfd, (const char *)hello, strlen(hello), 
	// 	MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
	// 		sizeof(servaddr)); 		
	// n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
	// buffer[n] = '\0'; 
	// printf("Server : %s\n", buffer); 

	close(sockfd); 
	fclose(file);
	return 0; 
}