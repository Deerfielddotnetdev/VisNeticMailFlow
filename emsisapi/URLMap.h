// URLMap.h: interface for the CURLMap class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ISAPIData.h"
#include "XMLDataClass.h"

// forward declarations
class CXMLDataClass;
class CISAPIData;

// function pointer type for creation functions
typedef void (*pNewFunction) ( CXMLDataClass*& p, CISAPIData& q );

template<class T> void Create(CXMLDataClass*& p, CISAPIData& q)
{
	p = new T(q);
}

class CURLAction
{
public:

	enum CURLActionEnum { NOT_FOUND, 
						  XSL_TRANSLATE,
						  REDIRECT_URL,
						  SEND_FILE,
 						  SEND_TEMP_FILE, 
						  DO_NOTHING,
						  SEND_XML,
						};

	// Default constructor
	CURLAction()
	{
		m_nAction = NOT_FOUND;
		m_pDataObjectCreator = NULL;
		m_bAllowXMLOutput = false;
		m_bRightHandPane = false;
	}

	// Virtual destructor
	~CURLAction()
	{
	}

	// Copy constructor
	CURLAction( const CURLAction& i ) :	m_PageName( i.m_PageName ),
										m_sFileName( i.m_sFileName),
										m_nAction( i.m_nAction ),
										m_bAllowXMLOutput( i.m_bAllowXMLOutput ),
										m_sPageTitle( i.m_sPageTitle ),
										m_bRightHandPane( i.m_bRightHandPane )
	{
		m_pDataObjectCreator = m_pDataObjectCreator;
	}

	// Assignmnet operator
	CURLAction& operator=(const CURLAction& i) 
	{
		if (this != &i) // trap for assignments to self
		{
			m_PageName = i.m_PageName;
			m_sFileName = i.m_sFileName;
			m_nAction = i.m_nAction;
			m_pDataObjectCreator = i.m_pDataObjectCreator;
			m_bAllowXMLOutput = i.m_bAllowXMLOutput;
			m_sPageTitle = i.m_sPageTitle;
			m_bRightHandPane = i.m_bRightHandPane;
		}
		return *this; // return reference to self
	}

	// Helper functions
	void SetXSLTranslate( TCHAR* szPageTitle, TCHAR* szXSLFile, pNewFunction NewFunc = NULL, bool bAllowXMLOutput = false )
	{
		m_sPageTitle = szPageTitle;
		m_PageName = szXSLFile;
		m_nAction = XSL_TRANSLATE;
		m_pDataObjectCreator = NewFunc;
		m_bAllowXMLOutput = bAllowXMLOutput;
	}
	
	void SetRedirectURL( TCHAR* szURL, pNewFunction NewFunc = NULL )
	{
		m_PageName = szURL;
		m_nAction = REDIRECT_URL;
		m_pDataObjectCreator = NewFunc;
	}

	void SetSendFile( pNewFunction NewFunc )
	{
		m_sPageTitle = _T("");
		m_PageName = _T("");
		m_nAction = SEND_FILE;
		m_pDataObjectCreator = NewFunc;
	}

	void SetSendFile( TCHAR* szPath, TCHAR* szFileName )
	{
		m_nAction = SEND_FILE;
		m_PageName = szPath;
		m_sFileName = szFileName;
		m_sPageTitle = _T("application/octet-stream");
	}


	void SetSendTempFile( TCHAR* szPath, TCHAR* szFileName )
	{
		m_nAction = SEND_TEMP_FILE;
		m_PageName = szPath;
		m_sFileName = szFileName;
		m_sPageTitle = _T("application/octet-stream");
	}
	
	// data members
	int				m_nAction;				// enum CURLActionEnum
	tstring			m_PageName;				// page name
	tstring			m_sFileName;			// xsl template / file
	pNewFunction    m_pDataObjectCreator;	// function pointer to create data object
	bool			m_bAllowXMLOutput;		// honor OuputXML (if action == XSL_TRANSLATE)
	tstring			m_sPageTitle;			// Default Page Title -- or Media Type/SubType for sending files
	bool			m_bRightHandPane;		// Set right hand pane in session
};

class CURLMap  
{
public:
	CURLMap();
	virtual ~CURLMap();

	void Initialize(void);

	void GetAction( CISAPIData* pISAPIData, CURLAction& action );

	enum URLMapConstants { MAX_PAGE_NAME_LENGTH = 32 };

protected:
	inline void XSLTranslate(CURLAction& action,
		                     TCHAR* szPageTitle,
							 TCHAR* szPage,
							 TCHAR* szXSLFile,
							 pNewFunction NewFunc,
							 bool rhp,
							 bool bAllowXMLOutput = false)
	{
		action.SetXSLTranslate(szPageTitle, szXSLFile, NewFunc, bAllowXMLOutput);
		action.m_bRightHandPane=rhp;
		AddToMap(szPage,action);
	}
	inline void SendFile(CURLAction& action, TCHAR* szPage, pNewFunction NewFunc)
	{
		action.SetSendFile(NewFunc);
		AddToMap(szPage,action);
	}
	inline void RedirectURL(CURLAction& action, TCHAR* szPage,  TCHAR* szURL, pNewFunction NewFunc)
	{
		action.SetRedirectURL(szURL,NewFunc);
		AddToMap(szPage,action);
	}
	void AddToMap( TCHAR* szPage, CURLAction& action );

	map<tstring,CURLAction> m_Map;
};
