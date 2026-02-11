///////////////////////////////////////////////////////////////////////////////
//
// Deerfield.com key generation and analysis routines
//
//  $Header: /root/MAILSTREAMLIBRARY/DCIKey.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $ 
//
// Copyright 2002 Deerfield.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DCIKey.h"

// version 1.2
 
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


// This clever bit counter shifts a mask instead of incrementing a counter.			STB
typedef struct BitCounter_t
{
	char* ptr;
	int Mask;
	int Checksum;
} BitCounter_t;

//  GLOBALS
////////////////////////////////////////////
extern unsigned char buffer[DCIKEY_DATASIZE];			// data.cpp (encrypted)
extern char g_endchar;


// LOCALS
////////////////////////////////////////////
int Initialized = 0;
unsigned char* pC2A = buffer;
unsigned char* pA2C = pC2A + 32;
unsigned char* pAPPIDs = pA2C+256;
unsigned char* pKey = pAPPIDs + DCIKEY_APPID_CNT*(DCIKEY_APPID_LENGTH+1);


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int ASCII2Code( int n )
{
	return (int)pA2C[n];
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int Code2ASCII( int n )
{
	return (int)pC2A[n];
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
void InitBitCounter( BitCounter_t* b, char* szBuff )
{
	memset( b, 0, sizeof(BitCounter_t) );
	b->Mask = 1;
	b->ptr = szBuff;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
void PutBits( int Value, int Bits, int nCheckSumFlag, BitCounter_t* b )
{
	int ValueMask = 1;

	if( nCheckSumFlag )
	{
		b->Checksum += Value;
	}

	while( Bits > 0 )
	{
		if( b->Mask == 0x100 )
		{
			b->ptr++;
			b->Mask = 1;
		}
	
		if( Value & ValueMask )
		{
			b->ptr[0] |= b->Mask;
		}

		Bits--;
		ValueMask = ValueMask << 1;
		b->Mask = b->Mask << 1;
	}	
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int GetBits( int Bits, int nCheckSumFlag, BitCounter_t* b )
{
	int ValueMask = 1;
	int Value = 0;

	while( Bits > 0 )
	{
		if( b->Mask == 0x100 )
		{
			b->ptr++;
			b->Mask = 1;
		}
	
		if( b->ptr[0] & b->Mask )
		{
			Value |= ValueMask;
		}

		Bits--;
		ValueMask = ValueMask << 1;
		b->Mask = b->Mask << 1;
	}	

	if( nCheckSumFlag )
	{
		b->Checksum += Value;
	}

	return Value;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int StuffBuffer( DCIKeyStruct* pKeyStruct, char* szBuff, BitCounter_t* b )
{
	int Code;

	// initialize
	memset( szBuff, 0, DCIKEY_BUFFSIZE );
	InitBitCounter( b, szBuff );

	// stuff the bits
	Code = ASCII2Code( pKeyStruct->szProductCode[0] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_ProductCode;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szProductCode[1] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_ProductCode;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szProductCode[2] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_ProductCode;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szProductCode[3] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_ProductCode;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szProductCode[4] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_ProductCode;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szAppID[0] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_AppID;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szAppID[1] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_AppID;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szAppID[2] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_AppID;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szAppID[3] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_AppID;
	PutBits( Code , 5, 1, b );
	Code = ASCII2Code( pKeyStruct->szAppID[4] );
	if( Code == 0xff ) return DCIKEY_Error_Invalid_AppID;
	PutBits( Code , 5, 1, b );
	PutBits( pKeyStruct->UserSize, 4, 1, b );
	PutBits( pKeyStruct->KeyType, 2, 1, b );
	PutBits( pKeyStruct->SerialNum, 30, 1, b );
	PutBits( pKeyStruct->ExpireYear, 7, 1, b );
	PutBits( pKeyStruct->ExpireMonth, 4, 1, b );
	PutBits( pKeyStruct->ExpireDay, 5, 1, b );
	PutBits( pKeyStruct->CreateYear, 7, 1, b );
	PutBits( pKeyStruct->CreateMonth, 4, 1, b );
	PutBits( pKeyStruct->CreateDay, 5, 1, b );
	PutBits( pKeyStruct->ResellerID, 16, 1, b );
	PutBits( pKeyStruct->UserType, 2, 1, b );

	pKeyStruct->Checksum = b->Checksum & 0xff;
	PutBits( pKeyStruct->Checksum, 8, 0, b );

	return DCIKEY_Success;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
void BufferToStruct( char* szBuff, DCIKeyStruct* pKeyStruct, BitCounter_t* b )
{
	// grab the bits
	InitBitCounter( b, szBuff );
	memset( pKeyStruct, 0, sizeof(DCIKeyStruct) );

	pKeyStruct->szProductCode[0] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szProductCode[1] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szProductCode[2] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szProductCode[3] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szProductCode[4] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szAppID[0] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szAppID[1] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szAppID[2] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szAppID[3] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->szAppID[4] = Code2ASCII( GetBits( 5, 1, b ) );
	pKeyStruct->UserSize = GetBits( 4, 1, b );
	pKeyStruct->KeyType = GetBits( 2, 1, b );
	pKeyStruct->SerialNum = GetBits( 30, 1, b );
	pKeyStruct->ExpireYear = GetBits( 7, 1, b );
	pKeyStruct->ExpireMonth = GetBits( 4, 1, b );
	pKeyStruct->ExpireDay = GetBits( 5, 1, b );
	pKeyStruct->CreateYear = GetBits( 7, 1, b );
	pKeyStruct->CreateMonth = GetBits( 4, 1, b );
	pKeyStruct->CreateDay = GetBits( 5, 1, b );
	pKeyStruct->ResellerID = GetBits( 16, 1, b );
	pKeyStruct->UserType = GetBits( 2, 1, b );
	pKeyStruct->Checksum = GetBits( 8, 0, b );
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int DCIKeyValidateAppID( char* szAppID )
{
	int i;

	if( Initialized == 0 )
		DCIKeyInitialize();

	for ( i = 0; i < DCIKEY_APPID_CNT; i++)
	{
		if( strncmp( szAppID, (char*)(pAPPIDs +i*(DCIKEY_APPID_LENGTH+1)),
			         DCIKEY_APPID_LENGTH ) == 0 )
			return DCIKEY_Success;
	}
	return DCIKEY_Error_Invalid_AppID;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int ValidateStruct( DCIKeyStruct* pKeyStruct, BitCounter_t* b )
{
	// Easy stuff - check the checksum
	if( (0xff & pKeyStruct->Checksum) != (0xff & b->Checksum) )
		return DCIKEY_Error_Invalid_Key;

	// Harder stuff - validate limited fields
	if( pKeyStruct->ExpireMonth == 0 || pKeyStruct->ExpireMonth > 12 )
		return DCIKEY_Error_Invalid_Month;

	if( pKeyStruct->CreateMonth == 0 || pKeyStruct->CreateMonth > 12 )
		return DCIKEY_Error_Invalid_Month;

	if( pKeyStruct->ExpireDay == 0 || pKeyStruct->CreateDay == 0 )
		return DCIKEY_Error_Invalid_Day;

	// Do this step last! Make sure we have a valid APP ID
	return DCIKeyValidateAppID( pKeyStruct->szAppID );
}

/************************************************

The Tiny Encryption Algorithm (TEA) by 
David Wheeler and Roger Needham of the
Cambridge Computer Laboratory

**** ANSI C VERSION ****

Notes:

TEA is a Feistel cipher with XOR and
and addition as the non-linear mixing
functions. 

Takes 64 bits of data in v[0] and v[1].
Returns 64 bits of data in w[0] and w[1].
Takes 128 bits of key in k[0] - k[3].

TEA can be operated in any of the modes
of DES. Cipher Block Chaining is, for example,
simple to implement.

n is the number of iterations. 32 is ample,
16 is sufficient, as few as eight may be OK.
The algorithm achieves good dispersion after
six iterations. The iteration count can be
made variable if required.

Note this is optimised for 32-bit CPUs with
fast shift capabilities. It can very easily
be ported to assembly language on most CPUs.

delta is chosen to be the real part of (the
golden ratio Sqrt(5/4) - 1/2 ~ 0.618034
multiplied by 2^32). 

************************************************/
void encipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k)
{
	register unsigned long y=v[0],z=v[1],sum=0,delta=0x9E3779B9,
		a=k[0],b=k[1],c=k[2],d=k[3],n=32;
	
	while(n-->0)
	{
		sum += delta;
		y += (z<<4)+a ^ z+sum ^ (z>>5)+b;
		z += (y<<4)+c ^ y+sum ^ (y>>5)+d;
	}
	
	w[0]=y; w[1]=z;
}

void decipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k)
{
	register unsigned long y=v[0],z=v[1],sum=0xC6EF3720,
		delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],
		d=k[3],n=32;
	
	/* sum = delta<<5, in general sum = delta * n */
	
	while(n-->0)
	{
		z -= (y<<4)+c ^ y+sum ^ (y>>5)+d;
		y -= (z<<4)+a ^ z+sum ^ (z>>5)+b;
		sum -= delta;
	}
	
	w[0]=y; w[1]=z;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// Note that nBytes must be at least 8
void DCIKeyEncrypt( char* szBuff, int nBytes )
{
	unsigned long nTemp[2];
	int i;

	if( Initialized == 0 )
		DCIKeyInitialize();

	for( i = 0; i < nBytes - 7; i++ )
	{
		encipher( (unsigned long *)&szBuff[i], nTemp, (unsigned long*)pKey );
		memcpy( &szBuff[i], nTemp, 8 );
	}
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// Note that nBytes must be at least 8
void DCIKeyDecrypt( char* szBuff, int nBytes )
{
	unsigned long nTemp[2];
	int i;

	if( Initialized == 0 )
		DCIKeyInitialize();

	for( i = nBytes - 8; i >= 0; i-- )
	{
		decipher( (unsigned long *)&szBuff[i], nTemp, (unsigned long*)pKey );
		memcpy( &szBuff[i], nTemp, 8 );
	}
}


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
void DCIKeyInitialize(void)
{
	unsigned long nTemp[2];
	int i;

	// don't do this twice.
	if( Initialized )
		return;

	for ( i = 0; i <= DCIKEY_DATASIZE - 16 - 8; i += 4 )
	{
		decipher( (unsigned long* const)(buffer + i), nTemp, 
			      (unsigned long* const)pKey );
		memcpy( (unsigned long* const)(buffer + i), nTemp, 8 );
	}

	Initialized = 1;
}


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int GetSerialNo(void)
{
	HKEY hKey;
	DWORD cbSize = sizeof(DWORD);
	DWORD dwValue;
	DWORD dwType;
	long retval;

	retval = RegCreateKeyExA( HKEY_LOCAL_MACHINE, "Software\\Deerfield.com", 0, NULL,
		                     NULL, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKey, NULL );

	if( retval == ERROR_SUCCESS )
	{
		retval = RegQueryValueExA( hKey, "DCIKeyIndex", 0, &dwType, 
			                      (LPBYTE)&dwValue, &cbSize );
	}

	if( retval != ERROR_SUCCESS )
	{
		dwValue = GetTickCount();
	}

	dwValue = (dwValue + 1) & 0x3ffffff;  // 30 bits

	RegSetValueExA( hKey, "DCIKeyIndex", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD) );

	RegCloseKey( hKey );

	return dwValue;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int DCIKeyAnalyze( char* szASCIIKey, DCIKeyStruct* pKeyStruct )
{
	char szBuff[DCIKEY_BUFFSIZE];
	BitCounter_t bc;
	int i,j=0;
	int n;

	if( Initialized == 0 )
		DCIKeyInitialize();

	// initialize
	memset( szBuff, 0, sizeof(szBuff) );
	InitBitCounter( &bc, szBuff );

	// translate
	for( i = 0; i < DCIKEY_KEYLENGTH; i++ )
	{
		if( (i+1) % DCIKEY_CHARGROUP == 0 )
		{
			if( szASCIIKey[j++] != '-')
				return DCIKEY_Error_Invalid_Key;
		}
		else if( i == DCIKEY_KEYLENGTH - 1 && szASCIIKey[j] != g_endchar )
		{
			return DCIKEY_Error_Invalid_Key;
		}
		else
		{
			n = ASCII2Code( szASCIIKey[j++] );
			if( n == -1 )
				return DCIKEY_Error_Invalid_Key;
			PutBits( n, 5, 0, &bc );
		}
	}

	// decrypt
	DCIKeyDecrypt( szBuff, 18 );

	// grab out our data
	BufferToStruct( szBuff, pKeyStruct, &bc );

	// validation
	if( ValidateStruct( pKeyStruct, &bc ) )
		return DCIKEY_Error_Invalid_Key;

	// de-normalize years
	pKeyStruct->ExpireYear += 2000;
	pKeyStruct->CreateYear += 2000;

	return DCIKEY_Success;
}


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int DCIKeyGenerate( DCIKeyStruct* pKeyStruct, char* szASCIIKey )
{
	char szBuff[DCIKEY_BUFFSIZE];
	BitCounter_t bc;
	int i,j = 0;
	int retval;

	if( Initialized == 0 )
		DCIKeyInitialize();

	// normalize expire year
	if( pKeyStruct->ExpireYear < 2000 || pKeyStruct->ExpireYear > 2127 )
		return DCIKEY_Error_Invalid_Year;
	pKeyStruct->ExpireYear -= 2000;

	// normalize create year
	if( pKeyStruct->CreateYear < 2000 || pKeyStruct->CreateYear > 2127 )
		return DCIKEY_Error_Invalid_Year;
	pKeyStruct->CreateYear -= 2000;


	// if serial num blank, grab from registry
	if( pKeyStruct->SerialNum == 0 )
	{
		pKeyStruct->SerialNum = GetSerialNo();
	}

	retval = StuffBuffer( pKeyStruct, szBuff, &bc );
	if( retval )
		return retval;

	// validation
	retval = ValidateStruct( pKeyStruct, &bc );
	if( retval )
		return retval;

	// encrypt
	DCIKeyEncrypt( szBuff, 18 );

	// translate
	InitBitCounter( &bc, szBuff );
	for( i = 0; i < DCIKEY_KEYLENGTH; i++ )
	{
		if( i == (DCIKEY_KEYLENGTH - 1) )
		{
			// last character is from data.cpp
			szASCIIKey[j] = g_endchar;
		}
		else if( (i+1) % DCIKEY_CHARGROUP == 0 )
		{
			szASCIIKey[j++] = '-';
		}
		else
		{
			szASCIIKey[j++] = Code2ASCII( GetBits( 5, 0, &bc ) );
		}
	}

	// de-normalize years
	pKeyStruct->ExpireYear += 2000;
	pKeyStruct->CreateYear += 2000;


	return DCIKEY_Success;
}


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int DCIKeyExpired( DCIKeyStruct* pi )
{
	SYSTEMTIME now;

	GetLocalTime( &now );

	// check Expire date
	if( pi->ExpireYear < now.wYear)
	{
		return DCIKEY_Error_Key_Expired;
	}
	else if( pi->ExpireYear == now.wYear)
	{
		if( pi->ExpireMonth < now.wMonth )
		{
			return DCIKEY_Error_Key_Expired;
		}
		else if( pi->ExpireMonth == now.wMonth )
		{
			if( pi->ExpireDay < now.wDay )
			{
				return DCIKEY_Error_Key_Expired;
			}
		}
	}

	// check Create date
	if( pi->CreateYear > now.wYear)
	{
		return DCIKEY_Error_Key_Expired;
	}
	else if( pi->CreateYear == now.wYear)
	{
		if( pi->CreateMonth > now.wMonth )
		{
			return DCIKEY_Error_Key_Expired;
		}
		else if( pi->CreateMonth == now.wMonth )
		{
			if( pi->CreateDay > now.wDay )
			{
				return DCIKEY_Error_Key_Expired;
			}
		}
	}

	return DCIKEY_Success;
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
int DCIKeyCreateEval( DCIKeyStruct* pi, char* szASCIIKey )
{
	SYSTEMTIME now;
	__int64 ft;

	GetLocalTime( &now );

	pi->KeyType = DCIKEY_KeyType_Evaluation;
	pi->SerialNum = 0;

	pi->CreateDay = now.wDay;
	pi->CreateYear = now.wYear;
	pi->CreateMonth = now.wMonth;

	SystemTimeToFileTime( &now, (FILETIME*) &ft );


	// Expire time is always 30 days.
	ft += (__int64)(30*24*60*60) * (__int64)(10000000);

	FileTimeToSystemTime( (FILETIME*)&ft, &now );
  

	pi->ExpireDay = now.wDay;
	pi->ExpireYear = now.wYear;
	pi->ExpireMonth = now.wMonth;

	return DCIKeyGenerate( pi, szASCIIKey );
}


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// This is a fast check for a well formed key string. It does not analyze the key 
// or guarantee its validity.
int DCIKeyValidate( char* szData )
{
	int nKeyCntr = 0;

	while( *szData != '\0' && nKeyCntr < DCIKEY_KEYLENGTH )
	{

		if( ((nKeyCntr + 1) % DCIKEY_CHARGROUP == 0) && *szData == '-' )
		{
			nKeyCntr++;
			szData++;
		}
		else if( ( *szData >= 'A' && *szData <= 'Z' ) || 
				 ( *szData >= '1' && *szData <= '6' ) )
		{
			nKeyCntr++;
			szData++;
		}
		else
		{
			// invalid char
			return DCIKEY_Error_Invalid_Key;
		}
	}

	if( nKeyCntr == DCIKEY_KEYLENGTH )
	{
		// looks good
		return DCIKEY_Success;
	}

	// key too short
	return DCIKEY_Error_Invalid_Key;
}


// Note the following are english string literals meant for debugging.
// They would be better off in the resources for the release version.


// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
char* DCIKeyTypeString( int KeyType )
{
	switch( KeyType )
	{
	case DCIKEY_KeyType_Evaluation:
		return "Evaluation";
	case DCIKEY_KeyType_Registered:
		return "Registered";
	}
	return "Undefined";
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
char* DCIKeyUserSizeString( int UserSize )
{
	switch( UserSize )
	{
	case DCIKEY_UserSize_1:
		return "1";
	case DCIKEY_UserSize_3:
		return "3";
	case DCIKEY_UserSize_6:
		return "6";
	case DCIKEY_UserSize_12:
		return "12";
	case DCIKEY_UserSize_25:
		return "25";
	case DCIKEY_UserSize_50:
		return "50";
	case DCIKEY_UserSize_100:
		return "100";
	case DCIKEY_UserSize_250:
		return "250";
	case DCIKEY_UserSize_500:
		return "500";
	case DCIKEY_UserSize_1000:
		return "1000";
	case DCIKEY_UserSize_Unlimited:
		return "Unlimited";
	case DCIKEY_UserSize_Site:
		return "Site";
	}
	return "Undefined";
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
char* DCIKeyUserTypeString( int UserType )
{
	switch( UserType )
	{
	case DCIKEY_UserType_User:
		return "User";
	case DCIKEY_UserType_Pack:
		return "Pack";
	}
	return "Undefined";
}

// +-----------------------+-----------------------+-----------------------+-----------------------+
// |31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0|
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
char* DCIKeyErrorString( int ErrorCode )
{
	switch( ErrorCode )
	{
	case DCIKEY_Success:
		return "Success";
	case DCIKEY_Error_Invalid_Day:
		return "Invalid Day";
	case DCIKEY_Error_Invalid_Month:
		return "Invalid Month";
	case DCIKEY_Error_Invalid_AppID:
		return "Invalid AppID";
	case DCIKEY_Error_Invalid_ProductCode:
		return "Invalid Product Code";
	case DCIKEY_Error_Invalid_Key:
		return "Invalid Key";
	case DCIKEY_Error_Invalid_Year:
		return "Invalid Year";
	case DCIKEY_Error_No_Key_Found:
		return "No Key Found";
	case DCIKEY_Error_Key_Expired:
		return "Key Expired";
	case DCIKEY_Error_Registered_Key_Exists:
		return "Can't replace with eval key";
	}
	return "Undefined";
}

