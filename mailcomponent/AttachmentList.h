// AttachmentList.h: interface for the CAttachmentList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATTACHMENTLIST_H__B91C95EB_303A_4C70_BB04_85A52BBE904D__INCLUDED_)
#define AFX_ATTACHMENTLIST_H__B91C95EB_303A_4C70_BB04_85A52BBE904D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Attachment.h"

#include <vector>
using namespace std;

class CAttachmentList  
{
public:
	CAttachmentList();
	virtual ~CAttachmentList();

	// copy constructor
	CAttachmentList(const CAttachmentList& aList);

	// assignment overload
	CAttachmentList& operator = (const CAttachmentList& aList);
	
    void AddAttachment(const CAttachment& addr);
	
    const int GetCount() const								{ return m_attachments.size(); }
    CAttachment* GetAttachment(const int index) const		{ return m_attachments[index]; }

	void DeleteAttachments(); // clear all attachments; free memory

private:
	vector<CAttachment*> m_attachments;

    void CopyAttachments(const CAttachmentList& aList);	// copy attachments to new list
};

#endif // !defined(AFX_ATTACHMENTLIST_H__B91C95EB_303A_4C70_BB04_85A52BBE904D__INCLUDED_)
