#pragma once

#include "XMLDataClass.h"

class CMsgAttach : public CXMLDataClass  
{
public:
	CMsgAttach(CISAPIData& ISAPIData);
	virtual ~CMsgAttach();

	int Run(CURLAction& action);
	
	void QueryMsg(void);
	void ListAttachments(void);
	void AddAttachment(void);
	void DeleteAttachment(void);

private:
	int m_nMsgID;
	int m_nMsgType;
	int m_nNoteID;
	int m_nNoteType;
};
