/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Download.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||
||                                         
||  COMMENTS:	Downloads Message Attachments   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Download.h"
#include "RegistryFns.h"
#include "AttachFns.h"
#include "ArchiveFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CDownload::Run( CURLAction& action )
{

	int nInbound = 0;
	int nMsgID = 0;
	int nID = 0;
	tstring sFileName;
	tstring sContentID;
	int nArchiveID = 0;
	tstring sLogName;
	
	GetISAPIData().GetURLLong( _T("ARCHIVEID"), nArchiveID, true );
	
	if( nArchiveID != 0 )
	{
		return GetAttachFromArchive( nArchiveID, action );
	}

	if( GetISAPIData().GetURLString( _T("CID"), sContentID, true ) 
		&& GetISAPIData().GetURLLong( _T("MSGID"), nMsgID, true ) 
		&& GetISAPIData().GetURLLong( _T("INBOUND"), nInbound, true ) )
	{
		DownloadByContentID( sContentID, nMsgID, nInbound, action );
	}
	else if( GetISAPIData().GetURLString( _T("FILENAME"), sFileName, true ) )
	{
		DownloadByFilename( sFileName, action );
	}
	else if( GetISAPIData().GetURLString( _T("LOGNAME"), sLogName, true ) )
	{
		DownloadByLogName( sLogName, action );
	}
	else if( GetISAPIData().GetURLLong( _T("ID"), nID, true ) )
	{
		DownloadByAttachmentID( nID, action );
	}
	else
	{
		GetISAPIData().GetURLString( _T("CID"), sContentID );
		DownloadByCID( sContentID, action );
	}
		
	return 0;
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Downloads an inbound attachment	              
\*--------------------------------------------------------------------------*/
void CDownload::DownloadByContentID( tstring& sContentID, int nMsgID, 
									 int nInbound, CURLAction& action )
{

	int nAttachmentID=0;

	//DebugReporter::Instance().DisplayMessage("Entering CDownload::DownloadByContentID", DebugReporter::ISAPI, GetCurrentThreadId());

	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nAttachmentID );
	BINDPARAM_LONG( GetQuery(), nMsgID );
	BINDPARAM_TCHAR_STRING( GetQuery(), sContentID );
		
	if( nInbound )
	{
		GetQuery().Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
							_T("INNER JOIN InboundMessageAttachments ")
							_T("ON Attachments.AttachmentID = InboundMessageAttachments.AttachmentID " )
							_T("WHERE InboundMessageID=? AND ContentID=?" ));
	}
	else
	{
		GetQuery().Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
							_T("INNER JOIN OutboundMessageAttachments ")
							_T("ON Attachments.AttachmentID = OutboundMessageAttachments.AttachmentID " )
							_T("WHERE OutboundMessageID=? AND ContentID=?" ));
	}
	if ( GetQuery().Fetch() != S_OK )
	{
		//Try again without referring to a message
		GetQuery().Initialize();

		BINDCOL_LONG_NOLEN( GetQuery(), nAttachmentID );
		BINDPARAM_TCHAR_STRING( GetQuery(), sContentID );
		GetQuery().Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
							_T("WHERE ContentID=?" ));

		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Error - Invalid Attachment ContentID (%s)"), sContentID.c_str());
			THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, sError );			
		}		
	}	
	
	if(nAttachmentID > 0)
	{
		DownloadByAttachmentID( nAttachmentID, action );
	}

	//DebugReporter::Instance().DisplayMessage("Leaving CDownload::DownloadByContentID", DebugReporter::ISAPI, GetCurrentThreadId());

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Downloads an inbound attachment	              
\*--------------------------------------------------------------------------*/
void CDownload::DownloadByCID( tstring& sContentID, CURLAction& action )
{

	int nAttachmentID=0;

	//DebugReporter::Instance().DisplayMessage("Entering CDownload::DownloadByCID", DebugReporter::ISAPI, GetCurrentThreadId());

	GetQuery().Initialize();

	BINDCOL_LONG_NOLEN( GetQuery(), nAttachmentID );
	BINDPARAM_TCHAR_STRING( GetQuery(), sContentID );
	GetQuery().Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
						_T("WHERE ContentID=?" ));

	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid Attachment ContentID (%s)"), sContentID.c_str());
		THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, sError );
	}		
	
	if(nAttachmentID > 0)
	{
		DownloadByAttachmentID( nAttachmentID, action );
	}

	//DebugReporter::Instance().DisplayMessage("Leaving CDownload::DownloadByCID", DebugReporter::ISAPI, GetCurrentThreadId());

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Downloads an inbound attachment	              
\*--------------------------------------------------------------------------*/
void CDownload::DownloadByFilename( tstring& sFileName, CURLAction& action )
{

	//DebugReporter::Instance().DisplayMessage("Entering CDownload::DownloadByFilename", DebugReporter::ISAPI, GetCurrentThreadId());

	int nAttachmentID=0;

	GetQuery().Initialize();

	// if we were called with a filename, get the AttachmentID 
	// file names are only supported for inbound attachments
	BINDCOL_LONG_NOLEN( GetQuery(), nAttachmentID );
	BINDPARAM_TCHAR( GetQuery(), (void*) sFileName.c_str() );
	
	GetQuery().Execute( _T( "SELECT TOP 1 AttachmentID FROM Attachments " )
						_T( "WHERE AttachmentLocation=? AND IsInbound=1" ));
	
	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid Inbound Attachment Filename (%s)"), sFileName.c_str());
		THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, sError );
	}

	if(nAttachmentID > 0)
	{
		DownloadByAttachmentID( nAttachmentID, action );
	}

	//DebugReporter::Instance().DisplayMessage("Leaving CDownload::DownloadByFilename", DebugReporter::ISAPI, GetCurrentThreadId());

}


