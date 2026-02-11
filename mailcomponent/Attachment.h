// Attachment.h: interface for the CAttachment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATTACHMENT_H__65F5225E_0300_4522_A381_84990F08DA9C__INCLUDED_)
#define AFX_ATTACHMENT_H__65F5225E_0300_4522_A381_84990F08DA9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <mimepp/mimepp.h>
#include <string>
using namespace std;

#include "EMSIDs.h"

class CAttachment  
{
public:
	CAttachment();
	virtual ~CAttachment();

	CAttachment(const CAttachment&);
	CAttachment& operator = (const CAttachment&);
	
	bool operator==(const CAttachment& a)  { return a.m_nAttachmentID == this->m_nAttachmentID; }

	void SetCharSet(const string& sCharSet)			{ m_sCharset = sCharSet; }
	void SetFileName(string sFileName);
	void SetDescription(const string& sDesrip)		{ m_sDescription = sDesrip; }
	void SetContent(const DwString& sContent)		{ m_sContent = sContent; }
	void SetFullPath(const tstring& sPath)			{ m_sFullPath = sPath; }
	void SetVirusScanState(const UINT nVal)			{ m_nVirusScanState = nVal; }
	void SetAttachmentID(const UINT nID)			{ m_nAttachmentID = nID; }
	void SetContentDisposition(const tstring& sVal) { m_sContentDisposition = sVal; }
	void SetVirusName(const tstring& sVal)			{ m_sVirusName = sVal; }
	void SetMediaType(const string& sType, const string& sSubType) 
													{
														m_sType = sType;
														m_sSubtype = sSubType;
													}
	void SetContentID(const string& sCID)			{ m_sContentID = sCID; }

	void SetIsInbound(const unsigned char IsInbound){ m_nIsInbound = IsInbound; }

	const int GetSize()	const						{ return m_sContent.size(); }
	const string& GetCharSet() const				{ return m_sCharset; }
	const string& GetFileName() const				{ return m_sFileName; }
	const string& GetDescription() const			{ return m_sDescription; }
	const string& GetType()	const					{ return m_sType; }
	const string& GetSubType() const				{ return m_sSubtype; }
	const DwString& GetContent() const				{ return m_sContent; }
	const tstring& GetFullpath() const				{ return m_sFullPath; }
	const UINT GetVirusScanState() const			{ return m_nVirusScanState; }
	const UINT GetAttachmentID() const				{ return m_nAttachmentID; }
	const tstring& GetContentDisposition() const	{ return m_sContentDisposition; }
	const tstring& GetVirusName()					{ return m_sVirusName; }
	const string& GetContentID() const					{ return m_sContentID; }
	const unsigned char& GetIsInbound()				{ return m_nIsInbound;	}

private:
	string m_sType;
    string m_sSubtype;
    string m_sCharset;
    string m_sFileName;
    string m_sDescription;
	string m_sContentID;
	tstring m_sFullPath;
	tstring m_sContentDisposition;
	UINT m_nVirusScanState;
    DwString m_sContent;
	UINT m_nAttachmentID; // database id
	tstring m_sVirusName;
	unsigned char m_nIsInbound;	// Is attachment in Inbound or Outbound attachments folder?	
};

#endif // !defined(AFX_ATTACHMENT_H__65F5225E_0300_4522_A381_84990F08DA9C__INCLUDED_)
