///////////////////////////////////////////////////////////////////////////////
// WebServer implementation file
// Written by Mark Mohr
// Created on 03/21/2002
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Include files
#include "stdafx.h"
#include "webserver.h"

using namespace iis;
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructor and Destructor
// WebServer Constructor
WebServer::WebServer() : SmartComObject(CLSID_MSAdminBase,TRUE),
		                 _admin(SmartComObject(CLSID_MSAdminBase,TRUE))
{
	_pBuf = new BYTE[BUF_SIZE];
	FillMemory(_pBuf,BUF_SIZE,'*');
	_metaRec.pbMDData = _pBuf;
	_metaRec.dwMDDataTag = 0;

	strcpy(_errString,"");
}

// WebServer Destructor
WebServer::~WebServer()
{
	delete [] _pBuf;
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// General Server Functions
// Close Function use after opening or else.
int WebServer::Close()
{
	HRESULT hr = S_OK;

	hr = _admin->CloseKey(_localMachine);
	
	return HRESULT_CODE(hr);
}

// Create function adds a new key to web server or web site.
int WebServer::Create(const char* site)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	HRESULT hr = S_OK;
	hr = _admin->AddKey(_localMachine,webSite);

	if(FAILED(hr))
	{
		if(hr == ERROR_INVALID_PARAMETER)
		{
			strcpy(_errString, "The parameter is incorrect");
		}
		else if(hr == ERROR_ACCESS_DENIED)
		{
			strcpy(_errString,"Access is denied. Either the open handle does not have read or write permission as needed, or the user does not have sufficient permissions to perform the operation");
		}
		else if(hr == ERROR_PATH_NOT_FOUND)
		{
			strcpy(_errString,"The specified path is not found in the metabase.");
		}
		else if(hr == ERROR_DUP_NAME)
		{
			strcpy(_errString,"A key of that name already exists in the metabase.");
		}
		else if(hr == ERROR_INVALID_NAME)
		{
			strcpy(_errString,"The specified name is invalid.");
		}
		else if(hr == ERROR_NOT_ENOUGH_MEMORY)
		{
			strcpy(_errString,"Not enough storage is available to process this command.");
		}
		else
		{
			strcpy(_errString,"Unknown error from IIS Admin Object");
		}
	}

	return HRESULT_CODE(hr);
}

// Delete a key from the Web Server or Web Site
int WebServer::Delete(const char* site)
{
	//_errString

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	HRESULT hr = S_OK;
	hr = _admin->DeleteKey(_localMachine,webSite);
	if(FAILED(hr))
	{
		if(hr == E_ACCESSDENIED)
		{
			strcpy(_errString,"Access is denied. Either the open handle does not have read or write permission as needed, or the user does not have sufficient permissions to perform the operation.");
		}
		else if(hr == E_INVALIDARG)
		{
			strcpy(_errString,"The parameter is incorrect.");
		}
		else
		{
			strcpy(_errString,"The specified path is not found in the metabase.");
		}
	}

	return HRESULT_CODE(hr);
}

int WebServer::DeleteAllData(const char* site)
{
	HRESULT hr = S_OK;

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	hr = _admin->DeleteAllData(_localMachine, webSite, ALL_METADATA, ALL_METADATA);

	return HRESULT_CODE(hr);
}

// EnumSites get and stores all sites in a string vector
#pragma warning (disable : 4786)
int WebServer::EnumSites(vector<string>* sites,const char* path)
{
	WCHAR webPath[BUF_SIZE];
	WCHAR webName[BUF_SIZE];
	char  temp[BUF_SIZE];
	DWORD idx = 0;

	MultiByteToWideChar(CP_ACP,0,path,-1,webPath,BUF_SIZE);

	HRESULT hr = S_OK;

	while(SUCCEEDED(hr))
	{
		hr = _admin->EnumKeys(_localMachine,webPath,webName,idx);
		WideCharToMultiByte(CP_ACP,0,webName,-1,temp,BUF_SIZE,NULL,NULL);
		if(strlen(temp) != 0)
		{
			string site;
			site = temp;
			sites->insert(sites->end(),site);
		}
		idx++;
	}

	return HRESULT_CODE(hr);
}
#pragma warning (default : 4786)

