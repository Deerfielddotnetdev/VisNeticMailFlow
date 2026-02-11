// VirusConfig.cpp: implementation of the CVirusConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VirusConfig.h"
#include "RegistryFns.h"
#include "DCIKey.h"
#include "AVPKeyStore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVirusConfig::CVirusConfig(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
	m_nAVEnabled = 0;
	m_nAVAction = 0;
}

CVirusConfig::~CVirusConfig()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Configure virus scanning options, which are stored in the registry
// 
////////////////////////////////////////////////////////////////////////////////
int CVirusConfig::Run( CURLAction& action )
{
	tstring sFolder;
	tstring sAction = _T("list");

	// Check security
	RequireAdmin();

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("virusconfig") ) == 0 )
	{
		// Get the current parameters from the cache and registry
		QueryDB();

		if ( m_ISAPIData.GetXMLPost() )
		{		
			DISABLE_IN_DEMO();
			DecodeForm();
			Update();
		}
		else
		{
			GenerateXML();
		}
	}
	else
	{
		if ( m_ISAPIData.GetXMLPost() )
		{		
			if( GetISAPIData().GetXMLString( _T("Action"), sAction, true ) )
			{
				if( sAction.compare( _T("generatekey") ) == 0 )
				{
					GenerateKey( action );
				}
				else if( sAction.compare( _T("register") ) == 0 )
				{
					UpdateAVP( action );
				}
			}
		}
		else
		{
			GenerateAVPXML();
		}		
	}

	return 0;
}


void CVirusConfig::Update(void)
{
	CEMSString sValue;

	sValue.Format( "%d", m_nAVEnabled );
	SetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sValue );

	if( m_nAVEnabled )
	{
		// [MER] - if AV action is "quarantine", let's verify that the quarantine folder is 
		// valid before saving it; specifically, that the path provided exists and points to 
		// a folder (not a file); could also check permissions if we really wanted to
		if (m_nAVAction == 2)
		{
			DWORD dwAttr = GetFileAttributes(m_sQuarantineFolder.c_str());
			if ((m_sQuarantineFolder.size() < 1) ||
				(dwAttr == 0xFFFFFFFF) ||
				(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY)))
			{
				CEMSString sError;
				sError.Format( _T("Quarantine folder does not exist. Please provide a valid folder path."), m_sQuarantineFolder.c_str() );
				THROW_EMS_EXCEPTION_NOLOG(  E_SystemError, sError );
			}
		}

		sValue.Format( "%d", m_nAVAction );
		SetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ACTION, sValue );
		
		WriteRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), m_sQuarantineFolder.c_str() );

		sValue.Format( "%d", m_nUnscannableAction );
		SetServerParameter( EMS_SRVPARAM_ANTIVIRUS_UNSCANNABLE_ACTION, sValue );
		
		sValue.Format( "%d", m_nSuspiciousAction );
		SetServerParameter( EMS_SRVPARAM_ANTIVIRUS_SUSPICIOUS_ACTION, sValue );

	}
	else
	{
		// The other form fields will be disabled, so grab the settings from the registry
		GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ACTION, sValue );
		m_nAVAction = _ttoi( sValue.c_str() );
		GetRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), m_sQuarantineFolder );	
	}

	GetRoutingEngine().ReloadConfig( EMS_ServerParameters );

	InvalidateServerParameters(true);
}


void CVirusConfig::QueryDB(void)
{
	tstring sValue;

	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sValue );
	m_nAVEnabled = _ttoi( sValue.c_str() );
	
	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ACTION, sValue );
	m_nAVAction = _ttoi( sValue.c_str() );

	GetRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), m_sQuarantineFolder );

	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_UNSCANNABLE_ACTION, sValue );
	m_nUnscannableAction = _ttoi( sValue.c_str() );

 	GetServerParameter( EMS_SRVPARAM_ANTIVIRUS_SUSPICIOUS_ACTION, sValue );
	m_nSuspiciousAction = _ttoi( sValue.c_str() );
}

