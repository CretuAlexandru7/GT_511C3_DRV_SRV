#ifndef GT_511C3_DRIVER_h
#define GT_511C3_DRIVER_h

/*****************************  Libraries /  Header files  ************************/
#include <windows.h>
#include "stdint.h"
/**********************************************************************************/


/*******  Compiler time constants are needed in order to use TRUE / FALSE  ********/
#ifndef __cplusplus
typedef unsigned char bool;
#define False ((bool)0)
#define True ((bool)1)
#endif
/**********************************************************************************/


/*******  GLOBAL CONST VARIALBES - used in both command and response packet  ******/
static const byte CMD_START_CODE_1 = 0X55;
static const byte CMD_START_CODE_2 = 0XAA;
static const WORD CMD_DEVICE_ID = 0x01;       /*  Device ID -> WORD (2 bytes)  */
static const WORD PACKET_SIZE = 12;
static const byte NULL_BYTE = 0x00;       /*  Device ID -> WORD (2 bytes)  */
/**********************************************************************************/


/*******         Structure used to store Command - Response Packet        ********/
struct sCMD_RSP_PKT {

	enum Commands
	{
		NOT_A_COMMAND = 0x00,				// Command packets will not have an ERROR_CODE
		Initialize_FP = 0x01,				// Initialization
		Close = 0x02,						// Termination
		UsbInternalCheck = 0x03,			// Check if the connected USB device is valid
		ChangeEBaudRate = 0x04,				// Change UART baud rate
		SetIAPMode = 0x05,					// Enter IAP Mode In this mode, FW Upgrade is available
		CmosLed = 0x12,						// Control CMOS LED
		GetEnrollCount = 0x20,				// Get enrolled fingerprint count
		CheckEnrolled = 0x21,				// Check whether the specified ID is already enrolled
		EnrollStart = 0x22,					// Start an enrollment
		Enroll1 = 0x23,						// Make 1st template for an enrollment
		Enroll2 = 0x24,						// Make 2nd template for an enrollment
		Enroll3 = 0x25,						// Make 3rd template for an enrollment, merge three templates into one template, save merged template to the database
		IsPressFinger = 0x26,				// Check if a finger is placed on the sensor
		DeleteID = 0x40,					// Delete the fingerprint with the specified ID
		DeleteAll = 0x41,					// Delete all fingerprints from the database
		Verify1_1 = 0x50,					// 1:1 Verification of the capture fingerprint image with the specified ID
		Identify1_N = 0x51,					// 1:N Identification of the capture fingerprint image with the database
		VerifyTemplate1_1 = 0x52,			// 1:1 Verification of a fingerprint template with the specified ID
		IdentifyTemplate1_N = 0x53,			// 1:N Identification of a fingerprint template with the database
		CaptureFinger = 0x60,				// Capture a fingerprint image(256x256) from the sensor
		MakeTemplate = 0x61,				// Make template for transmission
		GetImage = 0x62,					// Download the captured fingerprint image(256x256)
		GetRawImage = 0x63,					// Capture & Download raw fingerprint image(320x240)
		GetTemplate = 0x70,					// Download the template of the specified ID
		SetTemplate = 0x71,					// Upload the template of the specified ID
		GetDatabaseStart = 0x72,			// Start database download, obsolete
		GetDatabaseEnd = 0x73,				// End database download, obsolete
		UpgradeFirmware = 0x80,				// !!! NOT SUPPORTED - Firmware UPGRADE
		UpgradeISOCDImage = 0x81,			// !!! NOT SUPPORTED - Non-acknowledge
		Ack = 0x30,							// Acknowledge.
		Nack = 0x31							// Non-acknowledge
	}eCommands;


