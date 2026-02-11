/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MsgCompAttach.cpp,v 1.2.2.1.2.1 2006/07/18 12:55:03 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "MsgCompAttach.h"
#include "OutboundMessage.h"
#include "UploadMap.h"
#include "Ticket.h"
#include "AttachFns.h"
#include "RegistryFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CMsgCompAttach::CMsgCompAttach(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nMsgID = 0;

	// handle exceptions in the local EMS file
	GetISAPIData().SetUseExceptionEMS(false);
}

CMsgCompAttach::~CMsgCompAttach()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point
\*--------------------------------------------------------------------------*/
int CMsgCompAttach::Run(CURLAction& action)
{
	GetISAPIData().GetURLLong( _T("message"), m_nMsgID );
	
	tstring sAction;
	if(!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
		if(!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
			GetISAPIData().GetURLString( _T("Action"), sAction, true );
	
	try
	{
		tstring sTemp(sAction);
		if (sTemp.compare(_T("delete"))== 0)
		{
			OutputDebugString("CMsgCompAttach::Run - Before DeleteAttachment\n");
			DeleteAttachment();
		}
		if (sAction.compare(_T("add"))==0)
		{
			OutputDebugString("CMsgCompAttach::Run - Before AddAttachment\n");
			AddAttachment();
		}
	}
	catch(...)
	{
		// update the progress indicator that the upload is complete
		CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

		if(m_nMsgID > 0)
		{
			ListAttachments();
		}
		throw;
	}

	// update the progress indicator that the upload is complete
	CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

	if(m_nMsgID > 0)
	{
		ListAttachments();
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists attachments for the outbound message	              
\*--------------------------------------------------------------------------*/
void CMsgCompAttach::ListAttachments( void )
{
	int nTicketBoxID;
	int nOwnerID;
	
	// check security
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nTicketBoxID );
	BINDCOL_LONG_NOLEN( GetQuery(), nOwnerID );
	BINDPARAM_LONG( GetQuery(), m_nMsgID );
	
	GetQuery().Execute( _T("SELECT TicketBoxID, OwnerID FROM Tickets ")
		_T("WHERE TicketID = (SELECT TicketID FROM OutboundMessages WHERE OutboundMessageID=?)") );
	
	if ( GetQuery().Fetch() != S_OK )
		THROW_EMS_EXCEPTION( E_InvalidID, CEMSString( EMS_STRING_INVALID_ID ) );
	
	CTicket Ticket(m_ISAPIData);
	Ticket.m_TicketBoxID = nTicketBoxID;
	Ticket.m_OwnerID = nOwnerID;
	Ticket.RequireRead(false);
	
	// add the upload ID for use with the progress indicator
	GetXMLGen().AddChildElem( _T("Upload") );
	GetXMLGen().AddChildAttrib( _T("ID"), CUploadMap::GetInstance().GetUploadID() );
	
	// list the attachments
	CAttachment attachment(m_ISAPIData);
	list<CAttachment> AttachmentList;
	list<CAttachment>::iterator iter;

	attachment.ListOutboundMessageAttachments( m_nMsgID, AttachmentList );

	for ( iter = AttachmentList.begin(); iter != AttachmentList.end(); iter++ )
	{
		iter->GenerateXML();
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds an attachment to an outbound message	              
\*--------------------------------------------------------------------------*/
void CMsgCompAttach::AddAttachment( void )
{
	OutputDebugString("CMsgCompAttach::AddAttachment - Entered\n");

	CAttachment attach(m_ISAPIData);
	tstring sOrigFilename;
	tstring sActualPath;
	tstring sFilename;
	tstring sType;
	tstring sSubType;
	tstring sVirusName;
	int nResult;

	OutputDebugString("CMsgCompAttach::AddAttachment - upload and attempt to save the attachment\n");
	// upload and attempt to save the attachment
	if ( m_nMsgID > 0 )
	{
		nResult = GetISAPIData().GetFormFile( _T("UPLOADFILE"), sOrigFilename, 
												sActualPath, sType, sSubType, attach.m_FileSize );
	}
	else
	{
		nResult = GetISAPIData().GetFormFile( _T("UPLOADFILE"), sOrigFilename, 
												sActualPath, sType, sSubType, attach.m_FileSize, true );
	}

	if ( nResult == E_FormFieldNotFound )
	{
		return;
	}
	
	// if the file couldn't be saved because the disk
	// was full, send an alert and bail out
	if( nResult == E_DiskFull )
	{
		OutputDebugString("CMsgCompAttach::AddAttachment - file couldn't be saved disk was full\n");
		SendLowDiskSpaceAlert( sActualPath.c_str() );
		
		CEMSString sError;
		sError.Format( _T("Unable to save attachment (%s). The disk is full."), sOrigFilename.c_str() );
		THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
	}

	OutputDebugString("CMsgCompAttach::AddAttachment - get just the filename\n");
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
		OutputDebugString("CMsgCompAttach::AddAttachment - virus scan the uploaded file\n");
		// virus scan the uploaded file
		attach.m_VirusScanStateID = VirusScanFile( sActualPath.c_str(), sVirusName );

		// if the file was infected, throw an exception
		if ( EMS_IS_INFECTED( attach.m_VirusScanStateID ) )
		{
			CEMSString sMsg;
			sMsg.Format( EMS_STRING_ERROR_FILE_VIRUS, sOrigFilename.c_str(), sVirusName.c_str() );
			THROW_EMS_EXCEPTION_NOLOG( E_InfectedFile, sMsg.c_str() );	
		}

		OutputDebugString("CMsgCompAttach::AddAttachment - insert the record into the attachments table for the uploaded file\n");
		// insert the record into the attachments table for the uploaded file
		tstring sContentID;
		attach.m_IsInbound = 0;
		_tcsncpy( attach.m_FileName, sOrigFilename.c_str(), 255);
		_tcsncpy( attach.m_AttachmentLocation, sFilename.c_str(), 255);
		_tcsncpy( attach.m_MediaType, sType.c_str(), 125);
		_tcsncpy( attach.m_MediaSubType, sSubType.c_str(), 125);
		_tcsncpy( attach.m_VirusName, sVirusName.c_str(), 125);
		
		GetISAPIData().GetURLString( _T("CONTENTID"), sContentID, true );

		if (sContentID.length() > 0 )
		{
			_tcsncpy( attach.m_ContentID, sContentID.c_str(), 255);
			_tcscpy( attach.m_ContentDisposition, _T("inline") );
		}
		else
		{
			_tcscpy( attach.m_ContentDisposition, _T("attachment") );
		}
		attach.Insert( GetQuery() );

		if ( m_nMsgID > 0 )
		{
			OutputDebugString("CMsgCompAttach::AddAttachment - insert the attchment into the OutboundMessageAttachments table\n");
			// insert the attchment into the record into the OutboundMessageAttachments table
			TOutboundMessageAttachments outbound_msg_attach;
			outbound_msg_attach.m_AttachmentID = attach.m_AttachmentID;
			outbound_msg_attach.m_OutboundMessageID = m_nMsgID;
			outbound_msg_attach.Insert( GetQuery() );
		}

		OutputDebugString("CMsgCompAttach::AddAttachment - update the progress the indicator the upload is complete\n");
		// update the progress the indicator the upload is complete
		CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );
		
	}
	catch(...)
	{
		OutputDebugString("CMsgCompAttach::AddAttachment - something went wrong delete the attachment from disk\n");
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
void CMsgCompAttach::DeleteAttachment( void )
{
	int nTicketBoxID;
	int nOwnerID;
	int nAttachID;

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
	
	CEMSString sID;
	
	if (!GetISAPIData().GetURLString( _T("SelectID"), sID, true ))
	{
		if (!GetISAPIData().GetFormString( _T("SelectID"), sID, true ))
		{
			GetISAPIData().GetXMLString( _T("SelectID"), sID );				
		}
	}
	
	sID.CDLInit();
	while( sID.CDLGetNextInt( nAttachID ) )
	{
		CAttachment attach(m_ISAPIData);
		attach.m_AttachmentID = nAttachID;
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_nMsgID );
		BINDPARAM_LONG( GetQuery(), attach.m_AttachmentID );
		GetQuery().Execute( _T("DELETE FROM OutboundMessageAttachments WHERE OutboundMessageID=? AND AttachmentID=?") );
		
		if ( GetAttachmentReferenceCount( GetQuery(), attach.m_AttachmentID ) == 0 )
			attach.Delete();
	}
}
