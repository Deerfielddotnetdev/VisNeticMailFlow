/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Attachment.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CAttachment : public TAttachments, public CXMLDataClass  
{
public:
	CAttachment(CISAPIData& ISAPIData);
	virtual ~CAttachment();
	
	void Copy( Attachments_t* pAtt ) 
	{ CopyMemory( (Attachments_t*)this, pAtt, sizeof(Attachments_t) ); }

	virtual int Run(CURLAction& action) {return 0;}
	
	void Query( void );
	void GenerateXML( void );
	void ListInboundMessageAttachments( int nID, list<CAttachment>& AttachList );
	void ListOutboundMessageAttachments( int nID, list<CAttachment>& AttachList );
	void ListStdResponseAttachments( int nID, list<CAttachment>& AttachList );
	void ListNoteAttachments( int nID, int nTypeID, list<CAttachment>& AttachList );
	void Delete( void );
	void DeleteNoteAttachment( void );

	bool m_FileExists;

private:
	void BindColumns( void );
	void GetColumnList( LPCSTR& szColumns );
};
