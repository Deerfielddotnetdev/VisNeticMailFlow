/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Unsub.cpp,v 1.2.2.1.2.1 2006/07/18 12:55:02 markm Exp $
||
||
||                                         
||  COMMENTS:
||              
\\*************************************************************************/
#include "stdafx.h"
#include "Unsub.h"
#include "StringFns.h"
#include "OutboundMessage.h"
#include <time.h>
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CUnsub::CUnsub(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	// handle exceptions in the local EMS file
	GetISAPIData().SetUseExceptionEMS(false);
	sEmail.assign("");
	nContactID = 0;
	bUnsub = false;
	bSub = false;
	bConfirmed = false;
	nGroupID = 0;
	nOrigGroupID = 0;
	srand ( (unsigned) time(NULL) );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Destruction	              
\*--------------------------------------------------------------------------*/
CUnsub::~CUnsub(void)
{
}

/*---------------------------------------------------------------------------\             
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CUnsub::Run( CURLAction& action )
{
	tstring sAction = _T("search");	
		
	if(!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
		if(!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
			GetISAPIData().GetURLString( _T("Action"), sAction, true );

	if(!GetISAPIData().GetXMLString( _T("Email"), sEmail, true ))
		if(!GetISAPIData().GetFormString( _T("Email"), sEmail, true ))
			GetISAPIData().GetURLString( _T("Email"), sEmail, true );


	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("unsub2") ) == 0)
	{
		
		nContactID = 0;
			
		szEmail.Format(_T("%s"),sEmail.c_str());
		szEmail.EscapeSQL();

		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nContactID );
		BINDPARAM_TCHAR_STRING( GetQuery(), szEmail );
		GetQuery().Execute( _T("SELECT TOP 1 c.ContactID FROM Contacts c ")
							_T("INNER JOIN PersonalData pd ON c.DefaultEmailAddressID=pd.PersonalDataID ")
							_T("WHERE pd.PersonalDataTypeID=1 and pd.datavalue=? and c.IsDeleted=0") );
		GetQuery().Fetch();
		
		if(nContactID > 0)
		{
			if( sAction.compare( _T("unsub") ) == 0 )
			{
				
				if(!GetISAPIData().GetXMLString( _T("Group"), sIDs, true ))
					if(!GetISAPIData().GetURLString( _T("Group"), sIDs, true ))
						GetISAPIData().GetFormString( _T("Group"), sIDs, true );

				while ( sIDs.CDLGetNextInt( nGroupID ) )
				{
					if(nGroupID > 0)
					{
						UnsubGroup(nContactID,nGroupID);						
					}
				}
				GetGroups(nContactID);
				
			}
			else if( sAction.compare( _T("unsuball") ) == 0 )
			{
				UnsubGroup(nContactID,0);
				GetGroups(nContactID);
			}
			else if( sAction.compare( _T("sub") ) == 0 )
			{
				if(!GetISAPIData().GetXMLString( _T("Group"), sIDs, true ))
						if(!GetISAPIData().GetURLString( _T("Group"), sIDs, true ))
							GetISAPIData().GetFormString( _T("Group"), sIDs, true );

				while ( sIDs.CDLGetNextInt( nGroupID ) )
				{
					if(nGroupID > 0)
					{
						SubGroup(nContactID,nGroupID);						
					}
				}
				GetGroups(nContactID);
			}		
			else if( sAction.compare( _T("search") ) == 0 )
			{
				if(!GetISAPIData().GetXMLLong( _T("Group"), nOrigGroupID, true ))
						if(!GetISAPIData().GetURLLong( _T("Group"), nOrigGroupID, true ))
							GetISAPIData().GetFormLong( _T("Group"), nOrigGroupID, true );

				GetGroups(nContactID);	
			}
		}
		else
		{
			GetXMLGen().AddChildElem( _T("Unsub") );
			GetXMLGen().AddChildAttrib( _T("Email"), szEmail.c_str() );
		}
	}
	else if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("unsub3") ) == 0)
	{
		if( sAction.compare( _T("confirm") ) == 0 )
		{
			CEMSString szCode;
			if(!GetISAPIData().GetXMLString( _T("Code"), szCode, true ))
						if(!GetISAPIData().GetURLString( _T("Code"), szCode, true ))
							GetISAPIData().GetFormString( _T("Code"), szCode, true );

			szCode.EscapeSQL();
			tstring sCode(szCode.c_str());

			if(sCode.length() == 50)
			{
				ConfirmCode(sCode);
			}
			else
			{
				GetXMLGen().AddChildElem( _T("Error"), _T("Invalid Confirmation Code") );
			}
		}
	}
	return 0;
}

void CUnsub::UnsubGroup(int nContactID, int nGroupID)
{
	bUnsub = true;
		
	if(nContactID > 0)
	{
		GetQuery().Initialize();
		if(nGroupID > 0)
		{
			BINDPARAM_LONG( GetQuery(), nGroupID );
			BINDPARAM_LONG( GetQuery(), nContactID );
			GetQuery().Execute( _T("DELETE FROM ContactGrouping ")
								_T("WHERE ContactGroupID=? AND ContactID = ?") );

		}
		else
		{
			BINDPARAM_LONG( GetQuery(), nContactID );
			GetQuery().Execute( _T("DELETE FROM ContactGrouping ")
								_T("WHERE ContactID = ?") );
		}
	}
}


void CUnsub::SubGroup(int nContactID, int nGroupID)
{
	int nContactGroupID = 0;
	bool bExist = false;
	TCHAR szGroupName[255];
	long szGroupNameLen;
	
	if(nContactID > 0)
	{
		GetQuery().Initialize();		
		BINDCOL_LONG_NOLEN( GetQuery(), nContactGroupID);
		BINDPARAM_LONG( GetQuery(), nContactID );
		BINDPARAM_LONG( GetQuery(), nGroupID );
		GetQuery().Execute( _T("SELECT ContactGroupID FROM ContactGrouping ")
							_T("WHERE ContactID=? and ContactGroupID=?") );
		if(GetQuery().Fetch() == S_OK)
			bExist = true;


		if(!bExist)
		{
			TCHAR szRandomCode[51];
			ZeroMemory( szRandomCode, (51) * sizeof(TCHAR) );
			for( int i = 0; i < 50; i++ )
			{
				szRandomCode[i] = 'A' + rand() % 26;
			}
			
			bSub = true;
			GetQuery().Reset();	
			BINDCOL_TCHAR( GetQuery(), szGroupName );
			BINDPARAM_LONG( GetQuery(), nGroupID );
			BINDPARAM_LONG( GetQuery(), nGroupID );
			BINDPARAM_LONG( GetQuery(), nContactID );
			BINDPARAM_TCHAR( GetQuery(), szRandomCode );
			GetQuery().Execute( _T("(SELECT GroupName FROM ContactGroups WHERE ContactGroupID=?) INSERT INTO ContactGrouping ")
								_T("(ContactGroupID,ContactID,ConfirmCode,ConfirmID) ")
								_T("VALUES ")
								_T("(?,?,?,1)") );

			GetQuery().Fetch();

			COutboundMessage msg(m_ISAPIData);
			int nNewMsgID = msg.ReserveID( 0 );
			
			
			_sntprintf( msg.m_MediaType, OUTBOUNDMESSAGES_MEDIATYPE_LENGTH - 1, _T("text") );
			_sntprintf( msg.m_MediaSubType, OUTBOUNDMESSAGES_MEDIASUBTYPE_LENGTH - 1, _T("html") );
			
			msg.m_AgentID = 0;	
			GetTimeStamp( msg.m_EmailDateTime );
			msg.m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
			msg.m_TicketID = 0;
			msg.m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
			msg.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
			PutStringProperty(szEmail, &msg.m_EmailTo, &msg.m_EmailToAllocated );			
			_sntprintf( msg.m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH - 1, szEmail.c_str() );
			
			tstring sValue;
			if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
							_T("SubEmailFrom"), sValue ) != ERROR_SUCCESS)
			{
				GetServerParameter( EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, sValue );
			}
						
			_sntprintf( msg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH - 1, sValue.c_str() );
			PutStringProperty( sValue, &msg.m_EmailReplyTo, &msg.m_EmailReplyToAllocated );

			CEMSString sString;			
			UINT nSRID;
			if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
							_T("SubSendCodeID"), nSRID ) == ERROR_SUCCESS)
			{
				TStandardResponses stdresp;
				tstring sTemp;
				stdresp.m_StandardResponseID = nSRID;

				if( stdresp.Query( GetQuery() ) != S_OK )
				{
					sString.Format( _T("Hello,<br/><br/>")
					_T("Thank you for subscribing to the [<b>%s</b>] Mailing Group.<br/><br/>")
					_T("Your subscription request has been received but will not be activated until you <a href=\"https://subunsub.deerfield.net/unsub3.ems?code=%s\">confirm</a> the request by clicking the link below.<br/><br/>")
					_T("<a href=\"https://subunsub.deerfield.net/unsub3.ems?code=%s\">https://subunsub.deerfield.net/unsub3.ems?code=%s</a><br/><br/>")
					_T("If the link does not open in your web browser you can copy it to the clipboard and paste it in the browser URL Address bar.<br/><br/>")
					_T("If you did not request this subscription, please ignore this email and the request will be deleted from the queue.<br/><br/>")
					_T("Thank you,<br/><br/>")
					_T("VisNetic MailFlow Mailing Group Manager<br/>")
					_T("Deerfield Communications Inc.<br/>")
					_T("http://www.visnetic.com<br/>"), szGroupName, szRandomCode, szRandomCode, szRandomCode );

					_sntprintf( msg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("Mailing Group Confirmation") );
			
				}
				else
				{
					sTemp = stdresp.m_StandardResponse;
					tstring::size_type pos = sTemp.find( _T("$$Code$$") );
					while( pos != tstring::npos )
					{
						sTemp.replace( pos, 8, szRandomCode );
						pos = sTemp.find( _T("$$Code$$"));
					}

					pos = sTemp.find( _T("$$Email$$") );
					while( pos != tstring::npos )
					{
						sTemp.replace( pos, 9, szEmail.c_str() );
						pos = sTemp.find( _T("$$Email$$"));
					}
					
					pos = sTemp.find( _T("$$Group$$") );
					while( pos != tstring::npos )
					{
						sTemp.replace( pos, 9, szGroupName );
						pos = sTemp.find( _T("$$Group$$"));
					}

					sString.Format(_T("%s"),sTemp.c_str());

					sTemp = stdresp.m_Subject;
					pos = sTemp.find( _T("$$Code$$") );
					while( pos != tstring::npos )
					{
						sTemp.replace( pos, 8, szRandomCode );
						pos = sTemp.find( _T("$$Code$$"));
					}

					pos = sTemp.find( _T("$$Email$$") );
					while( pos != tstring::npos )
					{
						sTemp.replace( pos, 9, szEmail.c_str() );
						pos = sTemp.find( _T("$$Email$$"));
					}

					pos = sTemp.find( _T("$$Group$$") );
					while( pos != tstring::npos )
					{
						sTemp.replace( pos, 9, szGroupName );
						pos = sTemp.find( _T("$$Group$$"));
					}

					_sntprintf( msg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH - 1, sTemp.c_str() );
				}
			}
			else
			{
				sString.Format( _T("Hello,<br/><br/>")
                _T("Thank you for subscribing to the [<b>%s</b>] Mailing Group.<br/><br/>")
                _T("Your subscription request has been received but will not be activated until you <a href=\"https://subunsub.deerfield.net/unsub3.ems?code=%s\">confirm</a> the request by clicking the link below.<br/><br/>")
				_T("<a href=\"https://subunsub.deerfield.net/unsub3.ems?code=%s\">https://subunsub.deerfield.net/unsub3.ems?code=%s</a><br/><br/>")
                _T("If the link does not open in your web browser you can copy it to the clipboard and paste it in the browser URL Address bar.<br/><br/>")
                _T("If you did not request this subscription, please ignore this email and the request will be deleted from the queue.<br/><br/>")
                _T("Thank you,<br/><br/>")
                _T("VisNetic MailFlow Mailing Group Manager<br/>")
                _T("Deerfield Communications Inc.<br/>")
				_T("http://www.visnetic.com<br/>"), szGroupName, szRandomCode, szRandomCode, szRandomCode );

				_sntprintf( msg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("Mailing Group Confirmation") );
			}

			PutStringProperty( sString , &msg.m_Body, &msg.m_BodyAllocated );
			
			msg.m_IsDeleted=0;
			msg.Update(GetQuery());
			
			TIMESTAMP_STRUCT now;
			GetQuery().Initialize();
			
			GetTimeStamp( now );
			
			BINDPARAM_LONG( GetQuery(), msg.m_OutboundMessageID );
			BINDPARAM_TIME_NOLEN( GetQuery(), now );
			GetQuery().Execute( _T("INSERT INTO OutboundMessageQueue (OutboundMessageID, DateSpooled, IsApproved) ")
									_T("VALUES (?,?,1)"));			

			GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,EMSERR_ISAPI_EXTENSION,
				EMS_LOG_OUTBOUND_MESSAGING, 0 ), _T("[System] added Outbound Message ID: (%d) to the OutboundMessageQueue"), msg.m_OutboundMessageID ); 

		}
	}
}

void CUnsub::GetGroups(int nContactID)
{
	int ContactGroupID;
	vector<int> CGIDs;
	vector<int>::iterator iterIDs;
	TCHAR szGroupName[255];
	long szGroupNameLen;
	
	if(nContactID > 0)
	{
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), ContactGroupID );
		BINDCOL_TCHAR( GetQuery(), szGroupName );
		BINDPARAM_LONG( GetQuery(), nContactID );
		GetQuery().Execute( _T("SELECT g.ContactGroupID, g.GroupName from ContactGroups g ")
							_T("INNER JOIN ContactGrouping cg on g.ContactGroupID=cg.ContactGroupID ")
							_T("INNER JOIN Contacts c on c.ContactID=cg.ContactID ")
							_T("WHERE g.OwnerID=0 and g.GroupTypeID=1 and g.Unsubscribe=1 and cg.ContactID=? Order By g.GroupName") );

		CGIDs.clear();
		GetXMLGen().AddChildElem( _T("Groups") );
		GetXMLGen().IntoElem();	
		while( GetQuery().Fetch() == S_OK )
		{
			CGIDs.push_back(ContactGroupID);
			GetXMLGen().AddChildElem( _T("Group") );
			GetXMLGen().AddChildAttrib( _T("ID"), ContactGroupID );
			GetXMLGen().AddChildAttrib( _T("Name"), szGroupName );
		}
		GetXMLGen().OutOfElem();

		GetXMLGen().AddChildElem( _T("Unsub") );
		GetXMLGen().AddChildAttrib( _T("Email"), szEmail.c_str() );
		
		if(bUnsub)
		{
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Groups") );
			GetXMLGen().IntoElem();
			if(nGroupID == 0)
			{
				GetXMLGen().AddChildElem( _T("Group") );
				GetXMLGen().AddChildAttrib( _T("Name"), _T("All Subscribed Groups") );
			}
			else
			{
				sIDs.CDLInit();
				GetQuery().Initialize();
				BINDCOL_TCHAR( GetQuery(), szGroupName );
				CEMSString sQuery;
				sQuery.Format( _T("SELECT g.GroupName from ContactGroups g ")
							_T("WHERE g.OwnerID=0 and g.GroupTypeID=1 and ")
							_T("g.Unsubscribe=1 and g.ContactGroupID IN (%s) Order By g.GroupName"), sIDs.c_str() );
				GetQuery().Execute( sQuery.c_str() );
				
				while( GetQuery().Fetch() == S_OK )
				{
					GetXMLGen().AddChildElem( _T("Group") );
					GetXMLGen().AddChildAttrib( _T("Name"), szGroupName );
				}
			}
			GetXMLGen().OutOfElem();
			GetXMLGen().OutOfElem();
		}

		GetXMLGen().AddChildElem( _T("Subbed") );	
		if(bSub)
		{
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Groups") );
			GetXMLGen().IntoElem();
			sIDs.CDLInit();
			GetQuery().Initialize();
			BINDCOL_TCHAR( GetQuery(), szGroupName );
			CEMSString sQuery;
			sQuery.Format( _T("SELECT g.GroupName from ContactGroups g ")
							_T("WHERE g.OwnerID=0 and g.GroupTypeID=1 and ")
							_T("g.Subscribe=1 and g.ContactGroupID IN (%s) Order By g.GroupName"), sIDs.c_str() );
			GetQuery().Execute( sQuery.c_str() );
			
			while( GetQuery().Fetch() == S_OK )
			{
				GetXMLGen().AddChildElem( _T("Group") );
				GetXMLGen().AddChildAttrib( _T("Name"), szGroupName );
			}
			GetXMLGen().OutOfElem();
			GetXMLGen().OutOfElem();
		}

		GetXMLGen().AddChildElem( _T("Confirmed") );	
		if(bConfirmed)
		{
			GetXMLGen().IntoElem();
			GetXMLGen().AddChildElem( _T("Groups") );
			GetXMLGen().IntoElem();
			
			GetQuery().Initialize();
			BINDCOL_TCHAR( GetQuery(), szGroupName );
			CEMSString sQuery;
			sQuery.Format( _T("SELECT g.GroupName from ContactGroups g ")
							_T("WHERE g.OwnerID=0 and g.GroupTypeID=1 and ")
							_T("g.Subscribe=1 and g.ContactGroupID = %d "), nOrigGroupID );
			GetQuery().Execute( sQuery.c_str() );
			
			while( GetQuery().Fetch() == S_OK )
			{
				GetXMLGen().AddChildElem( _T("Group") );
				GetXMLGen().AddChildAttrib( _T("Name"), szGroupName );
			}
			GetXMLGen().OutOfElem();
			GetXMLGen().OutOfElem();
		}

		GetXMLGen().AddChildElem( _T("Sub") );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("Groups") );
		GetXMLGen().IntoElem();
		
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), ContactGroupID );
		BINDCOL_TCHAR( GetQuery(), szGroupName );		
		GetQuery().Execute( _T("SELECT g.ContactGroupID, g.GroupName from ContactGroups g ")
							_T("WHERE g.OwnerID=0 and g.GroupTypeID=1 and g.Subscribe=1 Order By g.GroupName") );
		
		while( GetQuery().Fetch() == S_OK )
		{
			bool bFound = false;
			for( iterIDs = CGIDs.begin(); iterIDs != CGIDs.end(); iterIDs++ )
			{
				if(ContactGroupID == *iterIDs)
					bFound = true;									
			}
			if(!bFound)
			{
				GetXMLGen().AddChildElem( _T("Group") );
				GetXMLGen().AddChildAttrib( _T("ID"), ContactGroupID );
				GetXMLGen().AddChildAttrib( _T("Name"), szGroupName );
			}
		}
		GetXMLGen().OutOfElem();
		GetXMLGen().OutOfElem();
	}

	if(nOrigGroupID > 0)
	{
		GetXMLGen().AddChildElem( _T("OrigGroup") );
		GetXMLGen().AddChildAttrib( _T("ID"), nOrigGroupID );
	}
	
}

void CUnsub::ConfirmCode(tstring sCode)
{
	int nContactGroupingID = 0;
	int nContactGroupID = 0;
	bool bExist = false;
	TCHAR szGroupName[255];
	long szGroupNameLen;
	TCHAR szAdd[255];
	long szAddLen;

	GetQuery().Initialize();		
	BINDCOL_TCHAR( GetQuery(), szGroupName );		
	BINDCOL_TCHAR( GetQuery(), szAdd );		
	BINDCOL_LONG_NOLEN( GetQuery(), nContactGroupingID);
	BINDCOL_LONG_NOLEN( GetQuery(), nContactGroupID);
	BINDCOL_LONG_NOLEN( GetQuery(), nContactID);
	BINDPARAM_TCHAR_STRING( GetQuery(), sCode );
	GetQuery().Execute( _T("SELECT g.GroupName,pd.DataValue,cg.ContactGroupingID,cg.ContactGroupID,cg.ContactID FROM ContactGrouping cg ")
						_T("INNER JOIN ContactGroups g ON g.ContactGroupID=cg.ContactGroupID ")
						_T("INNER JOIN Contacts c ON c.ContactID=cg.ContactID ")
						_T("INNER JOIN PersonalData pd ON pd.PersonalDataID=c.DefaultEmailAddressID ")
						_T("WHERE pd.PersonalDataTypeID=1 and c.IsDeleted=0 and cg.ConfirmCode=? and cg.ConfirmID=1") );
	if(GetQuery().Fetch() == S_OK)
		bExist = true;

	if(bExist)
	{
		
		GetQuery().Reset();	
		BINDPARAM_LONG( GetQuery(), nContactGroupingID );
		GetQuery().Execute( _T("UPDATE ContactGrouping ")
							_T("SET ConfirmCode='',ConfirmID=0 ")
							_T("WHERE ContactGroupingID =?") );

		nOrigGroupID = nContactGroupID;
		bConfirmed = true;
		szEmail.Format(_T("%s"),szAdd);

		COutboundMessage msg(m_ISAPIData);
		int nNewMsgID = msg.ReserveID( 0 );
				
		_sntprintf( msg.m_MediaType, OUTBOUNDMESSAGES_MEDIATYPE_LENGTH - 1, _T("text") );
		_sntprintf( msg.m_MediaSubType, OUTBOUNDMESSAGES_MEDIASUBTYPE_LENGTH - 1, _T("html") );
		
		msg.m_AgentID = 0;	
		GetTimeStamp( msg.m_EmailDateTime );
		msg.m_EmailDateTimeLen = sizeof( TIMESTAMP_STRUCT );
		msg.m_TicketID = 0;
		msg.m_OutboundMessageTypeID = EMS_OUTBOUND_MESSAGE_TYPE_NEW;
		msg.m_OutboundMessageStateID = EMS_OUTBOUND_MESSAGE_STATE_UNTOUCHED;
		_tcscpy( msg.m_EmailTo, szAdd );
		_sntprintf( msg.m_EmailPrimaryTo, OUTBOUNDMESSAGES_EMAILPRIMARYTO_LENGTH - 1, szAdd );
		
		tstring sValue;
		if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
						_T("SubEmailFrom"), sValue ) != ERROR_SUCCESS)
		{
			GetServerParameter( EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, sValue );
		}				
		_sntprintf( msg.m_EmailFrom, OUTBOUNDMESSAGES_EMAILFROM_LENGTH - 1, sValue.c_str() );
		PutStringProperty( sValue, &msg.m_EmailReplyTo, &msg.m_EmailReplyToAllocated );


		CEMSString sString;
		UINT nSRID;
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
						_T("SubConfirmedCodeID"), nSRID ) == ERROR_SUCCESS)
		{
			TStandardResponses stdresp;
			tstring sTemp;
			stdresp.m_StandardResponseID = nSRID;

			if( stdresp.Query( GetQuery() ) != S_OK )
			{
				sString.Format( _T("Hello,<br/><br/>")
				_T("Your subscription to the [<b>%s</b>] Mailing Group has been confirmed.<br/><br/>")
				_T("You can <a href=\"https://subunsub.deerfield.net/unsub.ems?email=%s\">manage</a> your subscriptions at the link below.<br/><br/>")
				_T("<a href=\"https://subunsub.deerfield.net/unsub.ems?email=%s\">https://subunsub.deerfield.net/unsub.ems?email=%s</a><br/><br/>")
				_T("If the link does not open in your web browser you can copy it to the clipboard and paste it in the browser URL Address bar.<br/><br/>")
				_T("Thank you,<br/><br/>")
				_T("VisNetic MailFlow Mailing Group Manager<br/>")
				_T("Deerfield Communications Inc.<br/>")
				_T("http://www.visnetic.com<br/>"), szGroupName, szAdd, szAdd, szAdd );

				_sntprintf( msg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("Mailing Group Confirmed") );		
		
			}
			else
			{
				sTemp = stdresp.m_StandardResponse;
				tstring::size_type pos = sTemp.find( _T("$$Email$$") );
				while( pos != tstring::npos )
				{
					sTemp.replace( pos, 9, szAdd );
					pos = sTemp.find( _T("$$Email$$"));
				}
				
				pos = sTemp.find( _T("$$Group$$") );
				while( pos != tstring::npos )
				{
					sTemp.replace( pos, 9, szGroupName );
					pos = sTemp.find( _T("$$Group$$"));
				}

				sString.Format(_T("%s"),sTemp.c_str());

				sTemp = stdresp.m_Subject;
				pos = sTemp.find( _T("$$Email$$") );
				while( pos != tstring::npos )
				{
					sTemp.replace( pos, 9, szAdd );
					pos = sTemp.find( _T("$$Email$$"));
				}

				pos = sTemp.find( _T("$$Group$$") );
				while( pos != tstring::npos )
				{
					sTemp.replace( pos, 9, szGroupName );
					pos = sTemp.find( _T("$$Group$$"));
				}

				_sntprintf( msg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH - 1, sTemp.c_str() );
			}
		}
		else
		{
			sString.Format( _T("Hello,<br/><br/>")
			_T("Your subscription to the [<b>%s</b>] Mailing Group has been confirmed.<br/><br/>")
			_T("You can <a href=\"https://subunsub.deerfield.net/unsub.ems?email=%s\">manage</a> your subscriptions at the link below.<br/><br/>")
			_T("<a href=\"https://subunsub.deerfield.net/unsub.ems?email=%s\">https://subunsub.deerfield.net/unsub.ems?email=%s</a><br/><br/>")
			_T("If the link does not open in your web browser you can copy it to the clipboard and paste it in the browser URL Address bar.<br/><br/>")
			_T("Thank you,<br/><br/>")
			_T("VisNetic MailFlow Mailing Group Manager<br/>")
			_T("Deerfield Communications Inc.<br/>")
			_T("http://www.visnetic.com<br/>"), szGroupName, szAdd, szAdd, szAdd );

			_sntprintf( msg.m_Subject, OUTBOUNDMESSAGES_SUBJECT_LENGTH - 1, _T("Mailing Group Confirmed") );
	
		}		                

		PutStringProperty( sString , &msg.m_Body, &msg.m_BodyAllocated );
		
		msg.m_IsDeleted=0;
		msg.Update(GetQuery());
		
		TIMESTAMP_STRUCT now;
		GetQuery().Initialize();
		
		GetTimeStamp( now );
		
		BINDPARAM_LONG( GetQuery(), msg.m_OutboundMessageID );
		BINDPARAM_TIME_NOLEN( GetQuery(), now );
		GetQuery().Execute( _T("INSERT INTO OutboundMessageQueue (OutboundMessageID, DateSpooled, IsApproved) ")
								_T("VALUES (?,?,1)"));			

		GetRoutingEngine().LogIt( EMSERROR( EMS_LOG_SEVERITY_INFORMATIONAL,EMSERR_ISAPI_EXTENSION,
			EMS_LOG_OUTBOUND_MESSAGING, 0 ), _T("[System] added Outbound Message ID: (%d) to the OutboundMessageQueue"), msg.m_OutboundMessageID ); 

	
		GetGroups(nContactID);
	
	}
	else
	{
		GetXMLGen().AddChildElem( _T("Error"), _T("Invalid Confirmation Code") );
	}
}
