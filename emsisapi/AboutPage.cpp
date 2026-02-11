// AboutPage.cpp: implementation of the CAboutPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AboutPage.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAboutPage::CAboutPage( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CAboutPage::~CAboutPage()
{

}


////////////////////////////////////////////////////////////////////////////////
// 
//  Run - the main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CAboutPage::Run( CURLAction& action )
{
	HSE_URL_MAPEX_INFO MapInfo;
	TCHAR szSrcPath[MAX_PATH];
	DWORD dwPathLength = MAX_PATH;

	// Copy the requested path
	_tcsncpy( szSrcPath, m_ISAPIData.m_pECB->lpszPathInfo, MAX_PATH );

	// zero terminate to be safe
	szSrcPath[MAX_PATH-1] = _T('\0');

	// find the last slash
	TCHAR* p = _tcsrchr( szSrcPath, _T('/') );

	if( p )
	{
		_tcsncpy( p+1, _T("about.xml"), MAX_PATH - ( p + 1 - szSrcPath ) );
	}

	// zero terminate to be safe
	szSrcPath[MAX_PATH-1] = _T('\0');
	
	ZeroMemory( &MapInfo, sizeof(MapInfo) );
	
	m_ISAPIData.m_pECB->ServerSupportFunction( m_ISAPIData.m_pECB->ConnID, 
		                                       HSE_REQ_MAP_URL_TO_PATH,
						                       szSrcPath, &dwPathLength, 
											   (DWORD*) &MapInfo );

	GetXMLGen().Load( szSrcPath );
	
	tstring sVersion;

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_VERSION_VALUE, sVersion );

	GetXMLGen().ResetPos();
	GetXMLGen().FindElem();
	GetXMLGen().AddChildElem( _T("Version") );
	GetXMLGen().AddChildAttrib( _T("Number"), sVersion.c_str() );
#if defined(HOSTED_VERSION)
	GetXMLGen().AddChildElem( _T("License"), _T("Hosted Version") );
#else
	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery() );

	if( GetISAPIData().m_LicenseMgr.m_MFKeyPresent == true )
	{
		if( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.KeyType == DCIKEY_KeyType_Registered )
		{		
			//DebugReporter::Instance().DisplayMessage("CAboutPage::Run - MailFlow license is registered", DebugReporter::ISAPI, GetCurrentThreadId());
			GetXMLGen().AddChildElem( _T("License"), _T("Registered Version") );
		}
		else
		{
			if( GetISAPIData().m_LicenseMgr.m_MFKeyExpired == false )
			{
				//DebugReporter::Instance().DisplayMessage("CAboutPage::Run - MailFlow has an evaluation license", DebugReporter::ISAPI, GetCurrentThreadId());
				GetXMLGen().AddChildElem( _T("License"), _T("Evaluation Version") );
			}
			else
			{
				//DebugReporter::Instance().DisplayMessage("CAboutPage::Run - MailFlow has an evaluation license that is expired", DebugReporter::ISAPI, GetCurrentThreadId());
				GetXMLGen().AddChildElem( _T("License"), _T("Evaluation Expired") );
			}
		}
	}
	else
	{
		//DebugReporter::Instance().DisplayMessage("CAboutPage::Run - MailFlow license is unregistered", DebugReporter::ISAPI, GetCurrentThreadId());
		GetXMLGen().AddChildElem( _T("License"), _T("Unregistered") );
	}
#endif
	return 0;
}