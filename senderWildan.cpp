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
#include <thread> 

#define PORT	 8080 
#define MAX_DATA_SIZE 1024
#define DATA_FRAME_SIZE 1034
#define TIMEOUT 10

using namespace std;


pthread_mutex_t lockQueueBuffer; 
pthread_mutex_t lockQueueFrame; 
pthread_mutex_t lockQueueReceiver;
 

struct sockaddr_in getServAddrClient (char *IPDestination, int port) {
	struct sockaddr_in	 servaddr; 
	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = inet_addr(IPDestination);

	return servaddr;
}

void fillBuffer(FILE* file, queue<frame>& buffer, int maxBuff, bool &isEOF) {
	unsigned char c;

    while (!isEOF) {
		unsigned int counter = 0;
        pthread_mutex_lock(&lockQueueBuffer);
		frame tempFrame;
		while (buffer.size() < maxBuff && !isEOF) {
            if (fscanf(file, "%c", &c) != EOF) {
                tempFrame.data[counter] = c;
				counter++;
            } else {
                isEOF = true;
            }
        }
		tempFrame.dataLength = counter;
		buffer.push(tempFrame);
        pthread_mutex_unlock(&lockQueueBuffer);
    }
}

void loadFrameFromBuffer(deque<frame>& bufferWindow, queue<frame>& bufferFile, unsigned int& seqNum, int maxFrameWindow) {
	bool empty = false;
	int counter;
	pthread_mutex_lock(&lockQueueBuffer);
	if (!bufferFile.empty()) {
		while (!bufferFile.empty() && bufferWindow.size() < maxFrameWindow) {
			frame tempFrame = bufferFile.front();
			bufferFile.pop();
			tempFrame.SOH = 0x1;
			tempFrame.sequenceNumber = seqNum;
			tempFrame.checksum = calculateChecksum(tempFrame.SOH, tempFrame.sequenceNumber, tempFrame.dataLength, tempFrame.data);
			seqNum++;
			bufferWindow.push_back(tempFrame);
		}
	}
	pthread_mutex_unlock(&lockQueueBuffer);
}

void receiveACK (int &sockfd,queue<ack> &bufferReceiveAck, bool &isSentAll,struct sockaddr_in &servaddr) {
    while (!isSentAll) {
        unsigned int n, len;
        unsigned char buffer[DATA_FRAME_SIZE];
        n = recvfrom(sockfd, (char *)buffer, DATA_FRAME_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        ack tempAck = convertToAck(buffer);

        pthread_mutex_lock(&lockQueueReceiver);
        bufferReceiveAck.push(tempAck);
        pthread_mutex_unlock(&lockQueueReceiver);
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

    //Init All
    // Deklarasi variabel
    queue<frame> bufferFrameFile;
    deque<frame> bufferFrame;
    queue<ack> bufferReceiveAck;
    bool isSentAll = false;
    bool isEOF = false;
    unsigned int seqNum = 0;
    unsigned int LAR = -1;
    unsigned int LFS = -1;


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

    //Urusan Thread
    thread bufferFile(fillBuffer, file, bufferFrameFile, bufferSize, isEOF);
	thread bufferReceive(receiveACK, sockfd, bufferReceiveAck, isSentAll, servaddr);
	
    while (!isSentAll) {
		
    }
	bufferFile.join();
	bufferReceive.join();
	close(sockfd); 
	fclose(file);
	return 0; 
}