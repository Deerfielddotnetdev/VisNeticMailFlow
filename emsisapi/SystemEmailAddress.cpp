// SystemEmailAddress.cpp: implementation of the CSystemEmailAddress class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SystemEmailAddress.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSystemEmailAddress::CSystemEmailAddress(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CSystemEmailAddress::~CSystemEmailAddress()
{

}


int CSystemEmailAddress::Run( CURLAction& action )
{
	CEMSString sValue;

	// Check security
	RequireAdmin();

	if( GetISAPIData().GetXMLPost() )
	{	
		DISABLE_IN_DEMO();

		GetISAPIData().GetXMLString( _T("SystemEmail"), sValue );

		if (!sValue.ValidateEmailAddr())
		{
			CEMSString sError;
			sError.Format( _T("The email address specified [%s] is invalid"), sValue.c_str() );
			THROW_VALIDATION_EXCEPTION( _T("systememail"), sError );
		}
		
		SetServerParameter( EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, sValue );
		InvalidateServerParameters( true );
		GetRoutingEngine().ReloadConfig( EMS_MessageDestinations );
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, sValue );
	}

	GetXMLGen().AddChildElem( _T("SystemEmailAddress") );
	GetXMLGen().SetChildData( sValue.c_str(), 1 );

	return 0;
}