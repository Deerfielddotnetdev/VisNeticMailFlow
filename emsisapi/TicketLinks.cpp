#include "stdafx.h"
#include "TicketLinks.h"
#include "TicketHistoryFns.h"
#include "Ticket.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CTicketLinks::CTicketLinks( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{
	nOwnerID = 0;
	nTicketBoxID = 0;
	nTicketID = 0;

	ZeroMemory( &m_Created, sizeof(m_Created) );
	m_Created.hour = 0;
	m_Created.minute = 0;
	m_Created.second = 0;
	m_Created.fraction = 0;
}

CTicketLinks::~CTicketLinks()
{
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Entry Point	              
\*--------------------------------------------------------------------------*/
int CTicketLinks::Run( CURLAction& action )
{
	tstring sAction = _T("list");	
	
	// get the action one way or another...
	if (!GetISAPIData().GetFormString( _T("Action"), sAction, true ))
		if (!GetISAPIData().GetXMLString( _T("Action"), sAction, true ))
            GetISAPIData().GetURLString( _T("Action"), sAction, true );

	if ( GetISAPIData().GetXMLPost() )
	{
		if( sAction.compare( _T("delete") ) == 0 )
		{
			DISABLE_IN_DEMO();
			Delete();
		}
	}
	
	if( sAction.compare( _T("addtolink") ) == 0 )
	{
		// Get URL Variables
		int nTicketBoxView;
		GetISAPIData().GetURLLong( _T("ChangeTicketBoxView"), nTicketBoxView );
		CEMSString sTicketIDs;
		GetISAPIData().GetURLString( _T("srcticketId"), sTicketIDs);
		
		// Add XML
		GetXMLGen().AddChildElem( _T("LinkTicket") );
		GetXMLGen().AddChildAttrib( _T("SelectLink"), _T("1") );		
		GetXMLGen().AddChildAttrib( _T("TicketID"), sTicketIDs.c_str() );
		GetXMLGen().AddChildAttrib( _T("TicketBoxViewID"), nTicketBoxView );

		action.m_sPageTitle.assign( "Select Ticket Link to add Ticket(s) to" );
		return List();
	}	
	
	action.m_sPageTitle.assign( "Ticket Links" );
	return List();		
}

////////////////////////////////////////////////////////////////////////////////
// 
// Query Ticket Links
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketLinks::List(void)
{
	CEMSString sQuery;
	dca::String where_clause;

	DecodeForm();

	if (GetIsAdmin())
	{
		sQuery.Format(_T("SELECT DISTINCT tl.LinkName,tl.DateCreated,tl.OwnerID,tl.TicketLinkID, ")
					  _T("(SELECT COUNT(*) FROM TicketLinksTicket WHERE TicketLinkID=tlt.TicketLinkID) ")
					  _T("FROM TicketLinks tl ")
					  _T("INNER JOIN TicketLinksTicket tlt ON tl.TicketLinkID=tlt.TicketLinkID ")
					  _T("INNER JOIN Tickets t ON tlt.TicketID=t.TicketID ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID "));			  
	}
	else
	{
		sQuery.Format(_T("SELECT DISTINCT tl.LinkName,tl.DateCreated,tl.OwnerID,tl.TicketLinkID, ")
					  _T("(SELECT COUNT(*) FROM TicketLinksTicket WHERE TicketLinkID=tlt.TicketLinkID) ")
					  _T("FROM TicketLinks tl ")
					  _T("INNER JOIN TicketLinksTicket tlt ON tl.TicketLinkID=tlt.TicketLinkID ")
					  _T("INNER JOIN Tickets t ON tlt.TicketID=t.TicketID ")
					  _T("INNER JOIN TicketBoxes tb ON t.TicketBoxID=tb.TicketBoxID ")
					  _T("INNER JOIN TicketBoxViews tbv ON tb.TicketBoxID=tbv.TicketBoxID "));
	}
	
	where_clause.reserve(256);

	if (!GetIsAdmin())
	{
		CEMSString sTicketBoxViews;
		if(!where_clause.length())
		{
			sTicketBoxViews.Format( _T("WHERE tbv.TicketBoxViewTypeID=6 AND tbv.AgentID = %d"), GetAgentID() );
		}
		else
		{
			sTicketBoxViews.Format( _T(" AND tbv.TicketBoxViewTypeID=6 AND tbv.AgentID = %d"), GetAgentID() );
		}
		where_clause.append( sTicketBoxViews );
	}
	
	if ( nOwnerID > 0 )
	{
		CEMSString sOwnerIDs;
		if(!where_clause.length())
		{
			sOwnerIDs.Format( _T("WHERE tl.OwnerID = %d"), nOwnerID );
		}
		else
		{
			sOwnerIDs.Format( _T(" AND tl.OwnerID = %d"), nOwnerID );
		}
		where_clause.append( sOwnerIDs );
	}		

	if ( nTicketBoxID > 0 )
	{
		CEMSString sTicketBoxIDs;
		if(!where_clause.length())
		{
			sTicketBoxIDs.Format( _T("WHERE t.TicketBoxID = %d"), nTicketBoxID );
		}
		else
		{
			sTicketBoxIDs.Format( _T(" AND t.TicketBoxID = %d"), nTicketBoxID );
		}
		where_clause.append( sTicketBoxIDs );
	}		

	if ( nTicketID > 0 )
	{
		CEMSString sTicketIDs;
		if(!where_clause.length())
		{
			sTicketIDs.Format( _T("WHERE tlt.TicketID = %d"), nTicketID );
		}
		else
		{
			sTicketIDs.Format( _T(" AND tlt.TicketID = %d"), nTicketID );
		}
		where_clause.append( sTicketIDs );
	}	
	
	if( sLinkName.length() > 0 )
	{
		sLinkName.EscapeSQL();		

		if(where_clause.length())
			where_clause.append( _T(" AND (tl.LinkName") );
		else
			where_clause.append( _T("WHERE (tl.LinkName") );		
		
		sLinkNameBegin.Format( _T("%%[ .,/?!@#$%%^&*()]%s"),sLinkName.c_str());
		sLinkNameEnd.Format( _T("%s[ .,/?!@#$%%^&*()]%%"),sLinkName.c_str());
		sLinkNameWild = sLinkName;
		sLinkNameWild.insert( 0, _T("%") );
		sLinkNameWild.append( _T("%") );
		CEMSString sTemp;
		sTemp.Format( _T("%%[ .,/?!@#$%%^&*()]%s[ .,/?!@#$%%^&*()]%%"),sLinkName.c_str() );
		sLinkName = sTemp;
		CEMSString sTemp2;
        sTemp2.Format( _T(" like '%s' OR tl.LinkName LIKE '%s' OR tl.LinkName LIKE '%s' OR tl.LinkName LIKE '%s')"),sLinkName.c_str(),sLinkNameBegin.c_str(),sLinkNameEnd.c_str(),sLinkNameWild.c_str());			
		where_clause.append(sTemp2.c_str());		
	}

	if( m_Created.year > 0 )
	{
		CEMSString sDate;
		CEMSString sDateTime;
		GetDateTimeString( m_Created, m_CreatedLen, sDateTime);
		if(!where_clause.length())
		{
			sDate.Format( _T("WHERE tl.DateCreated > '%s'"), sDateTime.c_str() );
		}
		else
		{
			sDate.Format( _T(" AND tl.DateCreated > '%s'"), sDateTime.c_str() );
		}
		where_clause.append( sDate );
	}

	sQuery.append(where_clause.c_str());

	int nNumTickets;

	long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
	TIMESTAMP_STRUCT tsLocal;
	long tsLocalLen=0;

	GetQuery().Initialize();
	BINDCOL_TCHAR( GetQuery(), m_LinkName );
	BINDCOL_TIME( GetQuery(), m_DateCreated );
	BINDCOL_LONG( GetQuery(), m_OwnerID );
	BINDCOL_LONG( GetQuery(), m_TicketLinkID );
	BINDCOL_LONG_NOLEN( GetQuery(), nNumTickets );
	GetQuery().Execute( sQuery.c_str() );
	
    GetXMLGen().AddChildElem( _T("TicketLinks") );
	GetXMLGen().AddChildAttrib( _T("OwnerID"), nOwnerID );
	GetXMLGen().AddChildAttrib( _T("TicketBoxID"), nTicketBoxID );
	if(nTicketID > 0)
	{
		GetXMLGen().AddChildAttrib( _T("TicketID"), nTicketID );
	}
	else
	{
		GetXMLGen().AddChildAttrib( _T("TicketID"), _T("") );
	}
	if(m_Created.year > 0)
	{
		CEMSString sDate;		
		GetDateTimeString( m_Created, m_CreatedLen, sDate );
		GetXMLGen().AddChildAttrib( _T("CreateDate"), sDate.c_str() );
	}	
	GetXMLGen().AddChildAttrib( _T("LinkName"), sSaveLinkName.c_str() );	
	GetXMLGen().IntoElem();
    while( GetQuery().Fetch() == S_OK )
	{
		CEMSString sDateCreated;
		GetXMLGen().AddChildElem( _T("TicketLink") );
		GetXMLGen().AddChildAttrib( _T("ID"), m_TicketLinkID );
		GetXMLGen().AddChildAttrib( _T("LinkName"), m_LinkName );
		GetXMLGen().AddChildAttrib( _T("OwnerID"), m_OwnerID );		
		AddAgentName( _T("Owner"), m_OwnerID );
		
		if(lTzBias != 1)
		{
			if(ConvertToTimeZone(m_DateCreated,lTzBias,tsLocal))
			{
				GetDateTimeString( tsLocal, tsLocalLen, sDateCreated );
			}
			else
			{
				GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDateCreated );
			}
		}
		else
		{
			GetDateTimeString( m_DateCreated, m_DateCreatedLen, sDateCreated );
		}
		
		GetXMLGen().AddChildAttrib( _T("DateCreated"), sDateCreated.c_str() );
		GetXMLGen().AddChildAttrib( _T("TicketCount"), nNumTickets );	
	}
	GetXMLGen().OutOfElem();

	ListAgentNames( EMS_READ_ACCESS );
	ListTicketBoxNames( EMS_READ_ACCESS, GetXMLGen() ); 

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Delete a ticket link
// 
////////////////////////////////////////////////////////////////////////////////
int CTicketLinks::Delete()
{
	CEMSString sID;
	tstring sName;
	CEMSString sData1;					
	int nAgentID = GetAgentID();
	list<int> m_tid;
	list<int>::iterator mtidIter;
	CTicket Ticket(m_ISAPIData);

	if (!GetISAPIData().GetFormString( _T("selectId"), sID, true ))
				GetISAPIData().GetXMLString( _T("selectId"), sID);

	while ( sID.CDLGetNextInt(m_TicketLinkID))
	{
		TTicketLinks::Query(GetQuery());
        sName = m_LinkName;
		sData1.Format(_T("%s"),sName.c_str());

		if(!GetIsAdmin())
		{
			if(nAgentID != m_OwnerID)
			{
				THROW_EMS_EXCEPTION( E_AccessDenied, _T("You can only Delete Ticket Links you own!") );
			}
		}
		
		//Get the TicketID's for this Ticket Link
		m_tid.clear();
		int nTicketID;
		long nTicketIDLen;
		GetQuery().Initialize();
		BINDCOL_LONG( GetQuery(), nTicketID );
		BINDPARAM_LONG( GetQuery(), m_TicketLinkID );
		GetQuery().Execute( _T("SELECT TicketID ")
					_T("FROM TicketLinksTicket WHERE TicketLinkID=?") );	              	
		while( GetQuery().Fetch() == S_OK )
		{
			m_tid.push_back(nTicketID);
		}

		for( mtidIter = m_tid.begin(); mtidIter != m_tid.end(); mtidIter++ )
		{
			Ticket.m_TicketID = *mtidIter;
				
			if(!GetIsAdmin())
			{
				Ticket.RequireEdit(true);
			}								

			GetQuery().Initialize();
			BINDPARAM_LONG( GetQuery(), Ticket.m_TicketID );		
			BINDPARAM_LONG( GetQuery(), m_TicketLinkID );		
			GetQuery().Execute( _T("DELETE FROM TicketLinksTicket ")
							_T("WHERE TicketID=? AND TicketLinkID=? ") );

			// Log Ticket History
			THUnlinkTicket( GetQuery(), Ticket.m_TicketID, nAgentID, sName );		

			//Log it to Agent Activity
			if( _ttoi( Ticket.sAal.c_str() ) > 0 )
			{
				if( _ttoi( Ticket.sTu.c_str() ) > 0 )
				{
					Ticket.LogAgentAction(nAgentID,29,Ticket.m_TicketID,m_TicketLinkID,0,sData1,_T(""));
				}
			}
		}
		
		GetQuery().Reset();
		BINDPARAM_LONG( GetQuery(), m_TicketLinkID );			
		GetQuery().Execute( _T("DELETE FROM TicketLinks ")
						_T("WHERE TicketLinkID=? ") );

		// Log Agent Audit
		if( _ttoi( Ticket.sAal.c_str() ) > 0 )
		{
			if( _ttoi( Ticket.sLd.c_str() ) > 0 )
			{
				Ticket.LogAgentAction(GetAgentID(),27,0,0,0,sData1,_T(""));
			}
		}
	}
		
	return 0;
}

void CTicketLinks::DecodeForm(void)
{
	// LinkName
	if (!GetISAPIData().GetFormString( _T("LinkName"), sLinkName, true ))
				GetISAPIData().GetURLString( _T("LinkName"), sLinkName, true);

	sLinkName.EscapeSQL();
	sSaveLinkName = sLinkName;
	
	// Owner?
	if (!GetISAPIData().GetFormLong( _T("OwnerID"), nOwnerID, true ))
				GetISAPIData().GetURLLong( _T("OwnerID"), nOwnerID, true);
	

	// TicketBox?
	if (!GetISAPIData().GetFormLong( _T("TicketBoxID"), nTicketBoxID, true ))
				GetISAPIData().GetURLLong( _T("TicketBoxID"), nTicketBoxID, true);

	// TicketBox?
	if (!GetISAPIData().GetFormLong( _T("TicketID"), nTicketID, true ))
				GetISAPIData().GetURLLong( _T("TicketID"), nTicketID, true);

	// CreateDate
	CEMSString sDate;
	if (GetISAPIData().GetFormString( _T("CREATE_START_DATE"), sDate, true ))
	{
		int nTemp;
		if(sDate.CDLGetNextInt(nTemp))
		{
			sDate.CDLGetTimeStamp( m_Created );

			long lTzBias = GetTimeZoneOffset(g_ThreadPool.GetSharedObjects().m_nTimeZone);
			TIMESTAMP_STRUCT tsLocal;
			long tsLocalLen=0;
			if(lTzBias != 1)
			{
				if(ConvertFromTimeZone(m_Created,lTzBias,tsLocal))
				{
					m_Created.year = tsLocal.year;
					m_Created.month = tsLocal.month;
					m_Created.day = tsLocal.day;
					m_Created.hour = tsLocal.hour;
					m_Created.minute = tsLocal.minute;
					m_Created.second = tsLocal.second;
				}			
			}
		}
	}
}
