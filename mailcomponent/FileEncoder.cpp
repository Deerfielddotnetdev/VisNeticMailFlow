// FileEncoder.cpp: implementation of the CFileEncoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileEncoder.h"
#include "MailComponents.h"
#include "base64\Base64Encoder.h"
#include "base64\Base64Decoder.h"
#include "qp\QpEncoder.h"
#include "qp\QpDecoder.h"
#include "Utility.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileEncoder::CFileEncoder(): m_nChunkSize(2048)
{
	// character buffer is larger than the byte buffer because
	// encoding goes from bytes to chars (and expands in the process);
	// in constrast, decoding goes from chars to bytes (and shrinks in the process)
	m_sBufByte.bytes = new char[m_nChunkSize];
	m_sBufChar.chars = new char[m_nChunkSize * 4];

	m_hFileSource = INVALID_HANDLE_VALUE;
	m_hFileDest = INVALID_HANDLE_VALUE;

	m_nEncodeType = type_base64;
}

CFileEncoder::~CFileEncoder()
{
	delete m_sBufByte.bytes;
	delete m_sBufChar.chars;
}

const int CFileEncoder::Encode(const tstring& sFileIn, const tstring& sFileOut, const UINT nEncodeType)
{
	assert(sFileIn.size() != 0);
	assert(sFileOut.size() != 0);

	// open source and destination files
	if (!OpenFiles(sFileIn, sFileOut))
	{
		LINETRACE(_T("Error opening files in CFileEncoder::Encode()\n"));
		assert(0);
		return ERROR_FILESYSTEM;
	}

	m_nEncodeType = nEncodeType;

	int nRet = DoEncoding();

	CloseFiles();

	return nRet;
}

