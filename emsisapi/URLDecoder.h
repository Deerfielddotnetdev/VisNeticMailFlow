// URLDecoder.h: interface for the CURLDecoder class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct ISAPIPARAM
{
	TCHAR*		m_szValue;		// pointer to where the encoded value is
	int			nLength;		// length of encoded value (always greater than decoded value)
	bool		m_bIsFile;		// is this a form field or a file?
	tstring		m_Filename;		// name of file
	tstring		m_ContentType;	// MIME type
};


class CURLDecoder  
{
	enum{ MAX_KEY_LENGTH=128 };

public:
	CURLDecoder();
	virtual ~CURLDecoder();

	// These methods find and decode parameters
	BOOL FindURLParam( LPCTSTR lpszKey, ISAPIPARAM& param );
	BOOL FindFormField( LPCTSTR lpszKey, ISAPIPARAM& param, bool bSpell = false );
	int DecodeValue( ISAPIPARAM& param, TCHAR* szBuffer, int nBufferLength, bool bURLEncoded );
	int DecodeValue( ISAPIPARAM& param, TCHAR* szBuffer, int nBufferLength )
	{ 
		return DecodeValue( param, szBuffer, nBufferLength, m_bURLEncoded );
	}

	// These methods are helper functions which use the methods above
	bool GetFormString(LPCTSTR szField, tstring& sValue, bool bOptional = false, bool bSpell = false);
	bool GetURLString(LPCTSTR szField, tstring& sValue, bool bOptional = false);
	bool GetFormTCHAR(LPCTSTR szField, TCHAR* szValue, int szValueLen, bool bOptional = false);
	bool GetURLTCHAR(LPCTSTR szField, TCHAR* szValue, int szValueLen, bool bOptional = false);
	bool GetFormTCHAR(LPCTSTR szField, TCHAR** ppBuffer, long& nLength, long& nAllocated, bool bOptional = false);
	bool GetFormLong(LPCTSTR szField, int& nValue, bool bOptional = false);
	bool GetURLLong(LPCTSTR szField, int& nValue, bool bOptional = false);
	bool GetFormLong(LPCTSTR szField, unsigned char& nValue, bool bOptional = false);
	bool GetURLLong(LPCTSTR szField, unsigned char& nValue, bool bOptional = false);
	void GetFormBit(LPCTSTR szField, unsigned char& nValue);
	int GetFormFile( LPCTSTR szField, tstring& sOrigFilename, tstring& sActualFilename, 
		             tstring& sMimeType, tstring& sMimeSubType, int& nBytes, bool bIsSig = false );
	int GetFormNoteFile( LPCTSTR szField, tstring& sOrigFilename, tstring& sActualFilename, 
		             tstring& sMimeType, tstring& sMimeSubType, int& nBytes );
public:
	// The maps
	map<tstring,ISAPIPARAM> m_URL;		// URL parameters map (a.k.a Query String)
	map<tstring,ISAPIPARAM> m_Form;	// Form fields map

protected:

	void SetBuffer( TCHAR* szBuffer, int nLength );
	int GetParameter( ISAPIPARAM& param );
	TCHAR* GetKey(void) { return szKey;}
	int HexDigitToInt( char c );
	void SetParameter( ISAPIPARAM& param );
	
	TCHAR* ptr,*start,*end,*keyptr,*keyend;
	TCHAR szKey[MAX_KEY_LENGTH];
	bool m_bGotKey;
	bool m_bURLEncoded;
	bool m_bGotEqualSign;
};