	enum Error_Codes
	{
		NOT_AN_ERROR = 0X000,					// Cmd packets will not have a COMMAND.
		NACK_TIMEOUT = 0x1001,					// Obsolete, capture timeout
		NACK_INVALID_BAUDRATE = 0x1002,			// Obsolete, Invalid serial baud rate
		NACK_INVALID_POS = 0x1003,				// The specified ID is not between 0~19
		NACK_IS_NOT_USED = 0x1004,				// The specified ID is not used
		NACK_IS_ALREADY_USED = 0x1005,			// The specified ID is already used
		NACK_COMM_ERR = 0x1006,					// Communication Error
		NACK_VERIFY_FAILED = 0x1007,			// 1:1 Verification Failure
		NACK_IDENTIFY_FAILED = 0x1008,			// 1:N Identification Failure
		NACK_DB_IS_FULL = 0x1009,				// The database is full
		NACK_DB_IS_EMPTY = 0x100A,				// The database is empty
		NACK_TURN_ERR = 0x100B,					// Obsolete, Invalid order of the enrollment (The order was not as: EnrollStart -> Enroll1 -> Enroll2 -> Enroll3)
		NACK_BAD_FINGER = 0x100C,				// Too bad fingerprint
		NACK_ENROLL_FAILED = 0x100D,			// Enrollment Failure
		NACK_IS_NOT_SUPPORTED = 0x100E,			// The specified command is not supported
		NACK_DEV_ERR = 0x100F,					// Device Error, especially if Crypto-Chip is trouble
		NACK_CAPTURE_CANCELED = 0x1010,			// Obsolete, The capturing is canceled
		NACK_INVALID_PARAM = 0x1011,			// Invalid parameter
		NACK_FINGER_IS_NOT_PRESSED = 0x1012,	// Finger is not pressed
	}eError_Codes;

	DWORD sParameter;

	/* Function pointers specific for the SENT PACKET */
	/* Function pointer to the computing packet function */
	byte* (*sCalcPacket)(struct sCMD_RSP_PKT*);
	/* Function pointer to the checksum calculation function */
	WORD(*sCalcChecksum)(byte* packet);
};
/**********************************************************************************/



/*******                    GT_511C3 FINGERPRINT SENSOR               ********/
struct GT_511C3
{
	bool sUseSerialDebug;							// Enables / Disables printing DEBUGing info over the serial communnication.

	void(*sGT_511C3)();								// New FP Scaner object
	void(*sInitialize_FP)();									// Initialize the FP Scanner
	void(*sClose)();
	bool(*sSetLed)(bool on_off);					// Turns on or off the LED backlight (NEEDED FOR SCANNING)
	bool(*sChangeBoundRate)(int boundR);			// Changes the boundrate of the connection
	int(*sGetEnrollCount)();							// Get the DataBase size
	bool(*sCheckEnrolled)(int id);					// Checks if the FP index is already used
	int(*sEnrollStart)(int id);						// Starts the enrolment process;
	int(*sEnroll1)();								// Get the first image of the FP
	int(*sEnroll2)();								// Get the second image of the FP
	int(*sEnroll3)();								// Get the third image of the FP
	bool(*sIsPressFinger)();							// Is the FP pressed?
	bool(*sDeleteID)(int ID);						// Delete a specific FP
	bool(*sDeleteAll)();								// Delete all FPs
	int(*sVerify1_1)();								// Compare the pressed FP to a specific one
	int(*sIdentify1_N)();							// Compare the pressed FP to all the FP from the database
	bool(*sCaptureFinger)(bool highquality);			// Campture the pressed FP image and temporary store it into RAM (TRUE = highquality)


	void(*SendToSerial)(byte data[], int length);	
	struct sCMD_RSP_PKT*(*GetResponse)();

	/* Function pointer to the seding packet function */
	void(*sSendCommand)(byte* packet);
	/* Function pointers specific for the RECEIVED PACKET */
	void(*sReceiveResponsePacket)(struct sCMD_RSP_PKT*);

	struct sCMD_RSP_PKT* sSENT_GT_PKT;
	struct sCMD_RSP_PKT* sRECEIVED_GT_PKT;
};
/**********************************************************************************/


#endif