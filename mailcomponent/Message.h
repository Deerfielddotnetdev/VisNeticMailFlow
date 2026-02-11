// Message.h: interface for the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGE_H__99311BE4_5BD8_464E_AEE3_1A41573E4993__INCLUDED_)
#define AFX_MESSAGE_H__99311BE4_5BD8_464E_AEE3_1A41573E4993__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;

#include <mimepp/mimepp.h>
#include "Attachment.h"
#include "EmailAddressList.h"
#include "EmailAddress.h"
#include "AttachmentList.h"
#include "EmailText.h"
#include "AttachmentList.h"
#include "EmailDate.h"
#include <CkString.h>
//#include "VirusScanner.h"
#include "StdAfx.h"	// Added by ClassView


class CMessage  
{
public:
	CMessage();
	virtual ~CMessage();

public:
	tstring GetToListString();
	tstring GetCcListString();
	tstring GetReplyToListString();
	const tstring GetAddressListString(CEmailAddressList& addrList);
	int ParseFrom(const DwString& aMessageString, const bool bParseAttachments = true, const bool bDateFilters = false, vector<DateFilters_t> m_df = std::vector<DateFilters_t>());
	void SerializeToString(DwString& aMessageString, BOOL b8bitmime = FALSE);

	CEmailAddressList& GetToRecipients()		{ return m_ToRecipients; }
	CEmailAddressList& GetCCRecipients()		{ return m_CcRecipients; }
	CEmailAddressList& GetBCCRecipients()		{ return m_BccRecipients; }
	CEmailAddressList& GetReplyToList()			{ return m_ReplyTo; }
	CEmailText& GetSubject()					{ return m_Subject; }
	CEmailText& GetMailer()						{ return m_Mailer; }
	CEmailText& GetInReplyTo()					{ return m_InReplyTo; }
	CEmailText& GetReferences()					{ return m_References; }
	CEmailText& GetMsgID()						{ return m_MsgID; }
	CAttachmentList& GetAttachments()			{ return m_Attachments; }
	CEmailDate& GetDate()						{ return m_Date; }
//	CEmailDate& GetReceivedDate()				{ return m_ReceivedDate; }
	CEmailAddress& GetOriginator()				{ return m_Originator; }
	CEmailAddress& GetReadReceiptTo()			{ return m_ReadReceiptTo; }
	CEmailAddressList& GetReturnPathList()		{ return m_ReturnPath; }
	CEmailText& GetMemoText()					{ return m_MemoText; }
	CEmailText& GetPopHeaders()					{ return m_PopHeaders; }
	const int GetMemoVirusScanState()			{ return m_nMemoVirusScanState; }
	const tstring GetVirusName()				{ return m_sVirusName; }
	const LONG GetMessageID()					{ return m_lMessageID; }
	const LONG GetPriorityID()					{ return m_lPriorityID; }
	const int GetTicketID()						{ return m_nTicketID; }
	const int GetReplyToMsgID()					{ return m_nReplyToMsgID; }
	CEmailAddress& GetReceivedFor()				{ return m_ReceivedFor; }
	LPCTSTR GetMsgFilePath() const				{ return m_sMsgFilePath.c_str(); }
	const unsigned char GetFooterLoc()			{ return m_FooterLocation; }
	const unsigned char GetReadReceipt()		{ return m_ReadReceipt; }
	const unsigned char GetDeliveryConf()		{ return m_DeliveryConfirmation; }

	void SetSubject(const CEmailText& sVal)		{ m_Subject = sVal; }
	void SetMailer(const CEmailText& sVal)		{ m_Mailer = sVal; }
	void SetInReplyTo(const CEmailText& sVal)	{ m_InReplyTo = sVal; }
	void SetReferences(const CEmailText& sVal)	{ m_References = sVal; }
	void SetMsgID(const CEmailText& sVal)		{ m_MsgID = sVal; }
	void SetMemoText(const CEmailText& sVal)	{ m_MemoText = sVal; }
	void SetDate(const CEmailDate& date)		{ m_Date = date; }
	void SetMemoVirusScanState(const int nVal)	{ m_nMemoVirusScanState = nVal; }
	void SetVirusName(const tstring& sVal)		{ m_sVirusName = sVal; }
	void SetMessageID(const LONG lVal)			{ m_lMessageID = lVal; }
	void SetPriorityID(const LONG lVal)			{ m_lPriorityID = lVal; }
	void SetTicketID(const int nVal)			{ m_nTicketID = nVal; }
	void SetFooterLoc(const unsigned char nVal)	{ m_FooterLocation = nVal; }
	void SetReadReceipt(const unsigned char nVal)	{ m_ReadReceipt = nVal; }
	void SetDeliveryConfirmation(const unsigned char nVal)	{ m_DeliveryConfirmation = nVal; }
	void SetReplyToMsgID(const int nVal)		{ m_nReplyToMsgID = nVal; }
	void SetMsgFilePath(LPCTSTR lpFilePath)		{ m_sMsgFilePath = lpFilePath; }

