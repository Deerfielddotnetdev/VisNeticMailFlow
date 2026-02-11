// QuarantinePage.cpp: implementation of the CQuarantinePage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegistryFns.h"
#include "QuarantinePage.h"
#include "AttachFns.h"
#include "MessageIO.h"
#include "KAVScan.h"
#include ".\MailFlowServer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuarantinePage::CQuarantinePage(CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	m_nCurrentPage = 1;
}

CQuarantinePage::~CQuarantinePage()
{

}


////////////////////////////////////////////////////////////////////////////////
// 
// Run
// 
////////////////////////////////////////////////////////////////////////////////
int CQuarantinePage::Run( CURLAction& action )
{
	tstring sAction = _T("list");

	// Check security
	RequireAdmin();

	GetISAPIData().GetURLLong( _T("CurrentPage"), m_nCurrentPage, true );

	if( GetISAPIData().GetXMLPost() )
	{
		DISABLE_IN_DEMO();

		GetISAPIData().GetXMLString( _T("Action"), sAction, true );

		if( sAction.compare( _T("delete") ) == 0 )
		{
			Delete( action );
		}
		else if( sAction.compare( _T("deleteall") ) == 0 )
		{
			DeleteAll( action );
		}
		else if( sAction.compare( _T("repair") ) == 0 )
		{
			Repair( action );
		}
		else if( sAction.compare( _T("repairall") ) == 0 )
		{
			RepairAll( action );
		}
		else if( sAction.compare( _T("restore") ) == 0 )
		{
			Restore( action );
		}

		return 0;
	}

	ListAll();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// DoesFileExist
// 
////////////////////////////////////////////////////////////////////////////////
bool CQuarantinePage::DoesFileExist(tstring& sFileName)
{
	dca::FindFile ff(sFileName.c_str());

	if(!ff.First())
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////
// 
// ListAll
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::ListAll( void )
{
	Attachments_t attach;
	tstring sQuarantineFolder;
	CEMSString sMediaType;
	CEMSString sDateTime;
	int nPage = 1;
	int nLine = 0;
	tstring sFilePath;
	bool bFilePresent;

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), attach.m_AttachmentID );
	BINDCOL_TCHAR( GetQuery(), attach.m_FileName );
	BINDCOL_TCHAR( GetQuery(), attach.m_AttachmentLocation );
	BINDCOL_LONG( GetQuery(), attach.m_FileSize );
	BINDCOL_TCHAR( GetQuery(), attach.m_MediaType );
	BINDCOL_TCHAR( GetQuery(), attach.m_MediaSubType );
	BINDCOL_TCHAR( GetQuery(), attach.m_VirusName );
	BINDCOL_LONG( GetQuery(), attach.m_IsInbound );

	// This has to be the UGLIEST query ever! a three-way union.
	GetQuery().Execute( _T("SELECT AttachmentID,FileName,")
						_T("AttachmentLocation,FileSize,")
		                _T("MediaType,MediaSubType,")
						_T("VirusName,IsInbound ")
						_T("FROM Attachments ")
						_T("WHERE VirusScanStateID = 4 OR VirusScanStateID = 7 OR VirusScanStateID = 8")
						_T("ORDER BY FileName ") );

	GetXMLGen().AddChildElem( _T("Quarantine") );
	GetXMLGen().IntoElem();

	while( GetQuery().Fetch() == S_OK )
	{
		nLine++;

		if( nLine >= GetSession().m_nMaxRowsPerPage )
		{
			nLine = 0;
			nPage++;
		}

		if( nPage == m_nCurrentPage )
		{
			GetXMLGen().AddChildElem( _T("File") );
			GetXMLGen().AddChildAttrib( _T("ID"), attach.m_AttachmentID );
			GetXMLGen().AddChildAttrib( _T("Size"), attach.m_FileSize );

			GetFullQuarantinePath( attach.m_AttachmentLocation, sFilePath );

			bFilePresent = DoesFileExist( sFilePath );
		
			GetXMLGen().IntoElem();

			GetXMLGen().AddChildElem( _T("Name") );
			GetXMLGen().SetChildData( attach.m_FileName, 1 );

			GetXMLGen().AddChildElem( _T("Location") );
			GetXMLGen().SetChildData( bFilePresent ?  attach.m_AttachmentLocation : _T("file not found"), 1 );

			GetXMLGen().AddChildElem( _T("MediaType") );
			sMediaType.Format( _T("%s/%s"), attach.m_MediaType, attach.m_MediaSubType );
			GetXMLGen().SetChildData( sMediaType.c_str(), 1 );

			GetXMLGen().AddChildElem( _T("VirusName") );
			GetXMLGen().SetChildData( attach.m_VirusName, 1 );
			
			GetXMLGen().OutOfElem();
		}
	}

	GetXMLGen().OutOfElem();

	GetRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), sQuarantineFolder );

	GetXMLGen().AddChildElem( _T("QuarantineFolder") );
	GetXMLGen().SetChildData( sQuarantineFolder.c_str(), 1 );

	GetXMLGen().AddChildElem( _T("Page") );
	GetXMLGen().AddChildAttrib( _T("Current"), m_nCurrentPage );
	GetXMLGen().AddChildAttrib( _T("Count"), nPage );

}


