#pragma once

#include "SessionMap.h"
#include "MarkupSTL.h"
#include "RoutingEngine.h"
#include "XMLCache.h"
#include "AccessControl.h"
#include "FileMap.h"
#include "URLDecoder.h"
#include "LicenseMgr.h"

class CISAPIData : public CURLDecoder
{
public:
	CISAPIData(CODBCQuery& query, CXMLCache& XMLCache, CSessionMap& SessionMap, CLicenseMgr& licenseMgr );
	virtual ~CISAPIData();

	void Initialize( EXTENSION_CONTROL_BLOCK* pECB );

	void SetErrorXML( int ErrorCode, const char* szErrorType, const char* szErrorMsg, LPCTSTR szField = _T("") );
	void GetParameters( bool& bURLEncoded );
	
	bool GetClientDataInMemFile( void )  { return m_bClientDataInMemFile; }
	void CloseMemFile( void )			 { m_MemFile.Close(); }

	void AddURLParamsToXML( CMarkupSTL& xml );
	void SetRightHandPane( LPCTSTR szURL = NULL );

	bool GetXMLTCHAR(LPCTSTR szField, TCHAR* szValue, int szValueLen, bool bOptional = false);
	bool GetXMLTCHAR(LPCTSTR szField, TCHAR** ppBuffer, long& nLength, long& nAllocated, bool bOptional = false);
	bool GetXMLLong(LPCTSTR szField, int& nValue, bool bOptional = false);
	bool GetXMLLong(LPCTSTR szField, unsigned char& nValue, bool bOptional = false);
	bool GetXMLString( LPCTSTR szField, tstring& sValue, bool bOptional = false );

	bool GetXMLPost( void ) { return m_bXMLPost; }
	bool GetUseExceptionEMS( void ) { return m_bUseExceptionEMS; }
	void SetUseExceptionEMS( bool bUseExceptionEMS ) { m_bUseExceptionEMS = bUseExceptionEMS; }
	int  GetUploadID(void) {return m_UploadID;}

public:
	CODBCQuery& m_query;				// shared query object
	CXMLCache& m_XMLCache;				// shared XML cache object
	CSessionMap& m_SessionMap;			// shared session map
	CLicenseMgr& m_LicenseMgr;			// shared license managager

	tstring m_sPage;					// name of the page requested (w/o extension)
	CAgentSession* m_pSession;
	CBrowserSession* m_pBrowserSession;
	CMarkupSTL m_xmlgen;				// XML to be translated with XSL
	CRoutingEngine	m_RoutingEngine;	// COM object for communicating w/ Routing Engine
	CMarkupSTL m_xmldoc;				// XML to be appended to HTML
	tstring m_sChgNotSavedMsg;			// Changes not saved message - for <DOCUMENT> Tag
	
	HANDLE m_hKillEvent;				// Worker thread's kill event
	TCHAR m_szContentType[MAX_PATH];
	EXTENSION_CONTROL_BLOCK* m_pECB;
	tstring m_sSessionID;
	tstring	m_sURLSubDir;
	
	map<tstring, tstring> m_XMLFormMap;

protected:
	int ReadClientData( void );
	int ParseMultiPart( void );
	void ParseXMLPost( void );
	int GetBoundary( void );
	int ExtractQuotedField( TCHAR* p, TCHAR* e, TCHAR* szFieldName,
		                    tstring& sField, BOOL UpperCase );
	int ExtractTag( TCHAR* p, TCHAR* e, TCHAR* szFieldName, tstring& sField );

	TCHAR* ClientBuffer(void);					// These functions return pointers the data
	unsigned int ClientBufferLength(void);		// sent by the client which may or may not
												// be in the mem file.

	CFileMap			m_MemFile;
	bool				m_bClientDataInMemFile;
	bool				m_bXMLPost;
	bool				m_bUseExceptionEMS;
	unsigned long		m_MemFileBytesUsed;
	tstring				m_sBoundary;
	int					m_UploadID;
};
