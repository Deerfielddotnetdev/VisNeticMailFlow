////////////////////////////////////////////////////////////////////////////////
//
//	$Header: /root/EMSISAPI/XSLTranslator.cpp,v 1.5.2.2 2006/02/23 20:37:13 markm Exp $
//
//  Copyright © 2002 Deerfield.com, all rights reserved
//
////////////////////////////////////////////////////////////////////////////////
//
// XSLTranslator.cpp: implementation of the CXSLTranslator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSLTranslator.h"
#include "EMSException.h"


#include <initguid.h>
#include ".\xsltranslator.h"

DEFINE_GUID( CLSID_MSXML3, 0xF5078F32, 0xC551, 0x11D3, 0x89,0xB9,0x00,0x00,0xF8,0x1F,0xE2,0x21 );
DEFINE_GUID( CLSID_MSXML4, 0x88D969C0, 0xF192, 0x11D4, 0xA6,0x5F,0x00,0x40,0x96,0x32,0x51,0xE5 );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CXSLTranslator::CXSLTranslator()
{
	m_pXML = NULL;
    m_pXSL = NULL;
}

CXSLTranslator::~CXSLTranslator()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Create the persistent XML components
// 
////////////////////////////////////////////////////////////////////////////////
int CXSLTranslator::CreateComponents(void)
{
	HRESULT hres;
	bool bUseV3 = false;

	hres = CoCreateInstance( CLSID_MSXML4, NULL, CLSCTX_INPROC_SERVER,
                             IID_IXMLDOMDocument, (void**)&m_pXML);
  
	if( FAILED(hres) )
	{
		bUseV3 = true;
		hres = CoCreateInstance( CLSID_MSXML3, NULL, CLSCTX_INPROC_SERVER,
                                 IID_IXMLDOMDocument, (void**)&m_pXML);
 
		if( FAILED(hres) )
		{
			m_pXML = NULL;
			return -1;
		}
	}

	if ( !bUseV3 )
	{
   		hres = CoCreateInstance( CLSID_MSXML4, NULL, CLSCTX_INPROC_SERVER,
			                     IID_IXMLDOMDocument, (void**)&m_pXSL);
	}
	else
	{
		hres = CoCreateInstance( CLSID_MSXML3, NULL, CLSCTX_INPROC_SERVER,
			                     IID_IXMLDOMDocument, (void**)&m_pXSL);
	}

	if( FAILED(hres) )
	{
		m_pXML->Release();
		m_pXML = NULL;
		m_pXSL = NULL;
		return -1;
	}

	return 0; // success
}

