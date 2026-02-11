/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Servers.cpp,v 1.2.2.1 2005/12/13 18:11:49 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Servers
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Servers.h"
#include "DCIKey.h"
#include "RegistryFns.h"
#include <CkService.h>

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CServers::CServers( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	bIsNew = false;
	m_nMasterServerID = 0;
}

CServers::~CServers()
{

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CServers::Run( CURLAction& action )
{
	tstring sAction = _T("list");
	CEMSString sID;

	// check security
	RequireAdmin();

	// get the ID one way or the other...
	if (!GetISAPIData().GetURLLong( _T("ID"), m_ServerID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_ServerID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_ServerID);
		}
	}
	
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

	if( GetISAPIData().m_sPage.compare( _T("server") ) == 0 )
	{
		// change title if ID is zero
		if( m_ServerID == 0 )
		{
			action.m_sPageTitle.assign( "New Server" );
		}
		else
		{
			action.m_sPageTitle.assign( "Edit Server" );
		}
		
		if( sAction.compare( _T("insert") ) == 0 )
		{
			DISABLE_IN_DEMO();
			bIsNew = true;
			New();
			return 0;
		}
		else if( sAction.compare( _T("update") ) == 0 )
		{
			DISABLE_IN_DEMO();

			if( m_ServerID == 0 )
			{
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
			
			UpdateSettings();
			return 0;
		}		
		else if( sAction.compare( _T("svcstop") ) == 0 )
		{
			DISABLE_IN_DEMO();

			if( m_ServerID == 0 )
			{
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
			
			StopService();						
		}
		else if( sAction.compare( _T("svcstart") ) == 0 )
		{
			DISABLE_IN_DEMO();

			if( m_ServerID == 0 )
			{
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) ); 
			}
			
			StartService();						
		}
		return Query();
	}
	
	if( sAction.compare( _T("delete") ) == 0 )
	{
		DISABLE_IN_DEMO();
		if( m_ServerID == 0 )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString(EMS_STRING_INVALID_ID) );

		Delete();
	}

	action.m_sPageTitle.assign( "Servers" );
	return ListAll();	
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query all servers
// 
////////////////////////////////////////////////////////////////////////////////
int CServers::ListAll(void)
{
	RequireAdmin();

	tstring sValue;
	CEMSString sCheckIn,sWebCheckIn;

	GetServerParameter( EMS_SRVPARAM_MASTER_SERVER, sValue );
	m_nMasterServerID = _ttoi( sValue.c_str() );

	TServers::PrepareList( GetQuery() );
	
	GetXMLGen().AddChildElem( _T("Servers") );
	GetXMLGen().AddChildAttrib( _T("MasterServerID"), m_nMasterServerID );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("Server") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_ServerID );
		GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
		GetXMLGen().AddChildAttrib( _T("Status"), m_Status );
		if(m_CheckInLen > 0)
		{
			GetDateTimeString( m_CheckIn, m_CheckInLen, sCheckIn);			
		}
		else
		{
			sCheckIn.Format( _T("Never") );			
		}
		GetXMLGen().AddChildAttrib(_T("CheckIn"), sCheckIn.c_str());
		GetXMLGen().AddChildAttrib( _T("WebStatus"), m_WebStatus );
		if(m_WebCheckInLen > 0)
		{
			GetDateTimeString( m_WebCheckIn, m_WebCheckInLen, sWebCheckIn);			
		}
		else
		{
			sWebCheckIn.Format( _T("Never") );			
		}
		GetXMLGen().AddChildAttrib(_T("WebCheckIn"), sWebCheckIn.c_str());
	}
	GetXMLGen().OutOfElem();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query for a particular server
// 
////////////////////////////////////////////////////////////////////////////////
int CServers::Query(void)

