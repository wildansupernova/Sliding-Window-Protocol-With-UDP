#include <bits/stdc++.h>
using namespace std;

#define ACKVALUE = 0x1;
#define NAKVALUE = 0x2;

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
        int timeStamp;
} frame;

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

bool isValidDataFrame(unsigned char *dataFrame) {
    int totalData = 1033;
    int resultChecksum = 0;

    for (int i=0;i<totalData;i++) {
        resultChecksum += dataFrame[i];
        resultChecksum = (resultChecksum + (resultChecksum >> 8)) & 0xFF;
    }
    char checksumCompare = dataFrame[1033];
    char resultCharChecksum = resultChecksum & 0xFF;
    return !(resultCharChecksum & checksumCompare);
}