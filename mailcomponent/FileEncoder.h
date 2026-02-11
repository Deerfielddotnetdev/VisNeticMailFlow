// FileEncoder.h: interface for the CFileEncoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEENCODER_H__93515242_23B7_447E_82E5_E98611F66EDB__INCLUDED_)
#define AFX_FILEENCODER_H__93515242_23B7_447E_82E5_E98611F66EDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "base64\ByteBuffer.h"
#include "base64\CharBuffer.h"

class CFileEncoder  
{
public:
	const int Decode(const HANDLE hFileIn, const HANDLE hFileOut, const UINT nEncodeType);
	const int Encode(const HANDLE hFileIn, const HANDLE hFileOut, const UINT nEncodeType);
	const int Decode(const tstring& sFileIn, const tstring& sFileOut, const UINT nEncodeType);
	const int Encode(const tstring& sFileIn, const tstring& sFileOut, const UINT nEncodeType);
	CFileEncoder();
	virtual ~CFileEncoder();

	enum _encoding_type
	{
		type_base64,
		type_qp
	};

private:
	const int DoEncoding();
	const int DoDecoding();
	const BOOL OpenFiles(const tstring& sFileIn, const tstring& sFileOut);
	void CloseFiles();

	ByteBuffer m_sBufByte;
	CharBuffer m_sBufChar;

	const int m_nChunkSize;

	int m_nEncodeType;

	HANDLE m_hFileSource;
	HANDLE m_hFileDest;

};

#endif // !defined(AFX_FILEENCODER_H__93515242_23B7_447E_82E5_E98611F66EDB__INCLUDED_)
