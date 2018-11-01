#include <bits/stdc++.h>
using namespace std;

#define ACKVALUE 1
#define NAKVALUE 2

#define PORT	 8080 
#define MAX_DATA_SIZE 1024
#define DATA_FRAME_SIZE 1034
#define ACK_FRAME_SIZE 6
#define TIMEOUT 5
#define RECVFROM_TIMEOUT 4294967295
#define SOH_EOF 6
#define SOH_DEFAULT 1
#define SOH_NOT_VALID 5

typedef struct  ACK { 
        unsigned char ack ;
        unsigned int nextSequenceNumber;
        unsigned char checksum;
} ack;

typedef struct  Frame { 
        unsigned char SOH = 0x1;
        unsigned int sequenceNumber;
        unsigned int dataLength;
        unsigned char data[1024];
        unsigned char checksum;
        bool acked = false;
        int timeStamp = -1;
} frame;

ack convertToAck (unsigned char * dataAck) {
    ack tempAck;
    tempAck.ack = dataAck[0];
    unsigned int seqNumber = 0;
    for (int i=1;i<=4;i++) {
        seqNumber <<= 8;
        seqNumber |= dataAck[i];
    }
    tempAck.nextSequenceNumber = seqNumber;
    tempAck.checksum = dataAck[5];
    return tempAck;
}

unsigned char* convertToAckFrame(ack inputAck){
    unsigned char* dataACK = new unsigned char[ACK_FRAME_SIZE];
    dataACK[0] = inputAck.ack;
    for (int i = 1; i <= 4; i++) {
        dataACK[i] = inputAck.nextSequenceNumber >> (8 * (4 - i));
    }
    dataACK[5] = inputAck.checksum;
    return dataACK;
}

unsigned char calculateChecksum(unsigned char ack,unsigned int nextSequenceNumber) {
    unsigned int result = 0;
    result += ack;

    unsigned int nextSequenceNumberTemp = nextSequenceNumber;
    while (nextSequenceNumberTemp != 0) {
        result += nextSequenceNumberTemp & 0xFF;
        result = (result + (result >> 8)) & 0xFF;
        nextSequenceNumberTemp = nextSequenceNumberTemp >> 8;
    }
    
    return ~(result & 0xFF);
}

unsigned char calculateChecksum(char SOH, unsigned int sequenceNumber,unsigned int  dataLength, unsigned char data[1024]){
    unsigned int result = 0;
    result += SOH;

    unsigned int sequenceNumberTemp = sequenceNumber;
    while (sequenceNumberTemp != 0) {
        result += sequenceNumberTemp & 0xFF;
        result = (result + (result >> 8)) & 0xFF;
        sequenceNumberTemp = sequenceNumberTemp >> 8;
    }

    unsigned int dataLengthTemp = dataLength;
    while (dataLengthTemp != 0) {
        result += dataLengthTemp & 0xFF;
        result = (result + (result >> 8)) & 0xFF;
        dataLengthTemp = dataLengthTemp >> 8;
    }

    for (int i=0;i<dataLength;i++) {
        result += data[i];
        result = (result + (result >> 8)) & 0xFF;
    }

    return ~(result & 0xFF);
}

frame convertToFrame(unsigned char *dataFrame) {
    //Data sudah valid
    int i;

    frame tempFrame;
    tempFrame.SOH = dataFrame[0];
    for (i = 1; i <= 4; i++) {
        tempFrame.sequenceNumber <<= 8;
        tempFrame.sequenceNumber |= dataFrame[i];
    }
    for (i = 5; i <= 8; i++) {
        tempFrame.dataLength <<= 8;
        tempFrame.dataLength |= dataFrame[i];
    }
    for (i = 9; i < tempFrame.dataLength + 9; i++) {
        tempFrame.data[i-9] = dataFrame[i];
    }
    tempFrame.checksum = dataFrame[tempFrame.dataLength + 9];

    return tempFrame;
}

bool isValidDataFrame(unsigned char *dataFrame, int n) {
    int resultChecksum = 0;
    int totalData = n-1;

    for (int i=0; i<totalData; i++) {
        resultChecksum += dataFrame[i];
        resultChecksum = (resultChecksum + (resultChecksum >> 8)) & 0xFF;
    }
    char checksumCompare = dataFrame[totalData];
    char resultCharChecksum = resultChecksum & 0xFF;
    return !(resultCharChecksum & checksumCompare);
}

bool isValidDataFrame(frame dataFrame) {
    unsigned char checksum = calculateChecksum(dataFrame.SOH,dataFrame.sequenceNumber,dataFrame.dataLength,dataFrame.data);
    return checksum == dataFrame.checksum;
}

bool isAckValid(unsigned char *dataAck) {
    int totalData = 5;
    int resultChecksum = 0;

    for (int i=0;i<totalData;i++) {
        resultChecksum += dataAck[i];
        resultChecksum = (resultChecksum + (resultChecksum >> 8)) & 0xFF;
    }
    char checksumCompare = dataAck[totalData];
    char resultCharChecksum = resultChecksum & 0xFF;
    return !(resultCharChecksum & checksumCompare);
}

unsigned char* convertToDataFrame(frame tempFrame) {
    int i;

    unsigned char* dataFrame;
    unsigned int dataLength = tempFrame.dataLength;

    dataFrame = new unsigned char[dataLength+10];
    dataFrame[0] = tempFrame.SOH;
    for (i = 1; i <= 4; i++) {
        dataFrame[i] = tempFrame.sequenceNumber >> (8 * (4 - i));
    }
    for (i = 5; i <= 8; i++) {
        dataFrame[i] = dataLength >> (8 * (8 - i));
    }
    for (i = 9; i < dataLength+9; i++) {
        dataFrame[i] = tempFrame.data[i-9];
    }
    dataFrame[dataLength+9] = tempFrame.checksum;

    return dataFrame;
}

string convertToDataFrameWithString(frame tempFrame) {
    int i;
    string s;
    unsigned int dataLength = tempFrame.dataLength;
    s.push_back(tempFrame.SOH);
    for (i = 1; i <= 4; i++) {
        s.push_back(tempFrame.sequenceNumber >> (8 * (4 - i)));
    }
    for (i = 5; i <= 8; i++) {
        s.push_back(dataLength >> (8 * (8 - i)));
    }
    for (i = 9; i < dataLength+9; i++) {
        s.push_back(tempFrame.data[i-9]);
    }
    s.push_back(tempFrame.checksum);

    return s;
}


bool isInWindow (unsigned int lfs,unsigned int windowSize, unsigned int seqNum) {
    unsigned int left = lfs-windowSize+1;
    unsigned int right = lfs;
    return left <= seqNum && seqNum <= right;
}


int calculateIndexInQueueSender (unsigned int lfs,unsigned int windowSize, unsigned int seqNum) {
    return seqNum - (lfs-windowSize+1);
}


int sizeOfFrame(frame &d) {
    return d.dataLength+sizeof(d.sequenceNumber)+sizeof(d.dataLength)+sizeof(d.SOH) + sizeof(d.checksum);
}





ack createACK (char ackVAL, unsigned int &seqNum) {
    ack temp;
    temp.ack = ackVAL;
    temp.nextSequenceNumber = seqNum+1;
    temp.checksum = calculateChecksum(ackVAL,temp.nextSequenceNumber);
    return temp;
}