////////////////////////////////////////////////////////////////////////////////
// 
// Delete
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::Delete( CURLAction& action )
{
	Attachments_t att;

	GetISAPIData().GetXMLLong( _T("selectId"), att.m_AttachmentID, false );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), att.m_AttachmentID );
	BINDCOL_TCHAR( GetQuery(), att.m_AttachmentLocation );
	BINDCOL_LONG( GetQuery(), att.m_VirusScanStateID );
	BINDCOL_BIT( GetQuery(), att.m_IsInbound );

	GetQuery().Execute( _T("SELECT AttachmentLocation,VirusScanStateID,IsInbound ")
						_T("FROM Attachments ")
						_T("WHERE AttachmentID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid AttachmentID %d\n"), att.m_AttachmentID );
		THROW_EMS_EXCEPTION(E_InvalidID, sError);
	}

	if( att.m_VirusScanStateID == 4 || att.m_VirusScanStateID == 7 || att.m_VirusScanStateID == 8 )
	{
		DeleteAttachment( att.m_AttachmentID, att.m_AttachmentLocation, att.m_IsInbound );
	}
	else
	{
		THROW_EMS_EXCEPTION(E_InvalidID, CEMSString( _T("Error - File no longer in quarantine\n")) );
	}

	
}



////////////////////////////////////////////////////////////////////////////////
// 
// DeleteAll
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::DeleteAll( CURLAction& action )
{
	list<CAnyAttachment> List;
	list<CAnyAttachment>::iterator iter;
	CAnyAttachment attach;

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), attach.m_AttachID );
	BINDCOL_TCHAR( GetQuery(), attach.m_AttachmentLocation );
	BINDCOL_LONG( GetQuery(), attach.m_VirusScanStateID );
	BINDCOL_BIT( GetQuery(), attach.m_IsInbound );

	GetQuery().Execute( _T("SELECT AttachmentID,AttachmentLocation,VirusScanStateID,IsInbound ")
					    _T("FROM Attachments ")
						_T("WHERE VirusScanStateID IN (4,7,8) ") );

	while ( GetQuery().Fetch() == S_OK )
	{
		List.push_back( attach );
	}

	for( iter = List.begin(); iter != List.end(); iter++ )
	{
		DeleteAttachment( iter->m_AttachID, iter->m_AttachmentLocation, iter->m_IsInbound );		
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// Repair
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::Repair( CURLAction& action )
{
	Attachments_t att;

	m_nFilesRepaired = 0;

	GetISAPIData().GetXMLLong( _T("selectId"), att.m_AttachmentID, false );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), att.m_AttachmentID );
	BINDCOL_TCHAR( GetQuery(), att.m_AttachmentLocation );
	BINDCOL_LONG( GetQuery(), att.m_VirusScanStateID );
	BINDCOL_BIT( GetQuery(), att.m_IsInbound );
	
	GetQuery().Execute( _T("SELECT AttachmentLocation,VirusScanStateID,IsInbound ")
						_T("FROM Attachments ")
						_T("WHERE AttachmentID=?") );

	if( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid AttachmentID %d\n"), att.m_AttachmentID );
		THROW_EMS_EXCEPTION(E_InvalidID, sError );
	}

	if( att.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
	{
		RepairAttachment( att.m_AttachmentID, att.m_AttachmentLocation, att.m_IsInbound );
	}
	else
	{
		THROW_EMS_EXCEPTION(E_InvalidID, CEMSString( _T("Error - File no longer in quarantine\n")) );
	}

	if( m_nFilesRepaired == 0)
	{
		GetXMLGen().AddChildElem( _T("postresults") );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("complete"), _T("false") );
		GetXMLGen().AddChildElem( _T("error"), _T("File could not be repaired") );
		GetXMLGen().OutOfElem();
	}

}

