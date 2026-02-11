/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TBVInfo.h,v 1.2 2005/11/29 21:16:27 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

class CTBVInfo  
{
	
public:
	CTBVInfo( ) { ZeroMemory( this, sizeof(CTBVInfo) ); } 
	virtual ~CTBVInfo() {};
	
	void Query(CODBCQuery& query);
	int  GetItemCount(CODBCQuery& query, int nFolderID = 0);
	
public:
	int m_TicketBoxViewID;
	int m_AgentID;
	int m_TicketBoxID;
	int m_AgentBoxID;
	int m_SortField;
	int m_ViewTypeID;
	unsigned char m_SortAsc;
	unsigned char m_ShowOwnedItems;
	unsigned char m_ShowClosedItems;
	unsigned char m_ShowState;
	unsigned char m_ShowPriority;
	unsigned char m_ShowNumNotes;
	unsigned char m_ShowTicketID;
	unsigned char m_ShowNumMsgs;
	unsigned char m_ShowSubject;
	unsigned char m_ShowContact;
	unsigned char m_ShowDate;
	unsigned char m_ShowCategory;
	unsigned char m_ShowOwner;	
	unsigned char m_ShowTicketBox;
	unsigned char m_UseDefault;

};