void CVirusConfig::DecodeForm(void)
{
	GetISAPIData().GetXMLLong( _T("IsEnabled"), (int&)m_nAVEnabled, false );

	if( m_nAVEnabled )
	{
		// Get the other form fields
		GetISAPIData().GetXMLLong( _T("VirusFoundAction"), (int&)m_nAVAction, false );
		GetISAPIData().GetXMLString( _T("QuarantineFolder"), m_sQuarantineFolder, false );
		GetISAPIData().GetXMLLong( _T("UnscannableAction"), (int&)m_nUnscannableAction, false );
		GetISAPIData().GetXMLLong( _T("SuspiciousAction"), (int&)m_nSuspiciousAction, false );
	}
}


void CVirusConfig::GenerateXML(void)
{
	tstring sVersion;
	tstring sLastUpdate;
	tstring sRecordCount;
	long nErr;

	// Generate the XML
	GetXMLGen().AddChildElem(_T("VirusScanConfig"));
	GetXMLGen().AddChildAttrib( _T("Enable"), m_nAVEnabled );
	GetXMLGen().AddChildAttrib( _T("Action"), m_nAVAction );
	GetXMLGen().AddChildAttrib( _T("QuarantineFolder"), m_sQuarantineFolder.c_str() );
	GetXMLGen().AddChildAttrib( _T("UnscannableAction"), m_nUnscannableAction );
	GetXMLGen().AddChildAttrib( _T("SuspiciousAction"), m_nSuspiciousAction );
	
	GetRoutingEngine().GetAVInfo( sVersion, sRecordCount, sLastUpdate, &nErr );
	GetXMLGen().AddChildAttrib( _T("Installed"), sVersion.size() > 1  ? _T("1") : _T("0") );

	GetXMLGen().AddChildElem(_T("VirusScanActions"));
	GetXMLGen().IntoElem();

	GetXMLGen().AddChildElem(_T("Action"));
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Repair") );

	GetXMLGen().AddChildElem(_T("Action"));
	GetXMLGen().AddChildAttrib( _T("ID"), 1 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Delete") );

	GetXMLGen().AddChildElem(_T("Action"));
	GetXMLGen().AddChildAttrib( _T("ID"), 2 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Quarantine") );

	GetXMLGen().OutOfElem();

	
	GetXMLGen().AddChildElem(_T("OtherVirusScanActions"));
	GetXMLGen().IntoElem();

	GetXMLGen().AddChildElem(_T("Action"));
	GetXMLGen().AddChildAttrib( _T("ID"), 1 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Delete") );

	GetXMLGen().AddChildElem(_T("Action"));
	GetXMLGen().AddChildAttrib( _T("ID"), 2 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Quarantine") );

	GetXMLGen().AddChildElem(_T("Action"));
	GetXMLGen().AddChildAttrib( _T("ID"), 3 );
	GetXMLGen().AddChildAttrib( _T("Description"), _T("Allow") );

	GetXMLGen().OutOfElem();

}




////////////////////////////////////////////////////////////////////////////////
// 
// GenerateAVPXML
// 
////////////////////////////////////////////////////////////////////////////////
void CVirusConfig::GenerateAVPXML(void)
{
	CEMSString string;
	int nInstalled = 0;
	CEMSString sDate;

	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery() );

	GetXMLGen().AddChildElem( _T("AntiVirus") );

#if defined(DEMO_VERSION)
	GetXMLGen().AddChildAttrib( _T("Key"), _T("Demo Version") );
#else
	GetXMLGen().AddChildAttrib( _T("Key"), GetISAPIData().m_LicenseMgr.m_szAVKey );
#endif

	//DEBUGPRINT("* EMSISPI.CVirusConfig::GenerateAVPXML - is avkey present %d", GetISAPIData().m_LicenseMgr.m_AVKeyPresent);

	if( GetISAPIData().m_LicenseMgr.m_AVKeyPresent )
	{
		GetXMLGen().AddChildAttrib( _T("ShowDetails"), 1 );

		//DEBUGPRINT("* EMSISPI.CVirusConfig::GenerateAVPXML - is avkey expired %d", GetISAPIData().m_LicenseMgr.m_AVKeyExpired);

		if( GetISAPIData().m_LicenseMgr.m_AVKeyExpired == false )
		{
			GetXMLGen().AddChildAttrib( _T("Status"), _T("Key is valid") );
		}
		else
		{
			GetXMLGen().AddChildAttrib( _T("Status"), _T("Key is expired") );
		}

		AddUserSize( GetISAPIData().m_LicenseMgr.m_AVKeyInfo.UserSize );
		GetXMLGen().AddChildAttrib( _T("UserType"), DCIKeyUserTypeString( GetISAPIData().m_LicenseMgr.m_AVKeyInfo.UserType) );
		GetXMLGen().AddChildAttrib( _T("KeyType"), DCIKeyTypeString( GetISAPIData().m_LicenseMgr.m_AVKeyInfo.KeyType) );
		GetDateString( GetISAPIData().m_LicenseMgr.m_AVKeyInfo.CreateYear,
			           GetISAPIData().m_LicenseMgr.m_AVKeyInfo.CreateMonth,
					   GetISAPIData().m_LicenseMgr.m_AVKeyInfo.CreateDay, sDate );
		GetXMLGen().AddChildAttrib( _T("Created"), sDate.c_str() );

		GetDateString( GetISAPIData().m_LicenseMgr.m_AVKeyInfo.ExpireYear,
			           GetISAPIData().m_LicenseMgr.m_AVKeyInfo.ExpireMonth,
					   GetISAPIData().m_LicenseMgr.m_AVKeyInfo.ExpireDay, sDate );

		GetXMLGen().AddChildAttrib( _T("Expires"), sDate.c_str() );

		GetXMLGen().AddChildAttrib( _T("ProductCode"),  GetISAPIData().m_LicenseMgr.m_AVKeyInfo.szProductCode );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("ShowDetails"),0 );
		GetXMLGen().AddChildAttrib( _T("Status"), _T("Key is not valid") );
		GetXMLGen().AddChildAttrib( _T("UserSize"), 0 );
		GetXMLGen().AddChildAttrib( _T("UserType"), _T("") );
		GetXMLGen().AddChildAttrib( _T("KeyType"), _T("") );
		GetXMLGen().AddChildAttrib( _T("Created"), _T("") );
		GetXMLGen().AddChildAttrib( _T("Expires"), _T(""));
		GetXMLGen().AddChildAttrib( _T("ProductCode"), _T("") );
	}

	tstring sVersion;
	tstring sLastUpdate;
	tstring sRecordCount;
	long nErr;

	GetRoutingEngine().GetAVInfo( sVersion, sRecordCount, sLastUpdate, &nErr );

	GetXMLGen().AddChildAttrib( _T("EngineVersion"), sVersion.c_str() );
	GetXMLGen().AddChildAttrib( _T("DataBaseRecCount"), sRecordCount.c_str() );
	GetXMLGen().AddChildAttrib( _T("DataBaseCurDate"), sLastUpdate.c_str() );
	
	if( sVersion.size() > 1 )
		nInstalled = 1;

	bool bIsEval = nInstalled && GetISAPIData().m_LicenseMgr.m_AVKeyInfo.KeyType == DCIKEY_KeyType_Evaluation;

	GetXMLGen().AddChildAttrib( _T("Installed"), nInstalled );
	
	GetXMLGen().AddChildAttrib( _T("NoKey"), 
		nInstalled && GetISAPIData().m_LicenseMgr.m_AVKeyPresent ? _T("0") : _T("1") );

	GetXMLGen().AddChildAttrib( _T("EvalKey"), bIsEval ? _T("0") : _T("1") );
	
	GetXMLGen().AddChildAttrib( _T("RegKeyExpired"), 
		nInstalled && GetISAPIData().m_LicenseMgr.m_AVKeyExpired && !bIsEval ? _T("1") : _T("0") );
	
	GetXMLGen().AddChildAttrib( _T("EvalKeyExpired"), 
		nInstalled && GetISAPIData().m_LicenseMgr.m_AVKeyExpired && bIsEval ? _T("1") : _T("0") );

	// load about.xml to obtain the URL where the AV plug-in may be downloaded
	HSE_URL_MAPEX_INFO MapInfo;
	TCHAR szSrcPath[MAX_PATH];
	DWORD dwPathLength = MAX_PATH;
	TCHAR* Path = _T("about.xml");
	CMarkupSTL xmlfile;

	_tcsncpy( szSrcPath, GetISAPIData().m_sURLSubDir.c_str(), dwPathLength - 1 );
	_tcsncat( szSrcPath, Path, (dwPathLength - 1) - _tcslen( szSrcPath)  );
	ZeroMemory( &MapInfo, sizeof(MapInfo) );

	GetISAPIData().m_pECB->ServerSupportFunction( GetISAPIData().m_pECB->ConnID, 
		HSE_REQ_MAP_URL_TO_PATH,
		szSrcPath, &dwPathLength, 
		(DWORD*) &MapInfo );

	xmlfile.Load( szSrcPath );

	GetXMLGen().AddChildSubDoc( xmlfile.GetDoc().c_str() );
}

void CVirusConfig::AddUserSize( long nUserSize )
{
	CEMSString sUserSize;
	
	switch( nUserSize )
	{
	case DCIKEY_UserSize_1:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("1") );
		return;

	case DCIKEY_UserSize_3:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("3") );
		return;

	case DCIKEY_UserSize_6:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("6") );
		return;

	case DCIKEY_UserSize_12:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("12") );
		return;

	case DCIKEY_UserSize_25:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("25") );
		return;

	case DCIKEY_UserSize_50:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("50") );
		return;

	case DCIKEY_UserSize_100:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("100") );
		return;

	case DCIKEY_UserSize_250:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("250") );
		return;

	case DCIKEY_UserSize_500:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("500") );
		return;
		
	case DCIKEY_UserSize_1000:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("1000") );
		return;

	case DCIKEY_UserSize_2500:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("2500") );
		return;

	case DCIKEY_UserSize_Unlimited:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("Unlimited") );
		return;

	case DCIKEY_UserSize_Site:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("Site") );
		return;

	case DCIKEY_UserSize_Enterprise:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("Enterprise") );
		return;

	case DCIKEY_UserSize_Custom:		
		sUserSize.Format( _T("%d"), GetISAPIData().m_LicenseMgr.m_MFKeyInfo.SerialNum%10000 );
		GetXMLGen().AddChildAttrib( _T("UserSize"), sUserSize.c_str() );
		return;	

	default:
	case DCIKEY_UserSize_Undefined_2:
		GetXMLGen().AddChildAttrib( _T("UserSize"), _T("Undefined") );
		return;
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// UpdateAVP
// 
////////////////////////////////////////////////////////////////////////////////
int CVirusConfig::UpdateAVP(CURLAction& action)
{
	IAVPKeyStore* pKeyStore = NULL;
	HRESULT hres;
	long nErr = 0;
	tstring sPluginKey;

	if( GetISAPIData().GetXMLString( _T("pluginkey"), sPluginKey, true ) && sPluginKey.size() > 0 )
	{
		wchar_t wcsKey[100];
		MultiByteToWideChar( CP_ACP, 0, sPluginKey.c_str(), sPluginKey.size() + 1,
			                 wcsKey, 100 );
		wcsKey[99] = L'\0';
		
	
		hres = CoCreateInstance( CLSID_AVPKeyStore, NULL, CLSCTX_INPROC, 
			                     IID_IAVPKeyStore, (void**) &pKeyStore );

		if( SUCCEEDED( hres ) )
		{
			BSTR strKey = SysAllocString( wcsKey );

			pKeyStore->put_Key( strKey );
			
			SysFreeString( strKey );

			pKeyStore->AddKey( TRUE, &nErr );
		
			// Now let's make sure that the key we just added comes
			// back when we query for it.
			if( nErr == DCIKEY_Success )
			{
				BSTR str = SysAllocString( L"3CI55" );
				pKeyStore->put_AppID( str );
				SysFreeString( str );

				pKeyStore->CheckKey( &nErr );

				if( SUCCEEDED( hres ) && nErr == 0 )
				{
					BSTR strKey;
					
					if( SUCCEEDED( pKeyStore->get_Key( &strKey ) ) )
					{
						if( wcscmp( strKey, wcsKey ) != 0 )
						{
							// different key
							GetXMLGen().AddChildElem( _T("postResults") );
							GetXMLGen().IntoElem();
							GetXMLGen().AddChildAttrib( _T("complete"), _T("false") );
							GetXMLGen().AddChildAttrib( _T("error"), _T("Key does not apply to this product") );
							GetXMLGen().OutOfElem();
							SysFreeString( strKey );
							pKeyStore->Release();
							return 0;
						}

						SysFreeString( strKey );
					}
				}
			}

			pKeyStore->Release();

			if( nErr == DCIKEY_Success )
			{
				tstring sOne = _T("1");

				GetISAPIData().m_LicenseMgr.m_bKeyInfoValid = false;
				SetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sOne );
				InvalidateServerParameters( true );
				m_nAVEnabled = 1;
			}
			else
			{
				GetXMLGen().AddChildElem( _T("postresults") );
				GetXMLGen().IntoElem();
				GetXMLGen().AddChildElem( _T("complete"), _T("false") );
				GetXMLGen().AddChildElem( _T("error"), DCIKeyErrorString( nErr ) );
				GetXMLGen().OutOfElem();
			}
		}
		else
		{
			GetXMLGen().AddChildElem( _T("postresults") );
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("complete"), _T("false") );
			GetXMLGen().AddChildElem( _T("error"), _T("Product not installed") );
			GetXMLGen().OutOfElem();
		}
	}
	else
	{
		GetXMLGen().AddChildElem( _T("postresults") );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("complete"), _T("false") );
		GetXMLGen().AddChildElem( _T("error"), _T("Type in a key") );
		GetXMLGen().OutOfElem();
	}
	
	return 0;
}


