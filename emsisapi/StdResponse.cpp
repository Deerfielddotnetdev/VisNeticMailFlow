   /***************************************************************************\
||             
||  $Header: /root/EMSISAPI/StdResponse.cpp,v 1.2.2.3 2006/02/23 20:37:12 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Standard Responses   
||              
\\*************************************************************************/

#include "stdafx.h"
#include "StdResponse.h"
#include "AttachFns.h"
#include "UploadMap.h"
#include ".\stdresponse.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CStdResponse::CStdResponse(CISAPIData& ISAPIData )
	:CXMLDataClass( ISAPIData )
	,m_lastCatID(0)
{
	m_access = EMS_NO_ACCESS;
	m_bDraftAccess = false;
	m_nUsageCount = 0;
	m_IsFavorite = 0;
	m_UpdateStdList = 0;
	m_FavoriteMod = 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Main Entry Point	              
\*--------------------------------------------------------------------------*/
int CStdResponse::Run(CURLAction &action)
{
	if( GetISAPIData().m_sPage.compare( _T("stdresponselist") ) == 0)
	{
		GetISAPIData().GetURLLong( _T("ID"), m_StandardResponse.m_StdResponseCatID );

		m_lastCatID = m_StandardResponse.m_StdResponseCatID;

		switch( m_StandardResponse.m_StdResponseCatID )
		{
		case EMS_STDRESP_FREQUENTLY_USED:
			ListMostFrequentUsed();
			break;
			
		case EMS_STDRESP_MY_FAVORITES:
			ListMyFavorites();
			break;

		case EMS_STDRESP_DRAFTS:
			ListDrafts();
			break;

		case EMS_STDRESP_SEARCH_RESULTS:
			{
				ListSearchResults();
			}
			break;
			
		default:
			RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, m_StandardResponse.m_StdResponseCatID, EMS_READ_ACCESS );
			ListStdResponses();
		}
	}
	else
	{	
		// insert a standard response directly into the message composition window
		// this method is used when a standard response is selected from the menu
		if ( GetISAPIData().GetURLLong( _T("InsertResponse"), m_StandardResponse.m_StandardResponseID, true ) )
		{
			Query( action );
			UsedResponse();
			return 0;
		}
		
		// this method is used to mark a standard response as having been used
		// after it is inserted from the standard response browser
		if ( GetISAPIData().GetURLLong( _T("UsedResponse"), m_StandardResponse.m_StandardResponseID, true ) )
		{
			// UsedResponse(); // was causing 2 entries to be put in std response
			return 0;
		}
		
		// get the ID as a URL or form parameter
		if (!GetISAPIData().GetURLLong( _T("ID"), m_StandardResponse.m_StandardResponseID, true ) )
			GetISAPIData().GetFormLong( _T("ID"), m_StandardResponse.m_StandardResponseID, true );
		
		GetISAPIData().GetURLLong( _T("CATEGORYID"), m_StandardResponse.m_StdResponseCatID, true );

		m_lastCatID = m_StandardResponse.m_StdResponseCatID;
		
		// get action as a URL or form parameter
		tstring sAction;
		
		if (( GetISAPIData().GetFormString( _T("ACTION"), sAction, true ) ) ||
			( GetISAPIData().GetURLString( _T("ACTION"), sAction, true ) ))
		{
			if ( sAction.compare( _T("update") ) == 0 )
			{
				if (m_StandardResponse.m_StandardResponseID == 0)
				{
					Insert( action );
				}
				else
				{
					Update( action );
				}
			}
			else if ( sAction.compare( _T("delete") ) == 0 )
			{
				Delete( action );
			}
			else if ( sAction.compare( _T("addfavorite") ) == 0 )
			{
				AddFavorite( action );
			}
			else if ( sAction.compare( _T("removefavorite") ) == 0 )
			{
				RemoveFavorite( action );
			}
		}
		
		// generate the XML
		Query( action );			
	}
	
	return 0;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate the XML for a standard response	              
\*--------------------------------------------------------------------------*/
void CStdResponse::GenerateXML( CURLAction& action )
{
	CEMSString sString;

	GetXMLGen().AddChildElem( _T("StdResponse") );
	GetXMLGen().AddChildAttrib( _T("UpdateStdList"), m_UpdateStdList );
	GetXMLGen().AddChildAttrib( _T("FavoriteMod"), m_FavoriteMod );
	GetXMLGen().AddChildAttrib( _T("Access"), m_access );
	GetXMLGen().AddChildAttrib( _T("DraftAccess"), m_bDraftAccess );
	GetXMLGen().AddChildAttrib( _T("ID"), m_StandardResponse.m_StandardResponseID );
	GetXMLGen().AddChildAttrib( _T("IsDeleted"), m_StandardResponse.m_IsDeleted );
	GetXMLGen().AddChildAttrib( _T("IsFavorite"), m_IsFavorite );
	if(m_lastCatID == EMS_STDRESP_SEARCH_RESULTS)
		GetXMLGen().AddChildAttrib( _T("CategoryID"), m_lastCatID);
	else
		GetXMLGen().AddChildAttrib( _T("CategoryID"), m_StandardResponse.m_StdResponseCatID );

	if ( m_StandardResponse.m_StandardResponseID != 0 ) 
	{
		CEMSString sDateTime;	
		GetDateTimeString( m_StandardResponse.m_DateCreated, m_StandardResponse.m_DateCreatedLen, sDateTime );
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateTime.c_str() );
		
		GetDateTimeString( m_StandardResponse.m_DateModified, m_StandardResponse.m_DateModifiedLen, sDateTime );
		GetXMLGen().AddChildAttrib( _T("LastModified"), sDateTime.c_str() );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("DateCreated"), _T("") );
		GetXMLGen().AddChildAttrib( _T("LastModified"), _T("") );
	}
	
	AddAgentName( _T("Author"), m_StandardResponse.m_AgentID, _T("Not Available") );
	AddAgentName( _T("ModifiedBy"), m_StandardResponse.m_ModifiedBy, _T("Not Available") );

	// subject
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Subject") );
	sString.assign( m_StandardResponse.m_Subject );
	sString.EscapeHTML();
	GetXMLGen().SetChildData( sString.c_str(), TRUE );
	GetXMLGen().OutOfElem();

	// subject (raw)
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("SubjectRaw") );
	sString.assign( m_StandardResponse.m_Subject );
	sString.EscapeJavascript();
	GetXMLGen().SetChildData( sString.c_str(), TRUE );
	GetXMLGen().OutOfElem();

	// body
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Body") );
	if( _tcsicmp( action.m_PageName.c_str(), _T("editstdresponse.ems") ) == 0 )
	{
		tstring sUserAgent(GetBrowserSession().m_UserAgent);
		dca::String sTemp = sUserAgent.c_str();
		dca::String sTemp2 = sTemp.substr(0,17);
		if ( sTemp2 == "Internet.Explorer" )
		{
			bool bIsOldSR = true;
			tstring sResponse;
			tstring::size_type posTagOpen;
			tstring::size_type posTagClose;
			sResponse = m_StandardResponse.m_StandardResponse;
			posTagOpen = sResponse.find(_T("<"));
			posTagClose = sResponse.find(_T(">"));
			if( posTagOpen != tstring::npos )
			{
				if ( posTagClose != tstring::npos && posTagClose > posTagOpen )
					bIsOldSR = false;
			}
			if ( bIsOldSR )
			{
				sString.assign( m_StandardResponse.m_StandardResponse );
				sString.EscapeHTMLAndNewLines();
				GetXMLGen().SetChildData( sString.c_str(), TRUE );			
			}
			else
			{
				GetXMLGen().SetChildData( m_StandardResponse.m_StandardResponse, TRUE );
			}
		}
		else
		{
			GetXMLGen().SetChildData( m_StandardResponse.m_StandardResponse, TRUE );
		}
	}
	else
	{
		sString.assign( m_StandardResponse.m_StandardResponse );
		sString.EscapeHTMLAndNewLines();
		GetXMLGen().SetChildData( sString.c_str(), TRUE );
	}
	GetXMLGen().OutOfElem();
	
	// note
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("Note") );
	if( _tcsicmp( action.m_PageName.c_str(), _T("editstdresponse.ems") ) == 0 )
	{
		GetXMLGen().SetChildData( m_StandardResponse.m_Note, TRUE );
	}
	else
	{
		sString.assign( m_StandardResponse.m_Note );
		sString.EscapeBody( 0, m_StandardResponse.m_StandardResponseID, 0 );
		GetXMLGen().SetChildData( sString.c_str(), TRUE );
	}
	GetXMLGen().OutOfElem();

	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("keyword"));
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem(_T("usekeywords"),m_StandardResponse.m_UseKeywords);
	if(m_StandardResponse.m_UseKeywords)
	{
		GetXMLGen().AddChildElem(_T("keywords"),(lstrlen(m_StandardResponse.m_Keywords))?m_StandardResponse.m_Keywords:"");
	}
	else
	GetXMLGen().AddChildElem(_T("keywords"),"");
	GetXMLGen().OutOfElem();
	GetXMLGen().OutOfElem();
	
	// list the attachments
	list<CAttachment>::iterator iter;

	for ( iter = m_AttachList.begin(); iter != m_AttachList.end(); iter++ )
	{
		iter->GenerateXML();
	}
	
	if( _tcsicmp( GetISAPIData().m_sPage.c_str(), _T("editstdresponse") ) == 0 )
	{
		// add the upload id
		GetXMLGen().AddChildElem( _T("Upload") );
		GetXMLGen().AddChildAttrib( _T("ID"), CUploadMap::GetInstance().GetUploadID() );
		
		// list the categories
		ListStdRespCategories(GetAgentID(),EMS_EDIT_ACCESS,GetXMLGen());
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Reads Form Parameters
\*--------------------------------------------------------------------------*/
void CStdResponse::DecodeForm()
{
	GetISAPIData().GetFormLong( _T("CATEGORYID"), m_StandardResponse.m_StdResponseCatID );
	
	GetISAPIData().GetFormTCHAR( _T("SUBJECT"), m_StandardResponse.m_Subject, 100 );
	
	GetISAPIData().GetFormTCHAR( _T("BODY"), &m_StandardResponse.m_StandardResponse, 
		m_StandardResponse.m_StandardResponseLen,
								m_StandardResponse.m_StandardResponseAllocated );
	
	GetISAPIData().GetFormTCHAR( _T("NOTES"), &m_StandardResponse.m_Note, 
		m_StandardResponse.m_NoteLen, m_StandardResponse.m_NoteAllocated );

	GetISAPIData().GetFormBit(_T("chkKeywords"), m_StandardResponse.m_UseKeywords);
	GetISAPIData().GetFormTCHAR(_T("keywords"), m_StandardResponse.m_Keywords, 255, true);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Processes attachments	              
\*--------------------------------------------------------------------------*/
void CStdResponse::ProcessAttachments()
{
	TAttachments attach;
	tstring sOrigFilename;
	tstring sActualPath;
	tstring sFilename;
	tstring sType;
	tstring sSubType;
	tstring sVirusName;
	int nResult;
	
	// iterate the form parameters map for files...
	map<tstring,ISAPIPARAM>::iterator iter;
	
	// loop through all the form fields looking for files
	for ( iter = m_ISAPIData.m_Form.begin(); iter != m_ISAPIData.m_Form.end(); iter++ )
	{
		if ( !iter->second.m_bIsFile )
			continue;
		
		nResult = GetISAPIData().GetFormFile( iter->first.c_str(), sOrigFilename, sActualPath, 
			                                  sType, sSubType, attach.m_FileSize );
		
		// just skip it if the form parameter wasn't found
		if ( nResult == E_FormFieldNotFound )
			continue;

		if( nResult == E_DiskFull )
		{
			SendLowDiskSpaceAlert( sActualPath.c_str() );
			
			CEMSString sError;
			sError.Format( _T("Unable to save attachment (%s). The disk is full."), sOrigFilename.c_str() );
			THROW_EMS_EXCEPTION( E_WritingAttachment, sError );
		}
			
		// Fix for bugzilla bug #46
		{
			dca::String sFullPath;
			GetOutboundAttachPath(sFullPath);

			sFilename = sActualPath;
			sFilename = sFilename.erase(0, sFullPath.size());
		}

		try
		{
			// virus scan the uploaded file
			attach.m_VirusScanStateID = VirusScanFile( sActualPath.c_str(), sVirusName );
			
			if ( EMS_IS_INFECTED( attach.m_VirusScanStateID ) )
			{
				CEMSString sMsg;
				sMsg.Format( EMS_STRING_ERROR_FILE_VIRUS, sOrigFilename.c_str(), sVirusName.c_str() );
				THROW_EMS_EXCEPTION_NOLOG( E_InfectedFile, sMsg.c_str() );	
			}

			// insert the attachment into the attachments table
			attach.m_IsInbound = 0;
			_tcsncpy( attach.m_FileName, sOrigFilename.c_str(), 255);
			_tcsncpy( attach.m_AttachmentLocation, sFilename.c_str(), 255);
			_tcsncpy( attach.m_MediaType, sType.c_str(), 125);
			_tcsncpy( attach.m_MediaSubType, sSubType.c_str(), 125);
			_tcsncpy( attach.m_VirusName, sVirusName.c_str(), 125);
			_tcscpy( attach.m_ContentDisposition, _T("attachment") );
			attach.Insert( GetQuery() );

			// insert the record in the StdRespAttachments table
			TStdResponseAttachments stdresp_attach;
			stdresp_attach.m_AttachmentID = GetQuery().GetLastInsertedID(); 
			stdresp_attach.m_StandardResponseID = m_StandardResponse.m_StandardResponseID;
			stdresp_attach.Insert( GetQuery() );
		}
		catch(...)
		{
			// update the progress the indicate the upload is complete
			CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );

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

	// update the progress the indicate the upload is complete
	CUploadMap::GetInstance().UpdateProgress( GetISAPIData().GetUploadID(), -1 );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Query a standard response	              
\*--------------------------------------------------------------------------*/
void CStdResponse::Query( CURLAction& action )
{
	// get the category ID
	GetISAPIData().GetURLLong( _T("category"), m_StandardResponse.m_StdResponseCatID, true );
	
	if(m_StandardResponse.m_StdResponseCatID != 0 && m_StandardResponse.m_StdResponseCatID != -3)
	{
		switch( m_StandardResponse.m_StdResponseCatID )
		{
		case EMS_STDRESP_FREQUENTLY_USED:
			break;		
		case EMS_STDRESP_MY_FAVORITES:
			break;
		case EMS_STDRESP_DRAFTS:
			m_bDraftAccess = true;
			break;
		case EMS_STDRESP_SEARCH_RESULTS:
			{
				RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, m_StandardResponse.m_StdResponseCatID, EMS_READ_ACCESS );
			}
			break;		
		default:
			RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, m_StandardResponse.m_StdResponseCatID, EMS_READ_ACCESS );
		}
	}
	
	if ( m_StandardResponse.m_StandardResponseID != 0 )
	{
		// query the exsisting standard response
		if ( m_StandardResponse.Query( GetQuery() ) != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid StandardResponseID (%d)"), m_StandardResponse.m_StandardResponseID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError );
		}
		
		if(m_StandardResponse.m_StdResponseCatID == 1)
		{
			m_access = 3;
		}
		else
		{
			m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, m_StandardResponse.m_StdResponseCatID, EMS_READ_ACCESS );
		}

		// is this response in "my favorites"
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
		BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
		
		GetQuery().Execute(	_T("SELECT TOP 1 StdResponseFavoritesID FROM StdResponseFavorites ")
			_T("WHERE StandardResponseID = ? AND AgentID = ? ") );
		
		m_IsFavorite = GetQuery().Fetch() == S_OK;
		
		// query for attachments
		if ( m_AttachList.size() == 0 )
		{
			CAttachment attachment(m_ISAPIData);
			attachment.ListStdResponseAttachments( m_StandardResponse.m_StandardResponseID, m_AttachList );
		}
	}
	else
	{
		m_access = 3;
	}
	
	// generate the XML
	GenerateXML( action );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Updates or Inserts a standard response	              
\*--------------------------------------------------------------------------*/
void CStdResponse::Update( CURLAction &action )
{
	// decode the form
	DecodeForm();
	
	// check security
	RequireAccessLevel( EMS_EDIT_ACCESS );
	
	// set last modified info
	m_StandardResponse.m_ModifiedBy = GetAgentID();
	GetTimeStamp( m_StandardResponse.m_DateModified );
	m_StandardResponse.m_DateModifiedLen = sizeof(TIMESTAMP_STRUCT);
	
	GetQuery().Initialize();
	
	BINDPARAM_TCHAR( GetQuery(), m_StandardResponse.m_Subject );
	BINDPARAM_LONG(  GetQuery(), m_StandardResponse.m_StdResponseCatID );
	BINDPARAM_TEXT(  GetQuery(), m_StandardResponse.m_StandardResponse );
	BINDPARAM_TEXT(  GetQuery(), m_StandardResponse.m_Note );
	BINDPARAM_TIME(	 GetQuery(), m_StandardResponse.m_DateModified );
	BINDPARAM_LONG(	 GetQuery(), m_StandardResponse.m_ModifiedBy );

	if( m_StandardResponse.m_UseKeywords)
	{
		BINDPARAM_BIT( GetQuery(), m_StandardResponse.m_UseKeywords );
		BINDPARAM_TCHAR( GetQuery(), m_StandardResponse.m_Keywords );
	}
	
	BINDPARAM_LONG(  GetQuery(), m_StandardResponse.m_StandardResponseID );
	
	if( m_StandardResponse.m_UseKeywords)
	{
		GetQuery().Execute( _T("UPDATE StandardResponses ")
			_T("SET Subject=?,StdResponseCatID=?,StandardResponse=?,Note=?, ")
			_T("DateModified=?, ModifiedBy=?, UseKeywords=?, Keywords=? ")
			_T("WHERE StandardResponseID=?") );
	}
	else
	{
		GetQuery().Execute( _T("UPDATE StandardResponses ")
			_T("SET Subject=?,StdResponseCatID=?,StandardResponse=?,Note=?, ")
			_T("DateModified=?, ModifiedBy=? ")
			_T("WHERE StandardResponseID=?") );
	}
	
	if (GetQuery().GetRowCount() != 1)
	{
		CEMSString sError;
		sError.Format( _T("Invalid StandardResponseID (%d)"), m_StandardResponse.m_StandardResponseID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
	
	// delete any attachments that we need to
	RemoveAttachments();
	
	// process any attachments
	ProcessAttachments();
	
	// set the ems file back to the one which
	// is used to view the standard response
	action.m_PageName = _T("stdresponse.ems");
	
	// the list needs to be refreshed
	m_UpdateStdList = TRUE;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes standard responses which were removed by the agent 
||				during an update.			              
\*--------------------------------------------------------------------------*/
void CStdResponse::RemoveAttachments( void )
{
	CAttachment attachment(m_ISAPIData);
	set<int> NewAttachList;
	CEMSString sList;
	int nID;

	// get the list of attachments that are still associated with the standard response...
	GetISAPIData().GetFormString( _T("SAVEDATTACHMENT"), sList );

	while ( sList.CDLGetNextInt( nID ) )
	{
		NewAttachList.insert( nID );
	}
	
	// get the list of attachments which are in the database
	// for the standard response	
	attachment.ListStdResponseAttachments( m_StandardResponse.m_StandardResponseID, m_AttachList );
	list<CAttachment>::iterator iter = m_AttachList.begin();
	
	while ( iter != m_AttachList.end() )
	{
		// the attachment is in the database, however not in 
		// the list that was posted... delete it.
		if ( NewAttachList.find( iter->m_AttachmentID ) == NewAttachList.end() )
		{
			// delete the StandardResponseAttachments record
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
			BINDPARAM_LONG( GetQuery(), iter->m_AttachmentID );
			GetQuery().Execute( _T("DELETE FROM StdResponseAttachments ")
				                _T("WHERE StandardResponseID=? AND AttachmentID=?") );

			// if the attachment is no longer referenced, delete it from the disk
			// and remove the record in the Attachments table
			if ( GetAttachmentReferenceCount( GetQuery(), iter->m_AttachmentID ) == 0 )
				iter->Delete();

			// remove the attachment from our list
			iter = m_AttachList.erase( iter );
		}
		else
		{
			iter++;
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Inserts a new standard response	              
\*--------------------------------------------------------------------------*/
void CStdResponse::Insert( CURLAction &action )
{
	// decode the form
	DecodeForm();
	
	// check security
	RequireAccessLevel( EMS_EDIT_ACCESS );
	
	// populate standard response data
	m_StandardResponse.m_ModifiedBy = GetAgentID();
	GetTimeStamp( m_StandardResponse.m_DateModified );
	m_StandardResponse.m_DateModifiedLen = sizeof(TIMESTAMP_STRUCT);
	
	m_StandardResponse.m_DateCreated = m_StandardResponse.m_DateModified;
	m_StandardResponse.m_DateCreatedLen = sizeof(TIMESTAMP_STRUCT);
	m_StandardResponse.m_AgentID = GetAgentID();
	m_StandardResponse.m_IsApproved = TRUE;
	m_StandardResponse.m_IsDeleted = FALSE;
	
	m_StandardResponse.Insert( GetQuery() );
	
	ProcessAttachments();

	// generate the XML
	m_UpdateStdList = TRUE;
//	GenerateXML( action );
	
	// set the ems file back to the one which
	// is used to view the standard response
	action.m_PageName = _T("stdresponse.ems");
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Deletes a standard response	              
\*--------------------------------------------------------------------------*/
void CStdResponse::Delete( CURLAction &action )
{
	
	// check security
	RequireAccessLevel( EMS_DELETE_ACCESS );
	
	TIMESTAMP_STRUCT time;
	GetTimeStamp(time);
	
	GetQuery().Initialize();
	
	BINDPARAM_TIME_NOLEN( GetQuery(), time  );
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	
	GetQuery().Execute( _T("UPDATE StandardResponses ")
		_T("SET DeletedTime=?, DeletedBy=?, IsDeleted=1 ")
		_T("WHERE StandardResponseID=?") );
	
	if ( GetQuery().GetRowCount() != 1 )
	{
		CEMSString sError;
		sError.Format( _T("Invalid StandardResponseID (%d)"), m_StandardResponse.m_StandardResponseID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
	
	// Make sure this standard response is not used in any routing rule auto replies
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	GetQuery().Execute( _T("UPDATE RoutingRules SET AutoReplyEnable=0,AutoReplyWithStdResponse=0 ")
					    _T("WHERE AutoReplyWithStdResponse=? ") );

	if( GetQuery().GetRowCount() > 0 )
	{
		GetRoutingEngine().ReloadConfig( EMS_RoutingRules );
	}

	// Make sure this standard response is not used in any ticketobx auto replies
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	GetQuery().Execute( _T("UPDATE TicketBoxes SET AutoReplyEnable=0,AutoReplyWithStdResponse=0 ")
					    _T("WHERE AutoReplyWithStdResponse=? ") );

	// set ID to the first one in the category
	m_StandardResponse.m_StandardResponseID = GetFirstIDInCategory();
	
	// set the ems file back to the one which
	// is used to view the standard response
	m_UpdateStdList = TRUE;
	action.m_PageName = _T("stdresponse.ems");
}


/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the standard responses in a category...          
\*--------------------------------------------------------------------------*/
void CStdResponse::ListStdResponses()
{	
	// check security
	m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, m_StandardResponse.m_StdResponseCatID, EMS_READ_ACCESS );
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_StandardResponse.m_StandardResponseID);
	BINDCOL_TCHAR_NOLEN( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );
	
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StdResponseCatID );
	
	GetQuery().Execute( _T("SELECT StandardResponseID, Subject, DateModified ")
		_T("FROM StandardResponses ")
		_T("WHERE StdResponseCatID = ? AND IsDeleted = 0 ")
		_T("ORDER BY Subject") );
	
	GenListXML(2);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the standard responses by ticket category...          
\*--------------------------------------------------------------------------*/
void CStdResponse::ListByTicketCategory( int m_TcID )
{	
	// check security
	//m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_READ_ACCESS );
	
	// Get the ticket category name from m_TcID
	string sTcName;
	GetTicketCategoryName( m_TcID, sTcName );
	
	// Get the standard response category ID that matches the ticket category name


	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_StandardResponse.m_StandardResponseID);
	//Put actual StdResponseCatID in m_AgentID
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_AgentID);
	BINDCOL_TCHAR_NOLEN( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );
	
	BINDPARAM_TCHAR( GetQuery(), (void*)sTcName.c_str() );
		
	GetQuery().Execute( _T("SELECT StandardResponses.StandardResponseID, StandardResponses.StdResponseCatID, Subject, DateModified, StandardResponses.StandardResponse ")
		_T("FROM StandardResponses ")
		_T("INNER JOIN StdResponseCategories on ")
		_T(" StandardResponses.StdResponseCatID = StdResponseCategories.StdResponseCatID ")
		_T("WHERE StdResponseCategories.CategoryName = ? AND StandardResponses.IsDeleted = 0 ")
		_T("ORDER BY StandardResponses.Subject") );
	
	GenListXML(0);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the standard responses by ticket box...          
\*--------------------------------------------------------------------------*/
void CStdResponse::ListByTicketBox( int m_TbID )
{	
	// check security
	// m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_READ_ACCESS );
	
	// Get the ticket box name from m_TbID
	string sTbName;
	GetTicketBoxName( m_TbID, sTbName );
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_StandardResponse.m_StandardResponseID);
	//Put actual StdResponseCatID in m_AgentID
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_AgentID);
	BINDCOL_TCHAR_NOLEN( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );
	
	BINDPARAM_TCHAR( GetQuery(), (void*)sTbName.c_str() );
	
	GetQuery().Execute( _T("SELECT StandardResponses.StandardResponseID, StandardResponses.StdResponseCatID, Subject, DateModified, StandardResponses.StandardResponse ")
		_T("FROM StandardResponses ")
		_T("INNER JOIN StdResponseCategories on ")
		_T(" StandardResponses.StdResponseCatID = StdResponseCategories.StdResponseCatID ")
		_T("WHERE StdResponseCategories.CategoryName = ? AND StandardResponses.IsDeleted = 0 ")
		_T("ORDER BY StandardResponses.Subject") );
	
	GenListXML(0);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the agent's most frequently used standard responses	              
\*--------------------------------------------------------------------------*/
void CStdResponse::ListMyFavorites( void )
{
	GetQuery().Initialize();
	
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID);
	//Put actual StdResponseCatID in m_AgentID
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_AgentID);
	BINDCOL_TCHAR( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );
	
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	
	GetQuery().Execute( _T("SELECT StandardResponses.StandardResponseID, StandardResponses.StdResponseCatID, Subject, DateModified, StandardResponses.StandardResponse ")
		_T("FROM StdResponseFavorites  ")
		_T("INNER JOIN StandardResponses on ")
		_T("StdResponseFavorites.StandardResponseID = StandardResponses.StandardResponseID ")
		_T("WHERE StdResponseFavorites.AgentID = ? AND StandardResponses.IsDeleted = 0 ")
		_T("ORDER BY Subject") );
	
	GenListXML(0);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the agent's draft standard responses	              
\*--------------------------------------------------------------------------*/
void CStdResponse::ListDrafts()
{
	// check security
	//m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_READ_ACCESS );
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_StandardResponse.m_StandardResponseID);
	BINDCOL_LONG_NOLEN( GetQuery(), m_StandardResponse.m_AgentID);
	BINDCOL_TCHAR_NOLEN( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );
	GetQuery().Execute( _T("SELECT StandardResponseID, AgentID, Subject, DateModified ")
		_T("FROM StandardResponses ")
		_T("WHERE StdResponseCatID = 1 AND IsDeleted = 0 ")
		_T("ORDER BY Subject") );	
	
	GenListXML(2);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Lists the agent's most frequently used standard responses	              
\*--------------------------------------------------------------------------*/
void CStdResponse::ListMostFrequentUsed()
{
	// check security
	//m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_READ_ACCESS );
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), m_nUsageCount );
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID);
	//Put actual StdResponseCatID in m_AgentID
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_AgentID);
	BINDCOL_TCHAR( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );
	
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	
	GetQuery().Execute( _T("SELECT TOP 25 COUNT(*), StandardResponses.StandardResponseID, StandardResponses.StdResponseCatID, Subject, DateModified, CAST(StandardResponses.StandardResponse as varchar(4000)) AS 'Body' FROM StandardResponseUsage INNER JOIN StandardResponses on StandardResponseUsage.StandardResponseID = StandardResponses.StandardResponseID WHERE StandardResponseUsage.AgentID = ? AND StandardResponses.IsDeleted = 0 GROUP BY StandardResponses.StandardResponseID, StandardResponses.StdResponseCatID, StandardResponses.Subject, StandardResponses.DateModified, CAST(StandardResponses.StandardResponse as varchar(4000)) ORDER BY 1 DESC") );
	
	GenListXML(0);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Generate XML for stdresponselist.ems		              
\*--------------------------------------------------------------------------*/
void CStdResponse::GenListXML(int nAccess)
{	
	// are we listing the standard responses for the menu on the message composition page?
	bool bMenu = (GetISAPIData().m_sPage.compare( _T("stdresponselist") ) != 0);

	if( !bMenu )
	{
		GetXMLGen().AddChildElem( _T("Category" ));

		if(m_lastCatID == EMS_STDRESP_SEARCH_RESULTS)
			GetXMLGen().AddChildAttrib( _T("ID"),  m_lastCatID );
		else
			GetXMLGen().AddChildAttrib( _T("ID"),  m_StandardResponse.m_StdResponseCatID );

		AddStdRespCategory( _T("Name"),  m_StandardResponse.m_StdResponseCatID, true );
		GetXMLGen().AddChildAttrib( _T("Access"), m_access );
	}
	
	CEMSString sSubject;
	CEMSString sDateTime;
	CEMSString sString;
	int nIndex = 0;
	int nAgentID = GetAgentID();

	sSubject.reserve( STANDARDRESPONSES_SUBJECT_LENGTH );

	vector<TStandardResponses> srList;
	vector<TStandardResponses>::iterator iter;

	while ( GetQuery().Fetch() == S_OK )
	{
		if( bMenu)
				GETDATA_TEXT( GetQuery(), m_StandardResponse.m_StandardResponse );
		
		srList.push_back(m_StandardResponse);
	}
	
	
	for( iter = srList.begin(); iter != srList.end(); iter++ )
	{
		int nAcc = nAccess;
				
		if(nAcc == 0)
		{
			nAcc = GetAgentRightLevel(EMS_OBJECT_TYPE_STD_RESP, iter->m_AgentID);			
		}
		if(m_StandardResponse.m_StdResponseCatID == 1 && !GetIsAdmin() && nAgentID != iter->m_AgentID)
		{
			nAcc = 1;
			if ( GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, iter->m_AgentID ) >= EMS_EDIT_ACCESS )
			{
				nAcc = 2;
			}
		}
		if(nAcc > 1)
		{
			GetXMLGen().AddChildElem( _T("StdResponse") );
			GetXMLGen().AddChildAttrib( _T("rowIndex"), nIndex++ );
			GetXMLGen().AddChildAttrib( _T("ID"), iter->m_StandardResponseID );
			
			sSubject.assign( iter->m_Subject );
			
			if( bMenu)
				sSubject.EscapeJavascript();
			
			sSubject.EscapeHTML();
			
			GetXMLGen().AddChildAttrib( _T("Subject"), sSubject.c_str() );
			
			GetDateTimeString( iter->m_DateModified, iter->m_DateModifiedLen, sDateTime );
			GetXMLGen().AddChildAttrib( _T("LastModified"), sDateTime.c_str() );
			GetXMLGen().AddChildAttrib( _T("UsageCount"), m_nUsageCount );
			if(m_lastCatID == EMS_STDRESP_SEARCH_RESULTS)
				GetXMLGen().AddChildAttrib( _T("CategoryName"), iter->m_Keywords);

			if( bMenu)
			{	
				// body
				GetXMLGen().IntoElem();
				sString.assign( iter->m_StandardResponse );
				sString.EscapeJavascript();
				sString.EscapeHTML();
				GetXMLGen().AddChildElem( _T("SRBody"), sString.c_str());
				GetXMLGen().OutOfElem();
			}
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Adds record to StandardResponseUsage to indicate the
||				standard response was used.
\*--------------------------------------------------------------------------*/
void CStdResponse::UsedResponse( void )
{
	TStandardResponseUsage ResponseUsage;
	
	ResponseUsage.m_StandardResponseID = m_StandardResponse.m_StandardResponseID;
	ResponseUsage.m_AgentID = GetAgentID();
	GetTimeStamp( ResponseUsage.m_DateUsed );
	ResponseUsage.m_DateUsedLen = sizeof(TIMESTAMP_STRUCT);
	
	ResponseUsage.Insert( GetQuery() );
	
	// associate standard response attachments
	TOutboundMessageAttachments attach;
	
	if ( GetISAPIData().GetURLLong( _T("MESSAGE"), attach.m_OutboundMessageID ) )
	{
		// list the attachments
		list<CAttachment>::iterator iter;
		tstring sFullAttachPath;
		
		for ( iter = m_AttachList.begin(); iter != m_AttachList.end(); iter++ )
		{
			// virus scan the attachment if it hasn't been already
			if ( iter->m_VirusScanStateID == EMS_VIRUS_SCAN_STATE_NOT_SCANNED )
				VirusScanAttachment( *iter );

			attach.m_AttachmentID = iter->m_AttachmentID;
			attach.Insert( GetQuery() );
		}
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the ID of the first standard response
||				that is in the same category as 
||				m_StandardResponse.m_StandardResponseID	              
\*--------------------------------------------------------------------------*/
int CStdResponse::GetFirstIDInCategory( void )
{
	int nID = 0;
	
	GetQuery().Initialize();
	
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_StdResponseCatID );
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	
	GetQuery().Execute( _T("SELECT TOP 1 StdResponseCatID FROM StandardResponses ")
		_T("WHERE StandardResponseID = ? " ) );
	
	if ( GetQuery().Fetch() != S_OK )
	{
		CEMSString sError;
		sError.Format( _T("Invalid StandardResponseID (%d)"), m_StandardResponse.m_StandardResponseID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}
	
	GetQuery().Initialize();
	
	BINDCOL_LONG_NOLEN( GetQuery(), nID );
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StdResponseCatID );
	
	GetQuery().Execute( _T("SELECT TOP 1 StandardResponseID FROM StandardResponses ")
		_T("WHERE StdResponseCatID = ? AND IsDeleted = 0" )
		_T("ORDER BY Subject") );
	
	GetQuery().Fetch();
	
	return nID;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Add the current standard responses to the user's
||				My Favorites	              
\*--------------------------------------------------------------------------*/
void CStdResponse::AddFavorite( CURLAction& action )
{
	// make sure it doesn't already exsist
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	
	GetQuery().Execute(	_T("SELECT TOP 1 StdResponseFavoritesID FROM StdResponseFavorites ")
		_T("WHERE StandardResponseID = ? AND AgentID = ? ") );
	
	if ( GetQuery().Fetch() == S_OK)
		return;
	
	// add it
	GetQuery().Initialize();
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	
	GetQuery().Execute(	_T("INSERT INTO StdResponseFavorites ")
		_T("(StandardResponseID, AgentID) ")
		_T("VALUES (?, ?)") );

	m_FavoriteMod = 1;
	
	// hack to fix Devtrack #143
	action.m_PageName = _T("stdresponse.ems");
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Remove the current standard response from the user's
||				My Favorites	              
\*--------------------------------------------------------------------------*/
void CStdResponse::RemoveFavorite( CURLAction& action )
{	
	GetQuery().Initialize();
	
	BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
	BINDPARAM_LONG( GetQuery(), m_ISAPIData.m_pSession->m_AgentID );
	
	GetQuery().Execute(	_T("DELETE FROM StdResponseFavorites ")
		_T("WHERE StandardResponseID = ? AND AgentID = ? ") );
	
	if ( GetQuery().GetRowCount() != 1 )
	{
		CEMSString sError;
		sError.Format( _T("Invalid StandardResponseID (%d)"), m_StandardResponse.m_StandardResponseID );
		THROW_EMS_EXCEPTION( E_InvalidID, sError );
	}

	m_FavoriteMod = 1;

	// hack to fix Devtrack #143
	action.m_PageName = _T("stdresponse.ems");
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the agent's access level	              
\*--------------------------------------------------------------------------*/
void CStdResponse::RequireAccessLevel( unsigned char AccessLevel )
{
	m_bDraftAccess = false;
	
	bool bIsDraft = m_StandardResponse.m_StdResponseCatID == EMS_STDRESP_DRAFTS;
	m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, m_StandardResponse.m_StdResponseCatID, !bIsDraft ? AccessLevel : EMS_NO_ACCESS );
	
	// check to see if the agent has access to create a new draft
	// or to modifiy a draft which they created...
	if ( bIsDraft && m_access < EMS_DELETE_ACCESS )
	{
		// an agent with read access can create a new draft
		if ( m_StandardResponse.m_StandardResponseID == 0 )
		{
			m_bDraftAccess = true;
			return;
		}
		
		// an agent has full rights to drafts which they created
		int nAgentID;
		int nCatID;
		GetQuery().Initialize();
		BINDCOL_LONG_NOLEN( GetQuery(), nAgentID );
		BINDCOL_LONG_NOLEN( GetQuery(), nCatID );
		BINDPARAM_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID );
		GetQuery().Execute( _T("SELECT AgentID,StdResponseCatID FROM StandardResponses WHERE StandardResponseID=?") );
		
		if ( GetQuery().Fetch() != S_OK )
		{
			CEMSString sError;
			sError.Format( _T("Invalid StandardResponseID (%d)"), m_StandardResponse.m_StandardResponseID );
			THROW_EMS_EXCEPTION( E_InvalidID, sError.c_str() );
		}
		
		// an agent has access to thier own drafts
		if ( nAgentID == GetAgentID() )
		{
			m_bDraftAccess = true;
			return;
		}
		
		// an agent can only move or delete SRs they created
		if ( nCatID == m_StandardResponse.m_StdResponseCatID && nAgentID == GetAgentID())
		{
			m_bDraftAccess = true;
			return;
		}

		// an agent can update a draft SR of an Agent they have Edit rights to
		// get action as a URL or form parameter
		tstring sAction;		
		if (( GetISAPIData().GetFormString( _T("ACTION"), sAction, true ) ) ||
			( GetISAPIData().GetURLString( _T("ACTION"), sAction, true ) ))
		{
			if ( sAction.compare( _T("update") ) == 0 )
			{
				if ( nCatID == m_StandardResponse.m_StdResponseCatID && nAgentID != GetAgentID())
				{
					if ( GetAgentRightLevel( EMS_OBJECT_TYPE_AGENT, nAgentID ) >= EMS_EDIT_ACCESS )
					{
						m_bDraftAccess = true;
						return;
					}
				}
			}
		}

		// any other actions require the requested access level
		if ( m_access < AccessLevel )
			m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, AccessLevel );
	}
}

void CStdResponse::ListSearchResults(void)
{
	// Get Form Data
	int nSearchTypeSubject;
	GetISAPIData().GetFormLong("optSubject", nSearchTypeSubject, true);

	std::string sSubject;
	GetISAPIData().GetFormString("txtSubject",sSubject,true);

	std::string sContent;
	GetISAPIData().GetFormString("txtContent",sContent,true);

	std::string sNote;
	GetISAPIData().GetFormString("txtNote",sNote,true);

	// Check Security
	// m_access = RequireAgentRightLevel( EMS_OBJECT_TYPE_STD_RESP, 0, EMS_READ_ACCESS );
	
	// Initialize Query
	GetQuery().Initialize();

	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_StandardResponseID);
	//Put CategoryName in m_Keywords
	BINDCOL_TCHAR( GetQuery(), m_StandardResponse.m_Keywords);
	//Put actual StdResponseCatID in m_AgentID
	BINDCOL_LONG( GetQuery(), m_StandardResponse.m_AgentID);
	BINDCOL_TCHAR( GetQuery(), m_StandardResponse.m_Subject );
	BINDCOL_TIME( GetQuery(), m_StandardResponse.m_DateModified );

	std::string sqlCmd("SELECT StandardResponses.StandardResponseID,  StdResponseCategories.CategoryName, StdResponseCategories.StdResponseCatID, StandardResponses.Subject, StandardResponses.DateCreated FROM StandardResponses INNER JOIN StdResponseCategories ON StandardResponses.StdResponseCatID = StdResponseCategories.StdResponseCatID ");

	if(!sSubject.empty())
	{
		if(!nSearchTypeSubject)
		{
			sqlCmd.append("WHERE StandardResponses.Subject LIKE '%");
			sqlCmd.append(sSubject);
			sqlCmd.append("%'");
		}
		else
		{
			sqlCmd.append("WHERE StandardResponses.Subject LIKE '");
			sqlCmd.append(sSubject);
			sqlCmd.append("'");
		}

		if(!sContent.empty())
		{
			sqlCmd.append("AND StandardResponses.StandardResponse LIKE '%");
			sqlCmd.append(sContent);
			sqlCmd.append("%'");
		}

		if(!sNote.empty())
		{
			sqlCmd.append("AND StandardResponses.Note LIKE '%");
			sqlCmd.append(sNote);
			sqlCmd.append("%'");
		}

		GetQuery().Execute(sqlCmd.c_str());

		GenListXML(0);
	}
	else if(!sContent.empty())
	{
		sqlCmd.append("WHERE StandardResponses.StandardResponse LIKE '%");
		sqlCmd.append(sContent);
		sqlCmd.append("%'");

		if(!sNote.empty())
		{
			sqlCmd.append("AND StandardResponses.Note LIKE '%");
			sqlCmd.append(sNote);
			sqlCmd.append("%'");
		}

		GetQuery().Execute(sqlCmd.c_str());

		GenListXML(0);
	}
	else if(!sNote.empty())
	{
		sqlCmd.append("WHERE StandardResponses.Note LIKE '%");
		sqlCmd.append(sNote);
		sqlCmd.append("%'");

		GetQuery().Execute(sqlCmd.c_str());

		GenListXML(0);
	}
}
