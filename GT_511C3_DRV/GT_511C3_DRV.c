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

/* Reduce the use of maginc numbers in code */
#define PACKET_SIZE 12
#define HEX_TO_DEC_ACK_30 48
#define HEX_TO_DEC_NACK_30 49
#define LOW_ERROR_CODE_POSITION 4
#define HIGH_ERROR_CODE_POSITION 5
#define READING_BYTE_BUFFER_SIZE 1
#define ACK_NACK_PACKET_POSITION 8

#define GET_FIRST_BYTE(x) (byte)((x) & 0xFF) 
#define GET_SECOND_BYTE(x) (byte)((x >> 8) & 0xFF) 
#define GET_THIRD_BYTE(x) (byte)((x >> 16) & 0xFF) 
#define GET_FOURTH_BYTE(x) (byte)((x >> 24) & 0xFF) 


/*************************   Coomand - Response Packet  ***********************/

/*Calculte and return the ERROR NAME */
const char* cDoGetErrorName(DWORD code) {
	for (byte i = 0; i < sizeof(error_codes) / sizeof(error_codes[0]); i++) {
		if (error_codes[i].code == code) {
			return error_codes[i].name;
		}
	}
	return "UNKNOWN_ERROR";
}

/* Returns the completed Packet Command */
byte* vDoCreatePacket(struct sCMD_RSP_PKT* GT_PKT)
{
	/* Allocate memory for the packet */
	byte* packet = (byte*)malloc(PACKET_SIZE * sizeof(byte));
	if (packet == NULL)
	{
		fprintf(stderr, "ERROR Unable to allocate memory in vDoCreatePacket function \n.");
		return;
	}

	WORD cmd = GT_PKT->eCommands;

	packet[0] = CMD_START_CODE_1;
	packet[1] = CMD_START_CODE_2;
	packet[2] = CMD_DEVICE_ID;
	packet[3] = NULL_BYTE;		
	packet[4] = GET_FIRST_BYTE(GT_PKT->sParameter);		/* sParameter is type DWORD -> it has 32 bits; */
	packet[5] = GET_SECOND_BYTE(GT_PKT->sParameter);		/* Shift the bits accordingly in order to get the coresponding bytes */
	packet[6] = GET_THIRD_BYTE(GT_PKT->sParameter);
	packet[7] = GET_FOURTH_BYTE(GT_PKT->sParameter);
 	packet[8] = (byte)GET_FIRST_BYTE(cmd);							/* First byte of the command */
	packet[9] = (byte)GET_SECOND_BYTE(cmd);					/* Second byte of the command */
	
	WORD checksum = GT_PKT->sCalcChecksum(packet);				/* Calculate the packet checksum */
	packet[10] = GET_FIRST_BYTE(checksum);
	packet[11] = GET_SECOND_BYTE(checksum);

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
	for (byte i = 0; i < PACKET_SIZE; i++)
	{
		printf("%02X ", *(packet + i));
	}

	// Send a (12 bytes) PACKET to the server
	if (send(clientSocket, packet, PACKET_SIZE, 0) < 0)
	{
		printf("Send failed. Error Code : %d", WSAGetLastError());
	}
}

/* Get as a parameter a response packet, and interprets it */
void vDoInterpretResponse(byte* pck_response)
{
	/* Only one test will be implemented here: */
	/* To test if the message is ACK or NOT_ACK and the value of the received param (error) */
	/* TObeDone: Length / sumcheck / correct cmd or address check*/
	if (*(pck_response + ACK_NACK_PACKET_POSITION) == HEX_TO_DEC_ACK_30)
	{
		printf("RESPONSE: ACK - OK");		
	}
	else
	{
		WORD full_error;
		const char* name;
		byte error_code[2];

		error_code[1] = *(pck_response + LOW_ERROR_CODE_POSITION);
		error_code[0] = *(pck_response + HIGH_ERROR_CODE_POSITION);
		full_error = error_code[0];
		full_error = (full_error << 8) | error_code[1];
		name = cDoGetErrorName(full_error);
		printf("NAK: Error code 0x%x is %s\n", full_error, name);
	}
	
}

/* Receive Response Packet */
void vDoReceivePacket(struct sCMD_RSP_PKT* RSP_PACKET)
{
	byte readbyte = 0;       /* Container for coming bytes */
	bool readFlag = True;    /* Flag used to mark the beginning of a command */

	/* Allocate memory for a 12 bytes packet */
	byte* response = (byte*)malloc(PACKET_SIZE * sizeof(byte));
	if (response == NULL)
	{
		fprintf(stderr, "ERROR Unable to allocate memory in vDoReceivePacket function \n.");
		return;
	}

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
		for (byte i = 1; i < PACKET_SIZE; i++)
		{
			byte_received = recv(clientSocket, &response[i], READING_BYTE_BUFFER_SIZE, 0);
			printf("%02x ", response[i]);
		}
		printf("\n");

		/* Interpret the response message */
		vDoInterpretResponse(response);

		/* Deallocate memory */
		free(response);
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
			fprintf(stderr, "ERROR FP_SENSOR pointer is NULL\n.");
			return;
		}

		/*  Memory allocation for Command(send) - Response(received) Packet variables  */
		FP_SENSOR->sSENT_GT_PKT = (struct sCMD_RSP_PKT*)malloc(sizeof(struct sCMD_RSP_PKT));
		if (FP_SENSOR->sSENT_GT_PKT == NULL)
		{
			fprintf(stderr, "ERROR FP_SENSOR->sSENT_GT_PKT pointer is NULL\n.");
			bMemoryLeak = True;
		}

		FP_SENSOR->sRECEIVED_GT_PKT = (struct sCMD_RSP_PKT*)malloc(sizeof(struct sCMD_RSP_PKT));
		if (FP_SENSOR->sRECEIVED_GT_PKT == NULL)
		{
			fprintf(stderr, "ERROR FP_SENSOR->sRECEIVED_GT_PKT pointer is NULL\n.");
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

			fprintf(stderr, "ERROR while allocating memory memory.\n.");
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

void vDoEnrollStart()
{
	if (FP_SENSOR == NULL)
	{
		fprintf(stderr, "Error FP_SENSOR pointer is NULL\n.");
		return;
	}
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
	FP_INSTANCE->sEnrollStart = &vDoEnrollStart;

	/* Test the 4 cases for the EnrollStart */
	/* One test must be OK */
	/* 3 must be: Not Acknowledge: FULL_DATABASE, OUT_OF_RANGE, ALREADY_USED */
	//printf("Testing Enrolment Function in a loop.\n");
	//byte input = 0;
	//while (True)
	//{
	//	printf("Give a value (int) for the FingerPrint Index: ");
	//	scanf_s("%d", &input);

	//	if (input == 0) {
	//		printf("Exiting loop.\n");
	//		break;
	//	}

	//}


	/*   TEST BYTES  packet */
	//byte* packet;
	//byte* packet = (byte*)malloc(12 * sizeof(byte));

	//vDoSendCommand(packet);

	// Close the socket and clean up Winsock
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}
