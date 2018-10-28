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
using namespace std;
#define PORT	 8080 
#define MAXLINE 1024 

// Driver code 
int main() { 
	int sockfd; 
	char buffer[MAXLINE]; 
	string data = "Hello from Math";
	const char *hello = data.c_str(); 
	const char *hello2 = data2.c_str();
	struct sockaddr_in	 servaddr; 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_addr.s_addr = inet_addr("192.168.88.174");
	
	unsigned int n, len; 
	
	while (true) {
	sendto(sockfd, (const char *)hello, strlen(hello), 
		MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr));
	sendto(sockfd, (const char *)hello2, strlen(hello2), 
		MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr));  
	}
		
	n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
	buffer[n] = '\0'; 
	printf("Server : %s\n", buffer); 

	close(sockfd); 
	return 0; 
}