// Registration.cpp: implementation of the CRegistration class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Registration.h"
#include "DCIKey.h"
#include "AVPKeyStore.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistration::CRegistration(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CRegistration::~CRegistration()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CRegistration::Run( CURLAction& action )
{
	tstring sAction = _T("list");

	// Check security
	RequireAdmin();

	GetISAPIData().GetFormString( _T("Action"), sAction, true );
	
	if( sAction.compare( _T("update") ) == 0 )
	{
		DISABLE_IN_DEMO();
		Update( action );
	}

	return ListAll();
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
int CRegistration::ListAll(void)
{
	GenerateXML();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update
// 
////////////////////////////////////////////////////////////////////////////////
int CRegistration::Update(CURLAction& action)
{
	DCIKeyStruct keyStruct;
	int nErr;

	if( GetISAPIData().GetFormString( _T("productkey"), m_sKey, true ) && m_sKey.size() > 0 )
	{

#ifdef _UNICODE
	#pragma error "This won't work under UNICODE"
#else
		nErr = DCIKeyAnalyze( (char*) m_sKey.c_str(), &keyStruct );
#endif
		GetXMLGen().AddChildElem( _T("postresults") );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("code"), nErr );
		GetXMLGen().OutOfElem();

		if( strcmp( keyStruct.szAppID, "3WGFQ" ) )
		{
			GetXMLGen().AddChildAttrib( _T("Description"), "Invalid Key" );
		}
		else
		{
			if( nErr == DCIKEY_Success )
				GetXMLGen().AddChildAttrib( _T("Description"), _T("Key entered successfully") );
			else
				GetXMLGen().AddChildAttrib( _T("Description"), DCIKeyErrorString( nErr ) );

			if( nErr == DCIKEY_Success )
			{
				SetServerParameter( EMS_SRVPARAM_LICENSE_KEY, m_sKey );
				GetISAPIData().m_LicenseMgr.m_bKeyInfoValid = false;
				GetRoutingEngine().ReloadConfig( EMS_Registration );
			}
		}	
	}
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CRegistration::GenerateXML(void)
{
	long nAgents;

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nAgents );
	GetQuery().Execute( _T("SELECT COUNT(*) FROM Agents WHERE IsEnabled=1 AND IsDeleted=0 AND AgentID<>1") );
	GetQuery().Fetch();

	CEMSString sDate;

	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery() );

	GetXMLGen().AddChildElem( _T("Product") );

#if defined(DEMO_VERSION)
	GetXMLGen().AddChildAttrib( _T("Key"), _T("Demo Version") );
#else
#if defined(HOSTED_VERSION)
	GetXMLGen().AddChildAttrib( _T("Key"), _T("Hosted Version") );
#else
	GetXMLGen().AddChildAttrib( _T("Key"), GetISAPIData().m_LicenseMgr.m_szMFKey );
#endif
#endif

#if defined(HOSTED_VERSION)
	GetXMLGen().AddChildAttrib( _T("ShowDetails"),1 );
	GetXMLGen().AddChildAttrib( _T("Status"), _T("Key is valid") );
	GetXMLGen().AddChildAttrib( _T("UserSize"), GetISAPIData().m_LicenseMgr.GetMaxAgents(GetQuery()) );
	GetXMLGen().AddChildAttrib( _T("UserType"), _T("Hosted") );
	GetXMLGen().AddChildAttrib( _T("KeyType"), _T("Hosted") );
	GetXMLGen().AddChildAttrib( _T("ServerSize"), GetISAPIData().m_LicenseMgr.GetMaxServers(GetQuery()) );
	GetXMLGen().AddChildAttrib( _T("NumUsers"), GetISAPIData().m_LicenseMgr.GetNumAgents(GetQuery()) );

	TIMESTAMP_STRUCT theDate;
	GetISAPIData().m_LicenseMgr.GetAgentEndDate( GetQuery(), theDate );
	GetDateString( theDate.year, theDate.month, theDate.day, sDate );						
	GetXMLGen().AddChildAttrib( _T("AgentsExpire"), sDate.c_str() );
	GetISAPIData().m_LicenseMgr.GetServerEndDate( GetQuery(), theDate );
	GetDateString( theDate.year, theDate.month, theDate.day, sDate );
	GetXMLGen().AddChildAttrib( _T("ServerExpires"), sDate.c_str() );
#else	
	if( GetISAPIData().m_LicenseMgr.m_MFKeyPresent )
	{
		GetXMLGen().AddChildAttrib( _T("ShowDetails"), 1 );

		if( GetISAPIData().m_LicenseMgr.m_MFKeyExpired == false )
		{
			GetXMLGen().AddChildAttrib( _T("Status"), _T("Key is valid") );
		}
		else
		{
			if( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.KeyType == DCIKEY_KeyType_Registered )
			{
				GetXMLGen().AddChildAttrib( _T("Status"), _T("Maintenance has expired") );
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("Status"), _T("Evaluation has expired") );
			}
		}

		AddUserSize( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.UserSize );
		int nServerSize = GetISAPIData().m_LicenseMgr.m_MFKeyInfo.ResellerID;
		if(nServerSize == 1153){nServerSize = 1;}
		GetXMLGen().AddChildAttrib( _T("ServerSize"), nServerSize );
		GetXMLGen().AddChildAttrib( _T("UserType"), DCIKeyUserTypeString( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.UserType) );
		GetXMLGen().AddChildAttrib( _T("KeyType"), DCIKeyTypeString( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.KeyType) );
		GetDateString( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.CreateYear,
					   GetISAPIData().m_LicenseMgr.m_MFKeyInfo.CreateMonth,
					   GetISAPIData().m_LicenseMgr.m_MFKeyInfo.CreateDay, sDate );	
		GetXMLGen().AddChildAttrib( _T("Created"), sDate.c_str() );	 
		GetDateString(	GetISAPIData().m_LicenseMgr.m_MFKeyInfo.ExpireYear,
						GetISAPIData().m_LicenseMgr.m_MFKeyInfo.ExpireMonth,
						GetISAPIData().m_LicenseMgr.m_MFKeyInfo.ExpireDay, sDate );
		GetXMLGen().AddChildAttrib( _T("Expires"), sDate.c_str() );
		GetXMLGen().AddChildAttrib( _T("ProductCode"),  GetISAPIData().m_LicenseMgr.m_MFKeyInfo.szProductCode );
		GetXMLGen().AddChildAttrib( _T("NumUsers"), nAgents );
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
		GetXMLGen().AddChildAttrib( _T("NumUsers"), 0 );
	}	
#endif
	tstring sVersion;

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_VERSION_VALUE, sVersion );

	GetXMLGen().AddChildElem( _T("Version") );
	GetXMLGen().AddChildAttrib( _T("Number"), sVersion.c_str() );

}


void CRegistration::AddUserSize( long nUserSize )
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