// Open a web server or web site make sure to close it
int WebServer::Open(char* key)
{
	WCHAR webRoot[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,key,-1,webRoot,BUF_SIZE);

	HRESULT hr = S_OK;
	hr = _admin->OpenKey(METADATA_MASTER_ROOT_HANDLE,
					     webRoot,
						 METADATA_PERMISSION_READ,
						 5000,
						 &_localMachine);


	if(FAILED(hr))
	{
		if(hr == ERROR_INVALID_PARAMETER)
		{
			strcpy(_errString,"The parameter is incorrect");
		}
		else if(hr == ERROR_PATH_BUSY)
		{
			strcpy(_errString,"The path specified cannot be used at this time because a handle to the key, or one of its ancestors or descendents, is already open.");
		}
		else if(hr == ERROR_PATH_NOT_FOUND)
		{
			strcpy(_errString,"The specified path is not found.");
		}
		else
		{
			strcpy(_errString,"Unknown error from IIS Admin Object");
		}
	}

	return HRESULT_CODE(hr);
}

// ReadOnly Function
int WebServer::ReadOnly()
{
	HRESULT hr = S_OK;
	hr = _admin->ChangePermissions(_localMachine,5000,METADATA_PERMISSION_READ);
	
	return HRESULT_CODE(hr);
}

// Save Function
int WebServer::Save()
{
	HRESULT hr = S_OK;

	hr = _admin->SaveData();
	
	return HRESULT_CODE(hr);
}

int WebServer::SetData(const char* site,CMetaRecord* pmdrMDData)
{
	HRESULT hr = S_OK;

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	hr = _admin->SetData(_localMachine,webSite,pmdrMDData);

	return HRESULT_CODE(hr);
}

int WebServer::GetData(const char* site, CMetaRecord* pmdrMDData, DWORD* pdwRegDataLen)
{
	HRESULT hr = S_OK;

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	hr = _admin->GetData(_localMachine,webSite,pmdrMDData,pdwRegDataLen);

	return HRESULT_CODE(hr);
}

