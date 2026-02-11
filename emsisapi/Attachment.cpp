/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Attachment.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "Attachment.h"
#include "AttachFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction
\*--------------------------------------------------------------------------*/
CAttachment::CAttachment(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_FileExists = false;
}

CAttachment::~CAttachment()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Queries the attachment. Note that access control is 
||				not checked.     
\*--------------------------------------------------------------------------*/
void CAttachment::Query( void )
{
	tstring sFullPath;
	
	if ( !m_AttachmentID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An AttachmentID must be specified") );
	
	if ( TAttachments::Query( GetQuery() ) != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Unable to query attachment (%d)"), m_AttachmentID );			
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	m_IsInbound ? GetFullInboundAttachPath( m_AttachmentLocation, sFullPath ) :
	              GetFullOutboundAttachPath( m_AttachmentLocation, sFullPath );
	
	m_FileExists = VerifyFileExists( sFullPath.c_str() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	GenerateXML for the attachment	              
\*--------------------------------------------------------------------------*/
void CAttachment::GenerateXML( void )
{
	CEMSString sBytes;
	CEMSString sState;

	sBytes.FormatBytes( m_FileSize, 0 );

	if ( !EMS_IS_INFECTED( m_VirusScanStateID ) && !m_FileExists )
	{
		sState.assign( _T("File not found on server") );
	}
	else
	{
		//GetVirusScanStateIDText( m_VirusScanStateID, sState );
		sState.assign( _T("Ok") );
	}

	GetXMLGen().AddChildElem( _T("Attachment") );
	
	/*dca::String o;
	o.Format("CAttachment::GenerateXML - Adding XML for AttachmentID: %d FileName: %s", m_AttachmentID, m_FileName);
	DebugReporter::Instance().DisplayMessage(o.c_str(), DebugReporter::ISAPI, GetCurrentThreadId());*/
	
	GetXMLGen().SetChildAttrib( _T("ID"), m_AttachmentID );
	GetXMLGen().SetChildAttrib( _T("FileExists"), m_FileExists ? _T("1") : _T("0"));
	GetXMLGen().SetChildAttrib( _T("IsInfected"), EMS_IS_INFECTED( m_VirusScanStateID ) );
	GetXMLGen().SetChildAttrib( _T("VirusScanStateID"), m_VirusScanStateID  );
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("FileSize") );
	GetXMLGen().SetChildData( sBytes.c_str() );
	GetXMLGen().OutOfElem();
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("FileName") );
	GetXMLGen().SetChildData( m_FileName );
	GetXMLGen().OutOfElem();
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("VirusName") );
	GetXMLGen().SetChildData( m_VirusName );
	GetXMLGen().OutOfElem();
	
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("State") );
	GetXMLGen().SetChildData( sState.c_str() );
	GetXMLGen().OutOfElem();
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists all attachments for the inbound message	              
\*--------------------------------------------------------------------------*/
void CAttachment::ListInboundMessageAttachments( int nID, list<CAttachment>& AttachList )
{
	CEMSString sQuery;
	LPCSTR szColumns = NULL;
	tstring sFullPath;
	
	GetQuery().Initialize();
	
	BindColumns();
	BINDPARAM_LONG( GetQuery(), nID );
	
	GetColumnList( szColumns );
	
	sQuery.Format( _T("SELECT %s FROM Attachments as A ")
		           _T("INNER JOIN InboundMessageAttachments as I on A.AttachmentID = I.AttachmentID ")
		           _T("WHERE InboundMessageID=? ORDER BY FileName"), szColumns );
	
	GetQuery().Execute( sQuery.c_str() );
	
	
	while ( GetQuery().Fetch() == S_OK )
	{
		m_IsInbound ? GetFullInboundAttachPath( m_AttachmentLocation, sFullPath ) :
	                  GetFullOutboundAttachPath( m_AttachmentLocation, sFullPath );
	
		m_FileExists = VerifyFileExists( sFullPath.c_str() );
	
		AttachList.push_back( *this );
	}

}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists all attachments for the outbound message     
\*--------------------------------------------------------------------------*/
void CAttachment::ListOutboundMessageAttachments( int nID, list<CAttachment>& AttachList )
{
	CEMSString sQuery;
	LPCSTR szColumns = NULL;
	tstring sFullPath;
	
	GetQuery().Initialize();
	
	BindColumns();
	BINDPARAM_LONG( GetQuery(), nID );
	
	GetColumnList( szColumns );
	
	sQuery.Format( _T("SELECT %s FROM Attachments as A ")
		_T("INNER JOIN OutboundMessageAttachments as O on A.AttachmentID = O.AttachmentID ")
		_T("WHERE OutboundMessageID=? ORDER BY FileName"), szColumns );
	
	GetQuery().Execute( sQuery.c_str() );
	
	
	while ( GetQuery().Fetch() == S_OK )
	{
		m_IsInbound ? GetFullInboundAttachPath( m_AttachmentLocation, sFullPath ) :
	                  GetFullOutboundAttachPath( m_AttachmentLocation, sFullPath );
	
	    m_FileExists = VerifyFileExists( sFullPath.c_str() );
	
	    AttachList.push_back( *this );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists all attachments for the standard resposne	              
\*--------------------------------------------------------------------------*/
void CAttachment::ListStdResponseAttachments( int nID, list<CAttachment>& AttachList )
{
	CEMSString sQuery;
	LPCSTR szColumns = NULL;
	tstring sFullPath;
	
	GetQuery().Initialize();
	
	BindColumns();
	BINDPARAM_LONG( GetQuery(), nID );
	
	GetColumnList( szColumns );

	sQuery.Format( _T("SELECT %s FROM Attachments as A ")
		           _T("INNER JOIN StdResponseAttachments as S on A.AttachmentID = S.AttachmentID ")
		           _T("WHERE StandardResponseID=? ORDER BY FileName"), szColumns );
	
	GetQuery().Execute( sQuery.c_str() );
	
	
	while ( GetQuery().Fetch() == S_OK )
	{
		m_IsInbound ? GetFullInboundAttachPath( m_AttachmentLocation, sFullPath ) :
	                  GetFullOutboundAttachPath( m_AttachmentLocation, sFullPath );
	
		m_FileExists = VerifyFileExists( sFullPath.c_str() );
	
		AttachList.push_back( *this );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists all attachments for a note    
\*--------------------------------------------------------------------------*/
void CAttachment::ListNoteAttachments( int nID, int nTypeID, list<CAttachment>& AttachList )
{
	CEMSString sQuery;
	LPCSTR szColumns = NULL;
	tstring sFullPath;
	
	GetQuery().Initialize();
	
	BindColumns();
	BINDPARAM_LONG( GetQuery(), nID );
	BINDPARAM_LONG( GetQuery(), nTypeID );
	
	GetColumnList( szColumns );
	
	sQuery.Format( _T("SELECT %s FROM Attachments as A ")
		_T("INNER JOIN NoteAttachments as N on A.AttachmentID = N.AttachmentID ")
		_T("WHERE NoteID=? AND NoteTypeID=? ORDER BY FileName"), szColumns );
	
	GetQuery().Execute( sQuery.c_str() );
	
	
	while ( GetQuery().Fetch() == S_OK )
	{
		GetFullNoteAttachPath( m_AttachmentLocation, sFullPath );
	
	    m_FileExists = VerifyFileExists( sFullPath.c_str() );
	
	    AttachList.push_back( *this );
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Helper for list functions	              
\*--------------------------------------------------------------------------*/
void CAttachment::BindColumns( void )
{
	BINDCOL_LONG( GetQuery(), m_AttachmentID );
	BINDCOL_TCHAR( GetQuery(), m_AttachmentLocation );
	BINDCOL_TCHAR( GetQuery(), m_MediaType );
	BINDCOL_TCHAR( GetQuery(), m_MediaSubType );
	BINDCOL_TCHAR( GetQuery(), m_ContentDisposition );
	BINDCOL_TCHAR( GetQuery(), m_FileName );
	BINDCOL_LONG( GetQuery(), m_FileSize );
	BINDCOL_LONG( GetQuery(), m_VirusScanStateID );
	BINDCOL_TCHAR( GetQuery(), m_VirusName );
	BINDCOL_TCHAR( GetQuery(), m_ContentID );
	BINDCOL_BIT( GetQuery(), m_IsInbound );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Helper for list functions	              
\*--------------------------------------------------------------------------*/
void CAttachment::GetColumnList( LPCSTR& szColumns )
{
	szColumns = _T("A.AttachmentID,AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,")
		        _T("FileSize,VirusScanStateID,VirusName,ContentID,IsInbound ");
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes the attachment record from the database
||				and deletes the file from the file system	              
\*--------------------------------------------------------------------------*/
void CAttachment::Delete( void )
{
	tstring sFullAttachPath;

	if ( !m_AttachmentID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An AttachmentID must be specified") );

	// query the attachment
	TAttachments::Query( GetQuery() );
	
	// delete the attachment from the database
	TAttachments::Delete( GetQuery() );

	if ( m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
	{
		m_IsInbound ? GetFullInboundAttachPath( m_AttachmentLocation, sFullAttachPath ) :
					  GetFullOutboundAttachPath( m_AttachmentLocation, sFullAttachPath );
	}
	else
	{
		GetFullQuarantinePath( m_AttachmentLocation, sFullAttachPath );
	}
	
	DeleteFile( sFullAttachPath.c_str() );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes the note attachment record from the database
||				and deletes the file from the file system	              
\*--------------------------------------------------------------------------*/
void CAttachment::DeleteNoteAttachment( void )
{
	tstring sFullAttachPath;

	if ( !m_AttachmentID )
		THROW_EMS_EXCEPTION( E_InvalidID, _T("An AttachmentID must be specified") );

	// query the attachment
	TAttachments::Query( GetQuery() );
	
	// delete the attachment from the database
	TAttachments::Delete( GetQuery() );

	if ( m_VirusScanStateID != EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
	{
		GetFullNoteAttachPath( m_AttachmentLocation, sFullAttachPath );
	}
	else
	{
		GetFullQuarantinePath( m_AttachmentLocation, sFullAttachPath );
	}
	
	DeleteFile( sFullAttachPath.c_str() );
}