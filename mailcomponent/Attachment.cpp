// Attachment.cpp: implementation of the CAttachment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Attachment.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAttachment::CAttachment()
{
	m_nAttachmentID = 0;
	m_nVirusScanState = EMS_VIRUS_SCAN_STATE_NOT_SCANNED;
}

CAttachment::CAttachment(const CAttachment& attach)
:	m_sType(attach.m_sType),
	m_sSubtype(attach.m_sSubtype),
	m_sFileName(attach.m_sFileName),
	m_sDescription(attach.m_sDescription),
	m_sContent(attach.m_sContent),
	m_sCharset(attach.m_sCharset),
	m_nAttachmentID(attach.m_nAttachmentID),
	m_nVirusScanState(attach.m_nVirusScanState),
	m_sContentDisposition(attach.m_sContentDisposition),
	m_sFullPath(attach.m_sFullPath),
	m_sVirusName(attach.m_sVirusName),
	m_sContentID(attach.m_sContentID),
	m_nIsInbound(attach.m_nIsInbound)
{
}

CAttachment& CAttachment::operator=(const CAttachment& attach)
{
    if (this != &attach)
	{
        m_sFileName		= attach.m_sFileName;
        m_sType			= attach.m_sType;
        m_sSubtype		= attach.m_sSubtype;
        m_sDescription	= attach.m_sDescription;
        m_sContent		= attach.m_sContent;
		m_sCharset		= attach.m_sCharset;
		m_nAttachmentID = attach.m_nAttachmentID;
		m_nVirusScanState = attach.m_nVirusScanState;
		m_sContentDisposition = attach.m_sContentDisposition;
		m_sFullPath = attach.m_sFullPath;
		m_sVirusName = attach.m_sVirusName;
		m_sContentID = attach.m_sContentID;
		m_nIsInbound = attach.m_nIsInbound;
    }
	
    return *this;
}

CAttachment::~CAttachment()
{

}

void CAttachment::SetFileName(string sFileName)
{
	// Check for illegal characters within the filename

	// Fixed by Mark Mohr on 9/29/2006.  - Tab is also invalid and we were not checking for it.
	// this caused problems when creating a file with a filename that had a tab in it.
	const char* badchars = "\\/:*?\"<>|\t\r\n";

	string sfcopy(sFileName);
	sFileName = "";
	for (size_t n=0; n < sfcopy.size(); n++)
	{
		bool badchar = false;
		for (size_t i=0; i < strlen(badchars); i++)
		{
			if (sfcopy[n] == badchars[i])
			{
				badchar = true;
				break;
			}
		}

		if (!badchar)				
			sFileName.push_back(sfcopy[n]);
	}

	// isolate the filename from the extension
	char fname[_MAX_FNAME] = {0};
	char fext[_MAX_EXT] = {0};
	_splitpath(sFileName.c_str(), NULL, NULL, fname, fext);

	// if there's nothing left after we've done our illegal character
	// stripping, then assign "unknown" filename
	if (strlen(fname) == 0)
	{
		string sfname("unknown");
		sFileName = sfname;
		strlen(fext) > 1 ? sFileName.append(fext) : sFileName.append(".ext");
	}
	// otherwise check against max len
	else if (strlen(fname) > _MAX_FNAME)
	{
		string sfname(fname);
		sfname = sfname.substr(0, _MAX_FNAME - 1);
		sFileName = sfname;
		sFileName.append(fext);
	}

	m_sFileName = sFileName;
}
