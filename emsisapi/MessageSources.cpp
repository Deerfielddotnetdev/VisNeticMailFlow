/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MessageSources.cpp,v 1.1.4.2 2006/01/11 17:02:28 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Message Sources   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "MessageSources.h"
#include "DateFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageSources::CMessageSources( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_MessageSourceID = 0;
	m_DateFilters = 0;
}

CMessageSources::~CMessageSources()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
//  The main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageSources::Run(CURLAction& action)
{
	tstring sAction = _T("list");
	CEMSString sID;
	
	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_MessageSourceID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_MessageSourceID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_MessageSourceID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );
	
	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("messagesource") ) == 0 )
	{
		if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			New();
			return 0;
		}
		else if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();
			Update();
			return 0;
		}
		
		// Change title if ID is zero
		if( m_MessageSourceID == 0 )
			action.m_sPageTitle.assign( _T("New Message Source") );
		
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0)
	{
		DISABLE_IN_DEMO();

		if( m_MessageSourceID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}
	else if( sAction.compare( _T("checkMail") ) == 0 )
	{
		DISABLE_IN_DEMO();
		GetRoutingEngine().ProcessInboundQueue();
	}	

	return ListAll();
}

////////////////////////////////////////////////////////////////////////////////
// 
// The default action - return all message sources
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageSources::ListAll(void)
{
	CEMSString sTemp;

	// query the database for all message sources
	PrepareList( GetQuery() );

	// generate an XML list of all message sources
	while( GetQuery().Fetch() == S_OK )
	{
		DecryptPassword();

		GetDateTimeString( m_LastChecked, m_LastCheckedLen, sTemp);

		GetXMLGen().AddChildElem(_T("MessageSource"));
		GetXMLGen().AddChildAttrib( _T("ID"), m_MessageSourceID );
		GetXMLGen().AddChildAttrib( _T("RemotePort"), m_RemotePort );
		GetXMLGen().AddChildAttrib( _T("IsAPOP"), m_IsAPOP );
		GetXMLGen().AddChildAttrib( _T("LeaveCopiesOnServer"), m_LeaveCopiesOnServer );
		GetXMLGen().AddChildAttrib( _T("IsActive"), m_IsActive );
		GetXMLGen().AddChildAttrib( _T("CheckFreqMins"), m_CheckFreqMins );
		GetXMLGen().AddChildAttrib( _T("ConnTimeoutSecs"), m_ConnTimeoutSecs );
		GetXMLGen().AddChildAttrib( _T("LastChecked"), sTemp.c_str() );
		GetXMLGen().AddChildAttrib( _T("ServerID"), m_ServerID );
		GetXMLGen().AddChildAttrib( _T("IsSSL"), m_IsSSL );
		GetXMLGen().AddChildAttrib( _T("OAuthHostID"), m_OAuthHostID );
		GetXMLGen().AddChildAttrib( _T("OfficeHours"), m_OfficeHours );
		GetXMLGen().AddChildAttrib( _T("ZipAttach"), m_ZipAttach );
		GetXMLGen().AddChildAttrib( _T("DupMsg"), m_DupMsg );
		GetXMLGen().AddChildAttrib( _T("LeaveCopiesDays"), m_LeaveCopiesDays );
		GetXMLGen().AddChildAttrib( _T("SkipDownloadDays"), m_SkipDownloadDays );
		GetXMLGen().AddChildAttrib( _T("DateFilters"), m_DateFilters );
		
		if ( m_MessageDestinationID == 0 )
		{
			GetXMLGen().AddChildAttrib( _T("MessageDestination"), _T("-- Default --") );
		}
		else
		{
			AddMessageDestinationName( _T("MessageDestination"), m_MessageDestinationID, true );
		}

		AddMessageSourceType( _T("MessageSourceType"), m_MessageSourceTypeID );

		GetXMLGen().IntoElem();

			GetXMLGen().AddChildElem( _T("AuthUserName"), m_AuthUserName );
			GetXMLGen().AddChildElem( _T("AuthPassword"), m_AuthPassword );
			GetXMLGen().AddChildElem( _T("RemoteAddress"), m_RemoteAddress );
			
			sTemp.assign( m_Description );
			sTemp.EscapeHTML();
			GetXMLGen().AddChildElem( _T("Description"), sTemp.c_str() );
		
		GetXMLGen().OutOfElem();
	}

	AddServersXML();

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular message source
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageSources::Query(void)
{
	//TMessageDestinations msgdest;

	if( m_MessageSourceID != 0 )
	{
		// query the message source
		if ( TMessageSources::Query(GetQuery()) != S_OK )
		{
			// bail out m_MessageSourceID was invalid
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
		}
	}

	DecryptPassword();
	
	GenerateXML();

	AddServersXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a message source
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageSources::Update()
{
	DecodeForm();

	if( m_MessageSourceID == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	
	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_MessageSourceID );
	BINDPARAM_TCHAR( GetQuery(), m_RemoteAddress );
	BINDPARAM_LONG( GetQuery(), m_RemotePort );
	BINDPARAM_TCHAR( GetQuery(), m_AuthUserName );
	GetQuery().Execute( _T("SELECT MessageSourceID FROM MessageSources ")
						_T("WHERE MessageSourceID<>? AND RemoteAddress=? AND RemotePort=? AND AuthUserName=?") );
	
	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Message sources must be unique")  );

	EncryptPassword();

	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_RemoteAddress );
	BINDPARAM_LONG( GetQuery(), m_RemotePort );
	BINDPARAM_TCHAR( GetQuery(), m_AuthUserName );
	BINDPARAM_TCHAR( GetQuery(), m_AuthPassword );
	BINDPARAM_BIT( GetQuery(), m_IsAPOP );
	BINDPARAM_BIT( GetQuery(), m_LeaveCopiesOnServer );
	BINDPARAM_BIT( GetQuery(), m_IsActive );
	BINDPARAM_LONG( GetQuery(), m_CheckFreqMins );
	BINDPARAM_LONG( GetQuery(), m_ConnTimeoutSecs );
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	BINDPARAM_LONG( GetQuery(), m_MessageSourceTypeID );
	BINDPARAM_LONG( GetQuery(), m_MaxInboundMsgSize );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	BINDPARAM_BIT( GetQuery(), m_UseReplyTo );
	BINDPARAM_LONG( GetQuery(), m_ServerID );
	BINDPARAM_BIT( GetQuery(), m_IsSSL );
	BINDPARAM_LONG( GetQuery(), m_OAuthHostID );
	BINDPARAM_LONG( GetQuery(), m_OfficeHours );
	BINDPARAM_LONG( GetQuery(), m_ZipAttach );
	BINDPARAM_LONG( GetQuery(), m_DupMsg );
	BINDPARAM_LONG( GetQuery(), m_LeaveCopiesDays );
	BINDPARAM_LONG( GetQuery(), m_SkipDownloadDays );
	BINDPARAM_LONG( GetQuery(), m_DateFilters );
	BINDPARAM_LONG( GetQuery(), m_MessageSourceID );
	
	GetQuery().Execute( _T("UPDATE MessageSources ")
						_T("SET RemoteAddress=?,RemotePort=?,AuthUserName=?,AuthPassword=?,IsAPOP=?,LeaveCopiesOnServer=?,IsActive=?,CheckFreqMins=?,ConnTimeoutSecs=?,MessageDestinationID=?,MessageSourceTypeID=?,MaxInboundMsgSize=?,Description=?,UseReplyTo=?,ServerID=?,IsSSL=?,OAuthHostID=?,OfficeHours=?,ZipAttach=?,DupMsg=?,LeaveCopiesDays=?,SkipDownloadDays=?,DateFilters=? ")
						_T("WHERE MessageSourceID=?") );

	// TODO - MJM why do we decrypt the password here?
	DecryptPassword();
	
	if(  GetQuery().GetRowCount() == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

	InvalidateMessageSources();
	GetRoutingEngine().ReloadConfig( EMS_MessageSources );	
		
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a message source
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageSources::Delete()
{
	// Clean up any UIDL data for the message source
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_MessageSourceID );
	GetQuery().Execute( _T("DELETE FROM UIDLData WHERE MessageSourceID=?") );

	if( TMessageSources::Delete( GetQuery() ) == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
	}	
	
	GetRoutingEngine().ReloadConfig( EMS_MessageSources );
	InvalidateMessageSources();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank message source
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageSources::New()
{
	DecodeForm();

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_RemoteAddress );
	BINDPARAM_LONG( GetQuery(), m_RemotePort );
	BINDPARAM_TCHAR( GetQuery(), m_AuthUserName );
	GetQuery().Execute( _T("SELECT MessageSourceID FROM MessageSources ")
						_T("WHERE RemoteAddress=? AND RemotePort=? AND AuthUserName=?") );
	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Message sources must be unique")  );
	}

	EncryptPassword();

	TMessageSources::Insert( GetQuery() );

	DecryptPassword();
	
	GetRoutingEngine().ReloadConfig( EMS_MessageSources );	
	InvalidateMessageSources();

	return 0;
}

void CMessageSources::DecodeForm(void)
{
	GetISAPIData().GetXMLTCHAR( _T("RemoteAddress"), m_RemoteAddress, 255 );
	GetISAPIData().GetXMLTCHAR( _T("AuthUserName"), m_AuthUserName, 125 );
	GetISAPIData().GetXMLTCHAR( _T("AuthPassword"), m_AuthPassword, 125 );
	GetISAPIData().GetXMLLong( _T("RemotePort"), m_RemotePort );
	GetISAPIData().GetXMLLong( _T("CheckFreqMins"), m_CheckFreqMins );
	GetISAPIData().GetXMLLong( _T("ConnTimeoutSecs"), m_ConnTimeoutSecs );
	GetISAPIData().GetXMLLong( _T("MessageDestination"), m_MessageDestinationID );
	GetISAPIData().GetXMLLong( _T("IsAPOP"), m_IsAPOP );
	GetISAPIData().GetXMLLong( _T("IsActive"), m_IsActive );
	GetISAPIData().GetXMLLong( _T("LeaveCopiesOnServer"), m_LeaveCopiesOnServer );
	GetISAPIData().GetXMLLong( _T("MessageSourceType"), m_MessageSourceTypeID );
	GetISAPIData().GetXMLLong( _T("MaxMsgSize"), m_MaxInboundMsgSize );
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 255 );
	GetISAPIData().GetXMLLong( _T("usereplyto"), m_UseReplyTo);
	GetISAPIData().GetXMLLong( _T("ServerID"), m_ServerID);
	GetISAPIData().GetXMLLong( _T("IsSSL"), m_IsSSL );
	GetISAPIData().GetXMLLong( _T("OAuthHostID"), m_OAuthHostID, true );
	GetISAPIData().GetXMLLong( _T("OfficeHours"), m_OfficeHours );
	GetISAPIData().GetXMLLong( _T("ZipAttach"), m_ZipAttach );
	GetISAPIData().GetXMLLong( _T("DupMsg"), m_DupMsg );
	GetISAPIData().GetXMLLong( _T("LeaveCopiesDays"), m_LeaveCopiesDays );
	GetISAPIData().GetXMLLong( _T("SkipDownloadDays"), m_SkipDownloadDays );
#if defined(HOSTED_VERSION)
	GetISAPIData().GetXMLLong( _T("DateFilters"), m_DateFilters, true );
#endif
	int nUseToken = 0;
	GetISAPIData().GetXMLLong( _T("ckUseToken"), nUseToken );
	if(nUseToken)
	{
		GetISAPIData().GetXMLLong( _T("OAuthHostID"), m_OAuthHostID );
	}
	else
	{
		m_OAuthHostID = 0;
	}
}


