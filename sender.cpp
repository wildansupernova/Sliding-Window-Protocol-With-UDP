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
#include <queue>
#include "utility.cpp"
#include <queue>

using namespace std;

#define PORT	 8080 
#define MAX_DATA_SIZE 1024
#define DATA_FRAME_SIZE 1034
#define TIMEOUT 10

struct sockaddr_in getServAddrClient (char *IPDestination, int port) {
	struct sockaddr_in	 servaddr; 
	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = inet_addr(IPDestination);

	return servaddr;
}

void fillBuffer(FILE* file, queue<char>& buffer, int maxBuff) {
	unsigned char c;

	while (buffer.size < maxBuff) {
		if (fscanf(file, "%c", &c) != EOF) {
			buffer.push(c);
		}
	}
}

void loadFrameFromBuffer(deque<frame>& bufferFrame, queue<char>& bufferChar, unsigned int& seqNum, int maxFrame) {
	bool empty = false;
	int counter;

	while (!empty && bufferFrame.size() < maxFrame) {
		frame tempFrame;
		counter = 0;

		while (!empty && counter < MAX_DATA_SIZE) {
			tempFrame.data[counter] = bufferChar.front();
			bufferChar.pop();
			counter++;
			
			if (bufferChar.empty()) {
				empty = true;
			}
		}

		tempFrame.SOH = 0x1;
		tempFrame.sequenceNumber = seqNum;
		tempFrame.dataLength = counter;
		tempFrame.checksum = calculateChecksum(tempFrame.SOH, tempFrame.sequenceNumber, tempFrame.dataLength, tempFrame.data);

		seqNum++;

		bufferFrame.push_back(tempFrame);
	}
}

// Driver code 
int main(int argc, char* argv[]) { 
	//Memasukkan argumen ke variabel
	char *fileName = argv[1];
	int windowSize = atoi(argv[2]); //frame
	int bufferSize = atoi(argv[3]); //byte
	char *IPDestination = argv[4];
	int portDestination = atoi(argv[5]);
	int sockfd; 

	struct sockaddr_in servaddr = getServAddrClient(IPDestination,portDestination); 

	// Bikin soket
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	}

	// Buka stream file
	FILE* file = fopen(fileName, "r");
	if (file == NULL) {
		exit(0);
	}

<<<<<<< HEAD
	unsigned char buffer[bufferSize];
	queue<char> buffer;
	int counterBuffer = 0;
=======
	// Deklarasi variabel
	queue<char> bufferChar;
	deque<frame> bufferFrame;
>>>>>>> 499b534a8631c2ab7e449d106c3f88a09fe1566c
	bool isSentAll = false;
	unsigned int seqNum = 0;
	unsigned int LAR = -1;
	unsigned int LFS = -1;

	fillBuffer(file, bufferChar, bufferSize);
	loadFrameFromBuffer(bufferFrame, bufferChar, seqNum, windowSize);

	while (!isSentAll) {
		//cek isi bufferFrame
		for(int i = 0; i < bufferFrame.size(); i++)
		{
			if (!bufferFrame[i].acked) {
				if (bufferFrame[i].timeStamp == -1 || bufferFrame[i].timeStamp < time(0)) {
					bufferFrame[i].timeStamp = time(0) + TIMEOUT;
					sendto(sockfd, (const char *)convertToDataFrame(bufferFrame[i]), DATA_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 	
				} 
			}
		}
		
		fillBuffer(file, bufferChar, bufferSize);
		loadFrameFromBuffer(bufferFrame, bufferChar, seqNum, windowSize);
		if (bufferFrame.size() == 0 ) {
			isSentAll = true;
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