/*---------------------------------------------------------------------------\                     
||  Comments:	Downloads an inbound attachment	              
\*--------------------------------------------------------------------------*/
void CDownload::DownloadByAttachmentID( int nID, CURLAction& action )
{

	//DebugReporter::Instance().DisplayMessage("Entering CDownload::DownloadByAttachmentID", DebugReporter::ISAPI, GetCurrentThreadId());

	if(nID > 0)
	{
		tstring sAttachType;	
		GetISAPIData().GetURLString( _T("TYPE"), sAttachType, true );

		TAttachments attach;

		attach.m_AttachmentID = nID;

		// query for attachment information
		if ( attach.Query( GetQuery() ) != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Error - Invalid AttachmentID %d"), nID );
			THROW_EMS_EXCEPTION_NOLOG( E_InvalidID, sError );
		}

		// build the path to the file
		if ( sAttachType == "note" )
		{
			GetFullNoteAttachPath( attach.m_AttachmentLocation, action.m_PageName );
		}
		else
		{
			attach.m_IsInbound ? GetFullInboundAttachPath( attach.m_AttachmentLocation, action.m_PageName ) :
								GetFullOutboundAttachPath( attach.m_AttachmentLocation, action.m_PageName );
		}

		action.m_sFileName = attach.m_FileName;
		
		/*dca::String o;
		o.Format("CDownload::DownloadByAttachmentID - Full Path: %s", action.m_PageName.c_str());
		DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
		
		// Store the content-type in the m_sPageTitle
		if( attach.m_MediaType[0] )
		{
			action.m_sPageTitle.assign( attach.m_MediaType );
			action.m_sPageTitle.append( _T("/") );
			action.m_sPageTitle.append( attach.m_MediaSubType );
		}

		// if it is known the file has a virus, throw an exception
		// (Actually the UI should prevent this from happening...)
		/*if( EMS_IS_INFECTED(attach.m_VirusScanStateID) )
		{
			CEMSString sMsg;
			sMsg.Format( EMS_STRING_ERROR_FILE_VIRUS, action.m_PageName.c_str(), attach.m_VirusName );
			THROW_EMS_EXCEPTION_NOLOG( E_InfectedFile, sMsg.c_str() );		
		}*/

		// verify that the file exists
		if (!VerifyFileExists( action.m_PageName.c_str() ))
		{
			CEMSString sError;
			sError.Format( _T("Error - Unable to find attachment %s\n"), action.m_PageName.c_str() );
			THROW_EMS_EXCEPTION_NOLOG( E_AttachNotFound, sError );
		}

		// virus scan the file before downloading it
		//if ( attach.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_NOT_SCANNED )
			//VirusScanAttachment( attach, true );
	}

	//DebugReporter::Instance().DisplayMessage("Leaving CDownload::DownloadByAttachmentID", DebugReporter::ISAPI, GetCurrentThreadId());
}

