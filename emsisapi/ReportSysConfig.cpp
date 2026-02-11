// ReportSysConfig.cpp: implementation of the CReportSysConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReportSysConfig.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReportSysConfig::CReportSysConfig( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	// This should only be seen by the admin
	RequireAdmin();

	m_hCDFFile = INVALID_HANDLE_VALUE;
	m_MaxRows = 250;
	m_OutputCDF = false;
}

CReportSysConfig::~CReportSysConfig()
{

}


////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CReportSysConfig::Run( CURLAction& action )
{
	
	
	if( m_ISAPIData.m_sPage.compare( _T("reportsysconfig") ) == 0)
	{
		try
		{
			tstring sAction;

			if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
			{
				if( sAction.compare( _T("download") ) == 0 )
					OpenCDF(action);
			}

			RunReport( action );
			
		}
		catch(...)
		{
			if ( m_OutputCDF )
			{
				CloseCDF();
				DeleteFile( m_szTempFile );
			}

			throw;
		}

		CloseCDF();

		return 0;		
	}
	else if( m_ISAPIData.m_sPage.compare( _T("dbsettings") ) == 0)
	{
		try
		{
			tstring sAction;

			if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
			{
				if( sAction.compare( _T("download") ) == 0 )
					OpenCDF(action);
			}

			RunDBReport( action );
			
		}
		catch(...)
		{
			if ( m_OutputCDF )
			{
				CloseCDF();
				DeleteFile( m_szTempFile );
			}

			throw;
		}

		CloseCDF();

		return 0;		
	}
	else if ( m_ISAPIData.m_sPage.compare( _T("effectiverights") ) == 0)
	{
		tstring sAction;

		if( GetISAPIData().GetURLString( _T("Action"), sAction, true ) )
		{
			if( sAction.compare( _T("show") ) == 0 )
			{
				m_ShowRights = true;
				DecodeForm();				
			}
		}
		else
		{
			m_AgentGroup = 0;
			m_ObjectID = 0;
			m_ShowRights = false;
			GenerateXML();
		}		
	}

	return  0;
}
	
void CReportSysConfig::GenerateXML( void )
{
	
	GetXMLGen().AddChildElem( _T("EffectiveRights") );
	GetXMLGen().AddChildAttrib( _T("AgentID"), m_AgentID );
	GetXMLGen().AddChildAttrib( _T("GroupID"), m_GroupID );
	GetXMLGen().AddChildAttrib( _T("AgentGroup"), m_AgentGroup );
	GetXMLGen().AddChildAttrib( _T("ObjectID"), m_ObjectID );
	GetXMLGen().AddChildAttrib( _T("AgentOID"), m_AgentOID );
	GetXMLGen().AddChildAttrib( _T("GroupOID"), m_GroupOID );
	GetXMLGen().AddChildAttrib( _T("TicketBoxID"), m_TicketBoxID );
	GetXMLGen().AddChildAttrib( _T("TicketCategoryID"), m_TicketCategoryID );
	GetXMLGen().AddChildAttrib( _T("SRCategoryID"), m_SRCategoryID );
		
	if ( m_ShowRights )
	{
        GetXMLGen().AddChildAttrib( _T("Level"), m_EffectiveRight );		
	}
	
	ListAgentNames();
	ListGroupNames();
	ListStdRespCategories();
	ListTicketBoxNames( GetXMLGen() );
		
	TTicketCategories pdt;
	pdt.PrepareList( GetQuery() );
	GetXMLGen().AddChildElem( _T("TicketCategoryNames") );
	GetXMLGen().IntoElem();
	while( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().AddChildElem( _T("TicketCategory") );
		GetXMLGen().AddChildAttrib( _T("ID"), pdt.m_TicketCategoryID );
		GetXMLGen().AddChildAttrib( _T("Description"), pdt.m_Description );
	}
	GetXMLGen().OutOfElem();

}


