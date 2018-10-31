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
	unsigned char * dataACK = convertToAckFrame(createACK(ACKVALUE,frameNumber));
	sendto(sockfd, (const char *)dataACK, ACK_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &clientaddr, sizeof(clientaddr)); 	
	delete dataACK;
}

void sendNAK(int &sockfd, struct sockaddr_in &clientaddr, unsigned int frameNumber) {
	unsigned char * dataACK = convertToAckFrame(createACK(NAKVALUE,frameNumber));
	sendto(sockfd, (const char *)dataACK, ACK_FRAME_SIZE, MSG_CONFIRM, (const struct sockaddr *) &clientaddr, sizeof(clientaddr)); 	
	delete dataACK;
}

void fetchFrame(bool *isEnd,int *sockfd, struct sockaddr_in *clientaddr, deque<frame> *bufferFrameQue) {
    unsigned int n, len; 
    unsigned char bufferFrame[DATA_FRAME_SIZE];
	frame tempFrame;
    cout<<"lol"<<endl;
    while (!(*isEnd)) {
        cout<<"sel"<<endl;
        n = recvfrom(*sockfd, (char *)bufferFrame, DATA_FRAME_SIZE, MSG_WAITALL, (struct sockaddr *) clientaddr, &len);
        cout<<"sel2"<<endl;
        tempFrame = convertToFrame(bufferFrame);
        if (isValidDataFrame(bufferFrame,n)) {
             
            if (n != RECVFROM_TIMEOUT && isValidDataFrame(bufferFrame,n)) {
                pthread_mutex_lock(&lockQueueRecv);
                cout<<"valid frame"<<endl;
                (*bufferFrameQue).push_back(tempFrame);
                pthread_mutex_unlock(&lockQueueRecv);
            }
             
            if (tempFrame.SOH == SOH_EOF) {
                *isEnd = true;
            }            
        } else {
            sendNAK(*sockfd,*clientaddr,tempFrame.sequenceNumber);
        }
        cout<<"sel4"<<endl;
    }
    cout<<"sel5"<<endl;
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
    
    bool isEnd = false;
    deque<frame> bufferFrameQue;
    frame dummy;
    deque<frame> slidingWindow(windowSize,dummy);
    unsigned int LAF = windowSize-1;
    unsigned int LFR = -1;
    thread t1(fetchFrame, &isEnd, &sockfd, &cliaddr, &bufferFrameQue);

    while (!(isEnd && slidingWindow.empty() && bufferFrameQue.empty())) {
        pthread_mutex_lock(&lockQueueRecv); 
        for (int i = 0; i < bufferFrameQue.size(); i++) {
            frame tempFrame;
			tempFrame = bufferFrameQue.front();
            cout<<"1"<<endl;
            printframe(tempFrame);
            cout<<"2"<<endl;
			bufferFrameQue.pop_front();
            if (isInWindow(LAF,windowSize,tempFrame.sequenceNumber)) {
                if (!slidingWindow[tempFrame.sequenceNumber-LFR-1].acked) {
                    slidingWindow[tempFrame.sequenceNumber-LFR-1] = tempFrame;
                    slidingWindow[tempFrame.sequenceNumber-LFR-1].acked = true;
                }
            } else if (tempFrame.sequenceNumber<=LFR) {
                sendACK(sockfd,cliaddr,tempFrame.sequenceNumber);
            }
		}
        pthread_mutex_unlock(&lockQueueRecv);
        while (slidingWindow.front().acked) {
            LAF++;
            LFR++;
            cout<<"sekali"<<endl;
            frame tempyak = slidingWindow.front();
            for (int i=0;i<tempyak.dataLength;i++) {
                cout<<tempyak.data[i];
            }
            cout<<"apa";
            slidingWindow.pop_front();
            if (!(isEnd && bufferFrameQue.empty())) {
                frame temp;
                slidingWindow.push_back(temp);
            }
        }
    }
    cout<<endl;
    t1.join();
    // unsigned int len, n; 
    // while(true){
    //     n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
    //                 MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
    //                 &len); 
    // }
    // sendto(sockfd, (const char *)hello, strlen(hello),  
    //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
    //         len);       
    return 0; 
} 