/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/TicketNotes.h,v 1.2 2005/11/29 21:16:28 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Ticket Notes  
||              
\\*************************************************************************/

// only include once
#pragma once

#include "XMLDataClass.h"
#include "Attachment.h"

struct TicketContactList_t
{
	int nContactID;
	TCHAR Name[CONTACTS_NAME_LENGTH];
	long  NameLen;
	TCHAR Email[PERSONALDATA_DATAVALUE_LENGTH];
};

class CTicketNotes : public CXMLDataClass , public TTicketNotes  
{
public:

	CTicketNotes(CISAPIData& ISAPIData)
		:CXMLDataClass( ISAPIData )
		,m_BeenRead(0)
		,m_UseCutoffDate(0)
	{};
	virtual ~CTicketNotes() {};
	
	int Run(CURLAction& action);
	int  ReserveID(void);

	void static UpdateTHNoteRead(CODBCQuery& query, int nAgentID, int nTicketID, int nRead, int nNoteID);
	
private:

	void List( void );
	void Query( void );
	void Insert( void );
	void Update( void );
	void Delete( void );
	void UpdateRead();
	void ListTicketContacts( int nTicketID );
	void GenMultipleContactXML( list<TicketContactList_t>& List );
	void DeleteAttachments( int nTicketNoteID );

	void GenerateXML( void );
private:
	void SetBeenRead();
	bool UseCutoffDate();

private:
	
	tstring m_sView;
	int	    m_BeenRead;
	int		m_UseCutoffDate;
	bool	m_IsPhone;
};
