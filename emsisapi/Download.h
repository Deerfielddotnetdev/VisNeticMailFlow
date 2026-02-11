/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/Download.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||
||                                         
||  COMMENTS:   Downloads Message Attachments
||              
\\*************************************************************************/

// only include once
#pragma once

#include "XMLDataClass.h"

class CDownload : public CXMLDataClass  
{
public:
	CDownload( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData ) {};
	virtual ~CDownload() {};

	virtual int Run(CURLAction& action);

private:
	void DownloadByAttachmentID( int nID, CURLAction& action );
	void DownloadByFilename( tstring& sFileName, CURLAction& action );
	void DownloadByContentID( tstring& sContentID, int nMsgID, 
							  int nInbound, CURLAction& action );
	int GetAttachFromArchive( int nArchiveID, CURLAction& action );
	void DownloadByCID( tstring& sContentID, CURLAction& action );
	void DownloadByLogName( tstring& sLogName, CURLAction& action );
	
};