{
	CEMSString sID;	
	
	if (!GetISAPIData().GetURLLong( _T("ID"), m_ServerID, true ))
	{
		if (!GetISAPIData().GetXMLLong( _T("ID"), m_ServerID, true ))
		{
			if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID, true );

			sID.CDLGetNextInt(m_ServerID);
		}
	}

	if ( m_ServerID > 0 )
	{
		TServers::Query(GetQuery());		
		GenerateXML();
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update a server
// 
////////////////////////////////////////////////////////////////////////////////
void CServers::UpdateSettings()
{
	TServers::Query( GetQuery() );

	tstring sOrigDescription = m_Description;	
	DecodeForm();

	//CheckForDuplicity();	

	_tcscpy( m_Description, sDescription.c_str() );
	_tcscpy( m_RegistrationKey, sRegistrationKey.c_str() );
	
	m_CheckInLen = SQL_NULL_DATA;

	TServers::Update(GetQuery());
	
	SetServerParameter( EMS_SRVPARAM_INBOUND_ATTACH, sInboundAttachFolder );
	SetServerParameter( EMS_SRVPARAM_OUTBOUND_ATTACH, sOutboundAttachFolder );
	SetServerParameter( EMS_SRVPARAM_NOTE_ATTACH, sNoteAttachFolder );
	SetServerParameter( EMS_SRVPARAM_BACKUP_PATH, sBackupFolder );
	SetServerParameter( EMS_SRVPARAM_ARCHIVE_PATH, sArchiveFolder );	

	WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("InboundAttachFolder"), sInboundAttachFolder.c_str() );
	WriteRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), sOutboundAttachFolder.c_str() );
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("NoteAttachFolder"), sNoteAttachFolder.c_str() );
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("BackupPath"), sBackupFolder.c_str() );
	WriteRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("ArchivePath"), sArchiveFolder.c_str() );
	
	UpdateServerTasks();

	if ( m_nServerType == 0 )
	{
		int nValue;
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nValue );
		GetQuery().Execute( _T("SELECT DataValue FROM ServerParameters WHERE ServerParameterID=72") );
		if ( GetQuery().Fetch() == S_OK )
		{
			if ( m_ServerID != nValue )
			{
				CEMSString sValue;
				sValue.Format( _T("%d"), m_ServerID );
				SetServerParameter( EMS_SRVPARAM_MASTER_SERVER, sValue );
			}
		}
	}

	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ServerParameters );
	InvalidateServerParameters(true);
}

