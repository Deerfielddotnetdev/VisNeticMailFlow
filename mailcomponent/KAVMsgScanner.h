// KAVMsgScanner.h: interface for the CKAVMsgScanner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KAVMSGSCANNER_H__4601C9EE_E4F4_499C_8AE0_C7FBFD0D2A97__INCLUDED_)
#define AFX_KAVMSGSCANNER_H__4601C9EE_E4F4_499C_8AE0_C7FBFD0D2A97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMessage;

class CKAVMsgScanner  
{
public:
	const BOOL GetIsScanningEnabled();
	const int SendInfectionAlert(CMessage* const pMsg);
	int ScanMsgBody(CMessage* const pMsg);
	int ScanMsgAttachments(CMessage* const pMsg);
	CKAVMsgScanner();
	virtual ~CKAVMsgScanner();
	bool QuarantineMsgBody(CMessage *const pMsg, tstring& sFileName);
};

#endif // !defined(AFX_KAVMSGSCANNER_H__4601C9EE_E4F4_499C_8AE0_C7FBFD0D2A97__INCLUDED_)