////////////////////////////////////////////////////////////////////////////////
// 
// Release the persistent XML components
// 
////////////////////////////////////////////////////////////////////////////////
void CXSLTranslator::ReleaseComponents(void)
{
	m_pXSL->Release();
	m_pXML->Release();
	m_pXML = NULL;
    m_pXSL = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// 
//  Load the XSL stylesheet from a file on disk
// 
////////////////////////////////////////////////////////////////////////////////
long CXSLTranslator::LoadXSLFromFile( tstring& xslfile )
{
	HRESULT hres;
	VARIANT vURL;
    VARIANT_BOOL vb;
	long err = 0;

	VariantInit(&vURL);

#ifdef _UNICODE
	V_BSTR(&vURL) = SysAllocString( xslfile.c_str() );	
#else
	wchar_t wcsPath[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, xslfile.c_str(), xslfile.size()+1, wcsPath, MAX_PATH );
	V_BSTR(&vURL) = SysAllocString( wcsPath );
#endif

	vURL.vt = VT_BSTR;

	hres = m_pXSL->load(vURL, &vb);

	SysFreeString( V_BSTR(&vURL) );

	if( vb == 0 )
	{
		TCHAR szErr[MAX_PATH];
		_sntprintf( szErr, MAX_PATH - 1, _T("Loading XSL: %s"), xslfile.c_str() );
		ThrowXMLError( m_pXSL, szErr );
	}

	return err;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Load the XML from a string
// 
////////////////////////////////////////////////////////////////////////////////
long CXSLTranslator::LoadXMLFromString( tstring& xml )
{
	wchar_t* wcsBuffer = NULL;
	BSTR bstrXML;
	HRESULT hres;
	VARIANT_BOOL vResult;
	long err = 0;

	/* Hack for chars under 32  - The real fix is to put all strings in CDATA */
	{
		TCHAR* p = (TCHAR*)xml.c_str();
		unsigned int i = 0;
		for( i = 0; i < xml.size(); i++ )
		{
			if( (*p >= 0 && *p < 9) || (*p == 11) || (*p == 12) || (*p > 13 && *p < 32) )
			{
				*p = _T('?');
			}
			p++;
		}
	}

	wcsBuffer = new wchar_t[xml.size()+1];

	MultiByteToWideChar( CP_ACP, 0, xml.c_str(), xml.size()+1, wcsBuffer, xml.size()+1 );

	bstrXML = SysAllocString( wcsBuffer );

	delete[] wcsBuffer;
	
	hres = m_pXML->loadXML( bstrXML, &vResult );

	if( vResult == 0 )
	{
		TCHAR szErr[MAX_PATH];
		_sntprintf( szErr, MAX_PATH, _T("Loading XML: %s"), xml.c_str() );
		ThrowXMLError( m_pXML, szErr );
	}

	SysFreeString( bstrXML );

	return err;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Transform the XML into HTML
// 
////////////////////////////////////////////////////////////////////////////////
long CXSLTranslator::Transform( tstring& html )
{
	BSTR bstrHTML;

#ifdef _DEBUG
	DWORD start = GetTickCount();
#endif

	// MJM - this function doesn't have a return code, instead it 
	// throw a "COM exception".  In order to get this information in 
	// C++ we need to call the ISupportErrorInfo interface...
	if (FAILED(m_pXML->transformNode( m_pXSL, &bstrHTML ))) 
	{
		CEMSString sError = _T("Unknown error transforming XML");
		IErrorInfo *pei = 0;

		if (GetErrorInfo(0, &pei) == S_OK) 
		{
			ISupportErrorInfo *psei;

			if (SUCCEEDED(m_pXML->QueryInterface(IID_ISupportErrorInfo, (void**)&psei))) 
			{
				if (psei->InterfaceSupportsErrorInfo(IID_IXMLDOMDocument) == S_OK) 
				{
					BSTR bstr; 
					pei->GetDescription(&bstr); 
					sError.Format( _T("Error transforming XML: %S"), bstr );
					SysFreeString(bstr);
				}
				psei->Release();
			}
			pei->Release();
		}

		THROW_EMS_EXCEPTION( E_XSLTranslation, sError.c_str() );
	}

#ifdef _DEBUG
	DWORD end = GetTickCount();
#endif
	
	int nChars = SysStringLen(bstrHTML)+1;
	html.resize( nChars, ' ' );

	WideCharToMultiByte( CP_ACP, 0, bstrHTML, nChars,
						 (char*)(html.c_str()), nChars, NULL, NULL );

	SysFreeString( bstrHTML );

	tstring::size_type pos = html.find( _T("UTF-16") );
	if( pos != tstring::npos )
	{
		html.replace( pos, 6, _T("UTF-8") );
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Translate the XML string using the XSL file into HTML
// 
////////////////////////////////////////////////////////////////////////////////
int CXSLTranslator::TranslateXML( tstring& xml, tstring& xslfile, tstring& html )
{
	if( m_pXSL == NULL || m_pXML == NULL )
		return -1;

	LoadXSLFromFile( xslfile );

	LoadXMLFromString( xml );

	Transform( html );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Gather error information from the XMLDOMDocument object and throw an
// EMS Exception
// 
////////////////////////////////////////////////////////////////////////////////
void CXSLTranslator::ThrowXMLError( IXMLDOMDocument* pXMLDoc, TCHAR* szOperation )
{
	HRESULT hres;

	IXMLDOMParseError  *pXMLError = NULL;
	hres = pXMLDoc->get_parseError(&pXMLError);

	if( SUCCEEDED( hres) && pXMLError )
	{
		BSTR bstrReason = NULL;
		BSTR bstrSrcText = NULL;
		long line = 0;
		long linepos = 0;

		pXMLError->get_reason( &bstrReason );
		pXMLError->get_line( &line );
		pXMLError->get_linepos( &linepos );
		pXMLError->get_srcText( &bstrSrcText );
		pXMLError->Release();

		tstring the_reason(_T("No reason was given"));
		tstring the_text(_T("Does not contain source text"));

#ifdef _UNICODE
		if(bstrReason)
		{
			the_reason.assign((LPCTSTR)(BSTR)bstrReason);
			SysFreeString( bstrReason );
		}
		else
		{
			SetError(pXMLDoc);
		}

		if(bstrSrcText)
		{
			the_text.assign((LPCTSTR)(BSTR)bstrSrcText);
			SysFreeString( bstrSrcText );
		}
		else
		{
			SetError(pXMLDoc);
		}
#else
		if(bstrReason)
		{
			dca::String temporary_reason((BSTR)bstrReason);
			the_reason.assign(temporary_reason.c_str());
			SysFreeString( bstrReason );
		}
		else
		{
			SetError(pXMLDoc);
		}

		if(bstrSrcText)
		{
			dca::String temporary_text((BSTR)bstrSrcText);
			the_text.assign(temporary_text.c_str());
			SysFreeString( bstrSrcText );
		}
		else
		{
			SetError(pXMLDoc);
		}
#endif

		CEMSString szMsg;
		szMsg.Format( _T("Error while (%s): Reason: \"%s\" Line: %d, Position: %d. %s"), 
			szOperation, the_reason.c_str(), line, linepos, the_text.c_str() );

		THROW_EMS_EXCEPTION( E_XSLTranslation, szMsg );
	}
	else
	{
		THROW_EMS_EXCEPTION( E_XSLTranslation, _T("XSL Translation Error - Unable to retrieve error information") );
	}
}

void CXSLTranslator::SetError(IXMLDOMDocument* pXMLDoc)
{
	if(pXMLDoc)
	{
		dca::SmartInterface<IErrorInfo> pei;

		dca::BString in;
		dca::String theErr;
		pXMLDoc->get_xml(in.GetRef());

		if(GetErrorInfo(0, pei.GetRef()) == S_OK) 
		{
			dca::SmartInterface<ISupportErrorInfo> psei;

			if (SUCCEEDED(pXMLDoc->QueryInterface(IID_ISupportErrorInfo, (void**)psei.GetRef()))) 
			{
				if (psei->InterfaceSupportsErrorInfo(IID_IXMLDOMDocument) == S_OK) 
				{
					dca::BString bstr; 
					pei->GetDescription(bstr.GetRef());
					CEMSString szMsg;
					if(bstr)
					{
#ifdef _UNICODE
						dca::String err(bstr);

						theErr = err;
#else
						dca::String err(bstr);

						theErr = err.c_str();
#endif
					}
					else
					{
						theErr.assign(_T("Could not a description for the xml error"));
					}

					szMsg.Format( _T("Error : %s"), theErr.c_str());

					THROW_EMS_EXCEPTION( E_XSLTranslation, szMsg);
				}
			}
		}
	}
}
