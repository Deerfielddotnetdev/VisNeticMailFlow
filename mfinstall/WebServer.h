///////////////////////////////////////////////////////////////////////////////
// WebServer class header file
// Written by Mark Mohr
// Created on 03/21/2002
///////////////////////////////////////////////////////////////////////////////

#ifndef WEBSERVER_H
#define WEBSERVER_H

///////////////////////////////////////////////////////////////////////////////
// Include files
#include <iadmw.h>
#include <iiscnfg.h>
#include <winerror.h>
#include <vector>
#include "olebase.h"
#include "objects.h"
#include "interface.h"
//#include "stl.h"

using namespace std;
///////////////////////////////////////////////////////////////////////////////

class CMetaRecord : public _METADATA_RECORD
{
public:
	enum{DEFAULT_SIZE = 1024};

public:
	CMetaRecord() :
		m_pBuffer(new BYTE[DEFAULT_SIZE + 1])
	{
		ZeroMemory(this,sizeof(METADATA_RECORD));
		ZeroMemory(m_pBuffer.get(),DEFAULT_SIZE + 1);

		pbMDData = m_pBuffer.get();
		dwMDDataLen = DEFAULT_SIZE;
	}

	~CMetaRecord()
	{
	}
	
	void Resize(DWORD dwSize)
	{
		pbMDData = 0;
		m_pBuffer.reset(new BYTE[dwSize + 1]);
		ZeroMemory(m_pBuffer.get(),dwSize + 1);

		pbMDData = m_pBuffer.get();
		dwMDDataLen = dwSize;
	}
private:
	std::auto_ptr<BYTE> m_pBuffer;
};

///////////////////////////////////////////////////////////////////////////////
namespace iis
{
	// WebServer declaration
	class WebServer : public SmartComObject
	{
		enum{BUF_SIZE = 256};
	public:
	// Constructor and Destructor
		WebServer();
		~WebServer();
		LPCSTR GetErrorString() { return _errString; }
		// General Server Functions
		int Close();
		int Create(const char* site);
		int Delete(const char* site);
		int DeleteAllData(const char* site);
		int EnumSites(vector<string>* sites,const char* path);
		int Open(char* key = "/LM/W3SVC/");
		int SetData(const char* site, CMetaRecord* pmdrMDData);
		int GetData(const char* site, CMetaRecord* pmdrMDData, DWORD* pdwRegDataLen);
		int ReadOnly();
		int Save();
		int WriteTo();
		// Get File Property Functions
		int GetScriptMaps(const char* site,char* scripts,const int size);
		// Get Server Property Functions
		int GetKeyType(const char* site,char* type);
		int GetServerBindings(const char* site,char* bindings);
		int GetServerComment(const char* site,char* comment);
		int GetServerState(const char* site,unsigned long& state);
		int GetIISMajorVersion(const char* site, DWORD& dwMajVer);
		// Set Server Property Functions
		int SetKeyType(const char* site,const char* type = "IIsWebServer");
		int SetServerAutoStart(const char* site,unsigned long start = 0);
		int SetServerBindings(const char* site,const char* bindings);
		int SetServerComment(const char* site,const char* comment);
		int SetServerCommand(const char* site,unsigned long command = 0);
		int SetServerSize(const char* site,unsigned long size = 1);
		int SetServerState(const char* site,unsigned long state = 0);
		int SetWin32Error(const char* site,unsigned long err = 0);
		int SetAppPoolId(const char* site,const char* val);
		// Set File Property Functions
		int SetAccessFlags(const char* site,unsigned long access = 1);
		int SetAppRoot(const char* site,const char* root);
		int SetDefaultDoc(const char* site,const char* doc);
		int SetPath(const char* site,const char* path);
		int SetScriptMaps(const char* site,const char* map);
		// Set Web Mangement Functions
		int SetAppIsolated(const char* site,unsigned long run);
		int SetAppFriendlyName(const char* site,const char* name);
	private:
		SmartObjectInterface<IMSAdminBase, &IID_IMSAdminBase>	_admin;
		METADATA_HANDLE											_localMachine;
		METADATA_RECORD											_metaRec;
		BYTE*													_pBuf;
		char													_errString[256];
	};
}
///////////////////////////////////////////////////////////////////////////////

#endif // WEBSERVER_H