void CReportSysConfig::DecodeForm( void )
{
	GetISAPIData().GetURLLong( _T("AgentGroup"), m_AgentGroup, true  );
	GetISAPIData().GetURLLong( _T("ObjectID"), m_ObjectID, true  );
	GetISAPIData().GetURLLong( _T("AgentID"), m_AgentID, true );
	GetISAPIData().GetURLLong( _T("GroupID"), m_GroupID, true  );
	GetISAPIData().GetURLLong( _T("AgentOID"), m_AgentOID, true );
	GetISAPIData().GetURLLong( _T("GroupOID"), m_GroupOID, true  );
	GetISAPIData().GetURLLong( _T("TicketBoxID"), m_TicketBoxID, true  );
	GetISAPIData().GetURLLong( _T("TicketCategoryID"), m_TicketCategoryID, true  );
	GetISAPIData().GetURLLong( _T("SRCategoryID"), m_SRCategoryID, true  );

	GetRights();	

}


void CReportSysConfig::GetRights( void )
{
	CEMSString sQuery;
	int nObjectTypeID;
	int nActualID = 0;
	unsigned char RightLevel;

	switch ( m_ObjectID )
	{
	case 0:
		nObjectTypeID = 1;
		nActualID = m_AgentOID;
		break;
	case 1:
		nObjectTypeID = 4;
		nActualID = m_GroupOID;
		break;
	case 2:
		nObjectTypeID = 2;
		nActualID = m_TicketBoxID;
		break;
	case 3:
		nObjectTypeID = 6;
		nActualID = m_TicketCategoryID;
		break;
	case 4:
		nObjectTypeID = 3;
		break;
	case 5:
		nObjectTypeID = 8;
		break;
	case 6:
		nObjectTypeID = 7;
		break;
	case 7:
		nObjectTypeID = 5;
		nActualID = m_SRCategoryID;
		break;
	}
	
	if ( m_AgentGroup == 0 )
	{
		RightLevel = CAccessControl::GetInstance().GetAgentRightLevel( GetQuery(), m_AgentID, nObjectTypeID, nActualID );
	}
	
	m_EffectiveRight = RightLevel;
	
	if( m_EffectiveRight >= 0 && m_EffectiveRight < 6 )
	{
		GenerateXML();
	}
	else
	{
		THROW_EMS_EXCEPTION( E_InvalidID, _T("Process failed to return valid right level!") );
	}	
	
}
////////////////////////////////////////////////////////////////////////////////
// 
// RunReport
// 
////////////////////////////////////////////////////////////////////////////////
void CReportSysConfig::RunReport( CURLAction& action )
{
	set<TServerParameters> ParamList;
	set<TServerParameters>::iterator iter;

	TServerParameters param;
	tstring sRegValue;
	CEMSString sInstallPath;
	CEMSString sVersion;
	CEMSString sEncrypted;
	CEMSString sLine;

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), param.m_ServerParameterID );
	BINDCOL_TCHAR( GetQuery(), param.m_Description );
	BINDCOL_TCHAR( GetQuery(), param.m_DataValue );
	GetQuery().Execute( _T("SELECT ServerParameterID,Description,DataValue ")
						_T("FROM ServerParameters ")
						_T("ORDER BY Description") );

	while( GetQuery().Fetch() == S_OK )
	{
		switch( param.m_ServerParameterID )
		{
		case EMS_SRVPARAM_ENABLE_DAILY_MAINTENANCE:
		case EMS_SRVPARAM_ENABLE_DAILY_BACKUPS:
		case EMS_SRVPARAM_ENABLE_DAILY_ARCHIVES:
		case EMS_SRVPARAM_ENABLE_DAILY_PURGES:
		case EMS_SRVPARAM_SESSION_IP_LOCKING:
		case EMS_SRVPARAM_FORCE_HTTPS:
		case EMS_SRVPARAM_AGENT_ACTIVITY_LOG:
		case EMS_SRVPARAM_DATABASE_LOGGING:
		case EMS_SRVPARAM_USE_CUTOFF_READ_DATE:
		case EMS_SRVPARAM_CUTOFF_READ_DATE:
		case EMS_SRVPARAM_MYTICKETS_READ_UPDATE:
		case EMS_SRVPARAM_PUBLIC_TICKETS_READ_UPDATE:
		case EMS_SRVPARAM_AGENTVIEWS_READ_UPDATE:
		case EMS_SRVPARAM_ENABLE_SCHEDULED_REPORTS:
		case EMS_SRVPARAM_VOIP_INTEGRATION:
		case EMS_SRVPARAM_AGENT_ACTIVITY_AUTHENTICATION:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_DELETE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CREATE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MOVE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_ESCALATE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_REASSIGN:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_RESTORE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_OPEN:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CLOSE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_ADD:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_DELETE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_REVOKE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_RELEASE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_RETURN:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_COPIED:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MERGE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_SAVED:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_LINK_CREATED:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_LINK_DELETED:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_LINKED:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_UNLINKED:	
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CHANGE_CATEGORY:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_CHANGE_FIELD:
		case EMS_SRVPARAM_AGENT_EMAIL_BYPASS:
		case EMS_SRVPARAM_SEARCH_REQUIRE_DATE:
		case EMS_SRVPARAM_SESSION_LOGOFF_ALL:
		case EMS_SRVPARAM_SESSION_EXPIRE_ALL:
		case EMS_SRVPARAM_ALLOW_AGENT_LOGOFF:
		case EMS_SRVPARAM_ARCHIVE_ACCESS:		
		case EMS_SRVPARAM_ALLOW_TICKET_ADMIN:
		case EMS_SRVPARAM_ALLOW_TICKET_AGENT:
		case EMS_SRVPARAM_ENABLE_TICKET_SEARCH_GETOLDEST:
		case EMS_SRVPARAM_DEFAULT_CLOSE_TICKET_AFTER_SEND:
		case EMS_SRVPARAM_DEFAULT_ROUTE_TO_ME:
		case EMS_SRVPARAM_DEF_RR_NEW_TICKET:
		case EMS_SRVPARAM_DEF_RR_ONLINE:
		case EMS_SRVPARAM_DEF_RR_AWAY:
		case EMS_SRVPARAM_DEF_RR_NOTAVAIL:
		case EMS_SRVPARAM_DEF_RR_OFFLINE:
		case EMS_SRVPARAM_DEF_RR_OOO:
		case EMS_SRVPARAM_DEF_RR_CHECK_OPEN_OWNER:
		case EMS_SRVPARAM_SESSION_KEEP_ALIVE:
		case EMS_SRVPARAM_DENY_TICKET_ONLINE:
		case EMS_SRVPARAM_DENY_TICKET_AWAY:
		case EMS_SRVPARAM_DENY_TICKET_NOTAVAIL:
		case EMS_SRVPARAM_DENY_TICKET_OFFLINE:
		case EMS_SRVPARAM_DENY_TICKET_OOO:
		case EMS_SRVPARAM_ENABLE_EMAIL_VERIFICATION:
		case EMS_SRVPARAM_MASTER_WEBSESSIONS:
		case EMS_SRVPARAM_DB_WEBSESSIONS:
		case EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_CATCHALL:
		case EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_DECREMENT:
		case EMS_SRVPARAM_VERIFY_CONTACT_EMAIL_NOMX:

			// Convert 1s and 0s to Enabled and Disabled
			if( _ttoi(param.m_DataValue) )
			{
				_tcscpy( param.m_DataValue, _T("Enabled") );
			}
			else
			{
				_tcscpy( param.m_DataValue, _T("Disabled") );
			}
			ParamList.insert( param );
			break;

		case EMS_SRVPARAM_ANTIVIRUS_ACTION:
		case EMS_SRVPARAM_ANTIVIRUS_ENABLE:
		case EMS_SRVPARAM_ANTIVIRUS_UNSCANNABLE_ACTION:
		case EMS_SRVPARAM_ANTIVIRUS_SUSPICIOUS_ACTION:
		case EMS_SRVPARAM_MSGROUTER_STATE:
		case EMS_SRVPARAM_MSGROUTER_CONTACTID:
		case EMS_SRVPARAM_MSGROUTER_INBOUNDMSGID:
		case EMS_SRVPARAM_MSGROUTER_RULEID:
		case EMS_SRVPARAM_MSGROUTER_NEWTICKET:
		case EMS_SRVPARAM_MSGROUTER_TICKETID:
		case EMS_SRVPARAM_LAST_MAILFLOW_REG_ALERT:
		case EMS_SRVPARAM_LAST_ANTIVIRUS_REG_ALERT:
		case EMS_SRVPARAM_3CX_PLUGIN_VERSION:
		case EMS_SRVPARAM_MAINTENANCE_HIGH:
		case EMS_SRVPARAM_MAINTENANCE_LOW:
		case EMS_SRVPARAM_VMS_INTEGRATION:
		case EMS_SRVPARAM_VMS_WEBMAIL_URL:
		case EMS_SRVPARAM_VMS_DSN:
		case EMS_SRVPARAM_AGENT_ACTIVITY_ALERT_READ:
		case EMS_SRVPARAM_AGENT_ACTIVITY_ALERT_DELETE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_MESSAGE_READ:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_NOTE_ADD:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_NOTE_DELETE:
		case EMS_SRVPARAM_AGENT_ACTIVITY_TICKET_NOTE_READ:		
		case EMS_SRVPARAM_CHARACTER_SET:
		case EMS_SRVPARAM_MAILFLOW_CLIENT_VERSION:

			// Ignore these parameters
			break;		

		case EMS_SRVPARAM_ARCHIVE_REMOVE_TICKETS:
			// Archive remove action
			switch( _ttoi(param.m_DataValue) )
			{
			case 0:
				_tcscpy( param.m_DataValue, _T("Nothing") );
				break;

			case 1:
				_tcscpy( param.m_DataValue, _T("Messages") );
				break;

			case 2:
				_tcscpy( param.m_DataValue, _T("Tickets and Messages") );
				break;
			}
			ParamList.insert( param );
			break;

		case EMS_SRVPARAM_TICKET_LIST_DATE:
			// Ticket list date
			switch( _ttoi(param.m_DataValue) )
			{
			case 0:
				_tcscpy( param.m_DataValue, _T("Latest Ticket Open Date") );
				break;

			case 1:
				_tcscpy( param.m_DataValue, _T("Latest Inbound/Outbound Message or Note Date") );
				break;

			case 2:
				_tcscpy( param.m_DataValue, _T("First Inbound Message Date") );
				break;
			}
			ParamList.insert( param );
			break;			

		case EMS_SRVPARAM_MESSAGE_DATE:
			// Message display date
			switch( _ttoi(param.m_DataValue) )
			{
			case 0:
				_tcscpy( param.m_DataValue, _T("Date Received by System") );
				break;

			case 1:
				_tcscpy( param.m_DataValue, _T("Message Date Header") );
				break;
			}
			ParamList.insert( param );
			break;			

		case EMS_SRVPARAM_DATE_ENTRY_FORMAT:
			// Date Format
			switch( _ttoi(param.m_DataValue) )
			{
			case 0:
				_tcscpy( param.m_DataValue, _T("MM/DD/YYYY") );
				break;

			case 1:
				_tcscpy( param.m_DataValue, _T("DD/MM/YYYY") );
				break;
			}
			ParamList.insert( param );
			break;			

		case EMS_SRVPARAM_TIME_ZONE_ID:
			_tcscpy( param.m_Description, _T("Display Time Zone") );				
			if(_ttoi(param.m_DataValue) == 0)
			{
				_tcscpy( param.m_DataValue, _T("Use Server Operating System Time Zone") );
			}
			else
			{
				TTimeZones tz;
				tz.PrepareList( GetQuery() );				
				while( GetQuery().Fetch() == S_OK )
				{
					if(tz.m_TimeZoneID == _ttoi(param.m_DataValue))
					{
						_tcscpy( param.m_DataValue,tz.m_DisplayName );
						break;
					}		
				}
			}
			ParamList.insert( param );
			break;	
		case EMS_SRVPARAM_TICKET_TRACKING:
			// Ticket Tracking
			switch( _ttoi(param.m_DataValue) )
			{
			case 0:
				_tcscpy( param.m_DataValue, _T("Place Ticket Tracking Data in Subject of Outbound Messages") );
				break;

			case 1:
				_tcscpy( param.m_DataValue, _T("Place Ticket Tracking Data in Body of Outbound Messages") );
				break;

			case 2:
				_tcscpy( param.m_DataValue, _T("Do not add Ticket Tracking Data to Outbound Messages") );
				break;
			}
			ParamList.insert( param );
			break;		

		case EMS_SRVPARAM_DEFAULT_MSGDEST_ID:
			// Default Message Destination
			XMessageDestinationNames msgdestname;
			if( GetXMLCache().m_MsgDestNames.Query( _ttoi(param.m_DataValue), msgdestname ) )
			{
				_tcscpy( param.m_DataValue, msgdestname.m_Name );
			}
			else
			{
				_tcscpy( param.m_DataValue, _T("Undefined") );
			}
			ParamList.insert( param );
			break;	
		
		case EMS_SRVPARAM_LICENSE_KEY:

#if defined(DEMO_VERSION)
	_tcscpy( param.m_DataValue, _T("Demo License") );	
#endif
#if defined(HOSTED_VERSION)
	_tcscpy( param.m_DataValue, _T("Hosted License") );	
#endif
			ParamList.insert( param );
			break;
		
		default:
			// Add as-is
			ParamList.insert( param );
			break;
		}
	}

	// URLSubDir
	_tcscpy( param.m_Description, _T("URL Subdirectory") );
	_tcscpy( param.m_DataValue, GetISAPIData().m_sURLSubDir.c_str() );
	ParamList.insert( param );

	// Database Login
	sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
	               EMS_DATABASE_DSN_VALUE, sRegValue );
	_tcscpy( param.m_Description, _T("Database Login") );
	sEncrypted.assign( sRegValue );
	sEncrypted.Decrypt();
	_tcsncpy( param.m_DataValue, sEncrypted.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	BlankOutDBPassword( param.m_DataValue );
#if defined(DEMO_VERSION)
	_tcscpy( param.m_DataValue, _T("Demo Version") );	
#endif
	ParamList.insert( param );

	// Database Type
	sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_DATABASE_TYPE_VALUE, sRegValue );
	_tcscpy( param.m_Description, _T("Database Type") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );

	ParamList.insert( param );

    ////////////////////////////////// paths

	/*sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_LOG_PATH_VALUE, sRegValue );
	_tcscpy( param.m_Description, _T("Log Path") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	ParamList.insert( param );*/

	sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_INSTALL_PATH_VALUE, sRegValue );
	_tcscpy( param.m_Description, _T("Install Path") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	ParamList.insert( param );
	sInstallPath.assign( sRegValue.c_str() );

	/*sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_ARCHIVE_PATH_VALUE, sRegValue );
	_tcscpy( param.m_Description, _T("Archive Path") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	ParamList.insert( param );*/

	/*sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_BACKUP_PATH_VALUE, sRegValue );
	_tcscpy( param.m_Description, _T("Backup Path") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	ParamList.insert( param );*/

	// Version info

	GetVersionInfo( sInstallPath, _T("MailComponents.dll"), sVersion );
	_tcscpy( param.m_Description, _T("Version of MailComponents.dll") );
	_tcscpy( param.m_DataValue, sVersion.c_str()  );
	ParamList.insert( param );

	GetVersionInfo( sInstallPath, _T("EMSIsapi.dll"), sVersion );
	_tcscpy( param.m_Description, _T("Version of EMSIsapi.dll") );
	_tcscpy( param.m_DataValue, sVersion.c_str()  );
	ParamList.insert( param );

	GetVersionInfo( sInstallPath, _T("MailFlowEngine.exe"), sVersion );
	_tcscpy( param.m_Description, _T("Version of MailFlowEngine.exe") );
	_tcscpy( param.m_DataValue, sVersion.c_str()  );
	ParamList.insert( param );

	GetVersionInfo( sInstallPath, _T("EMSResource.dll"), sVersion );
	_tcscpy( param.m_Description, _T("Version of EMSResource.dll") );
	_tcscpy( param.m_DataValue, sVersion.c_str()  );
	ParamList.insert( param );

	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          EMS_VERSION_VALUE, sVersion );
	_tcscpy( param.m_Description, _T("VisNetic MailFlow Version") );
	_tcscpy( param.m_DataValue, sVersion.c_str()  );
	ParamList.insert( param );

	GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), sRegValue );
	_tcscpy( param.m_Description, _T("Outbound Attachment Folder") );
	_tcscpy( param.m_DataValue, sRegValue.c_str()  );
	ParamList.insert( param );

	GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("InboundAttachFolder"), sRegValue );
	_tcscpy( param.m_Description, _T("Inbound Attachment Folder") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	ParamList.insert( param );

	GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("SpoolFolder"), sRegValue );
	_tcscpy( param.m_Description, _T("Spool Folder") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
	ParamList.insert( param );

	// Output the XML or File
	if( m_OutputCDF )
	{
		OutputCDF( tstring("Description, Value\r\n") );
	}

	for( iter = ParamList.begin(); iter != ParamList.end(); iter++ )
	{
		if( m_OutputCDF )
		{
			// add line to CSV file
			sLine.Format( "\"%s\",\"%s\"\r\n", iter->m_Description, iter->m_DataValue );
			OutputCDF( sLine );
		}
		else
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Description"), iter->m_Description );
			GetXMLGen().AddChildAttrib( _T("Value"), iter->m_DataValue );
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// RunDBReport
// 
////////////////////////////////////////////////////////////////////////////////
void CReportSysConfig::RunDBReport( CURLAction& action )
{
	set<TServerParameters> ParamList;
	set<TServerParameters>::iterator iter;

	TServerParameters param;
	tstring sRegValue;
	CEMSString sInstallPath;
	CEMSString sVersion;
	CEMSString sEncrypted;
	CEMSString sLine;

	// SQL Server
	sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          _T("SQLServer"), sRegValue );
	_tcscpy( param.m_Description, _T("SQL Server") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );

	ParamList.insert( param );

	// SQL Server Version
	GetQuery().Initialize();
	BINDCOL_TCHAR( GetQuery(), param.m_DataValue );
	GetQuery().Execute( _T("Select @@version") );
	if( GetQuery().Fetch() == S_OK )
	{
		_tcscpy( param.m_Description, _T("SQL Server Version") );
		ParamList.insert( param );	
	}
	
	// Database Name
	sRegValue.erase();
	GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,
		          _T("DBNAME"), sRegValue );
	_tcscpy( param.m_Description, _T("Database Name") );
	_tcsncpy( param.m_DataValue, sRegValue.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );

	ParamList.insert( param );

	/*SELECT 
	Physical_Name, (size*8)/1024 SizeMB
	FROM sys.master_files
	WHERE DB_NAME(database_id)='VisNetic MailFlow'*/

	// Database Files
	GetQuery().Initialize();
	BINDCOL_TCHAR( GetQuery(), param.m_DataValue );
	GetQuery().Execute( _T("Select @@version") );
	if( GetQuery().Fetch() == S_OK )
	{
		_tcscpy( param.m_Description, _T("SQL Server Version") );
		ParamList.insert( param );	
	}

	// Output the XML or File
	if( m_OutputCDF )
	{
		OutputCDF( tstring("Description, Value\r\n") );
	}

	for( iter = ParamList.begin(); iter != ParamList.end(); iter++ )
	{
		if( m_OutputCDF )
		{
			// add line to CSV file
			sLine.Format( "\"%s\",\"%s\"\r\n", iter->m_Description, iter->m_DataValue );
			OutputCDF( sLine );
		}
		else
		{
			GetXMLGen().AddChildElem( _T("Row") );
			GetXMLGen().AddChildAttrib( _T("Description"), iter->m_Description );
			GetXMLGen().AddChildAttrib( _T("Value"), iter->m_DataValue );
		}
	}
}