////////////////////////////////////////////////////////////////////////////////
// 
// Update server tasks
// 
////////////////////////////////////////////////////////////////////////////////
void CServers::UpdateServerTasks()
{
	TServerTasks tst;	
	TServerTasks st;
	vector<TServerTasks> vst;
	vector<TServerTasks>::iterator vstIter;
	
	tst.PrepareList( GetQuery() );
	while( GetQuery().Fetch() == S_OK )
	{
		vst.push_back(tst);
	}

	for( vstIter = vst.begin(); vstIter != vst.end(); vstIter++ )
	{	
		if ( vstIter->m_ServerID != m_ServerID )
		{
			switch ( vstIter->m_ServerTaskTypeID )
			{
			case 1:			
				if ( nAlertSender == 1 )
				{
					st.m_ServerID = m_ServerID;
					st.m_ServerTaskTypeID = 1;
					st.Insert(GetQuery());
					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nAlertSender = 2;
				}						
				break;
			case 2:
				if ( nDBMaint == 1 )
				{
					st.m_ServerID = m_ServerID;
					st.m_ServerTaskTypeID = 2;
					st.Insert(GetQuery());
					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nDBMaint = 2;
				} 
				break;
			case 3:
				if ( nDBMon == 1 )
				{
					st.m_ServerID = m_ServerID;
					st.m_ServerTaskTypeID = 3;
					st.Insert(GetQuery());
					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nDBMon = 2;
				} 
				break;
			case 5:
				if ( nMsgRtr == 1 )
				{
					st.m_ServerID = m_ServerID;
					st.m_ServerTaskTypeID = 5;
					st.Insert(GetQuery());
					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nMsgRtr = 2;
				} 
				break;
			case 7:
				if ( nRpt == 1 )
				{
					st.m_ServerID = m_ServerID;
					st.m_ServerTaskTypeID = 7;
					st.Insert(GetQuery());
					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nRpt = 2;
				} 
				break;
			case 8:
				if ( nTktMon == 1 )
				{
					st.m_ServerID = m_ServerID;
					st.m_ServerTaskTypeID = 8;
					st.Insert(GetQuery());
					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nTktMon = 2;
				}				 
				break;
			}
		}
		else
		{
			switch ( vstIter->m_ServerTaskTypeID )
			{
			case 1:			
				if ( nAlertSender == 1 )
				{
					nAlertSender = 2;
				}						
				break;
			case 2:
				if ( nDBMaint == 1 )
				{
					nDBMaint = 2;
				} 
				break;
			case 3:
				if ( nDBMon == 1 )
				{
					nDBMon = 2;
				} 
				break;
			case 4:
				if ( nMsgCol == 0 )
				{
					GetQuery().Initialize();
					BINDPARAM_LONG( GetQuery(), m_ServerID );
					GetQuery().Execute( _T("SELECT MessageSourceID FROM MessageSources WHERE ServerID=?") );
					if( GetQuery().Fetch() == S_OK )
						THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("There are Messages Sources utilzing this Server, message collection can not be disabled!")  );

					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nMsgCol = 2;
				}
				else
				{
					nMsgCol = 2;
				}
				break;
			case 5:
				if ( nMsgRtr == 1 )
				{
					nMsgRtr = 2;
				} 
				break;
			case 6:
				if ( nMsgSnd == 0 )
				{
					GetQuery().Initialize();
					BINDPARAM_LONG( GetQuery(), m_ServerID );
					GetQuery().Execute( _T("SELECT MessageDestinationID FROM MessageDestinations WHERE ServerID=?") );
					if( GetQuery().Fetch() == S_OK )
						THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("There are Messages Destinations utilzing this Server, message sending can not be disabled!")  );

					st.m_ServerTaskID = vstIter->m_ServerTaskID;
					st.Delete(GetQuery());
					nMsgSnd = 2;
				}
				else
				{
					nMsgSnd = 2;
				}
				break;			
			case 7:
				if ( nRpt == 1 )
				{
					nRpt = 2;
				} 
				break;
			case 8:
				if ( nTktMon == 1 )
				{
					nTktMon = 2;
				}				 
				break;
			}		
		}		
	}	

	st.m_ServerID = m_ServerID;
		
	if(nAlertSender == 1)
	{
		st.m_ServerTaskTypeID = 1;
		st.Insert(GetQuery());
	}

	if(nDBMaint == 1)
	{
		st.m_ServerTaskTypeID = 2;
		st.Insert(GetQuery());
	}

	if(nDBMon == 1)
	{
		st.m_ServerTaskTypeID = 3;
		st.Insert(GetQuery());
	}

	if(nMsgCol == 1)
	{
		st.m_ServerTaskTypeID = 4;
		st.Insert(GetQuery());
	}

	if(nMsgRtr == 1)
	{
		st.m_ServerTaskTypeID = 5;
		st.Insert(GetQuery());
	}

	if(nMsgSnd == 1)
	{
		st.m_ServerTaskTypeID = 6;
		st.Insert(GetQuery());
	}

	if(nRpt == 1)
	{
		st.m_ServerTaskTypeID = 7;
		st.Insert(GetQuery());
	}

	if(nTktMon == 1)
	{
		st.m_ServerTaskTypeID = 8;
		st.Insert(GetQuery());
	}

}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a server
// 
////////////////////////////////////////////////////////////////////////////////
int CServers::Delete()
{
	if ( m_ServerID )
	{
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(),m_ServerID );		
		GetQuery().Execute( _T("DELETE FROM ServerTasks ")
						_T("WHERE ServerID=? ") );

		TServers::Delete(GetQuery());		
	}
	
	return 0;
}

