#include "stdafx.h"
#include ".\MailFlowServer.h"

MailFlowServer::MailFlowServer(void)
{
}

MailFlowServer::~MailFlowServer(void)
{
}

bool MailFlowServer::EnsurePathExists(tstring sPath)
{
	DWORD attr;
	int pos;
	bool bRes = true;
	
	// Check for trailing slash:
	pos = sPath.find_last_of(_T("\\"));
	if (sPath.length() == pos + 1)	// last character is "\"
	{
		sPath.resize(pos);
	}
	
	// look for existing path part
	attr = GetFileAttributes(sPath.c_str());
	
	// if it doesn't exist
	if (0xFFFFFFFF == attr)
	{
		pos = sPath.find_last_of(_T("\\"));
		if (0 < pos)
		{
			// create parent dirs
			bRes = EnsurePathExists(sPath.substr(0, pos));
		}
		
		// create note
		bRes = bRes && CreateDirectory(sPath.c_str(), NULL);
	}
	else if (!(FILE_ATTRIBUTE_DIRECTORY & attr))
	{	
		// object already exists, but is not a dir
		SetLastError(ERROR_FILE_EXISTS);
		bRes = false;
	}
	
	return bRes;
}

