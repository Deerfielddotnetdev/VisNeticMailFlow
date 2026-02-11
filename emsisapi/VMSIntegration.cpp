// VMSIntegration.cpp: implementation of the CVMSIntegration class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VMSIntegration.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVMSIntegration::CVMSIntegration(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{

}

CVMSIntegration::~CVMSIntegration()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
// Configure Visnetic Mailserver Integration
// 
////////////////////////////////////////////////////////////////////////////////
int CVMSIntegration::Run( CURLAction& action )
{
	// Check security
	RequireAdmin();
	
	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("vmsconfig") ) == 0 )
	{
		tstring sAction = _T("list");
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
		
		// update configuration
		if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			UpdateConfig();
		}

		// generate the XML
		GenConfigXML();
	}
	else if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("vmsagents") ) == 0 )
	{
		return VMSAgents();
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Update VMS intergration configuration	              
\*--------------------------------------------------------------------------*/
void CVMSIntegration::UpdateConfig( void )
{
	int nEnabled;

	GetISAPIData().GetXMLLong( _T("Enabled"), nEnabled );
	SetServerParameter( EMS_SRVPARAM_VMS_INTEGRATION, nEnabled ? tstring("1") : tstring("0") );
	
	if( nEnabled )
	{
		tstring sDRIVER;
		tstring sSERVER;
		tstring sUID;
		tstring sPWD;
		tstring sDATABASE;
		
		// get parameters from posted XML document
		GetISAPIData().GetXMLString( _T("DRIVER"), sDRIVER );
		GetISAPIData().GetXMLString( _T("SERVER"), sSERVER );
		GetISAPIData().GetXMLString( _T("UID"), sUID );
		GetISAPIData().GetXMLString( _T("PWD"), sPWD );
		GetISAPIData().GetXMLString( _T("DATABASE"), sDATABASE );
		
		// validation
		if ( sDRIVER.length() < 1 )
			THROW_VALIDATION_EXCEPTION( _T("DRIVER"), _T("You must specify the ODBC driver type. For example SQL Server.") );
		
		if ( sSERVER.length() < 1 )
			THROW_VALIDATION_EXCEPTION( _T("SERVER"), _T("You must specify the database server name.") );
		
		if ( sUID.length() < 1 )
			THROW_VALIDATION_EXCEPTION( _T("UID"), _T("You must specify a user name.") );
		
		if ( sPWD.length() < 1 )
			THROW_VALIDATION_EXCEPTION( _T("PWD"), _T("You must specify a password.") );
		
		if ( sDATABASE.length() < 1 )
			THROW_VALIDATION_EXCEPTION( _T("DATABASE"), _T("You must specify the database name.") );
		
		// build the DSN string
		CEMSString sDSN;
		sDSN.Format( _T("DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s"), sDRIVER.c_str(), sSERVER.c_str(), 
			sUID.c_str(), sPWD.c_str(), sDATABASE.c_str() );
		
		// save the DSN
		SetServerParameter( EMS_SRVPARAM_VMS_DSN, sDSN );
	}
	
	InvalidateServerParameters( true );
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Breaks apart the DSN string used to connect to visnetic
||				mailserver and adds it to the XML.
||	
||				DRIVER={SQL Server};SERVER=jupiter2;UID=sa;PWD=pass;DATABASE=VMS	              
\*--------------------------------------------------------------------------*/
void CVMSIntegration::GenConfigXML( void )
{
	tstring sValue;
	tstring sDSN, sKey;
	size_t start, end;

	GetXMLGen().AddChildElem( _T("VMSParameters") );
	GetServerParameter( EMS_SRVPARAM_VMS_INTEGRATION, sValue );
	GetXMLGen().AddChildAttrib( _T("Enabled"), sValue.c_str() );

	// get the full DSN string
	GetServerParameter( EMS_SRVPARAM_VMS_DSN, sDSN );

	// add the driver
	sKey = _T("DRIVER={");
	sValue.erase();

	if ( (start = sDSN.find( sKey )) != CEMSString::npos )
	{
		if (( end = sDSN.find( _T("}") )) != CEMSString::npos )
			sValue = sDSN.substr( start + sKey.length(), end - (start + sKey.length()) );
	}
				
	GetXMLGen().AddChildAttrib( _T("DRIVER"), sValue.c_str() );


	// add the server	
	sKey = _T("SERVER=");
	sValue.erase();

	if ( (start = sDSN.find( sKey )) != CEMSString::npos )
	{
		if (( end = sDSN.find( _T(";"), start + sKey.length() )) != CEMSString::npos )
			sValue = sDSN.substr( start + sKey.length(), end - (start + sKey.length()) );
	}

	GetXMLGen().AddChildAttrib( _T("SERVER"), sValue.c_str() );
	

	// add the user id
	sKey = _T("UID=");
	sValue.erase();

	if ( (start = sDSN.find( sKey )) != CEMSString::npos )
	{
		if (( end = sDSN.find( _T(";"), start + sKey.length() )) != CEMSString::npos )
			sValue = sDSN.substr( start + sKey.length(), end - (start + sKey.length()) );
	}
	
	GetXMLGen().AddChildAttrib( _T("UID"), sValue.c_str() );
	

	// add the password
	sKey = _T("PWD=");
	sValue.erase();

	if ( (start = sDSN.find( sKey )) != CEMSString::npos )
	{
		if (( end = sDSN.find( _T(";"), start + sKey.length() )) != CEMSString::npos )
			sValue = sDSN.substr( start + sKey.length(), end - (start + sKey.length()) );
	}
	
	GetXMLGen().AddChildAttrib( _T("PWD"), sValue.c_str() );
	

	// add the database
	sKey = _T("DATABASE=");
	sValue.erase();

	if ( (start = sDSN.find( sKey )) != CEMSString::npos )
	{
		if (( end = sDSN.find( _T(";"), start + sKey.length() )) != CEMSString::npos )
			sValue = sDSN.substr( start + sKey.length(), end - (start + sKey.length()) );
		else
			sValue = sDSN.substr( start + sKey.length());
	}

	GetXMLGen().AddChildAttrib( _T("DATABASE"), sValue.c_str() );
}

int CVMSIntegration::VMSAgents(void)
{
	CODBCConn dbconn;
	CODBCQuery query( dbconn );
	tstring sDSN;
	TCHAR szUsername[256];
	TCHAR szRealname[256];
	TCHAR szPassword[256];
	TCHAR szDomain[256];
	long szUsernameLen;
	long szRealnameLen;
	long szPasswordLen;
	long szDomainLen;
	CEMSString sEmailAddr;

	GetServerParameter( EMS_SRVPARAM_VMS_DSN, sDSN );

	try
	{
		dbconn.Connect( sDSN.c_str() );

		query.Initialize();
		BINDCOL_TCHAR( query, szUsername );
		BINDCOL_TCHAR( query, szRealname );
		BINDCOL_TCHAR( query, szPassword );
		BINDCOL_TCHAR( query, szDomain );

		query.Execute( _T("SELECT U_Mailbox,U_Name,U_Password,U_Domain FROM Users") );

		while( query.Fetch() == S_OK )
		{
			// MJM - if the last character of the real name is 
			// a semi-colon, get rid of it.
			if ( szRealname[strlen(szRealname)-1] == _T(';') )
				szRealname[strlen(szRealname)-1] = _T('\0');

			GetXMLGen().AddChildElem( _T("VMSUser") );
			GetXMLGen().AddChildAttrib( _T("username"), szUsername );
			GetXMLGen().AddChildAttrib( _T("realname"), szRealname );
			GetXMLGen().AddChildAttrib( _T("password"), szPassword );
			sEmailAddr.Format( _T("%s@%s"), szUsername, szDomain );
			GetXMLGen().AddChildAttrib( _T("email"), sEmailAddr.c_str() );
		}
	}
	catch(ODBCError_t error) 
	{ 
		GetXMLGen().AddChildElem( _T("Error") );
		GetXMLGen().AddChildAttrib( _T("Code"), error.nErrorCode );
		GetXMLGen().AddChildAttrib( _T("Description"), (TCHAR*) error.szErrMsg );
		GetXMLGen().AddChildAttrib( _T("NativeCode"), error.nNativeErrorCode );
	}	
	catch( ... )
	{
		GetXMLGen().AddChildElem( _T("Error") );
		GetXMLGen().AddChildAttrib( _T("Code"), -1 );
		GetXMLGen().AddChildAttrib( _T("Description"), _T("Unhandled Exception") );
		GetXMLGen().AddChildAttrib( _T("NativeCode"), -1 );
	}
	
	return 0;
}