void CServers::CheckForDuplicity( void )
{
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_ServerID );
	BINDPARAM_TCHAR_STRING( GetQuery(), sDescription );
	BINDPARAM_TCHAR_STRING( GetQuery(), sRegistrationKey );
	GetQuery().Execute( _T("SELECT ServerID FROM Servers WHERE ServerID<>? AND (Description=? OR RegistrationKey=?)") );

	if( GetQuery().Fetch() == S_OK )
	{
		THROW_EMS_EXCEPTION_NOLOG( E_Duplicate_Name, _T("A Server with that description or registration key already exists,\nplease enter a unique Description and Registration Key!")  );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Create a new, blank scheduled report
// 
////////////////////////////////////////////////////////////////////////////////
int CServers::New()
{
	DecodeForm();

	//Check for unique Description
	CheckForDuplicity();
	
	m_CheckInLen = SQL_NULL_DATA;
	m_WebCheckInLen = SQL_NULL_DATA;
	
	TServers::Insert( GetQuery() );

	m_ISAPIData.m_RoutingEngine.ReloadConfig( EMS_ServerParameters );
	InvalidateServerParameters(true);

	return 0;
}

void CServers::DecodeForm(void)
{
	tstring sValue;
	
	GetISAPIData().GetXMLString( _T("Description"), sDescription, true );
	if ( sDescription.length() < 1 )
	{
		THROW_VALIDATION_EXCEPTION( _T("Description"), _T("Please specify a description for the server") );
	}
	else
	{
		_tcsncpy( m_Description, sDescription.c_str(), SERVER_DESCRIPTION_LENGTH - 1 );
	}

	/*GetISAPIData().GetXMLString( _T("RegistrationKey"), sRegistrationKey, true );
	if ( sRegistrationKey.length() < 1 )
	{
		THROW_VALIDATION_EXCEPTION( _T("Registration Key"), _T("Please enter a valid Registration Key") );
	}
	else
	{
		_tcsncpy( m_RegistrationKey, sRegistrationKey.c_str(), SERVER_REGISTRATION_KEY_LENGTH - 1 );
	}*/

	GetISAPIData().GetXMLLong( _T("ServerType"), m_nServerType );

	if( GetISAPIData().GetXMLString( _T("InboundAttach"), sInboundAttachFolder, true ) )
	{
		if ( sInboundAttachFolder.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_INBOUND_ATTACH, sInboundAttachFolder );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_INBOUND_ATTACH, sInboundAttachFolder );
	}
	VerifyDirectoryExists( sInboundAttachFolder );
	
	if( GetISAPIData().GetXMLString( _T("OutboundAttach"), sOutboundAttachFolder, true ) )
	{
		if ( sOutboundAttachFolder.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_OUTBOUND_ATTACH, sOutboundAttachFolder );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_OUTBOUND_ATTACH, sOutboundAttachFolder );
	}
	VerifyDirectoryExists( sOutboundAttachFolder );
	
	if( GetISAPIData().GetXMLString( _T("NoteAttach"), sNoteAttachFolder, true ) )
	{
		if ( sNoteAttachFolder.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_NOTE_ATTACH, sNoteAttachFolder );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_NOTE_ATTACH, sNoteAttachFolder );
	}
	VerifyDirectoryExists( sNoteAttachFolder );
	
	if( GetISAPIData().GetXMLString( _T("BackupFolder"), sBackupFolder, true ) )
	{
		if ( sBackupFolder.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_BACKUP_PATH, sBackupFolder );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_BACKUP_PATH, sBackupFolder );	
	}
	VerifyDirectoryExists( sBackupFolder );
	
	if( GetISAPIData().GetXMLString( _T("ArchiveFolder"), sArchiveFolder, true ) )
	{
		if ( sArchiveFolder.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_ARCHIVE_PATH, sArchiveFolder );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_ARCHIVE_PATH, sArchiveFolder );
	}
	VerifyDirectoryExists( sArchiveFolder );

	if( GetISAPIData().GetXMLString( _T("SpoolFolder"), sValue, true ) )
	{
		if ( sValue.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_SPOOL_FOLDER, sValue );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_SPOOL_FOLDER, sValue );
	}
	VerifyDirectoryExists( sValue );
	_tcsncpy( m_SpoolFolder, sValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	auto retVal = sValue.empty();

	if( GetISAPIData().GetXMLString( _T("TempFolder"), sValue, true ) )
	{
		if ( sValue.length() == 0 )
		{
			GetServerParameter( EMS_SRVPARAM_TEMP_FOLDER, sValue );					
		}
	}
	else
	{
		GetServerParameter( EMS_SRVPARAM_TEMP_FOLDER, sValue );
	}	
	VerifyDirectoryExists( sValue );
	_tcsncpy( m_TempFolder, sValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	
	if ( bIsNew == false )
	{
		//GetISAPIData().GetXMLLong( _T("chkAlertSender"), nAlertSender );
		nAlertSender = 0;
		nDBMon = 0;
		if ( m_nServerType == 0 )
		{
			nAlertSender = 1;
			nDBMon = 1;
		}
		GetISAPIData().GetXMLLong( _T("chkDBMaint"), nDBMaint );
		GetISAPIData().GetXMLLong( _T("chkMsgCol"), nMsgCol );
		GetISAPIData().GetXMLLong( _T("chkMsgRtr"), nMsgRtr );
		GetISAPIData().GetXMLLong( _T("chkMsgSnd"), nMsgSnd );
		GetISAPIData().GetXMLLong( _T("chkReporter"), nRpt );
		GetISAPIData().GetXMLLong( _T("chkTktMon"), nTktMon );
	}

}

void CServers::GenerateXML(void)
{
	
	CEMSString sValue;
	tstring sServiceStatus(_T("Unknown"));
	
	GetServerParameter( EMS_SRVPARAM_MASTER_SERVER, sValue );
	m_nMasterServerID = _ttoi( sValue.c_str() );
	GetServerParameter( EMS_SRVPARAM_INBOUND_ATTACH, sInboundAttachFolder );
	GetServerParameter( EMS_SRVPARAM_OUTBOUND_ATTACH, sOutboundAttachFolder );
	GetServerParameter( EMS_SRVPARAM_NOTE_ATTACH, sNoteAttachFolder );
	GetServerParameter( EMS_SRVPARAM_BACKUP_PATH, sBackupFolder );
	GetServerParameter( EMS_SRVPARAM_ARCHIVE_PATH, sArchiveFolder );

	UINT m_nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, m_nServerID ) == ERROR_SUCCESS)
	{
		if(m_ServerID == m_nServerID)
		{
			//get engine the service status
			CkService service;
			
			service.put_ServiceName("VisNetic MailFlow Engine");

			//  returns -1 for failure (inability to check)
			//  returns 1 if running.
			//  returns 0 if not running.
			long retval;
			retval = service.IsRunning();
			if (retval == 1)
			{
				sServiceStatus.assign(_T("Running"));
			}
			else if (retval == 0)
			{
				sServiceStatus.assign(_T("Stopped"));
			}
		}
	}

	GetXMLGen().AddChildElem( _T("Server") );
	GetXMLGen().AddChildAttrib( _T("MasterServerID"), m_nMasterServerID );
	GetXMLGen().AddChildAttrib( _T("InboundAttach"), sInboundAttachFolder.c_str() );	
	GetXMLGen().AddChildAttrib( _T("OutboundAttach"), sOutboundAttachFolder.c_str() );
	GetXMLGen().AddChildAttrib( _T("NoteAttach"), sNoteAttachFolder.c_str() );
	GetXMLGen().AddChildAttrib( _T("BackupFolder"), sBackupFolder.c_str() );	
	GetXMLGen().AddChildAttrib( _T("ArchiveFolder"), sArchiveFolder.c_str() );
	GetXMLGen().AddChildAttrib( _T("ID"), m_ServerID );
	GetXMLGen().AddChildAttrib( _T("Description"), m_Description );
	/*GetXMLGen().AddChildAttrib( _T("RegistrationKey"), m_RegistrationKey );*/
	GetXMLGen().AddChildAttrib( _T("SpoolFolder"), m_SpoolFolder );
	GetXMLGen().AddChildAttrib( _T("TempFolder"), m_TempFolder );
	GetXMLGen().AddChildAttrib( _T("ServiceStatus"), sServiceStatus.c_str() );
		
	if ( !m_ServerID )
	{
		GetXMLGen().AddChildAttrib( _T("IsNew"), 1 );
	}
		
	TServerTasks st;	
	st.PrepareList(GetQuery());
	
	while( GetQuery().Fetch() == S_OK )
	{
		if ( st.m_ServerID == m_ServerID )
		{
			switch ( st.m_ServerTaskTypeID )
			{
			case 1:
				nAlertSender = 1;
				break;
			case 2:
				nDBMaint = 1;
				break;
			case 3:
				nDBMon = 1;
				break;
			case 4:
				nMsgCol = 1;
				break;
			case 5:
				nMsgRtr = 1;
				break;
			case 6:
				nMsgSnd = 1;
				break;
			case 7:
				nRpt = 1;
				break;
			case 8:
				nTktMon = 1;
				break;
			}
		}	
	}
	
	GetXMLGen().AddChildAttrib( _T("AlertSender"), nAlertSender );
	GetXMLGen().AddChildAttrib( _T("DBMaint"), nDBMaint );
	GetXMLGen().AddChildAttrib( _T("DBMon"), nDBMon );
	GetXMLGen().AddChildAttrib( _T("MsgCol"), nMsgCol );
	GetXMLGen().AddChildAttrib( _T("MsgRtr"), nMsgRtr );
	GetXMLGen().AddChildAttrib( _T("MsgSnd"), nMsgSnd );
	GetXMLGen().AddChildAttrib( _T("Reporter"), nRpt );
	GetXMLGen().AddChildAttrib( _T("TktMon"), nTktMon );

    /*GenerateRegistrationXML( m_ServerID );*/
}