// WriteTo Function changes permission to enable making changes
int WebServer::WriteTo()
{
	HRESULT hr = S_OK;

	hr = _admin->ChangePermissions(_localMachine,5000,METADATA_PERMISSION_WRITE);

	return HRESULT_CODE(hr);
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Get File Property Functions
int WebServer::GetScriptMaps(const char* site,char* scripts,const int size)
{
	delete [] _pBuf;
	_pBuf = new BYTE[size];
	_metaRec.pbMDData = _pBuf;

	HRESULT hr = S_OK;
	DWORD	len = size;
	WCHAR*	webScripts = new WCHAR[size];

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SCRIPT_MAPS;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_FILE;
	_metaRec.dwMDDataType = ALL_METADATA;
	_metaRec.dwMDDataLen = len;

	hr = _admin->GetData(_localMachine,webSite,&_metaRec,&len);
	wcscpy(webScripts,(WCHAR*)_pBuf);
	WideCharToMultiByte(CP_ACP,0,webScripts,-1,scripts,size,NULL,NULL);

	delete [] _pBuf;
	_pBuf = new BYTE[BUF_SIZE];
	_metaRec.pbMDData = _pBuf;

	FillMemory(_pBuf,BUF_SIZE,'*');

	return HRESULT_CODE(hr);
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Get Server Property Functions
// GetKeyType Function
int WebServer::GetKeyType(const char* site,char* type)
{
	HRESULT hr = S_OK;
	DWORD	len = METADATA_MAX_NAME_LEN;
	WCHAR	webType[BUF_SIZE];

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_KEY_TYPE;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = ALL_METADATA;
	_metaRec.dwMDDataLen = METADATA_MAX_NAME_LEN;

	hr = _admin->GetData(_localMachine,webSite,&_metaRec,&len);
	wcscpy(webType,(WCHAR*)_pBuf);
	WideCharToMultiByte(CP_ACP,0,webType,-1,type,BUF_SIZE,NULL,NULL);
	FillMemory(_pBuf,BUF_SIZE,'*');

	return HRESULT_CODE(hr);
}

// GetServerBindings Function
int WebServer::GetServerBindings(const char* site,char* bindings)
{
	HRESULT hr = S_OK;
	DWORD	len = METADATA_MAX_NAME_LEN;
	WCHAR	webBindings[BUF_SIZE];

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SERVER_BINDINGS;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = ALL_METADATA;
	_metaRec.dwMDDataLen = METADATA_MAX_NAME_LEN;

	hr = _admin->GetData(_localMachine,webSite,&_metaRec,&len);
	wcscpy(webBindings,(WCHAR*)_pBuf);
	WideCharToMultiByte(CP_ACP,0,webBindings,-1,bindings,BUF_SIZE,NULL,NULL);
	FillMemory(_pBuf,BUF_SIZE,'*');

	return HRESULT_CODE(hr);
}

int WebServer::GetServerComment(const char* site,char* comment)
{
	HRESULT hr = S_OK;
	DWORD	len = METADATA_MAX_NAME_LEN;
	WCHAR	webComment[BUF_SIZE];

	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SERVER_COMMENT;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = ALL_METADATA;
	_metaRec.dwMDDataLen = METADATA_MAX_NAME_LEN;

	hr = _admin->GetData(_localMachine,webSite,&_metaRec,&len);
	wcscpy(webComment,(WCHAR*)_pBuf);
	WideCharToMultiByte(CP_ACP,0,webComment,-1,comment,BUF_SIZE,NULL,NULL);
	FillMemory(_pBuf,BUF_SIZE,'*');

	return HRESULT_CODE(hr);
}

int WebServer::GetServerState(const char* site,unsigned long& state)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);
	DWORD len = METADATA_MAX_NAME_LEN;

	_metaRec.dwMDIdentifier = MD_SERVER_STATE;
	_metaRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = ALL_METADATA;
	_metaRec.dwMDDataLen = len;
	
	HRESULT hr = S_OK;

	hr = _admin->GetData(_localMachine,webSite,&_metaRec,&len);

	state = _pBuf[0];

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

int WebServer::GetIISMajorVersion(const char* site, DWORD& dwMajVer)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);
	DWORD len = METADATA_MAX_NAME_LEN;

	_metaRec.dwMDIdentifier = 1101;
	_metaRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = 0;
	_metaRec.dwMDDataLen = sizeof(DWORD);
	
	HRESULT hr = S_OK;

	hr = _admin->GetData(_localMachine,webSite,&_metaRec,&len);

	memcpy(&dwMajVer,_pBuf,sizeof(DWORD));

	ZeroMemory(_pBuf,BUF_SIZE);
	return HRESULT_CODE(hr);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Set Server Property Functions
