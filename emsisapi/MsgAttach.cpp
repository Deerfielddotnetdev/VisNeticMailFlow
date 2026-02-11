#include "stdafx.h"
#include "MsgAttach.h"
#include "OutboundMessage.h"
#include "UploadMap.h"
#include "Ticket.h"
#include "AttachFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CMsgAttach::CMsgAttach(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nMsgID = 0;

	// handle exceptions in the local EMS file
	GetISAPIData().SetUseExceptionEMS(false);
}

CMsgAttach::~CMsgAttach()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point
\*--------------------------------------------------------------------------*/
int CMsgAttach::Run(CURLAction& action)
{
	GetISAPIData().GetURLLong( _T("message"), m_nMsgID, true );
	GetISAPIData().GetURLLong( _T("isinbound"), m_nMsgType, true );
	GetISAPIData().GetURLLong( _T("noteid"), m_nNoteID, true );
	GetISAPIData().GetURLLong( _T("notetype"), m_nNoteType, true );
	
	tstring sAction;
	GetISAPIData().GetFormString( _T("ACTION"), sAction, true );	
	
	try
	{
		if ( sAction.compare( _T("add") ) == 0 )
		{
			OutputDebugString("CMsgAttach::Run - Before AddAttachment\n");
			AddAttachment();
		}
		else if ( sAction.compare( _T("delete") ) == 0 )
		{
			OutputDebugString("CMsgAttach::Run - Before DeleteAttachment\n");
			DeleteAttachment();
		}
	}
	catch(...)
	{
		// update the progress indicator that the upload is complete
		CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

		ListAttachments();
		throw;
	}

	// update the progress indicator that the upload is complete
	CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

	ListAttachments();
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists attachments for the outbound message	              
\*--------------------------------------------------------------------------*/
void CMsgAttach::ListAttachments( void )
{
	int nTicketBoxID;
	int nOwnerID;

	//DebugReporter::Instance().DisplayMessage("Entering CMsgAttach::ListAttachments", DebugReporter::ISAPI, GetCurrentThreadId());

	if ( m_nMsgID > 0 )
	{
		// check security
		GetQuery().Initialize();
		
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
		BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
		BINDPARAM_LONG( GetQuery(), m_nMsgID );
		
		if ( m_nMsgType == 0 )
		{
			GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets ")
				_T("WHERE TicketID = (SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?)") );
		}
		else
		{
			GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets ")
				_T("WHERE TicketID = (SELECT TicketID FROM InboundMessages WHERE InboundMessageID=?)") );
		}

		if ( GetQuery().Fetch() != S_OK )		
			if (!GetIsAdmin())
				THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
		
		
		CTicket Ticket(m_ISAPIData);
		Ticket.m_TicketBoxID = nTicketBoxID;
		Ticket.m_OwnerID = nOwnerID;
		Ticket.RequireRead(false);
	}
		
	// add the upload ID for use with the progress indicator
	GetXMLGen().AddChildElem( _T("Upload") );
	GetXMLGen().AddChildAttrib( _T("ID"), CUploadMap::GetInstance().GetUploadID() );
	
	// list the attachments
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	if ( m_nMsgID > 0 )
	{
		if ( m_nMsgType == 0 )
		{
			attachment.ListOutboundMessageAttachments( m_nMsgID, AttachmentList );
		}
		else
		{
			attachment.ListInboundMessageAttachments( m_nMsgID, AttachmentList );
		}
	}
	else if ( m_nNoteID > 0 )
	{
		if ( m_nNoteType == 1 || m_nNoteType == 2 )
		{
			attachment.ListNoteAttachments( m_nNoteID, m_nNoteType, AttachmentList );
		}
	}

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		iter->GenerateXML();
	}

	//DebugReporter::Instance().DisplayMessage("Leaving CMsgAttach::ListAttachments", DebugReporter::ISAPI, GetCurrentThreadId());
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds an attachment to an outbound message	              
\*--------------------------------------------------------------------------*/
void CMsgAttach::AddAttachment( void )
{
	OutputDebugString("CMsgAttach::AddAttachment - Entered\n");

	CAttachment attach(m_ISAPIData);
	tstring sOrigFilename;
	tstring sActualPath;
	tstring sFilename;
	tstring sType;
	tstring sSubType;
	tstring sVirusName;
	long nFileID = 0;			

	OutputDebugString("CMsgAttach::AddAttachment - download and attempt to save the attachment\n");
	// download and attempt to save the attachment
	int nResult = GetISAPIData().GetFormFile( _T("UPLOADFILE"), sOrigFilename, 
		                                      sActualPath, sType, sSubType, attach.m_FileSize );

	if ( nResult == E_FormFieldNotFound )
	{
		return;
	}
	
	// if the file couldn't be saved because the disk
	// was full, send an alert and bail out
	if( nResult == E_DiskFull )
	{
		OutputDebugString("CMsgAttach::AddAttachment - file couldn't be saved disk was full\n");
		SendLowDiskSpaceAlert( sActualPath.c_str() );
		
		CEMSString sError;
		sError.Format( _T("Unable to save attachment (%s). The disk is full."), sOrigFilename.c_str() );
		THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
	}

	OutputDebugString("CMsgAttach::AddAttachment - get just the filename\n");
	// get just the filename from the path
	sFilename = sActualPath;
	TCHAR dir[_MAX_DIR];
	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	
	sFilename.resize(ATTACHMENTS_ATTACHMENTLOCATION_LENGTH-1, '\0');
	_tsplitpath(sFilename.c_str(), NULL, dir, file, ext);

	{
		tstring szFolder;
		if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("OutboundAttachFolder")).c_str(), szFolder))
		{
					// generate an absolute path using the provided filename
			if(szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
				szFolder += '\\';

			//tstring sDir(dir);
			//szFolder = szFolder.erase(0,2);

			//sFilename = sDir.substr(szFolder.size());
			
			sFilename = sFilename.substr(szFolder.length(),sFilename.length());
			
			//sFilename += (tstring)file + (tstring)ext;
		}
		else
			sFilename = (tstring)file + (tstring)ext;
	}
	

	try
	{
		OutputDebugString("CMsgAttach::AddAttachment - virus scan the uploaded file\n");
		// virus scan the uploaded file
		attach.m_VirusScanStateID = VirusScanFile( sActualPath.c_str(), sVirusName );

		// if the file was infected, throw an exception
		if ( EMS_IS_INFECTED( attach.m_VirusScanStateID ) )
		{
			CEMSString sMsg;
			sMsg.Format( EMS_STRING_ERROR_FILE_VIRUS, sOrigFilename.c_str(), sVirusName.c_str() );
			THROW_EMS_EXCEPTION_NOLOG( E_InfectedFile, sMsg.c_str() );	
		}

		// if the option to save attachments to DB is set, save to the Files table
		/*dca::String sSaveFileToDB("0");
		GetServerParameter(EMS_SRVPARAM_SAVE_ATTACHMENTS_DATABASE, sSaveFileToDB, "0");
		if(sSaveFileToDB.ToInt() != 0)
		{
			CoInitialize(NULL);
			CRoutingEngine*	pRoutingEngine = new CRoutingEngine;
			try
			{
				BOOL bRet;
				wchar_t* pFullPath;
				pFullPath = new wchar_t[sActualPath.size()+1];
				MultiByteToWideChar( CP_ACP, 0, sActualPath.c_str(), sActualPath.size()+1, pFullPath, sActualPath.size()+1 );	
				BSTR FullPath = SysAllocString( pFullPath );
				delete[] pFullPath;
				wchar_t* pFileName;
				pFileName = new wchar_t[sFilename.size()+1];
				MultiByteToWideChar( CP_ACP, 0, sFilename.c_str(), sFilename.size()+1, pFileName, sFilename.size()+1 );	
				BSTR FileName = SysAllocString( pFileName );
				delete[] pFileName;
				CODBCConn dbconn;
				CEMSString sDsn;
				dbconn.GetDSN(sDsn);
				wchar_t* pDsn;
				pDsn = new wchar_t[sDsn.size()+1];
				MultiByteToWideChar( CP_ACP, 0, sDsn.c_str(), sDsn.size()+1, pDsn, sDsn.size()+1 );	
				BSTR Dsn = SysAllocString( pDsn );
				delete[] pDsn;

				bRet = pRoutingEngine->TestSMTPAuth( FullPath, 0, FileName, Dsn, -100, &nFileID );				
				delete pRoutingEngine;
				CoUninitialize();
				if (!bRet)
				{
					THROW_EMS_EXCEPTION( E_AttachNotSavedToDb, _T("Unable to save attachment to database.") );
				}
			}
			catch( ... )
			{
				delete pRoutingEngine;
				CoUninitialize();
				THROW_EMS_EXCEPTION( E_AttachNotSavedToDb, _T("Unable to save attachment to database.") );
			}	
		}*/

		OutputDebugString("CMsgAttach::AddAttachment - insert the record into the attachments table for the uploaded file\n");
		// insert the record into the attachments table for the uploaded file
		attach.m_IsInbound = 0;
		//attach.m_FileID = nFileID;
		_tcsncpy( attach.m_FileName, sOrigFilename.c_str(), 255);
		_tcsncpy( attach.m_AttachmentLocation, sFilename.c_str(), 255);
		_tcsncpy( attach.m_MediaType, sType.c_str(), 125);
		_tcsncpy( attach.m_MediaSubType, sSubType.c_str(), 125);
		_tcsncpy( attach.m_VirusName, sVirusName.c_str(), 125);
		_tcscpy( attach.m_ContentDisposition, _T("attachment") );
		attach.Insert( GetQuery() );

		OutputDebugString("CMsgAttach::AddAttachment - insert the attchment into the record into the OutboundMessageAttachments table\n");
		// insert the attchment into the record into the OutboundMessageAttachments table
		TOutboundMessageAttachments outbound_msg_attach;
		outbound_msg_attach.m_AttachmentID = attach.m_AttachmentID;
		outbound_msg_attach.m_OutboundMessageID = m_nMsgID;
		outbound_msg_attach.Insert( GetQuery() );

		OutputDebugString("CMsgAttach::AddAttachment - update the progress the indicator the upload is complete\n");
		// update the progress the indicator the upload is complete
		CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );
		
	}
	catch(...)
	{
		OutputDebugString("CMsgAttach::AddAttachment - something went wrond delete the attachment from disk\n");
		// if anything went wrong, delete the attachment which has already been saved to the disk
		if ( attach.m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
		{
			DeleteFile( sActualPath.c_str() );
		}
		else
		{
			tstring sFullPath;
			GetFullQuarantinePath( (char*) sFilename.c_str(), sFullPath );
			DeleteFile( sFullPath.c_str() );
		}
		
		throw;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Remove an attachment from an outbound message	              
\*--------------------------------------------------------------------------*/
void CMsgAttach::DeleteAttachment( void )
{
	CAttachment attach(m_ISAPIData);
	GetISAPIData().GetFormLong( _T("SelectID"), attach.m_AttachmentID );
	attach.Query();
	// get the TicketBoxID and OwnerID of the ticket for which
	// outbound message is a part
	int nTicketBoxID;
	int nOwnerID;

	GetQuery().Initialize();
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
	BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
	BINDPARAM_LONG( GetQuery(), m_nMsgID );
	GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets ")
						_T("WHERE TicketID=(SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?)") );

	if ( GetQuery().Fetch() != S_OK )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );

	// require edit rights to the ticket to remove an attachment
	CTicket Ticket(m_ISAPIData);
	Ticket.m_TicketBoxID = nTicketBoxID;
	Ticket.m_OwnerID = nOwnerID;
	Ticket.RequireEdit(false);
	
	// delete the attachment from the OutboundMessageAttachments table
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_nMsgID );
	BINDPARAM_LONG( GetQuery(), attach.m_AttachmentID );
	GetQuery().Execute( _T("DELETE FROM OutboundMessageAttachments WHERE OutboundMessageID=? AND AttachmentID=?") );
	
	// if the attachment is no longer referenced, delete it
	if ( GetAttachmentReferenceCount( GetQuery(), attach.m_AttachmentID ) == 0 )
	{
		// if the attachment was saved to the Files table, delete it
		/*if(attach.m_FileID != 0)
		{
			GetQuery().Reset();
			BINDPARAM_LONG( GetQuery(), attach.m_FileID );
			GetQuery().Execute( _T("DELETE FROM Files WHERE FileID=?") );
		}*/
		// finally delete the attachment
		attach.Delete();
	}
}