int CDownload::GetAttachFromArchive( int nArchiveID, CURLAction& action )
{

	int nInbound = 0;
	int nMsgID = 0;
	int nID = 0;
	tstring sTempFile;
	tstring sFileName;
	tstring sContentID;
	tstring sMediaType;
	tstring sArchive;
	int retval;


	if( GetISAPIData().GetURLString( _T("CID"), sContentID, true ) 
		&& GetISAPIData().GetURLLong( _T("MSGID"), nMsgID, true ) 
		&& GetISAPIData().GetURLLong( _T("INBOUND"), nInbound, true ) )
	{
		retval = GetAttachmentFromArchive( GetQuery(), nArchiveID, 1, nID,
							                   nInbound, sContentID, sTempFile, sFileName, 
											   sMediaType, sArchive );
	}
	else if( GetISAPIData().GetURLString( _T("FILENAME"), sFileName, true ) )
	{
		retval = GetAttachmentFromArchive( GetQuery(), nArchiveID, 2, nID,
							                   nInbound, sContentID, sTempFile, sFileName, 
											   sMediaType, sArchive );
	}
	else
	{
		GetISAPIData().GetURLLong( _T("ID"), nID );

		retval = GetAttachmentFromArchive( GetQuery(), nArchiveID, 0, nID,
							                   nInbound, sContentID, sTempFile, sFileName, 
											   sMediaType, sArchive );
	}

	if( retval == Arc_Success )
	{
		action.SetSendTempFile( (LPTSTR) sTempFile.c_str(), (LPTSTR) sFileName.c_str() );
		action.m_sPageTitle = sMediaType;
	}
	else
	{
	}

	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Downloads a Log File	              
\*--------------------------------------------------------------------------*/
void CDownload::DownloadByLogName( tstring& sLogName, CURLAction& action )
{
	DebugReporter::Instance().DisplayMessage("Entering CDownload::DownloadByLogName", DebugReporter::ISAPI, GetCurrentThreadId());

	// Check security
	RequireAdmin();

	tstring sLogPath;

	if ( GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_LOG_PATH_VALUE, sLogPath ) != ERROR_SUCCESS)
	{
		CEMSString sError;
		sError.Format( _T("Error - Unable to get Log Path from registry\n"));
		THROW_EMS_EXCEPTION_NOLOG( E_EMSException, sError );
	}

	// Make sure we have a backslash at the end.
	if( sLogPath.length() > 0 && sLogPath.at( sLogPath.length() - 1 ) != _T('\\') )
	{
		sLogPath += _T("\\");
	}
	
	sLogPath += sLogName;

	// verify that the file exists
	if (!VerifyFileExists( sLogPath.c_str() ))
	{
		CEMSString sError;
		sError.Format( _T("Error - Unable to find Log File %s\n"), sLogPath.c_str() );
		THROW_EMS_EXCEPTION_NOLOG( E_EMSException, sError );
	}

	action.m_PageName.assign(sLogPath);
	action.m_sFileName.assign(sLogName);
	action.m_sPageTitle.assign( _T("text/plain") );

	DebugReporter::Instance().DisplayMessage("Leaving CDownload::DownloadByLogName", DebugReporter::ISAPI, GetCurrentThreadId());
}






