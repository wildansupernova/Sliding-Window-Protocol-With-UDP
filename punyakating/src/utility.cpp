#include <stdio.h>
#include "utility.h"
#include <stdlib.h>
#include <time.h>

unsigned char generateChecksumACK(PacketACK paket) {
  unsigned char result = 0;
  result += ACK(paket);
  result += AdvertisedWindowSize(paket) & 0xFF;
  result += NextSequenceNumber(paket) & 0xFF;
  return result;
}
 
unsigned char generateChecksumPaket(Segment paket) {
  unsigned char result = 0;
  result += SOH(paket);
  result += SequenceNumber(paket) & 0xFF;
  result += STX(paket);
  result += Data(paket);
  result += ETX(paket);
  return result;
}

const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "[%d/%m/%Y - %X] ", &tstruct);

    return buf;
}

Segment CreateSegment(unsigned int inputSequenceNumber, unsigned char inputData, unsigned char inputChecksum) {
  Segment S;
  SOH(S) = DefaultSOH;
  SequenceNumber(S) = inputSequenceNumber;
  STX(S) = DefaultSTX;
  Data(S) = inputData;
  ETX(S) = DefaultETX;
  Checksum(S) = inputChecksum;

  return S;
}

PacketACK CreatePacketACK(unsigned int inputNextSequenceNumber, unsigned int inputAdvertisedWindowSize, unsigned char inputChecksum) {
  PacketACK P;
  ACK(P) = DefaultACK;
  NextSequenceNumber(P) = inputNextSequenceNumber;
  AdvertisedWindowSize(P) = inputAdvertisedWindowSize;
  Checksum(P) = inputChecksum;

  return P;
}

// int main() {
//   unsigned char* data = ReadData("hehe.txt");
//   if(data != NULL) {
//     printf("Size of data: %d\n", sizeof(data));
//     for(int i = 0; i < sizeof(data); i++) {
//       printf("%d. %c\n", i, data[i]);
//     }
//   } else {
//     printf("return null");
//   }

//   return 0;
// }