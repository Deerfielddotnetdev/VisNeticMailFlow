/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactNotes.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Contact Notes  
||              
\\*************************************************************************/

// only include once
#pragma once

#include "XMLDataClass.h"
#include "Attachment.h"

class CContactNotes : public CXMLDataClass  
{
public:

	CContactNotes(CISAPIData& ISAPIData) : CXMLDataClass( ISAPIData ) {};
	virtual ~CContactNotes() {};
	
	int Run(CURLAction& action);
	int  ReserveID(void);
	
private:

	void List( void );
	void Query( void );
	void Insert( void );
	void Update( void );
	void Delete( void );

	void GenerateXML( void );
	void DecodeForm( void );
	void DeleteAttachments( int nContactNoteID );



private:
	
	TContactNotes m_ContactNote;
	tstring m_sView;
	bool	m_IsPhone;
	int		m_nViewID;
	int		m_nAgentBoxID;
};