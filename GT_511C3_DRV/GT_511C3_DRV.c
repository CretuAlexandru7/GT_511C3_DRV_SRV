/**************  Client - Server (socket) dependencies **********************/
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>				 /* IP address conversions functionality */
#pragma comment(lib, "ws2_32.lib")   /* Link required library */

/* Set the CLIENT/socket global to be easier accesed */
SOCKET clientSocket;
 /**************************************************************************/


#include "stdio.h"
#include "GT_511C3_DRV.h"

/* Singletone FP Sensor GT_511C3 instance variable */
static struct GT_511C3* FP_SENSOR = NULL;

#define READING_BYTE_BUFFER_SIZE 1

// TODO: if or not if is big endian
#define PARAMETER_BYTE(x) (byte)((x) & 0xFF) 

//packet[4] = (byte)(GT_PKT->sParameter & 0xFF);			/* sParameter is type DWORD -> it has 32 bits; */
//packet[5] = (byte)((GT_PKT->sParameter >> 8) & 0xFF);	/* Shift the bits accordingly in order to get the coresponding bytes */
//packet[6] = (byte)((GT_PKT->sParameter >> 16) & 0xFF);
//packet[7] = (byte)((GT_PKT->sParameter >> 24) & 0xFF);


/*************************   Coomand - Response Packet  ***********************/

/* Returns the completed Packet Command */
byte* vDoCreatePacket(struct sCMD_RSP_PKT* GT_PKT)
{
	/* Allocate memory for the packet */
	byte* packet = (byte*)malloc(12 * sizeof(byte));
	// TODO: verify:

	WORD cmd = GT_PKT->eCommands;

	packet[0] = CMD_START_CODE_1;
	packet[1] = CMD_START_CODE_2;
	packet[2] = CMD_DEVICE_ID;
	packet[3] = NULL_BYTE;	
	packet[4] = (byte)(GT_PKT->sParameter & 0xFF);			/* sParameter is type DWORD -> it has 32 bits; */
	packet[5] = (byte)((GT_PKT->sParameter >> 8) & 0xFF);	/* Shift the bits accordingly in order to get the coresponding bytes */
	packet[6] = (byte)((GT_PKT->sParameter >> 16) & 0xFF);
	packet[7] = (byte)((GT_PKT->sParameter >> 24) & 0xFF);	
	packet[8] = (byte)(cmd & 0x00FF);						/* First byte of the command */
	packet[9] = (byte)((cmd >> 8) & 0x00FF);				/* Second byte of the command */
	
	WORD checksum = GT_PKT->sCalcChecksum(packet);			/* Calculate the packet checksum */
	packet[10] = (byte)(checksum & 0x00FF);
	packet[11] = (byte)((checksum >> 8) & 0x00FF);

	return packet;
}

/* Calculate the checksum of a Command_Packet */
WORD vDoCalcChecksum(byte* packet)
{
	WORD checksum = 0;
	for (byte i = 0; i < 9; i++)
	{
		checksum += *(packet + i);
	}

	return checksum;
}