////////////////////////////////////////////////////////////////////////////////
// 
// RepairAll
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::RepairAll( CURLAction& action )
{
	list<CAnyAttachment> List;
	list<CAnyAttachment>::iterator iter;
	CAnyAttachment attach;

	m_nFilesRepaired = 0;

	GetQuery().Initialize();
	BINDCOL_LONG( GetQuery(), attach.m_AttachID );
	BINDCOL_TCHAR( GetQuery(), attach.m_AttachmentLocation );
	BINDCOL_LONG( GetQuery(), attach.m_VirusScanStateID );
	BINDCOL_BIT( GetQuery(), attach.m_IsInbound );

	GetQuery().Execute( _T("SELECT AttachmentID,AttachmentLocation,VirusScanStateID,IsInbound ")
					    _T("FROM Attachments ")
						_T("WHERE VirusScanStateID = 4 ") );

	while ( GetQuery().Fetch() == S_OK )
	{
		List.push_back( attach );
	}

	for( iter = List.begin(); iter != List.end(); iter++ )
	{
		if( iter->m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED )
		{
			RepairAttachment( iter->m_AttachID, iter->m_AttachmentLocation, iter->m_IsInbound );
		}
	}


	CEMSString sResults;
	sResults.Format( _T("Total files repaired: %d"), m_nFilesRepaired );

	GetXMLGen().AddChildElem( _T("postresults") );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("complete"), _T("true") );
	GetXMLGen().AddChildElem( _T("moreInfo"), sResults.c_str() );
	GetXMLGen().OutOfElem();
}

////////////////////////////////////////////////////////////////////////////////
// 
// RestoreAll
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::Restore( CURLAction& action )
{
	Attachments_t att;

	GetISAPIData().GetXMLLong( _T("selectId"), att.m_AttachmentID, false );

	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), att.m_AttachmentID );
	BINDCOL_TCHAR( GetQuery(), att.m_AttachmentLocation );
	BINDCOL_LONG( GetQuery(), att.m_VirusScanStateID );
	BINDCOL_BIT( GetQuery(), att.m_IsInbound );

	GetQuery().Execute( _T("SELECT AttachmentLocation,VirusScanStateID,IsInbound ")
						_T("FROM Attachments ")
						_T("WHERE AttachmentID=?") );
	
	if( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Error - Invalid AttachmentID %d\n"), att.m_AttachmentID );
		THROW_EMS_EXCEPTION(E_InvalidID, sError);
	}

	if( att.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED ||
		att.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_UNSCANNABLE || 
		att.m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_SUSPICIOUS)
	{
		RestoreAttachment( att.m_AttachmentID, att.m_AttachmentLocation, att.m_IsInbound );
	}
	else
	{
		THROW_EMS_EXCEPTION(E_InvalidID, CEMSString( _T("Error - File no longer in quarantine\n")) );
	}
}


