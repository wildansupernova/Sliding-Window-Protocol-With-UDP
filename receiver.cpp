
#include <bits/stdc++.h>// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include "utility.cpp"
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;
#define PORT     8080 
#define MAXLINE 1024 
#define FRAMESIZE 1034
#define MAXSEQUENCENUMBER 256

struct sockaddr_in getServAddr(int port){
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); 

    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port); 

    return servaddr;
}

void initQueueSequenceNumber(vector<unsigned int>& v, int windowSize){
    for(int i=0;i<windowSize;i++){
        v.push_back(i);
    }
}

bool isExistInVector(vector<unsigned int> v, unsigned int x){
    for(int i=0;i<v.size();i++){
        if(x==v[i]){
            return true;
        }
    }
    return false;
}
//send from fileBuffer to file
void sendBufferToFile(){
    ofstream myfile;
}
// Driver code 
int main(int argc, char* argv[]) { 
    //Input Processing
	char *fileName = argv[1];
	int windowSize = atoi(argv[2]); //frame
	int bufferSize = atoi(argv[3]); //byte
	int portDestination = atoi(argv[4]);
    char bufferSlidingWindow[FRAMESIZE*windowSize];

    //buffer to handle file transfer from sliding window to file
    unsigned char bufferFile[bufferSize];
    int currentBufferFileIdx = 0;

    int sockfd; 
    unsigned char buffer[MAXLINE]; 
    string serverStr = "Hello from server"; 
    const char *hello = serverStr.c_str(); 
    struct sockaddr_in servaddr, cliaddr; 

    servaddr = getServAddr(portDestination);
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
    
    frame frameReceived;
    unsigned int len, n; 
    unsigned int currentMinSlidingWindow = 0;
    vector<unsigned int> unfilledSequenceNumber;
    initQueueSequenceNumber(unfilledSequenceNumber, windowSize);
    while(true){
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len); 
        if(!isValidDataFrame((unsigned char*) buffer)){
            //If data is invalid
            ack ackFail;
            ackFail.ack = NAKVALUE;
            sort(unfilledSequenceNumber.begin(), unfilledSequenceNumber.begin()+windowSize);
            ackFail.nextSequenceNumber = unfilledSequenceNumber[0];
            ackFail.checksum = calculateChecksum(ackFail.ack, ackFail.nextSequenceNumber);
            unsigned char* ackFrameFail;
            ackFrameFail = convertToAckFrame(ackFail);

            sendto(sockfd, ackFrameFail, 6,  
                MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                    len);
        }
        else{//if data is valid
            frameReceived = convertToFrame((unsigned char *) buffer);
            unsigned int sequenceNumber = frameReceived.sequenceNumber;
            //delete filled sequence number
            for(int i=0;i<unfilledSequenceNumber.size();i++){
                if(unfilledSequenceNumber[i]==sequenceNumber){
                    unfilledSequenceNumber.erase(unfilledSequenceNumber.begin() + i);
                    break;
                }
            }
            //Get index in buffer where the frame should be put on bufferSlidingWindow
            //Kalau sequence number lebih kecil ?
            int startBufferIdx = (sequenceNumber-currentMinSlidingWindow)*FRAMESIZE;
            //Input to buffer of the sliding window
            for(int i=0;i<FRAMESIZE;i++){
                bufferSlidingWindow[startBufferIdx+i] = buffer[i];
            }
            //move the sliding window
            if(unfilledSequenceNumber[0]>currentMinSlidingWindow){
                unsigned int prevMinSlidingWindow = currentMinSlidingWindow;
                currentMinSlidingWindow = unfilledSequenceNumber[0];
                //add to unfilled vector
                for(unsigned int i=prevMinSlidingWindow;i<windowSize;i++){
                    //check masih error, yang lebih besar aja yang ditambahin
                    if(!isExistInVector(unfilledSequenceNumber,(currentMinSlidingWindow+i)%MAXSEQUENCENUMBER)){
                        unfilledSequenceNumber.push_back((currentMinSlidingWindow+i)%MAXSEQUENCENUMBER);
                    }
                }
            }

            //send to file buffer
            if(currentBufferFileIdx == bufferSize){
                sendBufferToFile();
            }

            printf("Client : %s\n", buffer);             
        }
 
    }
    sendto(sockfd, (const char *)hello, strlen(hello),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len); 
    printf("Hello message sent.\n");  
      
    return 0; 
} 