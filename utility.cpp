#include <bits/stdc++.h>
using namespace std;

#define ACKVALUE 1;
#define NAKVALUE 2;

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

ack convertToAck(unsigned char *ackFrame){
    ack tempAck;
    tempAck.ack = ackFrame[0];
    for (int i = 1; i <= 4; i++) {
        tempAck.nextSequenceNumber <<= 8;
        tempAck.nextSequenceNumber |= ackFrame[i];
    }
    tempAck.checksum = ackFrame[5];
    return tempAck;
}

unsigned char* convertToAckFrame(ack inputAck){
    unsigned char tempAckFrame[6];
    tempAckFrame[0] = inputAck.ack;
    for (int i = 1; i <= 4; i++) {
        tempAckFrame[i] = inputAck.nextSequenceNumber >> (8 * (4 - i));
    }
    tempAckFrame[5] = inputAck.checksum;
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

frame convertToFrame(unsigned char *dataFrame) {
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
    for (i = 9; i <= 1032; i++) {
        tempFrame.data[i-9] = dataFrame[i];
    }
    tempFrame.checksum = dataFrame[1033];

    return tempFrame;
}

unsigned char* convertToDataFrame(frame tempFrame) {
    int i;

    unsigned char dataFrame[1034];
    dataFrame[0] = tempFrame.SOH;
    for (i = 1; i <= 4; i++) {
        dataFrame[i] = tempFrame.sequenceNumber >> (8 * (4 - i));
    }
    for (i = 5; i <= 8; i++) {
        dataFrame[i] = tempFrame.dataLength >> (8 * (8 - i));
    }
    for (i = 9; i <= 1032; i++) {
        dataFrame[i] = tempFrame.data[i-9];
    }
    dataFrame[1033] = tempFrame.checksum;

    return dataFrame;
}