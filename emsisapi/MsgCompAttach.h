/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/MsgCompAttach.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CMsgCompAttach : public CXMLDataClass  
{
public:
	CMsgCompAttach(CISAPIData& ISAPIData);
	virtual ~CMsgCompAttach();

	int Run(CURLAction& action);
	
	void QueryMsg(void);
	void ListAttachments(void);
	void AddAttachment(void);
	void DeleteAttachment(void);

private:
	int m_nMsgID;
};
