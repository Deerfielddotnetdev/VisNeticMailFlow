#include "stdafx.h"
#include "NoteAttach.h"
#include "TicketNotes.h"
#include "ContactNotes.h"
#include "UploadMap.h"
#include "Ticket.h"
#include "AttachFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CNoteAttach::CNoteAttach(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nNoteID = 0;

	// handle exceptions in the local EMS file
	GetISAPIData().SetUseExceptionEMS(false);
}

CNoteAttach::~CNoteAttach()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point
\*--------------------------------------------------------------------------*/
int CNoteAttach::Run(CURLAction& action)
{
	GetISAPIData().GetURLLong( _T("noteid"), m_nNoteID );

	//get notetype 1=ticketnote 2=contactnote	
	GetISAPIData().GetURLLong( _T("notetype"), m_nNoteType );
	
	tstring sAction;
	GetISAPIData().GetFormString( _T("ACTION"), sAction, true );	
	
	try
	{
		if ( sAction.compare( _T("add") ) == 0 )
		{
			OutputDebugString("CNoteAttach::Run - Before AddAttachment\n");
			AddAttachment();
		}
		else if ( sAction.compare( _T("delete") ) == 0 )
		{
			OutputDebugString("CNoteAttach::Run - Before DeleteAttachment\n");
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
||  Comments:	Lists attachments for the note          
\*--------------------------------------------------------------------------*/
void CNoteAttach::ListAttachments( void )
{
	int nTicketBoxID;
	int nOwnerID;
	
	// check security
	if ( m_nNoteType == 1 )
	{
		
		GetQuery().Initialize();
		
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
		BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
		BINDPARAM_LONG( GetQuery(), m_nNoteID );
		
		GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets ")
				_T("WHERE TicketID = (SELECT TicketID FROM TicketNotes WHERE TicketNoteID=?)") );
		
		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
		
		CTicket Ticket(m_ISAPIData);
		Ticket.m_TicketBoxID = nTicketBoxID;
		Ticket.m_OwnerID = nOwnerID;
		Ticket.RequireRead(false);
	}
	else if ( m_nNoteType == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	}	
	
	// add the upload ID for use with the progress indicator
	GetXMLGen().AddChildElem( _T("Upload") );
	GetXMLGen().AddChildAttrib( _T("ID"), CUploadMap::GetInstance().GetUploadID() );
	
	// list the attachments
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	attachment.ListNoteAttachments( m_nNoteID, m_nNoteType, AttachmentList );

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		iter->GenerateXML();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds an attachment to an outbound message	              
\*--------------------------------------------------------------------------*/
void CNoteAttach::AddAttachment( void )
{
	OutputDebugString("CNoteAttach::AddAttachment - Entered\n");

	CAttachment attach(m_ISAPIData);
	tstring sOrigFilename;
	tstring sActualPath;
	tstring sFilename;
	tstring sType;
	tstring sSubType;
	tstring sVirusName;
	long nFileID = 0;			

	OutputDebugString("CNoteAttach::AddAttachment - download and attempt to save the attachment\n");
	// download and attempt to save the attachment
	int nResult = GetISAPIData().GetFormNoteFile( _T("UPLOADFILE"), sOrigFilename, 
		                                      sActualPath, sType, sSubType, attach.m_FileSize );

	if ( nResult == E_FormFieldNotFound )
	{
		return;
	}
	
	// if the file couldn't be saved because the disk
	// was full, send an alert and bail out
	if( nResult == E_DiskFull )
	{
		OutputDebugString("CNoteAttach::AddAttachment - file couldn't be saved disk was full\n");
		SendLowDiskSpaceAlert( sActualPath.c_str() );
		
		CEMSString sError;
		sError.Format( _T("Unable to save attachment (%s). The disk is full."), sOrigFilename.c_str() );
		THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
	}

	OutputDebugString("CNoteAttach::AddAttachment - get just the filename\n");
	// get just the filename from the path
	sFilename = sActualPath;
	TCHAR dir[_MAX_DIR];
	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	
	sFilename.resize(ATTACHMENTS_ATTACHMENTLOCATION_LENGTH-1, '\0');
	_tsplitpath(sFilename.c_str(), NULL, dir, file, ext);

	{
		tstring szFolder;
		if(!GetRegString(EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY,  tstring(_T("NoteAttachFolder")).c_str(), szFolder))
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
		OutputDebugString("CNoteAttach::AddAttachment - virus scan the uploaded file\n");
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

		OutputDebugString("CNoteAttach::AddAttachment - insert the record into the attachments table for the uploaded file\n");
		// insert the record into the attachments table for the uploaded file
		attach.m_IsInbound = 0;
		//attach.m_FileID = nFileID;
		_tcsncpy( attach.m_FileName, sOrigFilename.c_str(), 255);
		_tcsncpy( attach.m_AttachmentLocation, sFilename.c_str(), 255);
		_tcsncpy( attach.m_MediaType, sType.c_str(), 125);
		_tcsncpy( attach.m_MediaSubType, sSubType.c_str(), 125);
		_tcsncpy( attach.m_VirusName, sVirusName.c_str(), 125);
		_tcscpy( attach.m_ContentDisposition, _T("noteattachment") );
		attach.Insert( GetQuery() );

		OutputDebugString("CNoteAttach::AddAttachment - insert the attchment into the record into the NoteAttachments table\n");
		// insert the attchment into the record into the OutboundMessageAttachments table
		TNoteAttachments note_attach;
		note_attach.m_AttachmentID = attach.m_AttachmentID;
		note_attach.m_NoteID = m_nNoteID;
		note_attach.m_NoteTypeID = m_nNoteType;
		note_attach.Insert( GetQuery() );

		OutputDebugString("CNoteAttach::AddAttachment - update the progress indicator the upload is complete\n");
		// update the progress the indicator the upload is complete
		CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );
		
	}
	catch(...)
	{
		OutputDebugString("CNoteAttach::AddAttachment - something went wrong delete the attachment from disk\n");
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
||  Comments:	Remove an attachment from a note	              
\*--------------------------------------------------------------------------*/
void CNoteAttach::DeleteAttachment( void )
{
	CAttachment attach(m_ISAPIData);
	GetISAPIData().GetFormLong( _T("SelectID"), attach.m_AttachmentID );
	attach.Query();
	// get the TicketBoxID and OwnerID of the ticket for which
	// outbound message is a part
	int nTicketBoxID;
	int nOwnerID;

	// check security
	if ( m_nNoteType == 1 )
	{
		GetQuery().Initialize();		
		BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
		BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
		BINDPARAM_LONG( GetQuery(), m_nNoteID );
		
		GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets ")
				_T("WHERE TicketID = (SELECT TicketID FROM TicketNotes WHERE TicketNoteID=?)") );
		
		if ( GetQuery().Fetch() != S_OK )
			THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
		
		CTicket Ticket(m_ISAPIData);
		Ticket.m_TicketBoxID = nTicketBoxID;
		Ticket.m_OwnerID = nOwnerID;
		Ticket.RequireEdit(false);
	}
	else if ( m_nNoteType == 0 )
	{
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	}	
	
	
	// delete the attachment from the NoteAttachments table
	GetQuery().Reset();
	BINDPARAM_LONG( GetQuery(), m_nNoteID );
	BINDPARAM_LONG( GetQuery(), m_nNoteType );
	BINDPARAM_LONG( GetQuery(), attach.m_AttachmentID );
	GetQuery().Execute( _T("DELETE FROM NoteAttachments WHERE NoteID=? AND NoteTypeID=? AND AttachmentID=?") );
	
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
		attach.DeleteNoteAttachment();

	}
}