	static UINT GetCodePage( string charset );
	static char* GetSubjectCharset(const char *text);	
	
protected:
	CEmailAddress m_Originator;					// message originator
	CEmailAddress m_ReadReceiptTo;				// from disposition-notification-to header
	CEmailAddressList m_ToRecipients;			// to recipients
	CEmailAddressList m_CcRecipients;			// CC recipients
	CEmailAddressList m_BccRecipients;			// BCC recipients
	CEmailAddressList m_ReplyTo;				// reply-to list
	CEmailAddressList m_ReturnPath;				// return-path list
	CAttachmentList m_Attachments;				// attachment list
	CEmailDate m_Date;							// sent date
	CEmailDate m_ReceivedDate;					// received date
    CEmailText m_Subject;						// subject
	CEmailText m_Mailer;						// mailer
	CEmailText m_InReplyTo;						// inreplyto
	CEmailText m_References;					// references
	CEmailText m_MsgID;							// messageid
	CEmailText m_PopHeaders;					// POP3 message headers (untouched)
    CEmailText m_MemoText;						// memo text
	CEmailAddress m_ReceivedFor;				// original addressee
	tstring m_sMsgFilePath;						// message body file path
	int m_nMemoVirusScanState;					// message body virus scan state
	tstring m_sVirusName;						// virus name (hopefully empty)
	LONG m_lMessageID;							// message id (database)
	LONG m_lPriorityID;							// message priority id (database)
	int m_nTicketID;							// ticket id (database)
	int m_nTicketCategoryID;					// ticket category id (database)
	int m_nReplyToMsgID;						// reply to msg id
	unsigned int m_nUuencodeInText;
	size_t m_nUuencodeStartPos;
	unsigned char m_FooterLocation;
	unsigned char m_ReadReceipt;
	unsigned char m_DeliveryConfirmation;
	int m_nSubjectEncoding;
	int m_nSubjectCharset;
	int m_nBodyEncoding;
	int m_nBodyCharset;

    virtual void ParseOriginator(DwMessage& message, CEmailAddress& address);
	virtual void ParseReceivedFor(DwMessage& message, CEmailAddress& address);
    virtual void ParseRecipients(DwMessage& message, const string& which, CEmailAddressList& addresses);
    virtual void ParseSubject(DwMessage& message, CEmailText& subject);
    virtual void ParseInReplyTo(DwMessage& message, CEmailText& inreplyto);
    virtual void ParseReferences(DwMessage& message, CEmailText& references);
    virtual void ParseMsgID(DwMessage& message, CEmailText& messageid);
    virtual void ParseMemoText(DwMessage& message, CEmailText& memoText);
    virtual int ParseAttachments(DwMessage& message, CAttachmentList& list);
	virtual void ParseReadReceiptTo(DwMessage& message, CEmailAddress& address);
	virtual void ParseMailer(DwMessage& message, CEmailText& mailer);
    
	virtual int ParsePriority(DwMessage& message, LONG& lPriorityID);

	virtual void ParseDate(const DwMessage& message, CEmailDate& edate);
	
    virtual void ParseMemoText_Text(DwMessage& message, CEmailText& text);
    virtual void ParseMemoText_Message(DwMessage& message, CEmailText& text);
    virtual void ParseMemoText_Multipart(DwMessage& message, CEmailText& text);
	
    virtual int ParseAttachments_Simple(DwMessage& message, CAttachmentList& list);
    virtual void ParseAttachments_Message(DwMessage& message, CAttachmentList& list);
    virtual int ParseAttachments_Multipart(DwMessage& message, CAttachmentList& list);

	void ResolveContentDepends();

	void WrapLongLines(int aMaxLineLen, DwString& aStr);

	bool AddQuotesToRealNames( DwString& fBody );
	int GetNumberDaysFromNow( dca::String sResults, string sValue );
	static char* SearchQuote(const char **ptr);
	static char* GetSubjectEncoding(const char *text);
	
};

#endif // !defined(AFX_MESSAGE_H__99311BE4_5BD8_464E_AEE3_1A41573E4993__INCLUDED_)