/* Send command / packet */
void vDoSendCommand(byte* packet)
{
	printf("Command to be send:            ");
	for (byte i = 0; i < 12; i++)
	{
		printf("%02X ", *(packet + i));
	}

	// Send a (12 bytes) PACKET to the server
	if (send(clientSocket, packet, 12, 0) < 0)
	{
		printf("Send failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
}

/* Receive Response Packet */
void vDoReceivePacket(struct sCMD_RSP_PKT* RSP_PACKET)
{
	byte readbyte = 0;       /* Container for coming bytes */
	bool readFlag = True;    /* Flag used to mark the beginning of a command */

	/* Allocate memory for a 12 bytes packet */
	byte* response = (byte*)malloc(12 * sizeof(byte));
	// TODO: verify 

	/* Wait until the first byte of the response packet is received */
	while (readFlag)
	{
		/* Recv function return the no. of received bytes */
		byte byte_received = recv(clientSocket, &response[0], READING_BYTE_BUFFER_SIZE, 0);
		if (byte_received == -1)
		{
			/* Handle error */
		}
		else if (byte_received == 0)
		{
			/* Connection closed by server */
		}
		else
		{
			/* Check if this is the first byte of the command */
			if (response[0] == CMD_START_CODE_1)
			{
				readFlag = False;
			}
		}

		printf("\nResponse (from the server):    ");
		printf("%02x ", response[0]);
		for (byte i = 1; i < 12; i++)
		{
			byte_received = recv(clientSocket, &response[i], READING_BYTE_BUFFER_SIZE, 0);
			printf("%02x ", response[i]);
		}
		printf("\n");

		// TODO: implement vDoInterpretResponse(response);
	}
}

/*****************************************************************************/


/*****************************   GT_511C3   **********************************/

/* Initialize the device */
void Initialize_FP_Sensor()
{
	/* Initialize a SINGLETON instance of the FP Sensor */
	if (FP_SENSOR == NULL)
	{
		bool bMemoryLeak = False;

		FP_SENSOR = (struct GT_511C3*)malloc(sizeof(struct GT_511C3));
		if (FP_SENSOR == NULL)
		{			
			bMemoryLeak = True;
			// TODO: STDERR error
			return;
		}

		/*  Memory allocation for Command(send) - Response(received) Packet variables  */
		FP_SENSOR->sSENT_GT_PKT = (struct sCMD_RSP_PKT*)malloc(sizeof(struct sCMD_RSP_PKT));
		if (FP_SENSOR->sSENT_GT_PKT == NULL)
		{
			bMemoryLeak = True;
		}

		FP_SENSOR->sRECEIVED_GT_PKT = (struct sCMD_RSP_PKT*)malloc(sizeof(struct sCMD_RSP_PKT));
		if (FP_SENSOR->sRECEIVED_GT_PKT == NULL)
		{
			bMemoryLeak = True;
		}

		if (bMemoryLeak)
		{
			free(FP_SENSOR);
			free(FP_SENSOR->sSENT_GT_PKT);
			free(FP_SENSOR->sRECEIVED_GT_PKT);
			FP_SENSOR = NULL;
			FP_SENSOR->sSENT_GT_PKT = NULL;
			FP_SENSOR->sRECEIVED_GT_PKT = NULL;

			// TODO: STDERR error
			return;
		}

	
		/* Struct->send/received_packet variable name is too long -> create an alias: */
		struct sCMD_RSP_PKT* GT_SNT_PKT = FP_SENSOR->sSENT_GT_PKT;
		struct sCMD_RSP_PKT* GT_RCV_PKT = FP_SENSOR->sRECEIVED_GT_PKT;

		/* Default values for Initialization phase */
		// TODO:
		GT_SNT_PKT->eCommands = Initialize_FP;
		GT_SNT_PKT->eError_Codes = NOT_AN_ERROR;
		GT_SNT_PKT->sParameter = 0;
		GT_RCV_PKT->eCommands = NOT_A_COMMAND;
		GT_RCV_PKT->sParameter = 0;

		/* Assign function pointers for SENT Packet */
		FP_SENSOR->sSendCommand = &vDoSendCommand;
		GT_SNT_PKT->sCalcPacket = &vDoCreatePacket;
		GT_SNT_PKT->sCalcChecksum = &vDoCalcChecksum;

		/* Assign function pointers for RECEIVED Packet */
		FP_SENSOR->sReceiveResponsePacket = &vDoReceivePacket;

		/* Compute the packet bytes values */
		byte* packet = GT_SNT_PKT->sCalcPacket(FP_SENSOR->sSENT_GT_PKT);
		/* Send the packet / command */
		FP_SENSOR->sSendCommand(packet);
		/* Receive the response packet */
		FP_SENSOR->sReceiveResponsePacket(GT_RCV_PKT);
		
		/* Check if GT_RCV_PKT->eError_Codes == NO_ERROR before continuing, otherwise if it shows an error we should handle it */
		
		free(packet);
		free(GT_RCV_PKT);
	}	
}


/* Create a SINGLETON instance of the FP Sensor */
struct GT_511C3* GT_511C3_GetInstance()
{
	Initialize_FP_Sensor();
	if (FP_SENSOR != NULL)
	{
		/* Only for Hardware Device */
		// FP_SENSOR->sUseSerialDebug = True;
	}

	return FP_SENSOR;
}
/*****************************************************************************/

int main()
{
	/************************  CLIENT (for serve) CODE  **********************/
	/* Create socket, set up the server address variables,  connect to the server */
	WSADATA wsaData;
	struct sockaddr_in serverAddress;
	const char* serverIpAddress = "127.0.0.1"; // Change this to the IP address of your Python server
	int serverPort = 6666; // Change this to the port number of your Python server

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	// Create a socket
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		return 1;
	}

	// Set up the server address
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIpAddress, &serverAddress.sin_addr);

	// Connect to the server
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		printf("connect failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	
	/*****************************************************************************/

	struct GT_511C3* FP_INSTANCE = GT_511C3_GetInstance();
	//FP_INSTANCE->sEnrollStart = &vDoEnrollStart;

	/* Test the 4 cases for the EnrollStart */
	/* One test must be OK */
	/* 3 must be: Not Acknowledge: FULL_DATABASE, OUT_OF_RANGE, ALREADY_USED */
	printf("Testing Enrolment Function in a loop.\n");
	byte input = 0;
	while (True)
	{
		printf("Give a value (int) for the FingerPrint Index: ");
		scanf_s("%d", &input);

		if (input == 0) {
			printf("Exiting loop.\n");
			break;
		}

	}


	/*   TEST BYTES  packet */
	byte* packet;
	//byte* packet = (byte*)malloc(12 * sizeof(byte));

	//vDoSendCommand(packet);

	// Close the socket and clean up Winsock
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}