////////////////////////////////////////////////////////////////////////////////
// 
// GenerateRegistrationXML
// 
////////////////////////////////////////////////////////////////////////////////
void CServers::GenerateRegistrationXML( int nServerID )
{
	CEMSString sDate;

	GetISAPIData().m_LicenseMgr.GetKeyInfo( GetQuery(), nServerID  );

	GetXMLGen().AddChildElem( _T("Registration") );

#if defined(DEMO_VERSION)
	GetXMLGen().AddChildAttrib( _T("Key"), _T("Demo Version") );
#else
	GetXMLGen().AddChildAttrib( _T("Key"), GetISAPIData().m_LicenseMgr.m_szMFKey );
#endif
	
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
				GetXMLGen().AddChildAttrib( _T("Status"), _T("Upgrade protection has expired") );
			}
			else
			{
				GetXMLGen().AddChildAttrib( _T("Status"), _T("Key has expired") );
			}
		}

		AddUserSize( GetISAPIData().m_LicenseMgr.m_MFKeyInfo.UserSize );
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

}

void CServers::AddUserSize( long nUserSize )
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

void CServers::VerifyDirectoryExists( tstring& sFolder )
{
	CEMSString sFilename;
	SYSTEMTIME now;
	int n = 99;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if( sFolder.size() < 1 )
	{
		CEMSString sError;
		sError.Format( _T("Please provide a valid directory."), sFolder.c_str() );
		THROW_EMS_EXCEPTION( E_SystemError, sError );
	}

	GetLocalTime( &now );

	sFilename.Format( _T("%s\\VMF-%d-%d-%d-%d.TST"), sFolder.c_str(),
			  now.wYear, now.wMonth, now.wDay, n );

	hFile = CreateFile( sFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, 
			            FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile == INVALID_HANDLE_VALUE )
	{
		DWORD dwErr = GetLastError();

		if( dwErr != ERROR_ALREADY_EXISTS && dwErr != ERROR_FILE_EXISTS )
		{
			CEMSString sError;
			sError.Format( _T("%s is not a valid directory or the permissions are incorrect."), sFolder.c_str() );
			THROW_EMS_EXCEPTION( E_SystemError, sError );
		}		
	}
	else
	{
		CloseHandle( hFile );
		DeleteFile( sFilename.c_str() );
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Stop Service
// 
////////////////////////////////////////////////////////////////////////////////
void CServers::StopService()
{
	CkService service;

    service.put_ServiceName("VisNetic MailFlow Engine");

    bool success;
    success = service.Stop();
    if (success != true)
	{
        DebugReporter::Instance().DisplayMessage("CServers::StopService - Failed to stop service.", DebugReporter::ISAPI, GetCurrentThreadId());
    }
	else
	{
        DebugReporter::Instance().DisplayMessage("CServers::StopService - Service successfully stopped.", DebugReporter::ISAPI, GetCurrentThreadId());
    }    
}

////////////////////////////////////////////////////////////////////////////////
// 
// Start Service
// 
////////////////////////////////////////////////////////////////////////////////
void CServers::StartService()
{
	CkService service;

    service.put_ServiceName("VisNetic MailFlow Engine");

    bool success;
    success = service.Start();
    if (success != true)
	{
        DebugReporter::Instance().DisplayMessage("CServers::StopService - Failed to start service.", DebugReporter::ISAPI, GetCurrentThreadId());
    }
	else
	{
        DebugReporter::Instance().DisplayMessage("CServers::StopService - Service successfully started.", DebugReporter::ISAPI, GetCurrentThreadId());
    }
}

