#include "stdafx.h"
#include ".\CustomTicketBoxViews.h"

CustomTicketBoxViews::CustomTicketBoxViews(CISAPIData& ISAPIData)
	:CXMLDataClass(ISAPIData)
	,_AgentID(0)
{
}

CustomTicketBoxViews::~CustomTicketBoxViews(void)
{
}

int CustomTicketBoxViews::Run(CURLAction& action)
{
	GetISAPIData().GetURLLong( _T("AgentID"), _AgentID );

	tstring sAction;
	GetISAPIData().GetXMLString( _T("action"), sAction, true );

	if ( sAction.compare( _T("insert") ) == 0 )
	{
		AddToFolders(action);
	}
	else if ( sAction.compare( _T("delete") ) == 0 )
	{
		DeleteFromFolders(action);
	}
	else
	{
		ListFolders(action);
	}

	return 0;
}

void CustomTicketBoxViews::AddToFolders(CURLAction& action)
{
	// check security
	if(_AgentID)
	{
		RequireAgentRightLevel(EMS_OBJECT_TYPE_AGENT, _AgentID, EMS_EDIT_ACCESS);
	}
	else
	{
		DISABLE_IN_DEMO();
		RequireAdmin();
	}
	
	dca::String55 sFolder;
	ZeroMemory(sFolder, 56);

	GetISAPIData().GetXMLTCHAR( _T("txtFolder"), sFolder, 55);
	long lParentView = 0;

	if(lstrlen(sFolder))
	{
		{
			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), _AgentID);
			BINDPARAM_LONG( GetQuery(), _AgentID);
			BINDCOL_LONG_NOLEN( GetQuery(), lParentView);
			GetQuery().Execute(_T("SELECT TicketBoxViewID FROM TicketBoxViews WHERE (AgentID = ?) AND (TicketBoxID = 0) AND (AgentBoxID = ?) AND (TicketBoxViewTypeID = 1)"));

			GetQuery().Fetch();
		}

		// check that the folder isn't already defined
		GetQuery().Initialize();	
		BINDPARAM_LONG( GetQuery(), _AgentID);
		BINDPARAM_TCHAR( GetQuery(), sFolder);
		
		GetQuery().Execute( _T("SELECT FolderID FROM Folders WHERE AgentID = ? AND Name = ?"));
		
		if ( GetQuery().Fetch() == S_OK )
			THROW_VALIDATION_EXCEPTION( _T("txtFolder"), _T("The specified folder already exists") );

		GetQuery().Initialize();
		BINDPARAM_TCHAR( GetQuery(), sFolder);
		BINDPARAM_LONG( GetQuery(), _AgentID);\
		BINDPARAM_LONG(GetQuery(), lParentView);
		
		GetQuery().Execute( _T("INSERT INTO Folders (Name, AgentID, ParentID) VALUES(?, ?, ?)"));
	}
}

void CustomTicketBoxViews::ListFolders(CURLAction& action)
{
	// check security
	unsigned char access;

	if(_AgentID)
	{
		access = RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, _AgentID, EMS_READ_ACCESS );
	}
	else
	{
		RequireAdmin();
		access = EMS_DELETE_ACCESS;
	}

	GetXMLGen().AddChildElem( _T("access") );
	GetXMLGen().IntoElem();
	GetXMLGen().AddChildElem( _T("level"), access );
	GetXMLGen().OutOfElem();

	dca::String55 sFolder;
	long sFolderLen = 55;
	long lID = 0;
	ZeroMemory(sFolder, 56);

	GetQuery().Initialize();	
	BINDPARAM_LONG( GetQuery(), _AgentID);
	BINDCOL_LONG_NOLEN( GetQuery(), lID);
	BINDCOL_TCHAR( GetQuery(), sFolder);
	
	GetQuery().Execute( _T("SELECT FolderID, Name FROM Folders WHERE AgentID = ?"));
	
	GetXMLGen().AddChildElem( _T("myfolders") );

	while ( GetQuery().Fetch() == S_OK )
	{
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("folder") );
		GetXMLGen().AddChildAttrib( _T("id"), lID );
		GetXMLGen().IntoElem();
		GetXMLGen().AddChildElem( _T("name"), sFolder );
		GetXMLGen().OutOfElem();
		GetXMLGen().OutOfElem();
	}
}

void CustomTicketBoxViews::DeleteFromFolders(CURLAction& action)
{
	// check security
	_AgentID ? RequireAgentRightLevel( EMS_OBJECT_TYPE_AGENT, _AgentID, EMS_EDIT_ACCESS ) : RequireAdmin();

	CEMSString sQuery;
	CEMSString sSelectID;
	tstring sChunk;

	GetISAPIData().GetXMLString( _T("selectID"), sSelectID );

	while ( sSelectID.CDLGetNextChunk( EMS_CDL_CHUNK_SIZE, sChunk ) )
	{
		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), _AgentID );

		sQuery.Format(_T("UPDATE Tickets SET FolderID = 0 WHERE FolderID IN (%s) AND OwnerID = ?"), sChunk.c_str());
		GetQuery().Execute( sQuery.c_str() );

		GetQuery().Initialize();
		
		BINDPARAM_LONG( GetQuery(), _AgentID );

		sQuery.Format( _T("DELETE FROM Folders WHERE FolderID IN (%s) AND AgentID = ?"), sChunk.c_str() );
		GetQuery().Execute( sQuery.c_str() );
	}
}