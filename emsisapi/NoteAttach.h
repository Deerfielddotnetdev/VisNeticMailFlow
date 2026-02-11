#pragma once

#include "XMLDataClass.h"

class CNoteAttach : public CXMLDataClass  
{
public:
	CNoteAttach(CISAPIData& ISAPIData);
	virtual ~CNoteAttach();

	int Run(CURLAction& action);
	
	void QueryNote(void);
	void ListAttachments(void);
	void AddAttachment(void);
	void DeleteAttachment(void);

private:
	int m_nNoteID;
	int m_nNoteType;
};