/*-----------------------------------------------------------\\
|| Retrieves application version information from version
|| resource file
\*-----------------------------------------------------------*/
void CReportSysConfig::GetVersionInfo( CEMSString& sInstallPath, 
		 							   TCHAR* szBinary, 
									   CEMSString& sVersion )
{
	CEMSString sFile;

	if( sInstallPath.length() > 0 && sInstallPath.at( sInstallPath.length() - 1 ) != _T('\\') )
	{
		sInstallPath += _T("\\");
	}

	sFile.Format( _T("%sbin\\%s"), sInstallPath.c_str(), szBinary );

	DWORD	dwVersionHnd;

	// get the file version info size
	DWORD dwFileVersionInfoSize = GetFileVersionInfoSize( (LPTSTR) sFile.c_str(), &dwVersionHnd);

	// if we got a good size
	if (dwFileVersionInfoSize)
	{
		// allocate buffer to hold version info
		char* pVerBuff = new char[dwFileVersionInfoSize + 1];
		//ASSERT(pVerBuff);

		// get the file version info
		GetFileVersionInfo( (LPTSTR) sFile.c_str(), NULL, dwFileVersionInfoSize, (LPVOID)pVerBuff);

		// init receiver vars
		DWORD dwVersionLen = 0;
		LPSTR lpszVersion = NULL;

		// query for the version value
		VerQueryValue((LPVOID)pVerBuff, 
					  (LPSTR) _T("\\StringFileInfo\\040904b0\\FileVersion"), 
					  (LPVOID*)&lpszVersion, 
					  (UINT*)&dwVersionLen);

		// format the version string
		tstring::size_type pos;
		sVersion.assign( lpszVersion );

		pos = 0;
		while ( (pos = sVersion.find( _T(','), pos)) != tstring::npos )
		{
			sVersion.replace( pos, 1, 1, _T('.') );		
		}

		pos = 0;
		while ( (pos = sVersion.find( _T(' '), pos)) != tstring::npos )
		{
			sVersion.erase( pos, 1 );
			pos = 0;
		}

		// free the version buffer
		delete pVerBuff;
	} // if
	else
	{
		// if we get here, version info retrieval failed
		sVersion.assign( _T("N/A") );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Reformats a database DSN string
\*--------------------------------------------------------------------------*/
void CReportSysConfig::BlankOutDBPassword( LPTSTR szDSN )
{
	CEMSString sString( szDSN );
	size_t pos1, pos2;
	pos1 = sString.find( _T("PWD="), 0 );

	if( pos1 != CEMSString::npos ) 
	{
		pos1 += 4;	// advance past PWD=
		pos2 = sString.find( _T(';'), pos1 );

		sString.replace( pos1, pos2-pos1, _T("********") );
	}

	CEMSString::iterator pos;
	for ( pos = sString.begin(); pos != sString.end(); pos++ )
	{
		switch( *pos )
		{
			case _T(';'):
				pos = sString.insert( ++pos, _T(' ') );
				break;
		}
	}
	
	_tcsncpy( szDSN, sString.c_str(), SERVERPARAMETERS_DATAVALUE_LENGTH - 1 );
}

////////////////////////////////////////////////////////////////////////////////
// 
// OpenCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReportSysConfig::OpenCDF( CURLAction& action )
{
	TCHAR szTempPath[MAX_PATH];
	
	GetTempPath( MAX_PATH, szTempPath );
	GetTempFileName( szTempPath, _T("ems"), 0, m_szTempFile );
	
	m_hCDFFile = CreateFile( m_szTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
			                 FILE_ATTRIBUTE_NORMAL, NULL );

	if( m_hCDFFile == INVALID_HANDLE_VALUE )
		THROW_EMS_EXCEPTION( E_SystemError, _T("Error creating temp file") );
	
	action.SetSendTempFile( m_szTempFile, _T("Report.csv") );

	m_OutputCDF = true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// OutputCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReportSysConfig::OutputCDF( tstring& sLine )
{
	DWORD dwBytes;

	WriteFile( m_hCDFFile, sLine.c_str(), sLine.size(), &dwBytes, NULL );
}

////////////////////////////////////////////////////////////////////////////////
// 
// CloseCDF
// 
////////////////////////////////////////////////////////////////////////////////
void CReportSysConfig::CloseCDF(void)
{
	if( m_OutputCDF )
		CloseHandle( m_hCDFFile );
}