void CMessageSources::GenerateXML(void)
{
	CEMSString sDateTime;
	CEMSString sDesc;

	GetDateTimeString( m_LastChecked, m_LastCheckedLen, sDateTime);

	GetXMLGen().AddChildElem(_T("MessageSource"));
	GetXMLGen().AddChildAttrib( _T("ID"), m_MessageSourceID );
	GetXMLGen().AddChildAttrib( _T("RemotePort"), m_RemotePort );
	GetXMLGen().AddChildAttrib( _T("IsAPOP"), m_IsAPOP );
	GetXMLGen().AddChildAttrib( _T("LeaveCopiesOnServer"), m_LeaveCopiesOnServer );
	GetXMLGen().AddChildAttrib( _T("IsActive"), m_IsActive );
	GetXMLGen().AddChildAttrib( _T("CheckFreqMins"), m_CheckFreqMins );
	GetXMLGen().AddChildAttrib( _T("ConnTimeoutSecs"), m_ConnTimeoutSecs );
	
	GetDateTimeString( m_LastChecked, m_LastCheckedLen, sDateTime);
	GetXMLGen().AddChildAttrib( _T("LastChecked"), sDateTime.c_str() );

	GetDateTimeString( m_AccessTokenExpire, m_AccessTokenExpireLen, sDateTime);
	GetXMLGen().AddChildAttrib( _T("AccessTokenExpire"), sDateTime.c_str() );

	GetDateTimeString( m_RefreshTokenExpire, m_RefreshTokenExpireLen, sDateTime);
	GetXMLGen().AddChildAttrib( _T("RefreshTokenExpire"), sDateTime.c_str() );

	if(m_RefreshTokenLen == 0)
	{
		GetXMLGen().AddChildAttrib( _T("RefreshTokenExists"), "0" );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("RefreshTokenExists"), "1" );
	}

	GetXMLGen().AddChildAttrib( _T("MessageDestination"), m_MessageDestinationID );
	GetXMLGen().AddChildAttrib( _T("MessageSourceType"), m_MessageSourceTypeID );
	GetXMLGen().AddChildAttrib( _T("MaxMsgSize"), m_MaxInboundMsgSize );
	GetXMLGen().AddChildAttrib( _T("usereplyto"), m_UseReplyTo);
	GetXMLGen().AddChildAttrib( _T("ServerID"), m_ServerID);
	GetXMLGen().AddChildAttrib( _T("IsSSL"), m_IsSSL );
	GetXMLGen().AddChildAttrib( _T("OAuthHostID"), m_OAuthHostID );
	GetXMLGen().AddChildAttrib( _T("OfficeHours"), m_OfficeHours );
	GetXMLGen().AddChildAttrib( _T("ZipAttach"), m_ZipAttach );
	GetXMLGen().AddChildAttrib( _T("DupMsg"), m_DupMsg );
	GetXMLGen().AddChildAttrib( _T("LeaveCopiesDays"), m_LeaveCopiesDays );
	GetXMLGen().AddChildAttrib( _T("SkipDownloadDays"), m_SkipDownloadDays );
	GetXMLGen().AddChildAttrib( _T("DateFilters"), m_DateFilters );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("AuthUserName"), m_AuthUserName );
	GetXMLGen().AddChildElem( _T("AuthPassword"), m_AuthPassword );

	GetXMLGen().AddChildElem( _T("RemoteAddress"), m_RemoteAddress );

	sDesc.assign( m_Description );
	sDesc.EscapeJavascript();
	GetXMLGen().AddChildElem( _T("Description"), sDesc.c_str() );
	GetXMLGen().OutOfElem();

	ListMessageSourceTypes();

	GetXMLGen().SavePos();
	
	// message destinations and message source types
	ListMessageDestinations();

	GetXMLGen().RestorePos();
	GetXMLGen().FindChildElem( _T("MessageDestinations") );
	GetXMLGen().IntoElem();
	GetXMLGen().InsertChildElem( _T("MessageDestination") );
	GetXMLGen().AddChildAttrib( _T("ID"), 0 );
	GetXMLGen().AddChildAttrib( _T("Address"), _T("-- Default --") );

	GetXMLGen().OutOfElem();

	// list OAuth2 Hosts
	TOAuthHosts oh;
	oh.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("OauthHosts") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("OauthHost") );
		GetXMLGen().AddChildAttrib( _T("ID"), oh.m_OAuthHostID );
		GetXMLGen().AddChildAttrib( _T("Description"), oh.m_Description );
	}
	GetXMLGen().OutOfElem();
}


////////////////////////////////////////////////////////////////////////////////
// 
// EncryptPassword
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageSources::EncryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_AuthPassword );
	sPassword.Encrypt();
	strncpy( m_AuthPassword, sPassword.c_str(), MESSAGESOURCES_AUTHPASSWORD_LENGTH-1 );	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecryptPassword
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageSources::DecryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_AuthPassword );
	sPassword.Decrypt();
	strncpy( m_AuthPassword, sPassword.c_str(), MESSAGESOURCES_AUTHPASSWORD_LENGTH-1 );
}

void CMessageSources::AddServersXML()
{
	TServers ts;

	GetXMLGen().AddChildElem( _T("Servers") );
	GetXMLGen().IntoElem();
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), ts.m_ServerID );
	BINDCOL_TCHAR( GetQuery(), ts.m_Description );	
	GetQuery().Execute( _T("SELECT s.ServerID,s.Description FROM Servers s ")
						_T("INNER JOIN ServerTasks st ON s.ServerID=st.ServerID ")
		                _T("WHERE st.ServerTaskTypeID=4") );	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("ID"), ts.m_ServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), ts.m_Description );			
	}
	GetXMLGen().OutOfElem();
}
