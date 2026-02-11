// SecuritySettings.cpp: implementation of the CSecuritySettings class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SecuritySettings.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSecuritySettings::CSecuritySettings(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
}

CSecuritySettings::~CSecuritySettings()
{
}


int CSecuritySettings::Run( CURLAction& action )
{
	CEMSString sValue;
	tstring sAction;

	// Check security
	RequireAdmin();

	if( GetISAPIData().GetFormString( _T("Action"), sAction, true ) )
	{
		DISABLE_IN_DEMO();

		unsigned char nValue = 0;

		GetISAPIData().GetFormBit( _T("forcehttps"), nValue );
		sValue.Format( _T("%d"), nValue );
		SetServerParameter( EMS_SRVPARAM_FORCE_HTTPS,  sValue );
		GetISAPIData().GetFormBit( _T("sessioniplock"), nValue );
		sValue.Format( _T("%d"), nValue );
		SetServerParameter( EMS_SRVPARAM_SESSION_IP_LOCKING,  sValue );
		InvalidateServerParameters( true );
	}

	GetXMLGen().AddChildElem( _T("Settings") );
	GetServerParameter( EMS_SRVPARAM_FORCE_HTTPS, sValue );
	GetXMLGen().AddChildAttrib( _T("ForceHTTPS"), sValue.c_str() );

	GetXMLDoc().AddElem( _T("Settings") );
	GetXMLDoc().AddAttrib( _T("ForceHTTPS"), sValue.c_str() );

	GetServerParameter( EMS_SRVPARAM_SESSION_IP_LOCKING, sValue );
	GetXMLGen().AddChildAttrib( _T("SessionIPLock"), sValue.c_str() );

	return 0;
}