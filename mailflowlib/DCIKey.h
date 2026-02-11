///////////////////////////////////////////////////////////////////////////////
//
// Deerfield.com key generation and analysis routines
//
//  $Header: /root/MAILSTREAMLIBRARY/DCIKey.h,v 1.1 2005/05/06 14:56:44 markm Exp $ 
//
// Copyright 2002 Deerfield.com
//
///////////////////////////////////////////////////////////////////////////////

#ifndef DCIKEY_H
#define DCIKEY_H

// version 2.6

// Note: These are close to, but not exactly the bit positions. However, the field sizes are correct.
// +-----------------------+-----------------------+-----------------------+-----------------------+
// | 0  1  2  3  4  5  6  7| 8  9 10 11 12 13 14 15|16 17 18 19 20 21 22 23|24 25 26 27 28 29 30 31|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |     PC-1     |     PC-2     |     PC-3     |     PC-4     |     PC-5     |     AI-1     |AI-2A|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  AI-2B |     AI-3     |     AI-4     |     AI-5     | User Size |  KT |       CheckSum        |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |      Expire Year   | Expire Mon| Expire Day   |      Create Year   | Create Mon| Create Day   |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 Serial#                                                 | UT  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                   Reseller ID                 | The key data is 18 bytes (144 bits). 19 bytes
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ are used in the buffer because the 
//                                                   output symbols use 145 bits.

#define DCIKEY_APPID_LENGTH				5
#define DCIKEY_PRODUCT_CODE_LENGTH		5
#define DCIKEY_APPID_CNT				64
#define DCIKEY_BUFFSIZE					19
#define DCIKEY_KEYCHARS					30
#define DCIKEY_CHARGROUP				7
#define DCIKEY_KEYLENGTH				(DCIKEY_KEYCHARS + 4)
#define DCIKEY_DATASIZE					(32+256+DCIKEY_APPID_CNT*(DCIKEY_APPID_LENGTH+1)+16)

// user sizes
typedef enum DCIUserSizeEnum 
{ 
	DCIKEY_UserSize_1					= 0,
	DCIKEY_UserSize_3					= 1,	
	DCIKEY_UserSize_6					= 2,
	DCIKEY_UserSize_12					= 3,		
	DCIKEY_UserSize_25					= 4,
	DCIKEY_UserSize_50					= 5,
	DCIKEY_UserSize_100					= 6,
	DCIKEY_UserSize_250					= 7,
	DCIKEY_UserSize_500					= 8,
	DCIKEY_UserSize_1000				= 9,
	DCIKEY_UserSize_2500				= 10,
	DCIKEY_UserSize_Unlimited			= 11,
	DCIKEY_UserSize_Site				= 12,
	DCIKEY_UserSize_Enterprise			= 13,
	DCIKEY_UserSize_Custom				= 14,
	DCIKEY_UserSize_Undefined_2			= 15
} DCIUserSizeEnum;

// user types
typedef enum DCIUserTypeEnum 
{ 
	DCIKEY_UserType_User				= 0,
	DCIKEY_UserType_Pack				= 1,
	DCIKEY_UserType_Undefined1			= 2,
	DCIKEY_UserType_Undefined2			= 3,
} DCIUserTypeEnum;

// key types
typedef enum DCIKeyTypeEnum 
{ 
	DCIKEY_KeyType_Evaluation			= 0,
	DCIKEY_KeyType_Registered			= 1,
	DCIKEY_KeyType_Undefined1			= 2,
	DCIKEY_KeyType_Undefined2			= 3
} DCIKeyTypeEnum;

// error codes
typedef enum DCIErrorCodes 
{ 
	DCIKEY_Success						= 0,
	DCIKEY_Error_Invalid_Day			= 1,	
	DCIKEY_Error_Invalid_Month			= 2,
	DCIKEY_Error_Invalid_AppID			= 3,
	DCIKEY_Error_Invalid_ProductCode	= 4,
	DCIKEY_Error_Invalid_Key			= 5,
	DCIKEY_Error_Invalid_Year			= 6,
	DCIKEY_Error_No_Key_Found			= 7,
	DCIKEY_Error_Key_Expired			= 8,
	DCIKEY_Error_Registered_Key_Exists	= 9,

} DCIErrorCodes;

typedef struct DCIKeyStruct
{
	char szAppID[DCIKEY_APPID_LENGTH + 1];				// 25 bits 5 char App ID   (v)		
	char szProductCode[DCIKEY_PRODUCT_CODE_LENGTH + 1];	// 25 bits 5 char Product Code
	int UserSize;										// 4 bits  User Size
	int UserType;										// 2 bits  User or Pack
	int KeyType;										// 2 bits  Key type
	int SerialNum;										// 30 bits 
	int ExpireYear;										// 7 bits  2000 - 2127
	int ExpireMonth;									// 4 bits  1 - 12     (v)
	int ExpireDay;										// 5 bits  1 - 31     (v)
	int CreateYear;										// 7 bits  2000 - 2127
	int CreateMonth;									// 4 bits  1 - 12     (v*)
	int CreateDay;										// 5 bits  1 - 31     (v*)
	int Checksum;										// 8 bits			  (v)
	int ResellerID;										// 16 bits  0-65535
} DCIKeyStruct;


#ifdef __cplusplus
extern "C"{
#endif 

// call this before calling any others and do not call again!
void DCIKeyInitialize(void);

// function: encode a structure into ascii buffer of length DCIKEY_KEYLENGTH
//           this version sets the dates, key type, and serial number.
//   return: any of DCIErrorCodes
int DCIKeyCreateEval( DCIKeyStruct* pKeyStruct, char* szASCIIKey );

// function: encode a structure into ascii buffer of length DCIKEY_KEYLENGTH
//   return: any of DCIErrorCodes
int DCIKeyGenerate( DCIKeyStruct* pKeyStruct, char* szASCIIKey );

// function: decode a key of length DCIKEY_KEYLENGTH into a structure.
//   return: any of DCIErrorCodes
int DCIKeyAnalyze( char* szASCIIKey, DCIKeyStruct* pKeyStruct );

// function: compare 5 character APPID to allowed values
//   return: DCIKEY_Success or DCIKEY_Error_Invalid_AppID
int DCIKeyValidateAppID( char* szAppID );


// function: check if key contains valid 34 character sequence.
// This a quick character check and is suitable for use in scanning for keys.
int DCIKeyValidate( char* szKey );

// function: test if key is expired
// returns:   DCIKEY_Success or DCIKEY_Error_Key_Expired
int DCIKeyExpired( DCIKeyStruct* pKeyStruct );

// function: general-purpose encryption, nBytes must be at greater than 8
void DCIKeyEncrypt( char* szBuff, int nBytes );

// function: general-purpose decryption, nBytes must be at greater than 8
void DCIKeyDecrypt( char* szBuff, int nBytes );

// function: return strings for some enumerations        
char* DCIKeyTypeString( int KeyType );
char* DCIKeyUserTypeString( int UserType );
char* DCIKeyUserSizeString( int UserSize );
char* DCIKeyErrorString( int ErrorCode );

#ifdef __cplusplus
}
#endif 


#endif // DCIKEY_H