////////////////////////////////////////////////////////////////////////////////
// 
// RestoreAttachment
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::RestoreAttachment( int nID, LPTSTR szLocation, 
										 unsigned char IsInbound, int nNewVirusScanState )
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	tstring sFileSource;
	tstring sFileDest;
	CMessageIO msgIO;

	GetFullQuarantinePath( szLocation, sFileSource );

	if(DoesFileExist(sFileSource))
	{
		sFileDest = szLocation;

		if(IsInbound)
		{
			if(!msgIO.GetInboundAttachPath(sFileDest, 1))
			{
				// TODO: throw exception
			}
		}
		else
		{
			if(!msgIO.GetOutboundAttachPath(sFileDest, 1))
			{
				// TODO: throw exception
			}
		}

		TCHAR dir[_MAX_DIR];
		TCHAR drive[_MAX_DRIVE];

		_tsplitpath(sFileDest.c_str(), drive, dir, NULL, NULL);
		MailFlowServer::EnsurePathExists(tstring(drive) + tstring(dir));

		MoveFile( sFileSource.c_str(), sFileDest.c_str() );

		// split path and update InboundMessageAttachments

		_tsplitpath( sFileDest.c_str(), szDrive, szDir, szFname, szExt );

		{
			tstring szFolder;
			if(IsInbound)
			{
				if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("InboundAttachFolder")).c_str(), szFolder))
				{
					// generate an absolute path using the provided filename
					if(szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
						szFolder += '\\';

					tstring sDir(szDir);
					szFolder = szFolder.erase(0,2);

					sFileDest = sDir.substr(szFolder.size());

					sFileDest += (tstring)szFname + (tstring)szExt;
				}
				else
					sFileDest = (tstring)szFname + (tstring)szExt;
			}
			else
			{
				if(!GetRegString(EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE,  tstring(_T("OutboundAttachFolder")).c_str(), szFolder))
				{
					// generate an absolute path using the provided filename
					if(szFolder.length() > 0 && szFolder.at(szFolder.size()-1) != '\\')
						szFolder += '\\';

					tstring sDir(szDir);
					szFolder = szFolder.erase(0,2);

					sFileDest = sDir.substr(szFolder.size());

					sFileDest += (tstring)szFname + (tstring)szExt;
				}
				else
					sFileDest = (tstring)szFname + (tstring)szExt;
			}
		}

		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), (TCHAR*) sFileDest.c_str() );
		BINDPARAM_LONG( GetQuery(), nNewVirusScanState );
		BINDPARAM_LONG( GetQuery(), nID );

		GetQuery().Execute( _T("UPDATE Attachments ")
							_T("SET AttachmentLocation=?,VirusScanStateID=? ")
							_T("WHERE AttachmentID=?") );
	}
	else
	{
		// Set the file as deleted in the database
		GetQuery().Initialize();
		BINDPARAM_LONG( GetQuery(), nID );
		GetQuery().Execute( _T("UPDATE Attachments ")
							_T("SET VirusScanStateID=3 ")
							_T("WHERE AttachmentID=?") );
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// DeleteAttachment
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::DeleteAttachment( int nID, LPTSTR szLocation, unsigned char IsInbound )
{
	tstring sFilePath;

	// File is in quarantine folder.
	GetFullQuarantinePath( szLocation, sFilePath );

	if( DoesFileExist( sFilePath ) )
	{
		// Delete the file from the filesystem
		DeleteFile( sFilePath.c_str() );
	}

	// Set the file as deleted in the database
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), nID );

	GetQuery().Execute( _T("UPDATE Attachments ")
						_T("SET VirusScanStateID=3 ")
						_T("WHERE AttachmentID=?") );
}


////////////////////////////////////////////////////////////////////////////////
// 
// RepairAttachment
// 
////////////////////////////////////////////////////////////////////////////////
void CQuarantinePage::RepairAttachment( int nID, LPTSTR szLocation, unsigned char IsInbound )
{
	tstring sVirusName;
	tstring sNewLoc;
	tstring sFilePath;
	long nError = 0;

	GetFullQuarantinePath( szLocation, sFilePath );

	if( DoesFileExist( sFilePath ) )
	{
		GetRoutingEngine().VirusScanFile( sFilePath.c_str(), 1, sVirusName, sNewLoc, &nError );

		if( nError == CKAVScan::Success || nError == CKAVScan::Success_Object_Has_Been_Cleaned )
		{
			m_nFilesRepaired++;
			RestoreAttachment( nID, szLocation, IsInbound, EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED );
		}
	}
}