
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
#define FILENAME "outFile.txt"

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

//get the first idx in array
unsigned int getSlidingWindowIdx(unsigned int sequenceNumber, unsigned int currentFirstSlidingWindow){
    return ((sequenceNumber-currentFirstSlidingWindow)%MAXSEQUENCENUMBER)*FRAMESIZE;
}

//move from buffer to file
void moveBufferFileToFile(unsigned char* bufferFile, int countBufferFrame, string fileName){
    fstream outFile;
    outFile.open(FILENAME, ios_base::app);
    //cara kirim supaya jumlahnya terbatas sesuai countBufferFrame ?
    outFile<<bufferFile;
}


// Driver code 
int main(int argc, char* argv[]) { 
    //Input Processing
	char *fileName = argv[1];
	int windowSize = atoi(argv[2]); //frame
	int bufferSize = atoi(argv[3]); //byte
	int portDestination = atoi(argv[4]);
    char bufferSlidingWindow[FRAMESIZE*windowSize];
    int countBufferFrame = 0;
    fstream outputFile;


    //buffer to handle file transfer from sliding window to file
    unsigned char bufferFile[FRAMESIZE*bufferSize];
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
    unsigned int currentFirstSlidingWindow = 0;
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
            int startBufferIdx = getSlidingWindowIdx(sequenceNumber,currentFirstSlidingWindow);
            //Input to buffer of the sliding window
            for(int i=0;i<FRAMESIZE;i++){
                bufferSlidingWindow[startBufferIdx+i] = buffer[i];
            }
            //move the sliding window
            if(unfilledSequenceNumber[0]>currentFirstSlidingWindow){
                //Send frame to file buffer
                //if the buffer is full, send it to file
                int slidingWindowFrameIdx = unfilledSequenceNumber[0]-currentFirstSlidingWindow;
                if(countBufferFrame+slidingWindowFrameIdx == bufferSize){
                    //move buffer file to file
                    moveBufferFileToFile(bufferFile, countBufferFrame, FILENAME);
                    countBufferFrame = 0;
                }
                //move frames as the change of minSequenceNumber
                //move the files from sliding window to file buffer
                //move for each frame
                for(int j=0;j<slidingWindowFrameIdx;j++){
                    //move data in a frame
                    for(int k=0;k<FRAMESIZE;k++){
                        bufferFile[countBufferFrame*FRAMESIZE+j*FRAMESIZE+k] =  bufferSlidingWindow[j*FRAMESIZE+k];
                    }
                }
                countBufferFrame += slidingWindowFrameIdx;

                unsigned int prevMinSlidingWindow = currentFirstSlidingWindow;
                currentFirstSlidingWindow = unfilledSequenceNumber[0];
                //add to unfilled vector
                for(unsigned int i=prevMinSlidingWindow;i<windowSize;i++){
                    //check masih error, yang lebih besar aja yang ditambahin
                    if(!isExistInVector(unfilledSequenceNumber,(currentFirstSlidingWindow+i)%MAXSEQUENCENUMBER)){
                        unfilledSequenceNumber.push_back((currentFirstSlidingWindow+i)%MAXSEQUENCENUMBER);
                    }
                }
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