/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/listclasses.h,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   List Classes
||              
\\*************************************************************************/

// only include this file once
#pragma once

#include "EMSString.h"
#include "DateFns.h"
#include "SecurityFns.h"

class CTBVPrivateList
{
public:

	CTBVPrivateList() { ZeroMemory( this, sizeof(CTBVPrivateList));}
	
	int m_ViewID ;
	int m_AgentBoxID;
	int m_TypeID;
	unsigned char m_ShowClosedItems;
	
	void List(CODBCQuery& query, int nAgentID)
	{
		query.Initialize();
		
		BINDCOL_LONG_NOLEN(query, m_ViewID);
		BINDCOL_LONG_NOLEN(query, m_AgentBoxID);
		BINDCOL_LONG_NOLEN(query, m_TypeID);
		BINDCOL_BIT_NOLEN(query, m_ShowClosedItems);

		BINDPARAM_LONG(query, nAgentID);
	
		// excute the query
		query.Execute( _T("SELECT TicketBoxViewID, AgentBoxID, TicketBoxViewTypeID, ShowClosedItems ")
			           _T("FROM TicketBoxViews ")
			           _T("INNER JOIN Agents ON TicketBoxViews.AgentBoxID = Agents.AgentID ")
			           _T("WHERE TicketBoxViews.AgentID = ? " )
			           _T("ORDER BY Agents.Name, Agents.AgentID, TicketBoxViews.TicketBoxViewTypeID"));
	}
};

class CTBVPublicList
{
public:

	CTBVPublicList() { ZeroMemory( this, sizeof(CTBVPublicList));}
	
	unsigned int nViewID;
	unsigned int nTicketBoxID;
	char WaterMarkStatus;
	unsigned int nCount;
	unsigned char nShowOwnedItems;
	unsigned char nShowClosedItems;
	unsigned int nUnreadMode;	
	
	void List(CODBCQuery& query, int nAgentID, bool nIsAdmin, bool bUseAtc)
	{
		query.Initialize();
		
		BINDCOL_LONG_NOLEN (query, nViewID);
		BINDCOL_LONG_NOLEN (query, nTicketBoxID);
		BINDCOL_TINYINT_NOLEN (query, WaterMarkStatus);
		BINDCOL_BIT_NOLEN(query, nShowOwnedItems);
		BINDCOL_BIT_NOLEN(query, nShowClosedItems);
		BINDCOL_LONG_NOLEN (query, nUnreadMode);
		BINDCOL_LONG_NOLEN (query, nCount);
		BINDPARAM_LONG(query, nAgentID);		
		if ( !nIsAdmin && bUseAtc )
		{
			query.Execute( _T("SELECT TicketBoxViewID, TBV.TicketBoxID, WaterMarkStatus, ShowOwnedItems, ShowClosedItems, UnreadMode, ") 
						   _T("(SELECT COUNT(*) FROM Tickets INNER JOIN AgentTicketCategories ATC ON ")
						   _T("Tickets.TicketCategoryID=ATC.TicketCategoryID WHERE ATC.AgentID=TBV.AgentID AND ")
						   _T("TicketBoxID=TBV.TicketBoxID AND IsDeleted=0 AND TicketStateID>(1-ShowClosedItems) AND ")
						   _T("(ShowOwnedItems=1 OR OwnerID=0)) FROM TicketBoxViews TBV ") 
						   _T("INNER JOIN TicketBoxes TB ON TBV.TicketBoxID = TB.TicketBoxID ")
						   _T("WHERE TBV.AgentID=? ORDER BY TB.Name"));

		}
		else
		{
			
			query.Execute( _T("SELECT TicketBoxViewID, TBV.TicketBoxID, WaterMarkStatus, ShowOwnedItems, ShowClosedItems, UnreadMode, ")
						   _T("(SELECT COUNT(*) FROM Tickets WHERE TicketBoxID=TBV.TicketBoxID AND ")
						   _T("IsDeleted=0 AND TicketStateID>(1-ShowClosedItems) AND ")
						   _T("(ShowOwnedItems=1 OR OwnerID=0)) ")
                       	   _T("FROM TicketBoxViews TBV ")
			               _T("INNER JOIN TicketBoxes TB ON TBV.TicketBoxID = TB.TicketBoxID ")
			               _T("WHERE TBV.AgentID=? ORDER BY TB.Name"));
		}
		
	}
};

class CPersonalDataList : public TPersonalData
{
public:
	CPersonalDataList() { ZeroMemory( this, sizeof(CPersonalDataList));}
	
	void ListAgent(CODBCQuery& query, int nAgentID)
	{
		List(query, nAgentID, NULL);
	}

	void ListContact(CODBCQuery& query, int nContactID)
	{
		List(query, NULL, nContactID);
	}

	void List(CODBCQuery& query, int nAgentID, int nContactID)
	{
		query.Initialize();

		BINDCOL_LONG(query, m_PersonalDataID);
		BINDCOL_LONG(query, m_PersonalDataTypeID);
		BINDCOL_TCHAR(query, m_DataValue);
		BINDCOL_TCHAR(query, m_Note);
		
		BINDPARAM_LONG(query, nAgentID);
		BINDPARAM_LONG(query, nContactID);
		
		// excute the query
		query.Execute( _T("SELECT PersonalDataID, PersonalDataTypeID, DataValue, Note ")
			_T("FROM PersonalData ")
			_T("WHERE AgentID = ? AND ContactID = ? ")
			_T("ORDER BY PersonalDataTypeID"));
	}
};


class CContactHistTicketList
{
public:
	
	int m_TicketID;
	TCHAR m_Subject[TICKETS_SUBJECT_LENGTH];
	TIMESTAMP_STRUCT m_DateCreated;
	long m_DateCreatedLen;
	int m_PriorityID;
	int m_TicketStateID;
	int m_OwnerID;
	int m_TicketBoxID;

	void List(CODBCQuery& query, int nContactID)
	{
		query.Initialize();

		BINDCOL_LONG_NOLEN( query, m_TicketID );
		BINDCOL_TCHAR_NOLEN( query, m_Subject );
		BINDCOL_TIME( query, m_DateCreated );
		BINDCOL_LONG_NOLEN( query, m_PriorityID );
		BINDCOL_LONG_NOLEN( query, m_TicketStateID );
		BINDCOL_LONG_NOLEN( query, m_OwnerID );
		BINDCOL_LONG_NOLEN( query, m_TicketBoxID );

		BINDPARAM_LONG( query, nContactID );

		query.Execute( _T("SELECT DISTINCT Tickets.TicketID, Tickets.Subject, DateCreated, PriorityID, ")
			_T("TicketStateID, OwnerID, TicketBoxID ")
			_T("FROM Tickets ")
			_T("INNER JOIN TicketContacts ON Tickets.TicketID = TicketContacts.TicketID ")
			_T("WHERE TicketContacts.ContactID = ? AND Tickets.IsDeleted = 0 "));
	}
};
