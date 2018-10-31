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

pthread_mutex_t lockQueueRecv;

#define PORT     8080 
#define MAXLINE 1524 

struct sockaddr_in getServAddrServer (int port) {
	struct sockaddr_in	 servaddr; 
	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

	return servaddr;
}  

void sendACK(int &sockfd, struct sockaddr_in &clientaddr, unsigned int frameNumber) {
    cout<<"Send ACK "<<frameNumber<<endl;
	unsigned char * dataACK = convertToAckFrame(createACK(ACKVALUE,frameNumber));
	sendto(sockfd, (const char *)dataACK, ACK_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &clientaddr, sizeof(clientaddr)); 	
	delete dataACK;
}

void sendNAK(int &sockfd, struct sockaddr_in &clientaddr, unsigned int frameNumber) {
    cout<<"Send NAK "<<frameNumber<<endl;
	unsigned char * dataACK = convertToAckFrame(createACK(NAKVALUE,frameNumber));
	sendto(sockfd, (const char *)dataACK, ACK_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &clientaddr, sizeof(clientaddr)); 	
	delete dataACK;
}

void fetchFrame(int *sockfd,bool *isEnd, struct sockaddr_in *clientaddr, deque<frame> *bufferFrameQue) {
    unsigned int n, len; 
    unsigned char bufferFrame[DATA_FRAME_SIZE];
	frame tempFrame;
    while (!(*isEnd)) {
        cout<<"Init Receive Frame "<<endl;
        n = recvfrom(*sockfd, (char *)bufferFrame, DATA_FRAME_SIZE, MSG_WAITALL, (struct sockaddr *) clientaddr, &len);
        tempFrame = convertToFrame(bufferFrame);
        cout<<"Receive Frame "<<tempFrame.sequenceNumber<<endl;
        pthread_mutex_lock(&lockQueueRecv);
        (*bufferFrameQue).push_back(tempFrame);
        pthread_mutex_unlock(&lockQueueRecv);
    }

    cout<<"Selesai Fetch Frame"<<endl;
}


void printframe (frame temp) {
    for (int i=0;i<temp.dataLength;i++) {
        cout<<temp.data[i];
    }
}

// Driver code 
int main(int argc, char* argv[]) { 
    //Input Processing
	char *fileName = argv[1];
	int windowSize = atoi(argv[2]); //frame
	int bufferSize = atoi(argv[3]); //byte
	int portDestination = atoi(argv[4]);


    int sockfd;
    struct sockaddr_in servaddr = getServAddrServer(portDestination);
    struct sockaddr_in cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    // inisialisasi variabel
    bool isEnd = false;
    deque<frame> bufferFrameQue;
    frame dummy;
    dummy.SOH = 5;
    deque<frame> slidingWindow(windowSize, dummy);
    unsigned int LAF = windowSize-1;
    unsigned int LFR = -1;

    struct timeval tv;
    tv.tv_sec = 2;  /* 30 Secs Timeout */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

    thread t1(fetchFrame, &sockfd, &isEnd, &cliaddr, &bufferFrameQue);
    int count = 0;
    while (!(isEnd)) {
        pthread_mutex_lock(&lockQueueRecv);
        while (!bufferFrameQue.empty()) {
            frame temp = bufferFrameQue.front();
            // cout<<temp.sequenceNumber<<" "<<isValidDataFrame(temp);
            if (isValidDataFrame(temp)) {
                if (isInWindow(LAF,windowSize,temp.sequenceNumber)) {
                    if (!slidingWindow[temp.sequenceNumber-LFR-1].acked) {
                        slidingWindow[temp.sequenceNumber-LFR-1] = temp;
                        slidingWindow[temp.sequenceNumber-LFR-1].acked = true;
                    }
                    sendACK(sockfd,cliaddr,temp.sequenceNumber);
                } else if (temp.sequenceNumber<=LFR) {
                    sendACK(sockfd,cliaddr,temp.sequenceNumber);
                }
            } else {
                sendNAK(sockfd,cliaddr,temp.sequenceNumber);
            }
            bufferFrameQue.pop_front();
        }
        pthread_mutex_unlock(&lockQueueRecv);

        while (slidingWindow.front().acked && slidingWindow.front().SOH != SOH_NOT_VALID) {
            LAF++;
            LFR++;
            frame tempyak = slidingWindow.front();
            for (int i=0;i<tempyak.dataLength;i++) {
                cout<<tempyak.data[i];
            }
            cout<<endl;
            slidingWindow.pop_front();
            cout<<"SOH "<<(int)tempyak.SOH<<endl;
            if (tempyak.SOH == SOH_EOF) {
                isEnd = true;
            }
            if (!(isEnd)) {
                slidingWindow.push_back(dummy);
            }
        }
    }
    cout<<"Selesai"<<endl;
    cout << "sockfd: " << sockfd << endl;

    t1.join();
} 