void CVirusConfig::GenerateKey( CURLAction& action )
{
	IAVPKeyStore* pKeyStore = NULL;
	HRESULT hres;
	long nErr = 0;

	hres = CoCreateInstance( CLSID_AVPKeyStore, NULL, CLSCTX_INPROC, 
			                 IID_IAVPKeyStore, (void**) &pKeyStore );

	if( SUCCEEDED( hres ) )
	{
		BSTR str = SysAllocString( L"3CI55" );
		pKeyStore->put_AppID( str );
		SysFreeString( str );

		str = SysAllocString( L"AVVMF" );
		pKeyStore->put_ProductCode( str );
		SysFreeString( str );

		pKeyStore->put_UserType( DCIKEY_UserType_User );
		pKeyStore->put_KeyType( DCIKEY_KeyType_Evaluation );
		pKeyStore->put_ResellerID( 1153 );
		pKeyStore->put_SerialNum( 0 );
		pKeyStore->put_UserType( DCIKEY_UserType_User );
		pKeyStore->put_UserSize( DCIKEY_UserSize_Unlimited );

		pKeyStore->CreateEvalKey( &nErr );

		if( nErr == DCIKEY_Success )
		{
			tstring sOne = _T("1");

			GetISAPIData().m_LicenseMgr.m_bKeyInfoValid = false;
			SetServerParameter( EMS_SRVPARAM_ANTIVIRUS_ENABLE, sOne );
			InvalidateServerParameters( true );
			m_nAVEnabled = 1;
		}
		else
		{
			GetXMLGen().AddChildElem( _T("postresults") );
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("complete"), _T("false") );
			GetXMLGen().AddChildElem( _T("error"), _T("Error generating key.") );
			GetXMLGen().OutOfElem();
		}

		pKeyStore->Release();
	}	
}
