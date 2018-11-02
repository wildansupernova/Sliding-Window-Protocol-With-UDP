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
pthread_mutex_t lockClientAddr;

#define PORT     8080 
#define MAXLINE 1524 

struct sockaddr_in getServAddrServer (int port) {
	struct sockaddr_in	 servaddr; 
	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_port = htons(port);

	return servaddr;
}  

void sendACK(int &sockfd, struct sockaddr_in &clientaddr, unsigned int frameNumber) {
	unsigned char * dataACK = convertToAckFrame(createACK(ACKVALUE,frameNumber));
	sendto(sockfd, (const char *)dataACK, ACK_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &clientaddr, sizeof(clientaddr)); 	
    cout<<"Send ACK "<<frameNumber<<endl;
	delete[] dataACK;
}

void sendNAK(int &sockfd, struct sockaddr_in &clientaddr, unsigned int frameNumber) {
    cout<<"Send NAK "<<frameNumber<<endl;
	unsigned char * dataACK = convertToAckFrame(createACK(NAKVALUE,frameNumber));
	sendto(sockfd, (const char *)dataACK, ACK_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &clientaddr, sizeof(clientaddr)); 	
	delete[] dataACK;
}

void fetchFrame(int *sockfd,bool *isEnd, struct sockaddr_in *clientaddr, deque<frame> *bufferFrameQue) {
    unsigned int n, len; 
    unsigned char bufferFrame[DATA_FRAME_SIZE];
	frame tempFrame;

    fd_set readfds, masterfds;
    struct timeval timeout;

    timeout.tv_sec = 2;                    /*set the timeout to 10 seconds*/
    timeout.tv_usec = 0;
    while (!(*isEnd)) {
        FD_ZERO(&masterfds);
        FD_SET(*sockfd, &masterfds);

        memcpy(&readfds, &masterfds, sizeof(fd_set));

        if (select(*sockfd+1, &readfds, NULL, NULL, &timeout) < 0)
        {
            perror("on select");
            exit(1);
        }
        if (FD_ISSET(*sockfd, &readfds))
        {
            // read from the socket
            
            n = recvfrom(*sockfd, (char *)bufferFrame, DATA_FRAME_SIZE, MSG_WAITALL, (struct sockaddr *) clientaddr, &len);
            tempFrame = convertToFrame(bufferFrame);
            pthread_mutex_lock(&lockQueueRecv);
            (*bufferFrameQue).push_back(tempFrame);
            pthread_mutex_unlock(&lockQueueRecv);
        }
        else
        {
            // the socket timedout
        }

    }
}


void printframe (frame temp) {
    for (int i=0;i<temp.dataLength;i++) {
        cout<<temp.data[i];
    }
}

void printSockaddr(struct sockaddr_in addr){
    cout<<addr.sin_addr.s_addr<<" "<<addr.sin_family<<" "<<addr.sin_port<<" s "<<addr.sin_zero[1]<<endl;
}

bool isSockAddrSame(struct sockaddr_in &addr1,struct sockaddr_in &addr2) {
    if (
        (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr) && 
        (addr1.sin_family == addr2.sin_family) && 
        (addr1.sin_port == addr2.sin_port)
    ) {
        for (int i=0;i<8;i++) {
            if (addr1.sin_zero[i]!=addr2.sin_zero[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
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
    struct sockaddr_in paddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    memset(&paddr, 0, sizeof(paddr)); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    // membuka stream untuk menulis file
    ofstream ofile;
    ofile.open(fileName, std::ios_base::app);
    
    // inisialisasi variabel
    bool isEnd = false;
    deque<frame> bufferFrameQue;
    frame dummy;
    dummy.SOH = 5;
    deque<frame> slidingWindow(windowSize, dummy);
    unsigned int LAF = windowSize-1;
    unsigned int LFR = -1;

    // timeout
    struct timeval tv;
    tv.tv_sec = 2;  /* 30 Secs Timeout */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

    thread t1(fetchFrame, &sockfd, &isEnd, &cliaddr, &bufferFrameQue);
    int count = 0;
    bool isCliAddrSame = true;

    while (isSockAddrSame(cliaddr,paddr)) {        
    }

    while (!(isEnd)) {
        pthread_mutex_lock(&lockQueueRecv);
        while (!bufferFrameQue.empty()) {
            frame temp = bufferFrameQue.front();
            // cout<<temp.sequenceNumber<<" "<<isValidDataFrame(temp);
            if (isValidDataFrame(temp)) {
                cout<<"Receive Frame "<<temp.sequenceNumber<<endl;
                // usleep(3000000);
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

        while (!slidingWindow.empty() && slidingWindow.front().acked && slidingWindow.front().SOH != SOH_NOT_VALID) {
            LAF++;
            LFR++;
            frame tempyak = slidingWindow.front();
            for (int i=0;i<tempyak.dataLength;i++) {
                ofile<<tempyak.data[i];
            }
            slidingWindow.pop_front();
            if (tempyak.SOH == SOH_EOF) {
                isEnd = true;
            }

            slidingWindow.push_back(dummy);
        }
    }

    cout<<"Selesai"<<endl;

    t1.join();
    // close(sockfd);
} 