// SetKeyType function defaults to "IIsWebServer"
int WebServer::SetKeyType(const char* site,const char* type)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR iisWebServer[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,type,-1,iisWebServer,BUF_SIZE);

	DWORD len = (wcslen(iisWebServer) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_KEY_TYPE;
	_metaRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;

	memcpy(_pBuf,iisWebServer,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

int WebServer::SetAppPoolId(const char* site,const char* val)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR iisAppPool[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,val,-1,iisAppPool,BUF_SIZE);

	DWORD len = (wcslen(iisAppPool) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = 9101;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;

	memcpy(_pBuf,iisAppPool,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	ZeroMemory(_pBuf,BUF_SIZE);

	return HRESULT_CODE(hr);
}

// SetServerAutoStart Function
int WebServer::SetServerAutoStart(const char* site,unsigned long start)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SERVER_AUTOSTART;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(start);
	
	memcpy(_pBuf,&start,sizeof(start));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetServerBindings Function
int WebServer::SetServerBindings(const char* site,const char* bindings)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webBindings[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,bindings,-1,webBindings,BUF_SIZE);

	DWORD len = (wcslen(webBindings) + 1) * sizeof(WCHAR) + 2;


	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_SERVER_BINDINGS;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = MULTISZ_METADATA;
	_metaRec.dwMDDataLen = len;

	memcpy(_pBuf,webBindings,len);
	_pBuf[len - 2] = '\0';
	_pBuf[len - 1] = '\0';

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetServerCommand
int WebServer::SetServerCommand(const char* site,unsigned long command)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SERVER_COMMAND;
	_metaRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(command);
	
	memcpy(_pBuf,&command,sizeof(command));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetServerSize
int WebServer::SetServerSize(const char* site,unsigned long size)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SERVER_SIZE;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(size);
	
	memcpy(_pBuf,&size,sizeof(size));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetServerComment
int WebServer::SetServerComment(const char* site,const char* comment)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webComment[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,comment,-1,webComment,BUF_SIZE);

	DWORD len = (strlen(comment) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_SERVER_COMMENT;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;
	
	memcpy(_pBuf,webComment,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetServerState
int WebServer::SetServerState(const char* site,unsigned long state)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_SERVER_STATE;
	_metaRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(state);
	
	memcpy(_pBuf,&state,sizeof(state));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetWin32Error
int WebServer::SetWin32Error(const char* site,unsigned long err)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_WIN32_ERROR;
	_metaRec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	_metaRec.dwMDUserType = IIS_MD_UT_SERVER;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(err);
	
	memcpy(_pBuf,&err,sizeof(err));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Set File Property Functions
// SetAccessFlags
int WebServer::SetAccessFlags(const char* site,unsigned long access)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_ACCESS_PERM;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_FILE;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(access);
	
	memcpy(_pBuf,&access,sizeof(access));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

int WebServer::SetAppRoot(const char* site,const char* root)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webRoot[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,root,-1,webRoot,BUF_SIZE);

	DWORD len = (strlen(root) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_APP_ROOT;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_FILE;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;
	
	memcpy(_pBuf,webRoot,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);

}

int WebServer::SetDefaultDoc(const char* site,const char* doc)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webDoc[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,doc,-1,webDoc,BUF_SIZE);

	DWORD len = (strlen(doc) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_DEFAULT_LOAD_FILE;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_FILE;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;
	
	memcpy(_pBuf,webDoc,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetPath
int WebServer::SetPath(const char* site,const char* path)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webPath[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,path,-1,webPath,BUF_SIZE);

	DWORD len = (strlen(path) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_VR_PATH;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_FILE;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;
	
	memcpy(_pBuf,webPath,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}

// SetScriptMaps Function - This for isapi extensions
int WebServer::SetScriptMaps(const char* site,const char* map)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webMap[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,map,-1,webMap,BUF_SIZE);

	DWORD len = ((wcslen(webMap) + 1) * sizeof(WCHAR)) + 2;

	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_SCRIPT_MAPS;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_FILE;
	_metaRec.dwMDDataType = MULTISZ_METADATA;
	_metaRec.dwMDDataLen = len;

	memcpy(_pBuf,webMap,len);
	_pBuf[len - 2] = '\0';
	_pBuf[len - 1] = '\0';

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Set Web Mangement Functions
// SetAppFriendlyName Function
int WebServer::SetAppFriendlyName(const char* site,const char* name)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	WCHAR webAppName[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,name,-1,webAppName,BUF_SIZE);

	DWORD len = (strlen(name) + 1) * sizeof(WCHAR);
	HRESULT hr = S_OK;

	_metaRec.dwMDIdentifier = MD_APP_FRIENDLY_NAME;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_WAM;
	_metaRec.dwMDDataType = STRING_METADATA;
	_metaRec.dwMDDataLen = len;
	
	memcpy(_pBuf,webAppName,len);

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');

	return HRESULT_CODE(hr);
}

int WebServer::SetAppIsolated(const char* site,unsigned long run)
{
	WCHAR webSite[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,site,-1,webSite,BUF_SIZE);

	_metaRec.dwMDIdentifier = MD_APP_ISOLATED;
	_metaRec.dwMDAttributes = METADATA_INHERIT;
	_metaRec.dwMDUserType = IIS_MD_UT_WAM;
	_metaRec.dwMDDataType = DWORD_METADATA;
	_metaRec.dwMDDataLen = sizeof(run);
	
	memcpy(_pBuf,&run,sizeof(run));

	HRESULT hr = S_OK;

	hr = _admin->SetData(_localMachine,webSite,&_metaRec);

	FillMemory(_pBuf,BUF_SIZE,'*');
	return HRESULT_CODE(hr);
}
///////////////////////////////////////////////////////////////////////////////

