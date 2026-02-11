/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/MessageIO.h,v 1.2 2005/11/29 21:30:04 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/


#if !defined(AFX_MESSAGEIO_H__CECD7A8E_8843_4B60_A203_18E25149F47F__INCLUDED_)
#define AFX_MESSAGEIO_H__CECD7A8E_8843_4B60_A203_18E25149F47F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// the following structure defines 
struct _msg_file
{
	tstring sPopHeaders;
	tstring sCc;
	tstring sBcc;
	tstring sTo;
	tstring sReplyTo;
	tstring sMsgHeader;
	tstring sBody;
	tstring sFooter;
};

// msg i/o class def
class CMessageIO  
{
public:
	const BOOL GetQuarantinePath(const tstring& sFileName, tstring& sFileLoc, int nVerThree = 0);
	const BOOL GetOutboundSpoolPath(tstring& sFileLoc, const UINT nMsgID);
	const BOOL GetNoteAttachPath(tstring &sFileLoc, int nVerThree = 0);
	const BOOL GetOutboundAttachPath(tstring &sFileLoc, int nVerThree = 0);
	const BOOL GetOutboundAttachPath(const tstring& sFileName, tstring &sFileLoc, int nVerThree = 0);
	const BOOL GetInboundAttachPath(tstring& sFileLoc, int nVerThree = 0);
	const BOOL GetOutboundMsgPath(tstring& sFileLoc, const UINT nMsgID);
	const BOOL GetInboundMsgPath(tstring& sFileLoc, const UINT nMsgID);

	const UINT ReadMsgFile(const tstring& sFileLoc, _msg_file& msgfile);
	const UINT WriteMsgFile(const tstring& sFileLoc, const _msg_file& msgfile);
	CMessageIO();
	virtual ~CMessageIO();
private:
	const BOOL GenMsgPath(tstring &sFileLoc, const UINT nMsgID, tstring& sRegLoc);
	const BOOL GenAttachPath(tstring &sFileLoc, const tstring& sRegSection, const tstring &RegFolderLoc, int nVerThree = 0);
	const UINT TagExtract(const tstring& sFile, const tstring& sStart, const tstring& sEnd, tstring& sResult);
};

#endif // !defined(AFX_MESSAGEIO_H__CECD7A8E_8843_4B60_A203_18E25149F47F__INCLUDED_)
