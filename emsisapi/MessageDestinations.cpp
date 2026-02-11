// MessageDestinations.cpp: implementation of the CMessageDestinations class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageDestinations.h"
#include "DateFns.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageDestinations::CMessageDestinations( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_bDefaultSet = false;
}

CMessageDestinations::~CMessageDestinations()
{

}

////////////////////////////////////////////////////////////////////////////////
// 
//  The main entry point
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// Initialize
	m_MessageDestinationID = 0;
	
	// Check security
	RequireAdmin();
	
	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_MessageDestinationID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_MessageDestinationID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_MessageDestinationID);
		}
	}

	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("messagedestination") ) == 0 )
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
		if( m_MessageDestinationID == 0 )
			action.m_sPageTitle.assign( _T("New Message Destination") );

		return Query();
	} 
	else if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("deliveryfailures") ) == 0 )
	{
		if( m_MessageDestinationID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  

		if( sAction.compare( _T("respool_selected") ) == 0 )
		{
			DISABLE_IN_DEMO();
			RespoolSelected();
		}
		else if ( sAction.compare( _T("respool_olderthan") ) == 0 )
		{
			DISABLE_IN_DEMO();
			RespoolOlderThan();
		}

		return FailedDelivery( action );
	}
	
	if( sAction.compare( _T("delete") ) == 0)
	{
		DISABLE_IN_DEMO();

		if( m_MessageDestinationID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}
	else if ( sAction.compare( _T("makedefault") ) == 0 )
	{
		DISABLE_IN_DEMO();

		if( m_MessageDestinationID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );
		
		SetDefault( action );
	} 
	else if( sAction.compare( _T("sendMail") ) == 0 )
	{
		GetRoutingEngine().ProcessOutboundQueue();
	}
	
	return ListAll();
}

////////////////////////////////////////////////////////////////////////////////
// 
// The default action - return all message destinations
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::ListAll(void)
{
	CEMSString sTemp;
	unsigned int nMsgDstID;
	long nMsgDstIDLen;
	unsigned int nMsgCnt;
	long nMsgCntLen;
	map<int,int> MsgCnts;
	map<int,int>::iterator iter;
	tstring sDefMsgDest;

	if( !m_bDefaultSet )
	{
		GetServerParameter( EMS_SRVPARAM_DEFAULT_MSGDEST_ID, sDefMsgDest );
		m_nDefaultMsgDest = _ttoi( sDefMsgDest.c_str() );
	}

	// get the queued message count for all destinations
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), nMsgDstID );
	BINDCOL_LONG( GetQuery(), nMsgCnt );
	GetQuery().Execute( _T("SELECT MessageDestinationID,COUNT(*) ")
						_T("FROM OutboundMessageQueue INNER JOIN OutboundMessages ")
						_T("ON OutboundMessageQueue.OutboundMessageID = OutboundMessages.OutboundMessageID ")
						_T("WHERE OutboundMessages.OutboundMessageStateID IN (3,4) ")
						_T("GROUP BY MessageDestinationID") );

	while( GetQuery().Fetch() == S_OK )
	{
		MsgCnts[nMsgDstID] = nMsgCnt;
	}

	// query the database for all message destinations
	PrepareList( GetQuery() );
	
	// generate an XML list of all message destinations
	while( GetQuery().Fetch() == S_OK )
	{
		DecryptPassword();
		GetDateTimeString( m_LastProcessInterval, m_LastProcessIntervalLen, sTemp);

		GetXMLGen().AddChildElem(_T("MessageDestination"));
		GetXMLGen().AddChildAttrib( _T("ID"), m_MessageDestinationID );
		GetXMLGen().AddChildAttrib( _T("UseSMTPAuth"), m_UseSMTPAuth );
		GetXMLGen().AddChildAttrib( _T("SMTPPort"), m_SMTPPort );
		GetXMLGen().AddChildAttrib( _T("IsActive"), m_IsActive );
		GetXMLGen().AddChildAttrib( _T("ConnTimeoutSecs"), m_ConnTimeoutSecs );
		GetXMLGen().AddChildAttrib( _T("LastProcessInterval"), sTemp.c_str() );
		GetXMLGen().AddChildAttrib( _T("ProcessFreqMins"), m_ProcessFreqMins );
		GetXMLGen().AddChildAttrib( _T("IsDefault"), (m_MessageDestinationID == m_nDefaultMsgDest) ? 1 : 0 );
		GetXMLGen().AddChildAttrib( _T("ServerID"), m_ServerID );
		GetXMLGen().AddChildAttrib( _T("IsSSL"), m_IsSSL );
		GetXMLGen().AddChildAttrib( _T("OAuthHostID"), m_OAuthHostID );
		GetXMLGen().AddChildAttrib( _T("sslMode"), m_SSLMode );
		GetXMLGen().AddChildAttrib( _T("OfficeHours"), m_OfficeHours );

		iter = MsgCnts.find( m_MessageDestinationID );
		nMsgCnt = ( iter == MsgCnts.end() ) ? 0 : iter->second;
		GetXMLGen().AddChildAttrib( _T("QueuedMessages"), nMsgCnt );

		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("ServerAddress"), m_ServerAddress );
		GetXMLGen().AddChildElem( _T("AuthUser"), m_AuthUser );
		GetXMLGen().AddChildElem( _T("AuthPass"), m_AuthPass );

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
// Query for a particular message destination
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::Query(void)
{
	tstring sMsgSrcs;

	if( m_MessageDestinationID != 0 )
	{
		// query the message source
		if ( TMessageDestinations::Query(GetQuery()) != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  
	}

	DecryptPassword();

	GenerateXML();

	AddServersXML();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a message destination
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::Update()
{
	if( m_MessageDestinationID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  

	DecodeForm();

	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	BINDPARAM_TCHAR( GetQuery(), m_ServerAddress);
	BINDPARAM_LONG( GetQuery(), m_SMTPPort );
	BINDPARAM_TCHAR( GetQuery(), m_AuthUser );
	GetQuery().Execute( _T("SELECT MessageDestinationID FROM MessageDestinations ")
						_T("WHERE MessageDestinationID<>? AND ServerAddress=? AND SMTPPort=? AND AuthUser=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Message destinations must be unique")  );

	EncryptPassword();

	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_ServerAddress );
	BINDPARAM_BIT( GetQuery(), m_UseSMTPAuth );
	BINDPARAM_LONG( GetQuery(), m_SMTPPort );
	BINDPARAM_BIT( GetQuery(), m_IsActive );
	BINDPARAM_LONG( GetQuery(), m_ConnTimeoutSecs );
	BINDPARAM_LONG( GetQuery(), m_ProcessFreqMins );
	BINDPARAM_LONG( GetQuery(), m_MaxOutboundMsgSize );
	BINDPARAM_LONG( GetQuery(), m_MaxSendRetryHours );
	BINDPARAM_TCHAR( GetQuery(), m_AuthUser );
	BINDPARAM_TCHAR( GetQuery(), m_AuthPass );
	BINDPARAM_TCHAR( GetQuery(), m_Description );
	BINDPARAM_LONG( GetQuery(), m_ServerID );
	BINDPARAM_BIT( GetQuery(), m_IsSSL );
	BINDPARAM_LONG( GetQuery(), m_OAuthHostID );
	BINDPARAM_LONG( GetQuery(), m_SSLMode );
	BINDPARAM_LONG( GetQuery(), m_OfficeHours );
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );	
	GetQuery().Execute( _T("UPDATE MessageDestinations ")
						_T("SET ServerAddress=?,UseSMTPAuth=?,SMTPPort=?,IsActive=?,ConnTimeoutSecs=?,ProcessFreqMins=?,MaxOutboundMsgSize=?,MaxSendRetryHours=?, ")
						_T("AuthUser=?,AuthPass=?,Description=?,ServerID=?,IsSSL=?,OAuthHostID=?,SSLMode=?,OfficeHours=? ")
						_T("WHERE MessageDestinationID=?") );

	DecryptPassword();

	if( GetQuery().GetRowCount() == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  

	InvalidateMessageDestinations();
	GetRoutingEngine().ReloadConfig( EMS_MessageDestinations );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a message destination
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::Delete()
{
	int nMessageSourceID;
	
	// check if this message destination is in use by a message source
	GetQuery().Reset();
	BINDCOL_LONG_NOLEN( GetQuery(), nMessageSourceID );
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	GetQuery().Execute( _T("SELECT MessageSourceID FROM MessageSources WHERE MessageDestinationID=?") );
	
	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Message Destination cannot be deleted as it is\nin use by one or more Message Source(s)") ); 
	
	if( TMessageDestinations::Delete( GetQuery() ) == 0 )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  
	
	// reset messages in the outbound queue for this message destination
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	GetQuery().Execute( _T("UPDATE OutboundMessageQueue ")
		_T("Set MessageDestinationID=0 ")
		_T("WHERE MessageDestinationID=?") );
	
	// get the default message destination
	CEMSString sDefMsgDest;
	GetServerParameter( EMS_SRVPARAM_DEFAULT_MSGDEST_ID, sDefMsgDest );

	// if this was the default
	if ( m_MessageDestinationID == _ttoi(sDefMsgDest.c_str()) )
	{
		// make the first remaining message source the default
		m_nDefaultMsgDest = 0;
		BINDCOL_LONG_NOLEN( GetQuery(), m_nDefaultMsgDest );
		GetQuery().Execute( _T("SELECT TOP 1 MessageDestinationID FROM MessageDestinations") );
		GetQuery().Fetch();

		sDefMsgDest.Format(_T("%d"), m_nDefaultMsgDest);
		SetServerParameter(EMS_SRVPARAM_DEFAULT_MSGDEST_ID, sDefMsgDest);
		
		InvalidateServerParameters(true);
		m_bDefaultSet = true;
	}
	
	InvalidateMessageDestinations();
	GetRoutingEngine().ReloadConfig( EMS_MessageDestinations );
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank message destination
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::New()
{
	int nMsgDestCount = 0;

	DecodeForm();
		
	// Check for duplicates
	GetQuery().Initialize();
	BINDPARAM_TCHAR( GetQuery(), m_ServerAddress);
	BINDPARAM_LONG( GetQuery(), m_SMTPPort );
	BINDPARAM_TCHAR( GetQuery(), m_AuthUser );
	GetQuery().Execute( _T("SELECT MessageDestinationID FROM MessageDestinations ")
						_T("WHERE ServerAddress=? AND SMTPPort=? AND AuthUser=?") );

	if( GetQuery().Fetch() == S_OK )
		THROW_EMS_EXCEPTION( E_Duplicate_Name, _T("Message destinations must be unique")  );
	
	// Count the number of message destinations before the insert
	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nMsgDestCount );
	GetQuery().Execute( _T("SELECT COUNT(*) FROM MessageDestinations ") );
	GetQuery().Fetch();

	EncryptPassword();

	TMessageDestinations::Insert( GetQuery() );

	DecryptPassword();

	InvalidateMessageDestinations();
	GetRoutingEngine().ReloadConfig( EMS_MessageDestinations );

	// If there were no message destinations defined, make this one the default
	if( nMsgDestCount == 0 )
	{
		CEMSString sDefaultMsgDest;
		sDefaultMsgDest.Format( _T("%d"), m_MessageDestinationID );
		SetServerParameter( EMS_SRVPARAM_DEFAULT_MSGDEST_ID, sDefaultMsgDest );
		
		InvalidateServerParameters(true);
		
		m_nDefaultMsgDest = m_MessageDestinationID;
		m_bDefaultSet = true;
		
		GetRoutingEngine().ReloadConfig( EMS_MessageDestinations );		
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecodeForm
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageDestinations::DecodeForm(void)
{
	GetISAPIData().GetXMLTCHAR( _T("ServerAddress"), m_ServerAddress, 255 );
	GetISAPIData().GetXMLLong( _T("UseSMTPAuth"), m_UseSMTPAuth );
	GetISAPIData().GetXMLTCHAR( _T("AuthUser"), m_AuthUser, 125 );
	GetISAPIData().GetXMLTCHAR( _T("AuthPass"), m_AuthPass, 125 );
	GetISAPIData().GetXMLLong( _T("SMTPPort"), m_SMTPPort );
	GetISAPIData().GetXMLLong( _T("IsActive"), m_IsActive );
	GetISAPIData().GetXMLLong( _T("ConnTimeoutSecs"), m_ConnTimeoutSecs );
	GetISAPIData().GetXMLLong( _T("ProcessFreqMins"), m_ProcessFreqMins );
	GetISAPIData().GetXMLLong( _T("MaxMsgSize"), m_MaxOutboundMsgSize );
	GetISAPIData().GetXMLLong( _T("SendRetryHours"), m_MaxSendRetryHours );
	GetISAPIData().GetXMLTCHAR( _T("Description"), m_Description, 255 );
	GetISAPIData().GetXMLLong( _T("ServerID"), m_ServerID );
	GetISAPIData().GetXMLLong( _T("UseSSL"), m_IsSSL );
	GetISAPIData().GetXMLLong( _T("sslMode"), m_SSLMode );
	GetISAPIData().GetXMLLong( _T("OfficeHours"), m_OfficeHours );
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

////////////////////////////////////////////////////////////////////////////////
// 
// SetDefault
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageDestinations::SetDefault( CURLAction& action )
{
	CEMSString sDefaultMsgDest;
	sDefaultMsgDest.Format( _T("%d"), m_MessageDestinationID );
	SetServerParameter( EMS_SRVPARAM_DEFAULT_MSGDEST_ID, sDefaultMsgDest );
	
	InvalidateServerParameters(true);
	
	m_nDefaultMsgDest = m_MessageDestinationID;
	m_bDefaultSet = true;
	
	GetRoutingEngine().ReloadConfig( EMS_MessageDestinations );
}

////////////////////////////////////////////////////////////////////////////////
// 
// FailedDelivery
// 
////////////////////////////////////////////////////////////////////////////////
int CMessageDestinations::FailedDelivery( CURLAction& action )
{
	TOutboundMessages outboundmsg;
	CEMSString sDateTime;
	TIMESTAMP_STRUCT LastAttemptedDelivery;
	long LastAttemptedDeliveryLen;
	TCHAR szMessageDestinationName[MESSAGEDESTINATIONS_SERVERADDRESS_LENGTH];
	long szMessageDestinationNameLen;
	CEMSString sSubject;

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	BINDCOL_TCHAR( GetQuery(), szMessageDestinationName );
	GetQuery().Execute( _T("SELECT ServerAddress FROM MessageDestinations ")
		                _T("WHERE MessageDestinationID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );  
	}

	GetXMLGen().AddChildElem( _T("FailedMessages") );
	GetXMLGen().AddChildAttrib( _T("hostName"), szMessageDestinationName );
	GetXMLGen().AddChildAttrib( _T("ID"), m_MessageDestinationID );
	
	GetXMLGen().IntoElem();


	// Get the failed messages for this destination
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	BINDCOL_LONG( GetQuery(), outboundmsg.m_OutboundMessageID );
	BINDCOL_TIME( GetQuery(), outboundmsg.m_EmailDateTime );
	BINDCOL_TCHAR( GetQuery(), outboundmsg.m_Subject );
	BINDCOL_TCHAR( GetQuery(), outboundmsg.m_EmailFrom );
	BINDCOL_TIME( GetQuery(), LastAttemptedDelivery );

	GetQuery().Execute( _T("SELECT OutboundMessages.OutboundMessageID,EmailDateTime,Subject,EmailFrom,LastAttemptedDelivery,EmailTo ")
						_T("FROM OutboundMessages ")
						_T("INNER JOIN OutboundMessageQueue ")
						_T("ON OutboundMessages.OutboundMessageID = OutboundMessageQueue.OutboundMessageID ")
						_T("WHERE OutboundMessageQueue.MessageDestinationID=? ") 
						_T("AND OutboundMessageStateID=5 ") );

	while ( GetQuery().Fetch() == S_OK )
	{
		GETDATA_TEXT( GetQuery(), outboundmsg.m_EmailTo );

		GetDateTimeString( outboundmsg.m_EmailDateTime, outboundmsg.m_EmailDateTimeLen, sDateTime );

		GetXMLGen().AddChildElem( _T("FailedMessage") );
		GetXMLGen().AddChildAttrib( _T("ID"), outboundmsg.m_OutboundMessageID );
		GetXMLGen().AddChildAttrib( _T("EmailDateTime"), sDateTime.c_str() );
		sSubject.assign( outboundmsg.m_Subject );
		sSubject.EscapeHTML();
		GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );
		GetXMLGen().AddChildAttrib( _T("To"), outboundmsg.m_EmailTo );
		GetXMLGen().AddChildAttrib( _T("From"), outboundmsg.m_EmailFrom );
		GetDateTimeString( LastAttemptedDelivery, LastAttemptedDeliveryLen, sDateTime );
		GetXMLGen().AddChildAttrib( _T("LastAttempt"), sDateTime.c_str() );
	}

	GetXMLGen().OutOfElem();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// RespoolSelected
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageDestinations::RespoolSelected( void )
{
	CEMSString sIDs;
	CEMSString sSQL;
	tstring sChunk;

	GetISAPIData().GetFormString( _T("IDCollection"), sIDs, false );

	sIDs.CDLInit();
	while ( sIDs.CDLGetNextChunk( 2, sChunk ) )
	{
		sSQL.Format( _T("UPDATE OutboundMessageQueue ")
			         _T("SET MessageDestinationID=0 ")
					 _T("WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		GetQuery().Initialize();
		GetQuery().Execute( sSQL.c_str() );
		
		sSQL.Format( _T("UPDATE OutboundMessages ")
			         _T("SET OutboundMessageStateID=3 ")
					 _T("WHERE OutboundMessageID IN (%s)"), sChunk.c_str() );
		GetQuery().Initialize();
		GetQuery().Execute( sSQL.c_str() );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// RespoolOlderThan
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageDestinations::RespoolOlderThan( void )
{
	CEMSString sSQL;
	int nDays;

	GetISAPIData().GetFormLong( _T("numdays"), nDays, false );

	sSQL.Format( _T("UPDATE OutboundMessages ")
			     _T("SET OutboundMessageStateID=3 ")
				 _T("WHERE OutboundMessageID IN ")
				 _T("(SELECT OutboundMessageID ")
				 _T(" FROM OutboundMessageQueue ")
				 _T(" WHERE DATEDIFF(day,EmailDateTime,getdate())> %d)"), nDays );
	GetQuery().Initialize();
	GetQuery().Execute( sSQL.c_str() );
}


////////////////////////////////////////////////////////////////////////////////
// 
// GenerateXML
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageDestinations::GenerateXML(void)
{
	CEMSString sDateTime;
	CEMSString sDesc;
	int msgSrcID;

	GetXMLGen().AddChildElem(_T("MessageDestination"));
	GetXMLGen().AddChildAttrib( _T("ID"), m_MessageDestinationID );
	GetXMLGen().AddChildAttrib( _T("UseSMTPAuth"), m_UseSMTPAuth );
	GetXMLGen().AddChildAttrib( _T("SMTPPort"), m_SMTPPort );
	GetXMLGen().AddChildAttrib( _T("IsActive"), m_IsActive );
	GetXMLGen().AddChildAttrib( _T("ConnTimeoutSecs"), m_ConnTimeoutSecs );
	
	GetDateTimeString( m_LastProcessInterval, m_LastProcessIntervalLen, sDateTime);
	GetXMLGen().AddChildAttrib( _T("LastProcessInterval"), sDateTime.c_str() );
	
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

	GetXMLGen().AddChildAttrib( _T("ProcessFreqMins"), m_ProcessFreqMins );
	GetXMLGen().AddChildAttrib( _T("MaxMsgSize"), m_MaxOutboundMsgSize );
	GetXMLGen().AddChildAttrib( _T("SendRetryHours"), m_MaxSendRetryHours );
	GetXMLGen().AddChildAttrib( _T("ServerID"), m_ServerID );
	GetXMLGen().AddChildAttrib( _T("UseSSL"), m_IsSSL );
	GetXMLGen().AddChildAttrib( _T("sslMode"), m_SSLMode );
	GetXMLGen().AddChildAttrib( _T("OAuthHostID"), m_OAuthHostID );
	GetXMLGen().AddChildAttrib( _T("OfficeHours"), m_OfficeHours );

	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("AuthUser"), m_AuthUser );
	GetXMLGen().AddChildElem( _T("AuthPass"), m_AuthPass );
	GetXMLGen().AddChildElem( _T("ServerAddress"), m_ServerAddress );

	sDesc.assign( m_Description );
	sDesc.EscapeJavascript();
	GetXMLGen().AddChildElem( _T("Description"), sDesc.c_str() );

	GetQuery().Reset();
	BINDCOL_LONG_NOLEN( GetQuery(), msgSrcID );
	BINDPARAM_LONG( GetQuery(), m_MessageDestinationID );
	GetQuery().Execute( _T("SELECT MessageSourceID ")
						_T("FROM MessageSources ")
						_T("WHERE MessageDestinationID=?") );

	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("MessageSource") );
		AddMessageSourceName( _T("RemoteAddress"), msgSrcID, true );
	}

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
void CMessageDestinations::EncryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_AuthPass );
	sPassword.Encrypt();
	strncpy( m_AuthPass, sPassword.c_str(), MESSAGEDESTINATIONS_AUTHPASS_LENGTH-1 );	
}

////////////////////////////////////////////////////////////////////////////////
// 
// DecryptPassword
// 
////////////////////////////////////////////////////////////////////////////////
void CMessageDestinations::DecryptPassword()
{
	CEMSString sPassword;

	sPassword.assign( m_AuthPass );
	sPassword.Decrypt();
	strncpy( m_AuthPass, sPassword.c_str(), MESSAGEDESTINATIONS_AUTHPASS_LENGTH-1 );
}

void CMessageDestinations::AddServersXML()
{
	TServers ts;

	GetXMLGen().AddChildElem( _T("Servers") );
	GetXMLGen().IntoElem();
	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), ts.m_ServerID );
	BINDCOL_TCHAR( GetQuery(), ts.m_Description );	
	GetQuery().Execute( _T("SELECT s.ServerID,s.Description FROM Servers s ")
						_T("INNER JOIN ServerTasks st ON s.ServerID=st.ServerID ")
		                _T("WHERE st.ServerTaskTypeID=6") );	
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("ID"), ts.m_ServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), ts.m_Description );			
	}
	GetXMLGen().OutOfElem();
}
