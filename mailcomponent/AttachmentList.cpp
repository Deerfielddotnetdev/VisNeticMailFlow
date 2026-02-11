// AttachmentList.cpp: implementation of the CAttachmentList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AttachmentList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAttachmentList::CAttachmentList()
{

}

CAttachmentList::CAttachmentList(const CAttachmentList& aList)
{
    CopyAttachments(aList);
}

CAttachmentList::~CAttachmentList()
{
	DeleteAttachments();
}

CAttachmentList& CAttachmentList::operator = (const CAttachmentList& aList)
{
    if (this != &aList) 
	{
        DeleteAttachments();
        CopyAttachments(aList);
    }

    return *this;
}

void CAttachmentList::AddAttachment(const CAttachment& addr)
{
	CAttachment* pAttach = new CAttachment(addr);
	assert(pAttach);

	m_attachments.push_back(pAttach);
}

void CAttachmentList::CopyAttachments(const CAttachmentList& aList)
{
	vector<CAttachment*>::iterator ppaddr = m_attachments.begin();
    while (ppaddr != m_attachments.end()) 
	{
        CAttachment* addr = new CAttachment(**ppaddr);
        m_attachments.push_back(addr);
        ++ppaddr;
    }
}

void CAttachmentList::DeleteAttachments()
{
    vector<CAttachment*>::iterator ppaddr = m_attachments.begin();
    while (ppaddr != m_attachments.end()) 
	{
        if (*ppaddr) 
			delete *ppaddr;
		
        *ppaddr = 0;
        ++ppaddr;
    }

    m_attachments.clear();
}
