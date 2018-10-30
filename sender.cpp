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
#include <thread>

using namespace std;

#define PORT	 8080 
#define MAX_DATA_SIZE 1024
#define DATA_FRAME_SIZE 1034
#define ACK_FRAME_SIZE 6
#define TIMEOUT 10
#define RECVFROM_TIMEOUT 4294967295


pthread_mutex_t lockQueueRecv;

struct sockaddr_in getServAddrClient (char *IPDestination, int port) {
	struct sockaddr_in	 servaddr; 
	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = inet_addr(IPDestination);

	return servaddr;
}

void fillBuffer(FILE* file, deque<frame>& buffer, int maxBuff, bool& isEOF, unsigned int& seqNum) {
	unsigned char c;

	while (buffer.size() < maxBuff) {
        frame tempFrame;
        int counter = 0;

		while (!isEOF && counter < MAX_DATA_SIZE) {
            if (fscanf(file, "%c", &c) != EOF) {
                tempFrame.data[counter] = c;
                counter++;
            } else {
                isEOF = true;
                tempFrame.SOH = 0;
            }
		}

        tempFrame.sequenceNumber = seqNum;
        tempFrame.dataLength = counter;
        tempFrame.checksum = calculateChecksum(tempFrame.SOH, tempFrame.sequenceNumber, tempFrame.dataLength, tempFrame.data);

        buffer.push_back(tempFrame);
	}
}

void fetchACK(bool *isSentAll, int *sockfd, struct sockaddr_in *servaddr, queue<ack> *bufferACK) {
    unsigned int n, len; 
    unsigned char bufferAck[ACK_FRAME_SIZE];
	ack tempACK;
 
    while (!(*isSentAll)) {
        n = recvfrom(*sockfd, (char *)bufferAck, ACK_FRAME_SIZE, MSG_WAITALL, (struct sockaddr *) servaddr, &len);

		tempACK = convertToAck(bufferAck);
		pthread_mutex_lock(&lockQueueRecv); 
		if (n != RECVFROM_TIMEOUT) {
			(*bufferACK).push(tempACK);
		}
		pthread_mutex_unlock(&lockQueueRecv); 
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

	// Deklarasi variabel
	deque<frame> bufferFrame;
	queue<ack> bufferACK;
	bool isSentAll = false;
    bool isEOF = false;
	ack tempACK;
	unsigned int seqNum = 0;
	unsigned int LAR = -1;
	unsigned int LFS = -1;

	fillBuffer(file, bufferFrame, bufferSize, isEOF, seqNum);

    struct timeval tv;
    tv.tv_sec = 5;  /* 30 Secs Timeout */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

	std::thread t1(fetchACK, &isSentAll, &sockfd, &servaddr, &bufferACK);

	while (!isSentAll) {
		//cek isi bufferACK
		pthread_mutex_lock(&lockQueueRecv); 
		for (int i = 0; i < bufferACK.size(); i++) {
			tempACK = bufferACK.front();
			bufferACK.pop();

			bufferFrame[tempACK.nextSequenceNumber-LAR-2].acked = true;
		}
		
		pthread_mutex_unlock(&lockQueueRecv); 
		//pop frame paling kiri yang udah acked
		while (bufferFrame.front().acked) {
			
			bufferFrame.pop_front();
			LAR++;
		}
		//cek isi bufferFrame
		for(int i = 0; i < windowSize && i < bufferFrame.size(); i++)
		{
			if (!bufferFrame[i].acked) {
				if (bufferFrame[i].timeStamp == -1) {
					bufferFrame[i].timeStamp = time(0) + TIMEOUT;
					sendto(sockfd, (const char *)convertToDataFrame(bufferFrame[i]), DATA_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 	
					if (bufferFrame[i].sequenceNumber > LFS) {
						LFS = bufferFrame[i].sequenceNumber;
					}
				} else if (bufferFrame[i].timeStamp < time(0)) {
					bufferFrame[i].timeStamp = time(0) + TIMEOUT;
					sendto(sockfd, (const char *)convertToDataFrame(bufferFrame[i]), DATA_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 	
				}
			}
		}
		if (!isEOF) {
			fillBuffer(file, bufferFrame, bufferSize, isEOF, seqNum);
		}
		if (bufferFrame.empty() && isEOF) {
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