#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utility.h"
#include "send_window.h"
#include <unistd.h>
#include <iostream>
#include <fstream>

using namespace std;

void die(char* errorMessage) {
	perror(errorMessage);
	exit(0);
}

int main(int argc, char* argv[]) {
	// fstream Logfile;
	// Logfile.open("Logfile.txt", ios::out);
	char* fileName = argv[1];
	unsigned int windowSize = 8;
	unsigned int bufferSize = 256;
	char* destIP = argv[4];
	int port = atoi(argv[5]);


	// Urusan Socket
	int udpSocket;
	struct sockaddr_in clientAddress;

	unsigned char buffer[bufferSize];
	
	FILE* file = fopen(fileName, "r");
	if (file == NULL) {
		exit(0);
	}

	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		die("socket");
	}

	//Urusan Timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 255000;
	setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));

	//Urusan Address
	memset((char *)&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_port = htons(port);
	clientAddress.sin_addr.s_addr = inet_addr("192.168.88.171");

	unsigned int slen = sizeof(clientAddress);
	unsigned int i = 0;

	//Urusan Segment dan Pengiriman
	Segment paket;
	PacketACK ack;
	unsigned int counterBuffer = 0;
	unsigned int counterSequence = 0;
	int alreadyReadAll = 0;
	unsigned int advertisedWindowSize = 256;
	unsigned int bufferSizeOffset = 0;

	unsigned char c;

	//Urusan window
	SendingWindow sendingWindow;
	createNew(&sendingWindow, windowSize);

	unsigned int repeatEnd, repeatStart;
	unsigned int paramSend = 0;
	unsigned int iterator = 0;

	srand(time(NULL));

	while (1) {
		// Logfile << currentDateTime() << "Sender: Sending data" << endl;
		
		//Urusan Buffer	
		while (counterBuffer < bufferSize && !alreadyReadAll) {
			if (fscanf(file, "%c", &c) == EOF) {
				alreadyReadAll = 1;
				break;
			}
			buffer[counterBuffer] = c;
			// Logfile << currentDateTime() << "Sender: Read data " << counterBuffer + bufferSizeOffset << " " << buffer[counterBuffer] << endl;
			counterBuffer++;
		}

		//Sending Frame
		// Logfile << currentDateTime() << "Sender: Processing to send frame" << endl;
		// Logfile << currentDateTime() << "Sender: bufferSize " << bufferSize << " bufferSizeOffset " << bufferSizeOffset << endl;
		// Logfile << currentDateTime() << "Sender: LAR " << LAR(sendingWindow) << " LFS " << LFS(sendingWindow) << endl;
		repeatStart = LAR(sendingWindow);
		repeatEnd = LFS(sendingWindow);
		
		paramSend = 0;
		while (LFS(sendingWindow) < LAR(sendingWindow) + windowSize && LFS(sendingWindow) < counterBuffer + bufferSizeOffset && paramSend < advertisedWindowSize) {

			//Create Segment
			// Logfile << currentDateTime() << "Sender: Creating segment" << endl;
			paket = CreateSegment(LFS(sendingWindow), buffer[LFS(sendingWindow) - bufferSizeOffset], 0);
			Checksum(paket) = generateChecksumPaket(paket);
			LFS(sendingWindow) = LFS(sendingWindow) + 1;
			// Logfile << currentDateTime() << "Sender: Frame Number " << LFS(sendingWindow)-1 << " Data " << Data(paket) << endl;

			char* segment = (char *) &paket;
			if (sendto(udpSocket, segment, sizeof(paket), 0, (struct sockaddr *) &clientAddress, slen) == -1) {
				die("sendto");
			}
			paramSend++;
		}
			
		//Sending Lost Frame
		// Logfile << currentDateTime() << "Sender: Sending lost frame(s)" << endl;
		int i;
		for (i = repeatStart; i < repeatEnd; i++) {
			paket = CreateSegment(i, buffer[i - bufferSizeOffset], 0);
			Checksum(paket) = generateChecksumPaket(paket);
			
			// Logfile << currentDateTime() << "Sender: Repeat send Frame Number " << i << " Data " << Data(paket) << endl;

			char* segment = (char *) &paket;
			if (sendto(udpSocket, segment, sizeof(paket), 0, (struct sockaddr *) &clientAddress, slen) == -1) {
				die("sendto");
			}
			paramSend++;
		}

		//Receive ACK
		// Logfile << currentDateTime() << "Sender: Receiving ACK" << endl;
		for (i = 0; i < paramSend; i++) {
			char* acksegment = (char *) &ack;
			if (recvfrom(udpSocket, acksegment, sizeof(ack), 0, (struct sockaddr*) &clientAddress, &slen) >= 0) {
				if (Checksum(ack) == generateChecksumACK(ack)) {
					advertisedWindowSize = AdvertisedWindowSize(ack);
					LAR(sendingWindow) = NextSequenceNumber(ack);
				} else {
					// Logfile << currentDateTime() << "Sender: Wrong ACK checksum" << endl;
				}
			} else {
				// Logfile << currentDateTime() << "Sender: Timeout" << endl;
			}
			// Logfile << currentDateTime() << "Sender: Received ACK " << NextSequenceNumber(ack) << endl;
		}
		

		if (alreadyReadAll == 1 && LAR(sendingWindow) > counterBuffer + bufferSizeOffset - 1) {
			break;
		}

		//Buffer Size Offset Increase
		// Logfile << currentDateTime() << "Sender: bufferSizeOffset increases " << LAR(sendingWindow) << " " << bufferSize + bufferSizeOffset << endl;
		if (LAR(sendingWindow) == bufferSize + bufferSizeOffset) {
			counterBuffer = 0;
			bufferSizeOffset += bufferSize;
			// Logfile << currentDateTime() << "Sender: Current bufferSizeOffset " << bufferSizeOffset << endl;
		}
	}

	//Sending Ending Sequence
	PacketACK finalACK;
	NextSequenceNumber(finalACK) = 0;
	Segment finalSegment;
	finalSegment = CreateSegment(0, 0, 0);
	SOH(finalSegment) = 0x2;
	Checksum(finalSegment) = generateChecksumPaket(finalSegment);
	// Logfile << currentDateTime() << "Sender: Sending ending sequence " << generateChecksumPaket(finalSegment) << " " << Checksum(finalSegment) << endl;
	cout <<(generateChecksumPaket(finalSegment) == Checksum(finalSegment)) << endl;
	cout << NextSequenceNumber(finalACK) << endl;
	while (NextSequenceNumber(finalACK) == 0 || generateChecksumACK(finalACK) != Checksum(finalACK)) {
		// Logfile << currentDateTime() << "Sender: Sending final packet" << endl;
		char* segment = (char *) &finalSegment;
		sendto(udpSocket, segment, sizeof(finalSegment), 0, (struct sockaddr *) &clientAddress, slen);

		char* acksegment = (char *) &finalACK;
		recvfrom(udpSocket, acksegment, sizeof(finalACK), 0, (struct sockaddr*) &clientAddress, &slen);
	}

	// Logfile << currentDateTime() << "Sender: All data has been sent successfully" << endl;
	printf("All data has been sent successfully\n");

	close(udpSocket);
  // Logfile.close();
}