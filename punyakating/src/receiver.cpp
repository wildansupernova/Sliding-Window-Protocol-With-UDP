#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utility.h"
#include <fstream>
#include <math.h>
#include <unistd.h>

using namespace std;

unsigned int bufferSize;

void die(char *s) {
	perror(s);
	exit(1);
}

void flush(bool *alreadyReceived) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		alreadyReceived[i] = false;
	}
}

int main(int argc, char* argv[]) {
	// fstream Logfile;
	// Logfile.open("Logfile.txt", ios::out);
	char* fileName = argv[1];
	unsigned int windowSize = 8;
	bufferSize = 256;
	int port = atoi(argv[4]);
	// Logfile << currentDateTime() << "Receiver: fileName " << fileName << " windowSize " << windowSize << " bufferSize " << bufferSize << " port " << port << endl;
	unsigned char buffer[bufferSize];
	bool alreadyReceived[bufferSize];

	//Urusan Socket
	int udpSocket;
	struct sockaddr_in serverAddr, clientAddr;
	unsigned int slen = sizeof(serverAddr); 

	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		// die("socket");
	}

	//Urusan Timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 255000;
	setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
	

	//Urusan Address
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(port);
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind (udpSocket, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) == -1) {
		die("bind");
	}

	//Urusan File
	FILE* file = fopen(fileName, "w");

	//Urusan Receive
	int recv_len;
	Segment paket;
	PacketACK ack;
	unsigned int lastFrameCorrect = 0;
	unsigned int LFR = 0;
	bool first = true;
	unsigned int LAF = windowSize;
	unsigned int counterBuffer = 0;
	unsigned int counterBufferOffset = 0;
	unsigned int advertisedWindowSize = (bufferSize > windowSize) ? windowSize : bufferSize;
	int finish = 0;

	flush(alreadyReceived);
	
	//Urusan Random Number
	srand(time(NULL));
	while (1) {
		// Logfile << currentDateTime() << "Receiver: Waiting for data..." << endl;
		fflush(stdout);

		char * recvBuf = (char *) &paket;
		if (recvfrom(udpSocket, recvBuf, sizeof(paket), 0, (struct sockaddr *) &serverAddr, &slen) >= 0) {
			// Logfile << currentDateTime() << "Receiver: generateChecksumPaket(paket) " << generateChecksumPaket(paket) << " Checksum " << Checksum(paket) << endl;
			if (generateChecksumPaket(paket) == Checksum(paket)) {
				if (SOH(paket) == 0x02) {
					finish = 1;
				}
				// Logfile << currentDateTime() << "Receiver: Received packet from " << inet_ntoa(serverAddr.sin_addr) << " " << ntohs(serverAddr.sin_port) << endl;
				// Logfile << currentDateTime() << "Receiver: Frame Number " << SequenceNumber(paket) << " Data " << Data(paket) << endl;
				// Logfile << currentDateTime() << "Receiver: SeqNum " << SequenceNumber(paket) << " LFR " << LFR << " LAF " << LAF << endl;
				if (SequenceNumber(paket) >= LFR && SequenceNumber(paket) <= LAF) {
					if (SequenceNumber(paket) == LFR) {
						LFR++;
						// Logfile << currentDateTime() << "Receiver: SeqNum " << SequenceNumber(paket) << " LFR " << LFR << " LAF " << LAF << " counterBufferOffset " << counterBufferOffset << endl;
					
						//Writing Data to Buffer
						buffer[SequenceNumber(paket) - counterBufferOffset] = Data(paket);
						counterBuffer++;
						advertisedWindowSize--;
						if (advertisedWindowSize == 0) {
							advertisedWindowSize = (bufferSize > windowSize) ? windowSize : bufferSize;
						}
					}
				}

				// Logfile << currentDateTime() << "Receiver: windowSize " << windowSize << endl;
				// Logfile << currentDateTime() << "Receiver: advertisedWindowSize " << advertisedWindowSize << endl;
				// Logfile << currentDateTime() << "Receiver: LAF " << LAF << " LFR " << LFR << endl;
				LAF = LFR + min(windowSize, advertisedWindowSize);
			} else {
				// Logfile << currentDateTime() << "Receiver: Wrong Checksum" << endl;
			} 
		} else {
			// Logfile << currentDateTime() << "Receiver: Failed to receive" << endl;
		}

		//Sending ACK 
		// Logfile << currentDateTime() << "Receiver: Sending ACK " << LFR << endl;
		ack = CreatePacketACK(LFR, advertisedWindowSize, '0');
		Checksum(ack) = generateChecksumACK(ack);

		char *sendBuf = (char *) &ack;
		sendto(udpSocket, sendBuf, sizeof(ack), 0, (struct sockaddr*) &serverAddr, slen);

		//Writing Data to File
		// Logfile << currentDateTime() << "Receiver: Processing to write data to file" << endl;
		// Logfile << currentDateTime() << "Receiver: counterBuffer " << counterBuffer << " bufferSize " << bufferSize << endl;
		if (counterBuffer == bufferSize) {
			counterBufferOffset += bufferSize;
			counterBuffer = 0;
			for (int i = 0; i < bufferSize; i++) {
				fputc(buffer[i], file);	
			}
			// flush(alreadyReceived);
			// Logfile << currentDateTime() << "Receiver: Writing data to file" << endl;
		}

		if (finish) {
			break;
		}
	}
	if (counterBuffer != 0) {
		// Logfile << currentDateTime() << "Receiver: Writing remaining buffer to file" << endl;
		printf("Writing remaining buffer to File\n");
		for (int i = 0; i < counterBuffer; i++) {
			fputc(buffer[i], file);
		}
	}
	// Logfile << currentDateTime() << "Receiver: All data has been written successfully" << endl;
	printf("All data has been received succesfully\n");
	
	fclose(file);
	// Logfile.close();
}