#pragma once

#include "XMLDataClass.h"

class CTicketLinks : public CXMLDataClass, public TTicketLinks
{
public:
	CTicketLinks( CISAPIData& ISAPIData );
	virtual ~CTicketLinks();

	virtual int Run( CURLAction& action );

protected:
	int List(void);	
	int Delete();	
	void DecodeForm(void);

private:
	CEMSString sLinkName;
	CEMSString sLinkNameBegin;
	CEMSString sLinkNameEnd;
	CEMSString sLinkNameWild;
	CEMSString sSaveLinkName;
	int	nOwnerID;
	int nTicketBoxID;
	int nTicketID;
	TIMESTAMP_STRUCT m_Created;
	long m_CreatedLen;
};