const BOOL CFileEncoder::OpenFiles(const tstring& sFileIn, const tstring& sFileOut)
{
	// open the source file
	m_hFileSource = CreateFile(	sFileIn.c_str(), 
								GENERIC_READ,			// generic read ops
								FILE_SHARE_READ,		// read sharing
								NULL,					// no security
								OPEN_EXISTING,			// open existing or fail
								FILE_ATTRIBUTE_NORMAL,	// normal file
								NULL);
	if (m_hFileSource == INVALID_HANDLE_VALUE)
	{
		LINETRACE(_T("Could not open source file for encoding: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		assert(0);
		return FALSE;
	}
	
	// create the destination file
	m_hFileDest = CreateFile(	sFileOut.c_str(), 
								GENERIC_WRITE,			// generic write ops
								0,						// exclusive lock
								NULL,					// no security
								CREATE_ALWAYS,			// always create
								FILE_ATTRIBUTE_NORMAL,	// normal file
								NULL);
	if (m_hFileDest == INVALID_HANDLE_VALUE)
	{
		LINETRACE(_T("Could not open destination file for encoding: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
		assert(0);
		return FALSE;
	}

	return TRUE; // success
}

const int CFileEncoder::Decode(const tstring& sFileIn, const tstring& sFileOut, const UINT nEncodeType)
{
	assert(sFileIn.size() != 0);
	assert(sFileOut.size() != 0);
	
	// open source and destination files
	if (!OpenFiles(sFileIn, sFileOut))
	{
		LINETRACE(_T("Error opening files in CFileEncoder::Decode()\n"));
		assert(0);
		return ERROR_FILESYSTEM;
	}

	m_nEncodeType = nEncodeType;

	int nRet = DoDecoding();

	CloseFiles();

	return nRet;
}

const int CFileEncoder::DoEncoding()
{
	int nRet = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;
	bool bQPInit = false;

	// declare encoder structs
	Base64Encoder encoder64;
	QpEncoder encoderQP;

	// initialize only the encoder we intend to use
	switch (m_nEncodeType)
	{
	case type_base64:
		Base64Encoder_setMaxLineLen(&encoder64, 76);
		Base64Encoder_setOutputCrLf(&encoder64, true);
		Base64Encoder_start(&encoder64);
		break;
	case type_qp:
		QpEncoder_initialize(&encoderQP);
		bQPInit = true;
		break;
	default:
		assert(0);
		nRet = ERROR_BAD_PARAMETER;
		goto cleanup;
	}
	
	while (1)
	{
		// read a chunk from the source file
		if (!ReadFile(	m_hFileSource,
						m_sBufByte.bytes,
						m_nChunkSize,
						&dwBytesRead,
						NULL))
		{
			LINETRACE(_T("Unable to read source file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			assert(0);
			nRet = ERROR_FILESYSTEM;
			goto cleanup;
		}

		// init input buffer
		m_sBufByte.pos = 0;
		m_sBufByte.endPos = dwBytesRead;

		// init output buffer
		m_sBufChar.pos = 0;
		m_sBufChar.endPos = m_nChunkSize * 4;

		// if EOF
		if (dwBytesRead == 0)
		{
			if (m_nEncodeType == type_base64)
				Base64Encoder_finish(&encoder64, &m_sBufChar);
			else
				QpEncoder_finish(&encoderQP, &m_sBufChar);

			// write out an encoded chunk to the destination file
			if (!WriteFile(	m_hFileDest,
							m_sBufChar.chars,
							m_sBufChar.pos,
							&dwBytesWritten,
							NULL))
			{
				LINETRACE(_T("Unable to write destination file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
				assert(0);
				nRet = ERROR_FILESYSTEM;
				goto cleanup;
			}

			break;
		}

		// perform the encoding
		switch (m_nEncodeType)
		{
		case type_base64:
			Base64Encoder_encode(&encoder64, &m_sBufByte, &m_sBufChar);
			break;
		case type_qp:
			QpEncoder_encode(&encoderQP, &m_sBufByte, &m_sBufChar);
			break;
		default:
			assert(0);
			nRet = ERROR_BAD_PARAMETER;
			goto cleanup;
		}

		// the input buffer should have been completely consumed
		assert(m_sBufByte.pos == m_sBufByte.endPos);

		// output buffer shouldn't be totally full
		assert(m_sBufChar.pos != m_sBufChar.endPos);

		// write out an encoded chunk to the destination file
		if (!WriteFile(	m_hFileDest,
						m_sBufChar.chars,
						m_sBufChar.pos,
						&dwBytesWritten,
						NULL))
		{
			LINETRACE(_T("Unable to write destination file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			assert(0);
			nRet = ERROR_FILESYSTEM;
			goto cleanup;
		}
	}

cleanup:
	if (m_nEncodeType == type_qp && bQPInit)
		QpEncoder_finalize(&encoderQP);

	return nRet;
}

const int CFileEncoder::Encode(const HANDLE hFileIn, const HANDLE hFileOut, const UINT nEncodeType)
{
	assert(hFileIn != INVALID_HANDLE_VALUE);
	assert(hFileOut != INVALID_HANDLE_VALUE);

	m_hFileSource = hFileIn;
	m_hFileDest = hFileOut;

	m_nEncodeType = nEncodeType;

	return DoEncoding();

	// note: file handles are NOT closed automatically in this function; provides
	// lots of flexibility (e.g. appending files during encoding, etc.), but don't
	// forget to close those handles!
}

void CFileEncoder::CloseFiles()
{
	if (m_hFileDest != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileDest);

	if (m_hFileSource != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileSource);
}

const int CFileEncoder::DoDecoding()
{		
	int nRet = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;
	bool bQPInit = false;
	
	// declare decoder structs
	Base64Decoder decoder64;
	QpDecoder decoderQP;
	
	// initialize only the decoder we intend to use
	switch (m_nEncodeType)
	{
	case type_base64:
		Base64Decoder_start(&decoder64);
		break;
	case type_qp:
		QpDecoder_initialize(&decoderQP);
		QpDecoder_setOutputCrLf(&decoderQP, true);
		bQPInit = true;
		break;
	default:
		assert(0);
		nRet = ERROR_BAD_PARAMETER;
		goto cleanup;
	}
	
	while (1)
	{
		// read a chunk from the source file
		if (!ReadFile(	m_hFileSource,
						m_sBufChar.chars,
						m_nChunkSize,
						&dwBytesRead,
						NULL))
		{
			LINETRACE(_T("Unable to read source file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			assert(0);
			nRet = ERROR_FILESYSTEM;
			goto cleanup;
		}

		// init in buffer
		m_sBufChar.pos = 0;
		m_sBufChar.endPos = dwBytesRead;
		
		// init out buffer
		m_sBufByte.pos = 0;
		m_sBufByte.endPos = m_nChunkSize * 4;

		// if EOF
		if (dwBytesRead == 0)
		{
			// apparently Base64Decoder doesn't have a finish call?
			if (m_nEncodeType == type_qp)
				QpDecoder_finish(&decoderQP, &m_sBufByte);

			if (!WriteFile(	m_hFileDest,
							m_sBufByte.bytes,
							m_sBufByte.pos,
							&dwBytesWritten,
							NULL))
			{
				LINETRACE(_T("Unable to write destination file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
				assert(0);
				nRet = ERROR_FILESYSTEM;
				goto cleanup;
			}

			break;
		}
		
		// perform the decoding
		switch (m_nEncodeType)
		{
		case type_base64:
			Base64Decoder_decode(&decoder64, &m_sBufChar, &m_sBufByte);
			break;
		case type_qp:
			QpDecoder_decode(&decoderQP, &m_sBufChar, &m_sBufByte);
			break;
		default:
			assert(0);
			nRet = ERROR_BAD_PARAMETER;
			goto cleanup;
		}
		
		// write out an encoded chunk to the destination file
		if (!WriteFile(	m_hFileDest,
						m_sBufByte.bytes,
						m_sBufByte.pos,
						&dwBytesWritten,
						NULL))
		{
			LINETRACE(_T("Unable to write destination file: %d (%s)\n"), GetLastError(), CUtility::GetErrorString(GetLastError()).c_str());
			assert(0);
			nRet = ERROR_FILESYSTEM;
			goto cleanup;
		}
	}
	
cleanup:

	if (m_nEncodeType == type_qp && bQPInit)
		QpDecoder_finalize(&decoderQP);

	return nRet;
}

const int CFileEncoder::Decode(const HANDLE hFileIn, const HANDLE hFileOut, const UINT nEncodeType)
{
	assert(hFileIn != INVALID_HANDLE_VALUE);
	assert(hFileOut != INVALID_HANDLE_VALUE);

	m_hFileSource = hFileIn;
	m_hFileDest = hFileOut;

	m_nEncodeType = nEncodeType;

	return DoDecoding();

	// note: file handles are NOT closed automatically in this function; provides
	// lots of flexibility (e.g. appending files during encoding, etc.), but don't
	// forget to close those handles!
}
