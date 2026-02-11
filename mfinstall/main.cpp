//*****************************************************************************
//
// MailFlowInstall.DLL - A helper dll for Wise Installer
// Property of Deerfield.com (copyright 2002)
//
// Written by Mark Mohr on 03/19/2002
// main.cpp - Implementation file
//
//*****************************************************************************

//*****************************************************************************

// Header files and other declarations
#pragma warning ( disable : 4786 )

#include "stdafx.h"
#include "main.h"
#include "DBConn.h"
#include "LogIt.h"
#include "iwamreg.h"

#include "MailStreamEngine.h"

static BOOL g_bFatalError = FALSE;

//******************************************************************************

HINSTANCE g_hInst = 0;

typedef vector<string> SITES;
typedef list<string> SERVERS;
typedef char String[METADATA_MAX_NAME_LEN];
typedef BOOL (WINAPI* IsWow64Process_type)(HANDLE hProcess, PBOOL Wow64Process);


//32DCD893-06C9-4FF7-B1C6-5C4D913BCD35
static const IID IID_IRoutingEngineAdmin = 
{
	0x32DCD893, 0x06C9, 0x4FF7, 
    { 0xB1, 0xC6, 0x5C, 0x4D, 0x91, 0x3B, 0xCD, 0x35 }
};

const wchar_t IIS_APPLICATION_POOL_W[] = L"IIsApplicationPool";
const wchar_t JMF_TYPE[] = L".jmf,application/x-javascript";
const wchar_t VMF_TYPE[] = L".vmf,application/x-vbscript";
const wchar_t OLD_DLL_EXT[] = L"0,*.dll";
const wchar_t NEW_DLL_EXT[] = L"1,*.dll";
const char VMF_ISAPI_DLL[] = "1,%s,1,,VisNetic MailFlow";

//******************************************************************************


//******************************************************************************
//
// DllMain
//
//******************************************************************************

BOOL WINAPI DllMain(HINSTANCE hInst,DWORD reason,void** pResv)
{
	try
	{
		switch(reason)
		{
		case DLL_PROCESS_ATTACH:
			g_hInst = hInst;
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
		return TRUE;
	}
	catch(...)
	{
		::MessageBoxA(0,"Unknown Exception in MailFlowInstall.dll",
			            "Critical Exception",MB_OK | MB_ICONEXCLAMATION);
	}
}

//******************************************************************************
//
// External functions
//
//******************************************************************************


#pragma warning ( default : 4101 )



#pragma warning (default : 4786)

///////////////////////////// Internal functions //////////////////////////////

//******************************************************************************
//
// CreateDatabase functions
//
//******************************************************************************

void CreateDatabase(CDatabase& database, dca::SAXDBTABLELIST& tableList)
{

	if( g_bFatalError == FALSE )
	{
		if( wcsicmp( database.m_wcsType, L"MSSQL" ) == 0 )
		{
			if( CreateMSSQLDatabase( database, tableList) != 0 )
			{
				g_bFatalError = TRUE;
			}
		}
		/*else if( wcsicmp( database.m_wcsType, L"MySQL" ) == 0 )
		{
			if( CreateMySQLDatabase( database, tableList)!= 0 )
			{
				g_bFatalError = TRUE;
			}
		}*/
		else
		{
			throw "Unknown database type, ignoring...";
		}
	}
}

//******************************************************************************
//
// GetAppRoot functions
//
//******************************************************************************

void GetAppRoot(const char* site,char* appPath)
{
	char temp[METADATA_MAX_NAME_LEN];
	ResString rootDir(g_hInst,IDS_ROOT_DIR);

	strcpy(temp,"/LM/W3SVC/");
	strcat(temp,site);
	strcat(temp,"/");
	strcat(temp,rootDir);
	
	strcpy(appPath,temp);
}

//******************************************************************************
//
// GetHexFromChar functions
//
//******************************************************************************

int GetHexFromChar(char c)
{
	switch(c)
	{
	case 48:
		return 0;
	case 49:
		return 1;
	case 50:
		return 2;
	case 51:
		return 3;
	case 52:
		return 4;
	case 53:
		return 5;
	case 54:
		return 6;
	case 55:
		return 7;
	case 56:
		return 8;
	case 57:
		return 9;
	case 65:
	case 97:
		return 10;
	case 66:
	case 98:
		return 11;
	case 67:
	case 99:
		return 12;
	case 68:
	case 100:
		return 13;
	case 69:
	case 101:
		return 14;
	case 70:
	case 102:
		return 15;
	default:
		return -1;
	}

}

//******************************************************************************
//
// GetIsapiMap1 functions
//
//******************************************************************************

void GetIsapiMap1(const char* lnPath,char* mapName)
{
	char temp[METADATA_MAX_NAME_LEN];
	char shPath[METADATA_MAX_NAME_LEN];
	//ResString lnPath(g_hInst,IDS_ISAPI_MAP1);
	ResString ext(g_hInst,IDS_EXT1);
	ResString verbs(g_hInst,IDS_VERBS1);

	int rc = GetShortPathNameA(lnPath,shPath,METADATA_MAX_NAME_LEN);

	strcpy(temp,ext);
	strcat(temp,",");
	if(rc != 0)
	{
		strcat(temp,shPath);
	}
	else
	{
		strcat(temp,lnPath);
	}
	strcat(temp,",1,");
	strcat(temp,verbs);

	strcpy(mapName,temp);
}

//******************************************************************************
//
// GetIsapiMap2 functions
//
//******************************************************************************

void GetIsapiMap2(const char* lnPath,char* mapName)
{
	char temp[METADATA_MAX_NAME_LEN];
	char shPath[METADATA_MAX_NAME_LEN];
	//ResString lnPath(g_hInst,IDS_ISAPI_MAP1);
	ResString ext(g_hInst,IDS_EXT1);

	int rc = GetShortPathNameA(lnPath,shPath,METADATA_MAX_NAME_LEN);

	strcpy(temp,ext);
	strcat(temp,",");
	if(rc != 0)
	{
		strcat(temp,shPath);
	}
	else
	{
		strcat(temp,lnPath);
	}
	strcat(temp,",1");

	strcpy(mapName,temp);
}

void GetIsapiMap3(const char* lnPath, char* mapName)
{
	char temp[METADATA_MAX_NAME_LEN];
	//ResString lnPath(g_hInst,IDS_ISAPI_MAP1);
	ResString ext(g_hInst,IDS_EXT1);

	strcpy(temp,ext);
	strcat(temp,",\"");
	strcat(temp,lnPath);
	strcat(temp,"\",1");

	strcpy(mapName,temp);
}

//******************************************************************************
//
// GetRootPath functions
//
//******************************************************************************

void GetRootPath(const char* site,char* rootPath)
{
	char temp[METADATA_MAX_NAME_LEN];
	ResString rootDir(g_hInst,IDS_ROOT_DIR);

	strcpy(temp,site);
	strcat(temp,"/");
	strcat(temp,rootDir);

	strcpy(rootPath,temp);
}

int HasValidCharacters(char* in)
{
	int len;
	len = strlen(in);
	for(int i = 0; i < len; i++)
	{
		if(in[i] == '\"')
		{
			return 0;
		}
	}
	return 1;
}

//******************************************************************************
//
// MakeHex functions
//
//******************************************************************************

int MakeHex(char* hex)
{
	int len;
	len = strlen(hex);
	if(len < 2)
	{
		return GetHexFromChar(hex[0]);
	}
	else if(len == 2)
	{
		int hex1 = GetHexFromChar(hex[0]);
		int hex2 = GetHexFromChar(hex[1]);

		int temp1 = hex1 * 16;
		temp1 = temp1 + hex2;
		return temp1;
	}
	else
	{
		return -1;
	}
}

//******************************************************************************
//
// ShiftText functions
//
//******************************************************************************

void ShiftText(char* in,char* out) 
{ 
	int ch; 
	int i;
	int len;

	len = strlen(in);

	for(i = 0;i < len;i++)
	{
		ch = in[i];
		out[i] = (ch ^ 73) + 64;
	}
	out[i] = '\0';
} 

////////////////////////// Fixed External Functions ///////////////////////////

//******************************************************************************
// ChangeDBOwner function 
//******************************************************************************

int ChangeDBOwner(char* dsn, char* type, char* user,char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wuser[256];
	wchar_t  cmd[256];
	char buf[256];

	try
	{
		// Variables
		CDBConn dbConn;

		// Change DSN over to WCHAR for CDBConn connection string.
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,user,-1,wuser,256);

	
		// Open log file and print function header
		logIt.Line();
		logIt.Date();
		logIt.Write("* ChangeDBOwner function");

		// Connect to database
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			// If successfull
			logIt.Write("+ Successfully connected to database server");
			wsprintfW(cmd, L"exec sp_changedbowner '%s', TRUE", wuser);

			// Attempt to change dbowner
			logIt.Write("* Attempting to change database owner");

			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				// If it fails
				logIt.Write("- Failed to changed database owner");
				dca::String e(dbConn.GetErrorString());
				dca::String tCmd(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), tCmd.c_str());

				strcpy(errString,buf);

				// Write error to log file
				sprintf(buf, "- Error : %s", e.c_str());
				logIt.Write(buf);

				// Disconnect from database
				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				dca::WString dbName(L"VisNetic MailFlow");
				try{
					dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
					lmrk.QueryStringValue(L"DBNAME", dbName);
				}
				catch(...){
				}
				dca::String d(dbName.c_str());

				// Successfully changed
				logIt.Write("+ Successfully changed database owner");
				wsprintfW(cmd, L"exec sp_defaultdb '%s','%s'",wuser, dbName.c_str());

				// Assign user to database
				logIt.Write("* Attempting to assign database to user");
				ret = dbConn.ExecuteSQL(cmd);
				if( !SQL_SUCCEEDED( ret ) )
				{
					// If it fails
					logIt.Write("- Failed to changed database owner");
					dca::String e(dbConn.GetErrorString());
					dca::String tCmd(cmd);
					sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),tCmd.c_str());

					strcpy(errString,buf);

					// Write error to log file
					wsprintfA(buf,"- Error : %s", e.c_str());
					logIt.Write(buf);

					dbConn.Disconnect();

					logIt.LineBreak();
					logIt.Line();
					return 1;
				}
				else
				{
					// If it succeeds
					logIt.Write("+ Successfully assigned database to user");
					strcpy(errString,"");
				}	
			}
			// Close connection
			dbConn.Disconnect();
		}
		else
		{
			// Failed to connect to database
			logIt.Write("- Failed to connect to the database server");
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
		
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s", e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		// Success
		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function ChangeDBOwner";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// ChangeServiceToAuto function 
//******************************************************************************

int ChangeServiceToAuto(char* errString)
{
	try
	{
		SC_HANDLE	scHandle;
		SC_HANDLE	scService;
		SC_LOCK		scLock;
		ResString	servName(g_hInst,IDS_SERVICE);

		// Open a handle to the SC Manager database. 
		scHandle = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
 
		if(scHandle == NULL)
		{
			throw "Open Service Control Manager Failed";
		}

		// Lock Service Database
		scLock = LockServiceDatabase(scHandle);

		if(scLock == NULL)
		{
			CloseServiceHandle(scHandle);
			throw "Lock Service Database Failed";
		}

		// Open a handle to the service. 
		scService = OpenServiceA(scHandle,servName,SERVICE_CHANGE_CONFIG);
		if(scService == NULL)
		{
			UnlockServiceDatabase(scLock);
			CloseServiceHandle(scHandle);
			throw "Open Service Failed";
		}

		if (!ChangeServiceConfig(scService,SERVICE_NO_CHANGE,SERVICE_AUTO_START,
								 SERVICE_NO_CHANGE,NULL,NULL,NULL,NULL,NULL,NULL,NULL))
		{
			UnlockServiceDatabase(scLock);
			CloseServiceHandle(scHandle);
			throw "Failed to Changed Service";
		}

		// Cleanup
		UnlockServiceDatabase(scLock);
		CloseServiceHandle(scHandle);
		strcpy(errString,"");
		return 0;
	}
	catch(char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception occurred in the function ChangeServiceToAuto");
	}
	return 1;
}

//******************************************************************************

//******************************************************************************
// ConfigDataBase function 
//******************************************************************************

int ConfigDataBase(char* file, char* dsn, char* type, char* errString)
{
	LogFile logIt;
	logIt.Line();
	logIt.Date();
	// Stamping it create because that is what it really does.
	// Ignore the old function.
	logIt.Write("* CreateDatabase Function with DSN using xml schema [ %s ]", file);

	try
	{
		dca::UseCom useCom;

		dca::WString dbName(L"VisNetic MailFlow");
		try{
			dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
			lmrk.QueryStringValue(L"DBNAME", dbName);			
		}
		catch(...){
		}

		wchar_t wDBName[255];
		wsprintfW(wDBName, L"[%s]",dbName.c_str());

				
		WCHAR waction[] = L"CREATE";

		dca::WString wdsn(dsn);
		dca::WString wtype(type);

		logIt.Write("* CreateDatabase Function setting DBNAME to  [ %s ]", dbName.c_str());

		CDatabase db(wDBName, wcslen(wDBName));

		db.SetDSN(wdsn.c_str(), wdsn.size());
		db.SetType(wtype.c_str(), wtype.size());
		db.SetAction(waction,wcslen(waction));

		{
			dca::SAXDBTABLELIST tableList;
			HRESULT hr = S_OK;

			std::auto_ptr<dca::SAXDBContentHandler> pContent(new dca::SAXDBContentHandler(tableList));

			{
				dca::SmartComObject comSaxReader(CLSID_SAXXMLReader);
				dca::SmartComInterface<ISAXXMLReader, IID_ISAXXMLReader> iSaxReader(comSaxReader);

				iSaxReader->putContentHandler(pContent.get());

				logIt.Write("* Successfully intialized XML Parser");
				SAXErrorHandlerImpl Ec;
				iSaxReader->putErrorHandler(&Ec);

				dca::BString bURL(file);
			
				hr = iSaxReader->parseURL(bURL);

				if( hr != S_OK)
				{
					logIt.Write("- Failed to parse document");
					throw "Failed to parse document";
				}
			}
			
			logIt.Write("* Successfully parsed document");

			if( hr == S_OK )
			{
				//tableList.sort();

				logIt.Write("* Creating database");
				CreateDatabase(db, tableList);
			}
		}
		
		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,"");
		return 0;
	}
	catch(char* msg)
	{
		logIt.Write(msg);
		strcpy(errString,msg);
	}
	catch(dca::Exception e)
	{
		dca::String err(e.GetMessage());
		logIt.Write(err.c_str());
	}
	catch(...)
	{
		char errBuf[] = "Unknown Exception Occurred in function ConfigDataBase";
		strcpy(errString,errBuf);
		logIt.Write(errBuf);
	}

	logIt.LineBreak();
	logIt.Line();
	return 1;
}

//******************************************************************************

//******************************************************************************
// CreateDBUser function 
//******************************************************************************

int CreateDBUser(char* dsn, char* type, char* user, char* pass, char* errString)
{
	// Create log file object and write function name
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wuser[256];
	WCHAR wpass[256];
	wchar_t  cmd[256];
	char buf[256];

	try
	{
		logIt.Line();
		logIt.Date();
		logIt.Write("* CreateDBUser function");

		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,user,-1,wuser,256);
		MultiByteToWideChar(CP_ACP,0,pass,-1,wpass,256);

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			wsprintfW(cmd, L"exec sp_addlogin '%s', '%s'",wuser,wpass);
			logIt.Write("* Attempting to create database user");
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				logIt.Write("- Failed to create database user");
				logIt.Write(buf);
				strcpy(errString,buf);
				dbConn.Disconnect();
				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully created database user");
				strcpy(errString,"");
			}
			dbConn.Disconnect();
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			logIt.Write("- Failed to connect to the database");
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function CreateDBUser";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// CreateWeb function 
//******************************************************************************
#pragma warning (disable:4786)
int CreateWeb(char* siteNum,char* bindings,char* webDir,char* isapiPath,int reset,int isVirt,char* errString,int servExt)
{
	int		err;
	char	virSitePath[256];
	char	errBuf[256];
	DWORD   dwMajVer = 0;

	wsprintfA(errBuf,"\nSiteNum = %s\nBindings = %s\nWebDir = %s\nIsapiPath = %s\nReset = %d\nIsVirt = %d\n",siteNum,bindings,webDir,isapiPath,reset,isVirt);
	dca::WString d(errBuf);
	DcaTrace(d.c_str());

	try
	{
		UseOle			useOle;				// Initialiaze COM
		SITES			iisSites;			// List variable to hold our sites
		iis::WebServer	webServ;			// our webserver object
		String			rootPath;			// to hold our root path
		String			appRoot;			// to hold our application path
		String			map;				// to hold our map path
		Win::SysInfo	si;					// system info object

		// Get values from resource
		ResString comment(g_hInst,IDS_WEB_COMMENT);
		ResString rootDir(g_hInst,IDS_ROOT_DIR);
		ResString defDoc(g_hInst,IDS_DEFAULT_DOC);

		// Is this a virtual site
		if(isVirt == 0)
		{
			// If not then get our root path and our apppath
			GetRootPath(siteNum,rootPath);
			GetAppRoot(siteNum,appRoot);
		}

		// Open our web server
		err = webServ.Open();
		if(err != 0)
		{
			wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
			dca::WString e(errBuf);
			DcaTrace(e.c_str());
			throw errBuf;
		}

		webServ.GetIISMajorVersion("/INFO",dwMajVer);

		// What is our NT version so we can get the appropiate mapping
		int ver = si.GetVersion();
		if(ver < 6)
		{
			GetIsapiMap1(isapiPath,map);
		}
		else
		{
			if(dwMajVer < 6)
			{
				GetIsapiMap2(isapiPath,map);
			}
			else
			{
				GetIsapiMap3(isapiPath,map);
			}
		}

		// Change access value so we will be able to write to web server
		webServ.WriteTo();

		// Test to see if we are creating a virtual web site
		if(isVirt == 1)
		{
			strcpy(virSitePath,"1/ROOT/MailFlow");

			dca::WString v(virSitePath);
			DcaTrace(L"Attempting to create virtual web site %s\n", v.c_str());
	
			// If so then try to create it
			err = webServ.Create(virSitePath);
			if(err != ERROR_SUCCESS)
			{
				// If it failed and we are not resetting it up then error out
				if(reset == 0)
				{
					webServ.Close();
					wsprintfA(errBuf,"Failed to create the web site with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
					dca::WString e(errBuf);
					DcaTrace(e.c_str());
					throw errBuf;
				}
			}
		}
		else
		{
			// Else try to create web server
			err = webServ.Create(siteNum);
			if(err != ERROR_SUCCESS)
			{
				if(reset == 0)
				{
					// If it failed and we are not resetting it up then error out
					webServ.Close();
					wsprintfA(errBuf,"Failed to create the web site with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
					dca::WString e(errBuf);
					DcaTrace(e.c_str());
					throw errBuf;
				}
			}
		}

		// "Setting Defaults for non virtual web server
		if(isVirt == 0)
		{
			webServ.SetKeyType(siteNum);
			webServ.SetServerCommand(siteNum);
			webServ.SetServerState(siteNum);
			webServ.SetServerAutoStart(siteNum,1);
			webServ.SetServerSize(siteNum);
			webServ.SetWin32Error(siteNum);
			webServ.SetServerComment(siteNum,comment);
			webServ.SetServerBindings(siteNum,bindings);

			if(dwMajVer >= 6)
			{
				webServ.SetAppPoolId(siteNum,"VisNetic MailFlow");
			}
		}

		//Closing connection to Web Server
		webServ.Close();

		// Set Web Application up
		if(isVirt == 0)
		{
			// Setup WAM
			if(dwMajVer < 6)
			{
				SmartComObject wamObj(CLSID_WamAdmin,TRUE);
				SmartObjectInterface<IWamAdmin,&IID_IWamAdmin> wamAdmin(wamObj);
				
				char cSite[256];
				wsprintfA(cSite,"/LM/W3SVC/%s/ROOT",siteNum);

				WCHAR wSite[256];
				MultiByteToWideChar(CP_ACP,0,cSite,-1,wSite,256);

				HRESULT hr = wamAdmin->AppCreate(wSite,TRUE);
				if(FAILED(hr))
					throw "Failed to create Web Application";
			}
			else
			{
				// Now making a connection to Web Service for App Pool
				err = webServ.Open();
				if(err != 0)
				{
					wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
					dca::WString e(errBuf);
					DcaTrace(e.c_str());
					throw errBuf;
				}

				webServ.Create("/AppPools/VisNetic MailFlow");

				webServ.Close();

				err = webServ.Open("/LM/W3SVC/AppPools");
				if(err != 0)
				{
					wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
					dca::WString e(errBuf);
					DcaTrace(e.c_str());
					throw errBuf;
				}

				webServ.WriteTo();

				webServ.DeleteAllData("/VisNetic MailFlow");

				CMetaRecord mr;
				mr.dwMDUserType = IIS_MD_UT_SERVER;
				mr.dwMDIdentifier = 1002;
				mr.dwMDDataType = STRING_METADATA;
				memcpy(mr.pbMDData,IIS_APPLICATION_POOL_W,(wcslen(IIS_APPLICATION_POOL_W) * sizeof(wchar_t)));
				mr.dwMDDataLen = (DWORD)(wcslen(IIS_APPLICATION_POOL_W) * sizeof(wchar_t));
				mr.dwMDDataLen += 2;

				webServ.SetData("/VisNetic MailFlow",&mr);

				DWORD dwVal = 2;
				mr.dwMDIdentifier = 9027;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				memcpy(mr.pbMDData,&dwVal,sizeof(DWORD));
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);

				dwVal = 65535;
				mr.dwMDIdentifier = 9017;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				memcpy(mr.pbMDData,&dwVal,sizeof(DWORD));
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);

				dwVal = 0;
				mr.dwMDIdentifier = 9021;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				memcpy(mr.pbMDData,&dwVal,sizeof(DWORD));
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);

				mr.dwMDIdentifier = 9001;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);

				mr.dwMDIdentifier = 9004;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);

				mr.dwMDIdentifier = 9005;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);

				mr.dwMDIdentifier = 9006;
				mr.dwMDDataType = DWORD_METADATA;
				ZeroMemory(mr.pbMDData,CMetaRecord::DEFAULT_SIZE);
				mr.dwMDDataLen = sizeof(DWORD);

				webServ.SetData("/VisNetic MailFlow",&mr);
				DcaTrace(_T("Saving Web configuration"));
				webServ.Save();
				DcaTrace(_T("Closing Web Server"));
				webServ.Close();
			}
		}

		if(isVirt == 1)
		{
			// Now making a connection to Web Service for virtual directory
			err = webServ.Open();
			if(err != 0)
			{
				wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
				dca::WString e(errBuf);
				DcaTrace(e.c_str());
				throw errBuf;
			}

			webServ.WriteTo();

			// Creating Root Directory
			webServ.SetKeyType(virSitePath,"IIsWebVirtualDir");
			webServ.SetWin32Error(virSitePath);

			// Setting Path
			webServ.SetPath(virSitePath,webDir);
			webServ.SetAccessFlags(virSitePath,513);
			webServ.SetAppFriendlyName(virSitePath,comment);

			strcpy(appRoot,"/LM/W3SVC/");
			strcat(appRoot,virSitePath);
			webServ.SetAppRoot(virSitePath,appRoot);
			webServ.SetAppIsolated(virSitePath,0);

			webServ.SetScriptMaps(virSitePath,map);
			webServ.SetDefaultDoc(virSitePath,defDoc);
		}
		else
		{
			// Now making a connection to Web Service for virtual directory

			err = webServ.Open();
			if(err != 0)
			{
				wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
				throw errBuf;
			}

			webServ.WriteTo();

			// Creating Root Directory
			webServ.SetKeyType(rootPath,"IIsWebVirtualDir");
			webServ.SetWin32Error(rootPath);

			// Setting Path
			webServ.SetPath(rootPath,webDir);
			webServ.SetAccessFlags(rootPath,513);
			webServ.SetAppFriendlyName(rootPath,comment);

			webServ.SetScriptMaps(rootPath,map);
			webServ.SetDefaultDoc(rootPath,defDoc);

			if(dwMajVer >= 6)
			{
				webServ.SetAppPoolId(rootPath,"VisNetic MailFlow");
				webServ.SetAppRoot(rootPath,"/LM/W3SVC/3152/Root");
				webServ.SetAppIsolated(rootPath,2);
			}
		}

		//Closing connection to Web Server
		webServ.Close();

		AddMimeType(webServ,JMF_TYPE);
		AddMimeType(webServ,VMF_TYPE);

		if(servExt && (dwMajVer >= 6))
		{
			//DEBUGSTRING(_T("Opening /LM"));

			webServ.Open("/LM");

			if(err != 0)
			{
				wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
				dca::WString e(errBuf);
				DcaTrace(e.c_str());
				throw errBuf;
			}

			CMetaRecord mr;
			mr.dwMDUserType = IIS_MD_UT_SERVER;
			mr.dwMDIdentifier = 2168;
			DWORD dwLen;
			bool bSuccess = false;

			DcaTrace(_T("Getting id 2168 for /W3SVC"));

			err = webServ.GetData("/W3SVC",&mr,&dwLen);

			if(err == 0)
			{
				bSuccess = true;
				DcaTrace(_T("Getting id 2168 for /W3SVC was successul"));
			}
			else
			{
				DcaTrace(_T("Error was [ %d ]"),err);

				if(err == ERROR_INSUFFICIENT_BUFFER)
				{
					mr.Resize(dwLen);
					err = webServ.GetData("/W3SVC",&mr,&dwLen);
					if(err == 0)
					{
						bSuccess = true;
						DcaTrace(_T("Getting id 2168 for /W3SVC was successul"));
					}
				}
			}

			if(bSuccess)
			{
				wchar_t* pString = (wchar_t*)mr.pbMDData;
				bool bFound = false;

				DcaTrace(_T("Ready to parse the data"));

				while(pString[0] != 0x00)
				{
					if(!_wcsicmp(pString,OLD_DLL_EXT))
					{
						wmemcpy(pString, NEW_DLL_EXT, wcslen(NEW_DLL_EXT));
						bFound = true;
						break;
					}

					pString += wcslen(pString) + 1;
				}

				if(bFound)
				{
					DcaTrace(_T("Ready to update the data"));
					webServ.WriteTo();

					mr.dwMDDataType = MULTISZ_METADATA;
					err = webServ.SetData("/W3SVC",&mr);

					if(err == 0)
					{
						webServ.Save();
					}
				}

			}

			webServ.Close();
		}
		else if(servExt == 0 && (dwMajVer >= 6))
		{
			// Configure size for isapiPath
			size_t nLen = strlen(VMF_ISAPI_DLL) + 1;
			nLen += strlen(isapiPath) + 1;

			// Create isapiPath ANSI
			std::auto_ptr<char> pTempIsapi(new char[nLen]);
			sprintf(pTempIsapi.get(), VMF_ISAPI_DLL, isapiPath);

			// Convert it to UNICODE
			dca::WString isapiWebExt(pTempIsapi.get());

			webServ.Open("/LM");

			if(err != 0)
			{
				wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
				dca::WString e(errBuf);
				DcaTrace(e.c_str());
				throw errBuf;
			}

			CMetaRecord mr;
			mr.dwMDUserType = IIS_MD_UT_SERVER;
			mr.dwMDIdentifier = 2168;
			DWORD dwLen;
			DWORD dwNewLen = 0;
			bool bSuccess = false;

			DcaTrace(_T("Getting id 2168 for /W3SVC"));

			err = webServ.GetData("/W3SVC", &mr, &dwLen);

			if(err == 0)
			{
				// Store off in temp buffer
				DWORD dwOldLen = mr.dwMDDataLen;
				std::auto_ptr<BYTE> bTemp(new BYTE[mr.dwMDDataLen]);
				memcpy(bTemp.get(), mr.pbMDData, mr.dwMDDataLen);

				dwNewLen = mr.dwMDDataLen;
				dwNewLen += (DWORD)(isapiWebExt.length() * sizeof(wchar_t));
				dwNewLen += 2;

				mr.Resize(dwNewLen);

				// Refill metarecord
				memcpy(mr.pbMDData, bTemp.get(), dwOldLen);

				DcaTrace(_T("Getting id 2168 for /W3SVC was successful"));

				bSuccess = true;
			}
			else
			{
				DcaTrace(_T("Error was [ %d ]"),err);

				if(err == ERROR_INSUFFICIENT_BUFFER)
				{
					dwNewLen += (DWORD)(isapiWebExt.length() * sizeof(wchar_t));
					dwLen += 2;
					dwNewLen = dwLen;
					mr.Resize(dwLen);
					err = webServ.GetData("/W3SVC",&mr,&dwLen);
					if(err == 0)
					{
						DcaTrace(_T("Getting id 2168 for /W3SVC was successful"));
						bSuccess = true;
					}
				}
			}

			if(bSuccess)
			{
				wchar_t* pString = (wchar_t*)mr.pbMDData;
				bool bFound = false;

				while(pString[0] != 0x00)
				{
					if(!_wcsicmp(pString,isapiWebExt.c_str()))
					{
						bFound = true;
						break;
					}

					pString += wcslen(pString) + 1;
				}

				if(!bFound)
				{
					wcscpy(pString, isapiWebExt.c_str());

					mr.dwMDDataLen = dwNewLen;
					mr.dwMDDataType = MULTISZ_METADATA;

					webServ.WriteTo();

					err = webServ.SetData("/W3SVC",&mr);
					if(err == 0)
					{
						DcaTrace(_T("Saving Web configuration"));
						webServ.Save();
					}
					else
					{
						DcaTrace(_T("Error [ %d ]"),err);
					}
				}

			}

			webServ.Close();
		}
		
		strcpy(errString,"");
		return 0;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred - %s",msg);
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function CreateWeb");
	}

	return 1;
}
#pragma warning (default:4786)
//******************************************************************************

//******************************************************************************
// AddMimeType function 
//******************************************************************************

void AddMimeType(iis::WebServer& webServ, const wchar_t* type)
{
	char	errBuf[256];

	int err = webServ.Open("/LM");
	if(err != 0)
	{
		wsprintfA(errBuf,"Failed to open a connection to the web server with the following error from IIS : %s [%d]",webServ.GetErrorString(),err);
		dca::WString e(errBuf);
		DcaTrace(e.c_str());
		throw errBuf;
	}
	
	CMetaRecord mr;
	mr.dwMDUserType = IIS_MD_UT_FILE;
	mr.dwMDIdentifier = 6015;
	DWORD dwLen;
	DWORD dwNewLen = 0;
	bool bSuccess = false;

	err = webServ.GetData("/MimeMap",&mr,&dwLen);
	if(err == 0)
	{
		// Store off in temp buffer
		DWORD dwOldLen = mr.dwMDDataLen;
		std::auto_ptr<BYTE> bTemp(new BYTE[mr.dwMDDataLen]);
		memcpy(bTemp.get(), mr.pbMDData, mr.dwMDDataLen);

		dwNewLen = mr.dwMDDataLen;
		dwNewLen += (DWORD)(wcslen(type) * sizeof(wchar_t));
		dwNewLen += 2;

		mr.Resize(dwNewLen);

		// Refill metarecord
		memcpy(mr.pbMDData, bTemp.get(), dwOldLen);

		bSuccess = true;
	}
	else
	{
		if(err == ERROR_INSUFFICIENT_BUFFER)
		{
			dwLen += (DWORD)(wcslen(type) * sizeof(wchar_t));
			dwLen += 2;
			dwNewLen = dwLen;
			mr.Resize(dwLen);
			err = webServ.GetData("/MimeMap",&mr,&dwLen);
			if(err == 0)
			{
				bSuccess = true;
			}
		}
	}

	if(bSuccess)
	{
		wchar_t* pString = (wchar_t*)mr.pbMDData;
		bool bFound = false;

		while(pString[0] != 0x00)
		{
			if(!_wcsicmp(pString,type))
			{
				bFound = true;
				break;
			}

			pString += wcslen(pString) + 1;
		}

		if(!bFound)
		{
			wcscpy(pString,type);

			mr.dwMDAttributes = METADATA_INHERIT;
			mr.dwMDDataType = MULTISZ_METADATA;
			mr.dwMDDataLen = dwNewLen;

			webServ.WriteTo();

			err = webServ.SetData("/MimeMap",&mr);

			if(err == 0)
			{
				webServ.Save();
			}
		}

	}

	webServ.Close();
}

//******************************************************************************

//******************************************************************************
// DeleteDB function 
//******************************************************************************

int DeleteDB(char* dsn,char* user,char* password,char* errstring)
{
	LogFile logIt;
	char buf[256];
	WCHAR wdsn[256];
	dca::WString cmd;
	int retval = 0;
	
	try
	{
		logIt.Line();
		logIt.Date();
		logIt.Write("* DeleteDB function");

		dca::WString dbName(L"VisNetic MailFlow");
		try{
			dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
			lmrk.QueryStringValue(L"DBNAME", dbName);
		}
		catch(...){
		}
		dca::String d(dbName.c_str());
	
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		// Connect to database
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		SQLRETURN sqlRet = dbConn.Connect(wdsn);
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			logIt.Write("- Failed to connect to the database");
			dca::String e(dbConn.GetErrorString());
			strcpy(errstring, e.c_str());

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		logIt.Write("+ Successfully connected to database server");
		//Begin transaction
		/*logIt.Write("* Starting database transaction");
		sqlRet = dbConn.ExecuteSQL(L"begin transaction");
		if( !SQL_SUCCEEDED( sqlRet ) )
		{
			logIt.Write("- Failed to begin transaction");
			dca::String e(dbConn.GetErrorString());
			strcpy(errstring, e.c_str());

			wsprintfA(buf,"- Error : %s", e.c_str());
			logIt.Write(buf);

			dbConn.Disconnect();

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		logIt.Write("* Attempting to change the dbowner");

		cmd.Format(L"use [%s] exec sp_changedbowner 'sa'", dbName.c_str());
		sqlRet = dbConn.ExecuteSQL((wchar_t*)cmd.c_str());
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			if(dbConn.GetNativeErrorCode() != 5701)
			{
				logIt.Write("- Failed to change the dbowner");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd.c_str());
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errstring,buf);
				logIt.Write("* Rolling back transaction");
				dbConn.ExecuteSQL(L"rollback transaction");
				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
		}

		dbConn.ExecuteSQL(L"commit transaction");

		cmd.assign(L"use master exec sp_droplogin 'VMFAdmin'");
		sqlRet = dbConn.ExecuteSQL((wchar_t*)cmd.c_str());
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			logIt.Write("- Failed to drop VMFAdmin user");
			dca::String e(dbConn.GetErrorString());
			dca::String c(cmd.c_str());
			sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
			strcpy(errstring,buf);
			logIt.Write("* Rolling back transaction");
			dbConn.ExecuteSQL(L"rollback transaction");
			dbConn.Disconnect();

			logIt.LineBreak();
			logIt.Line();
			retval = 1;
		}

		dbConn.Disconnect();

		if(retval)
		{
			CreateDBUser(dsn,NULL,user,password,errstring);
			ChangeDBOwner(dsn,NULL,user,errstring);
			return retval;
		}

		sqlRet = dbConn.Connect(wdsn);
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			logIt.Write("- Failed to connect to the database");
			dca::String e(dbConn.GetErrorString());
			strcpy(errstring, e.c_str());

			wsprintfA(buf,"- Error : %s", e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}*/

		logIt.Write("* Attempting to drop the VisNetic MailFlow database");
		cmd.Format(L"use [master] Drop Database [%s]", dbName.c_str());
		sqlRet = dbConn.ExecuteSQL((wchar_t*)cmd.c_str());
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			logIt.Write("- Failed to drop %s database", d.c_str());
			dca::String e(dbConn.GetErrorString());
			dca::String c(cmd.c_str());
			sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
			strcpy(errstring,buf);
			//logIt.Write("* Rolling back transaction");
			//dbConn.ExecuteSQL(L"rollback transaction");
			dbConn.Disconnect();

			logIt.LineBreak();
			logIt.Line();
			retval =  1;
		}

		logIt.Write("- Successfully dropped the %s database", d.c_str());
		dbConn.Disconnect();

		/*if(retval)
		{
			CreateDBUser(dsn,NULL,user,password,errstring);
			ChangeDBOwner(dsn,NULL,user,errstring);
			return retval;
		}*/
	
		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function DeleteDB";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errstring,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// DeleteWeb function 
//******************************************************************************

int DeleteWeb(char* siteNum,char* errString)
{
	int err;
	char	errBuf[256];

	try
	{
		UseOle	useOle;

		SITES::iterator i;
		SITES	iisSites;
		
		iis::WebServer webServ;

		// Attempting to open connection to Local Machine
		err = webServ.Open("/LM/");
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}
		
		// Getting a list of current web sites
		webServ.EnumSites(&iisSites,"W3SVC");

		webServ.Close();

		// Now making a connection to Web Service
		err = webServ.Open();
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}

		for(i = iisSites.begin(); i != iisSites.end(); i++)
		{
			string temp = *i;
			if(!temp.compare(siteNum))
			{
				webServ.WriteTo();
				err = webServ.Delete(temp.c_str());
				if(err != 0)
				{
					throw "Failed to delete site";
				}
			}
		}

		// Closing connection to Web Server
		webServ.Close();
	
		strcpy(errString,"");
		return 0;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",msg,::GetLastError());
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function DeleteWeb");
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// DetectWeb function 
//******************************************************************************

int DetectWeb(char* siteNum,char* errString)
{
	int err;
	char	errBuf[256];

	try
	{
		UseOle	useOle;

		SITES::iterator i;
		SITES	iisSites;
		
		iis::WebServer webServ;

		String	  name = "VisNetic MailFlow";
		ResString comment(g_hInst,IDS_WEB_COMMENT);

		// Attempting to open connection to Local Machine
		err = webServ.Open("/LM/");
		if(err != 0)
		{
			throw "Failed to Open Local Machine";
		}
		
		// Getting a list of current web sites\n";
		webServ.EnumSites(&iisSites,"W3SVC");

		webServ.Close();

		// Now making a connection to Web Service
		err = webServ.Open();
		if(err != 0)
		{
			throw "Failed to Open Web Server";
		}

		for(i = iisSites.begin(); i != iisSites.end(); i++)
		{
			string temp = *i;
			if(!temp.compare(siteNum))
			{
				char buf[256];
				webServ.GetServerComment(temp.c_str(),buf);
				if(!strcmp(buf,comment))
				{
					webServ.Close();
					strcpy(errString,"");
					return 1;
				}
			}
		}

		// Closing connection to Web Server
		webServ.Close();
	
		strcpy(errString,"");
		return 0;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",msg,::GetLastError());
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function DetectWeb");
	}
	return 1;
}

//******************************************************************************

//******************************************************************************
// DoesDBExist function 
//******************************************************************************

int DoesDBExist(char* dsn, char* dbVer, char* errstring)
{
	WCHAR wdsn[256];
	LogFile logIt;
	//strcpy(dbVer,"6.6.0.3");
	//return 0;

	try
	{

		// Convert dsn to WCHAR for CDBConn
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		CDBConn dbConn;
		wchar_t	ret[80];
		char	buf[256];

		// Open log file and print function name
		logIt.Line();
		logIt.Date();
		logIt.Write("* DoesDBExist function");
	
		// Connect to server
		logIt.Write("* Attempting connection to database server");

		SQLRETURN sqlRet = dbConn.Connect(wdsn);
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			// Failed to connect
			logIt.Write("- Failed to connect to database server");
			dca::String e(dbConn.GetErrorString());
			strcpy(errstring, e.c_str());

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		// Successfully connected
		logIt.Write("+ Successfully connected to database server");

		dca::WString dbName(L"VisNetic MailFlow");
		try{
			dca::LocalMachineRegKey lmrk(L"SOFTWARE\\Deerfield.com\\VisNetic MailFlow", dca::RegKey::OPEN, KEY_READ);
			lmrk.QueryStringValue(L"DBNAME", dbName);
		}
		catch(...){
		}
		dca::String d(dbName.c_str());

		// Attempt to get MailFlow id.
		logIt.Write("* Attempting to get %s id", d.c_str());
		dca::WString sqlCmd;
		sqlCmd.Format(L"USE master SELECT DB_ID('%s')", dbName.c_str());
		//sqlCmd.Format(L"USE [%s] SELECT DB_ID('%s')", dbName.c_str(), dbName.c_str());
		sqlRet = dbConn.GetData((wchar_t*)sqlCmd.c_str(), ret);
		if ( !SQL_SUCCEEDED( sqlRet ) )
		{
			dca::String e(dbConn.GetErrorString());
			strcpy(errstring,e.c_str());

			logIt.Write("- Failed to get id for %s", d.c_str());
			wsprintfA(buf,"- Error : %s", e.c_str());
			logIt.Write(buf);

			strcpy(dbVer,"");

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		int r = _wtoi(ret);

		logIt.Write("* Attempting to disconnect from database server");
		dbConn.Disconnect();

		if(r > 0)
		{
			logIt.Write("* Attempting connection to database server");
			SQLRETURN sqlRet = dbConn.Connect(wdsn);
			if ( !SQL_SUCCEEDED( sqlRet ) )
			{
				logIt.Write("- Failed to connect to database server");
				dca::String e(dbConn.GetErrorString());
				strcpy(errstring, e.c_str());

				wsprintfA(buf,"- Error : %s", e.c_str());
				logIt.Write(buf);

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}

			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to get %s database version", d.c_str());
			dca::WString sqlCmd;
			sqlCmd.Format(L"use [%s] select DataValue from ServerParameters where ServerParameterId = 24", dbName.c_str());
			sqlRet = dbConn.GetData((wchar_t*)sqlCmd.c_str(), ret);
			if ( !SQL_SUCCEEDED( sqlRet ) )
			{
				dca::String e(dbConn.GetErrorString());
				strcpy(errstring, e.c_str());

				logIt.Write("- Failed to get %s database version", d.c_str());
				wsprintfA(buf,"- Error : %s", e.c_str());
				logIt.Write(buf);

				strcpy(dbVer,"");
				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}

			logIt.Write("- Successfully got the %s database version", d.c_str());
			dca::String x(ret);

			strcpy(dbVer,x.c_str());
			dbConn.Disconnect();

			logIt.Write("+ OK");
			logIt.LineBreak();
			logIt.Line();
			return 0;
		}
	
		logIt.Write("* Database does not exist");
		logIt.Write("+ OK");
		strcpy(dbVer,"");

		logIt.LineBreak();
		logIt.Line();
		return 1;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function DoesDBExist";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errstring,errStr);
	}
	return 1;
}

//******************************************************************************

//******************************************************************************
// EncryptPassword function 
//******************************************************************************

int EncryptPassword(char* inPass,char* outPass,char* errString)
{
	char temp[256];
	int len;

	try
	{
		len = strlen(inPass);
		if((len > 0) && (len < 256))
		{
			ShiftText(inPass,temp);
			strcpy(outPass,temp);
			strcpy(errString,"");
			return 0;
		}

		strcpy(outPass,"");
		strcpy(errString,"");
		return 1;
	}
	catch(...)
	{
		strcpy(outPass,"");
		strcpy(errString,"Unknown Exception Occurred in function EncryptPassword");
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// EncryptString function 
//******************************************************************************

void EncryptString(char* text, char* errString)
{
	try
	{
		dca::WString w(text);
		dca::EString eStr(w.c_str());
		eStr = eStr.Encrypt();

		std::auto_ptr<char> redo(new char[eStr.length() + 1]);
		::WideCharToMultiByte(CP_ACP, 0, eStr.c_str(), -1, redo.get(), eStr.length() + 1, 0, 0);

		strcpy(text, redo.get());
		strcpy(errString,"");
	}
	catch(...)
	{
		strcpy(text,"");
		strcpy(errString,"Unknown Exception Occurred in function EncryptString");
	}
}

//******************************************************************************

//******************************************************************************
// GetIISSiteCount function 
//******************************************************************************

int DecryptPassword(char* inPass,char* outPass,char* errString)
{
	char temp[256];
	int len;
	
	try
	{
		len = strlen(inPass);
		if((len > 0) && (len < 256))
		{
			ShiftText(inPass,temp);
			strcpy(outPass,temp);
			strcpy(errString,"");
			return 0;
		}
	
		strcpy(outPass,"");
		strcpy(errString,"");
		return 1;
	}
	catch(...)
	{
		strcpy(outPass,"");
		strcpy(errString,"Unknown Exception Occurred in function DecryptPassword");
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// DecryptString function 
//******************************************************************************

void DecryptString(char* text,char* errString)
{
	try
	{
		std::auto_ptr<wchar_t> temp(new wchar_t[strlen(text) + 1]);

		::MultiByteToWideChar(CP_ACP, 0, text, -1, temp.get(), strlen(text) + 1);
		dca::EString eStr(temp.get());
		eStr = eStr.Decrypt();

		dca::String a(eStr.c_str());
		strcpy(text, a.c_str());
		strcpy(errString,"");
	}
	catch(...)
	{
		strcpy(text,"");
		strcpy(errString,"Unknown Exception Occurred in function DecryptString");
	}
}

//******************************************************************************
// GetIISSiteCount function 
//******************************************************************************

int GetIISSiteCount(char* errString)
{
	int err;
	char	errBuf[256];

	try
	{
		UseOle	useOle;

		SITES::iterator i;
		SITES	iisSites;
		int	idx = 0;
		
		iis::WebServer webServ;

		// Attempting to open connection to Local Machine
		err = webServ.Open("/LM/");
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}
		
		webServ.EnumSites(&iisSites,"W3SVC");

		webServ.Close();

		for(i = iisSites.begin(); i != iisSites.end(); i++)
		{
			string temp = *i;
			if((temp.compare("Info")) && (temp.compare("Filters")) && (temp.compare("AppPools")))
			{
				idx++;
			}
		}
		
		strcpy(errString,"");
		return idx;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",msg,::GetLastError());
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function GetIISSiteCount");
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// GetIISSiteID function 
//******************************************************************************

int GetIISSiteID(char* sSiteName,char* errString)
{
	int err;
	char	errBuf[256];

	try
	{
		UseOle	useOle;

		SITES::iterator i;
		SITES	iisSites;
		
		iis::WebServer webServ;

		// Attempting to open connection to Local Machine
		err = webServ.Open("/LM/");
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}
		
		// Getting a list of current web sites";
		webServ.EnumSites(&iisSites,"W3SVC");

		webServ.Close();

		// Now making a connection to Web Service
		err = webServ.Open();
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}

		for(i = iisSites.begin(); i != iisSites.end(); i++)
		{
			string temp = *i;
			if((temp.compare("Info")) && (temp.compare("Filters")) && (temp.compare("AppPools")))
			{
				char buf[80];
				webServ.GetServerComment(temp.c_str(),buf);

				if(!strcmp(buf, sSiteName))
				{
					webServ.Close();
					strcpy(errString,"");
					return atoi(temp.c_str());
				}
			}
		}
	
		webServ.Close();
		strcpy(errString,"");
		return 0;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",msg,::GetLastError());
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function GetIISSiteID");
	}

	return 0;
}

//******************************************************************************

//******************************************************************************
// GetIISSiteNames function 
//******************************************************************************

int GetIISSiteNames(char* sSiteNames,char* errString)
{
	int err;
	char	errBuf[256];

	try
	{
		UseOle	useOle;

		SITES::iterator i;
		SITES	iisSites;
		
		iis::WebServer webServ;

		DcaTrace(L"Attempting to open connection to Local Machine\n");

		// Attempting to open connection to Local Machine
		err = webServ.Open("/LM/");
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}
		
		DcaTrace(L"Getting a list of current web sites\n");

		// Getting a list of current web sites";
		webServ.EnumSites(&iisSites,"W3SVC");

		webServ.Close();

		char eErr[256];
		int count = GetIISSiteCount(eErr);
		if(count == 0)
		{
			throw "Failed to retrieve count of IIS websites";
		}

		DcaTrace(L"Site array has %d entries\n", iisSites.size());

		count = count * 80;
		auto_ptr<char> aBuf(new char[count]);
		char* pBuf = aBuf.get();

		DcaTrace(L"Now making a connection to Web Service\n");

		// Now making a connection to Web Service
		err = webServ.Open();
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}

		int j = 0;
		for(i = iisSites.begin(); i != iisSites.end(); i++)
		{
			string temp = *i;

			if((temp.compare("Info")) && (temp.compare("Filters")) && (temp.compare("AppPools")))
			{
				char buf[80];
				webServ.GetServerComment(temp.c_str(), buf);

				dca::WString site(temp.c_str());
				dca::WString name(buf);
				DcaTrace(L"Found sited %s - %s\n", site.c_str(), name.c_str());

				if(j != 0)
				{
					strcat(pBuf,";");
					strcat(pBuf,buf);
				}
				else
				{
					strcpy(pBuf,buf);
					j = 1;
				}

			}
		}
		
		strcpy(sSiteNames,pBuf);
		webServ.Close();
		strcpy(errString,"");

		dca::WString w(sSiteNames);
		DcaTrace(L"Returning from function with this string %s\n", w.c_str());

		return 0;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",msg,::GetLastError());
		strcpy(errString,errBuf);
	}
	catch(dca::Exception& e)
	{
		wsprintfA(errBuf,"The following exception occurred [%s]", e.GetMessage());
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function GetIISSiteNames");
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// GetInsError functions
//******************************************************************************

void GetInsError(char* err,char* errString)
{
	try
	{
		ResString Error(g_hInst,IDS_INS_ERROR);
		strcpy(err,Error);
		strcpy(errString,"");
	}
	catch(...)
	{
		strcpy(err,"");
		strcpy(errString,"Unknown Exception Occurred in function GetInsError");
	}
}

//******************************************************************************

//******************************************************************************
// GetInsWarning functions
//******************************************************************************

void GetInsWarning(char* warn,char* errString)
{
	try
	{
		ResString Warn(g_hInst,IDS_INS_WARNING);
		strcpy(warn,Warn);
		strcpy(errString,"");
	}
	catch(...)
	{
		strcpy(warn,"");
		strcpy(errString,"Unknown Exception Occurred in function GetInsWarning");
	}
}

//******************************************************************************

//******************************************************************************
// GetLicense function 
//******************************************************************************

int GetLicense(char* dsn, char* license, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	char cmd[1024];
	wchar_t temp[256];
	char buf[256];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* GetLicense function");
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to retrieve license value from database");
			ret = dbConn.GetData(L"select DataValue from ServerParameters where ServerParameterId = '22'", temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to retrieve license value from database");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully retrieved license value from database");
				dca::String s(temp);
				strcpy(license,s.c_str());
			}
		}
		else
		{
			logIt.Write("- Failed to connect to database server");
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function GetLicense";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// GetSQLServers function 
//******************************************************************************

int GetSQLServers(char* server,char* errString)
{
	try
	{
		UseOle useOle;

		SERVERS::iterator i;
		SERVERS servers;
		char* cur;

		sql::Application app;

		app.ListAvailableServers();
		app.EnumServers(&servers);
		
		long me = servers.size();

		if(me == 0)
		{
			throw "Did not find any SQL Servers";
		}

		me = me * 80;
		cur = new char[me];

		int j = 0;
		for (i = servers.begin(); i != servers.end(); ++i)
		{
			string temp;
			temp = *i;

			if(j != 0)
			{
				strcat(cur,";");
				strcat(cur,temp.c_str());
			}
			else
			{
				strcpy(cur,temp.c_str());
				j = 1;
			}
        }

		strcpy(server,cur);
		delete [] cur;
		strcpy(errString,"");
		return 0;
	}
	catch(char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function GetSQLServers");
	}
	strcpy(server,"");
	return 1;
}

//******************************************************************************

//******************************************************************************
// InsertRoutingRule function 
//******************************************************************************

int InsertRoutingRule(char* dsn,char* desc, int tickBox, char* toEM, char* fromEM, char* errString)
{
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wdesc[256];
	WCHAR wtoEM[256];
	WCHAR wfromEM[256];
	wchar_t  cmd[256];
	wchar_t  temp[256];
	char buf[256];
	int rrID = 0;
	int match = 0;
	LogFile logIt;

	try
	{
		logIt.Line();
		logIt.Date();
		logIt.Write("* InsertRoutingRule function");

		int toLen = strlen(toEM);
		int fromLen = strlen(fromEM);

		if((toLen != 0) && (fromLen == 0))
		{
			match = 1;
		}
		else if((toLen == 0) && (fromLen != 0))
		{
			match = 2;
		}

		if(!HasValidCharacters(desc))
		{
			strcpy(errString,"Cannot create a Routing Rule description that has double qoutes in the description name");
			logIt.Write("- Cannot create a Routing Rule description that has double qoutes in the description name");
			logIt.LineBreak();
			logIt.Line();
			return 0;
		}

		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,desc,-1,wdesc,256);
		MultiByteToWideChar(CP_ACP,0,toEM,-1,wtoEM,256);
		MultiByteToWideChar(CP_ACP,0,fromEM,-1,wfromEM,256);

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			// Begin transaction
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Beginning transaction");
			ret = dbConn.ExecuteSQL(L"begin transaction");
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to begin transaction");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s", e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}

			// Insert Routing rule
			logIt.Write("* Attempting to insert routing rule");
			wsprintfW(cmd, L"SET QUOTED_IDENTIFIER OFF insert into RoutingRules (RuleDescrip,OrderIndex,AssignToTicketBox,AutoReplyFrom) values (\"%s\",1,%d,\"\") SET QUOTED_IDENTIFIER ON", wdesc, tickBox);
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to insert routing rule");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);
				logIt.Write("* Rolling back transaction");
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}

			logIt.Write("+ Successfully inserted routing rule");

			// Get Routing Rule Id
			logIt.Write("* Attempting to get routing rule id");
			wsprintfW(cmd, L"SET QUOTED_IDENTIFIER OFF select RoutingRuleID from RoutingRules where RuleDescrip = \"%s\" SET QUOTED_IDENTIFIER ON", wdesc);
			ret = dbConn.GetData(cmd, temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to get routing rule id");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);
				logIt.Write("* Rolling back transaction");
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
		
			logIt.Write("+ Successfully got routing rule id");
			rrID = _wtoi(temp);
		
			switch(match)
			{
			case 1:
				logIt.Write("* Attempting to insert id into MatchToAddress");
				wsprintfW(cmd, L"insert into MatchToAddresses (EmailAddress,RoutingRuleID) values ('%s',%d)", wtoEM, rrID);
				ret = dbConn.ExecuteSQL(cmd);
				if( !SQL_SUCCEEDED( ret ) )
				{
					logIt.Write("- Failed to insert id into MatchToAddress");
					dca::String e(dbConn.GetErrorString());
					dca::String c(cmd);
					sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
					strcpy(errString,buf);
					dbConn.ExecuteSQL(L"rollback transaction");

					wsprintfA(buf,"- Error : %s", e.c_str());
					logIt.Write(buf);

					dbConn.Disconnect();

					logIt.LineBreak();
					logIt.Line();
					return 1;
				}
				logIt.Write("+ Successfully inserted id into MatchToAddress");
				break;
			case 2:
				logIt.Write("* Attempting to insert id into MatchFromAddress");
				wsprintfW(cmd, L"insert into MatchFromAddresses (EmailAddress,RoutingRuleID) values ('%s',%d)", wfromEM, rrID);
				ret = dbConn.ExecuteSQL(cmd);
				if( !SQL_SUCCEEDED( ret ) )
				{
					logIt.Write("- Failed to insert id into MatchFromAddress");
					dca::String e(dbConn.GetErrorString());
					dca::String c(cmd);
					sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
					strcpy(errString,buf);
					dbConn.ExecuteSQL(L"rollback transaction");

					wsprintfA(buf,"- Error : %s",e.c_str());
					logIt.Write(buf);

					dbConn.Disconnect();

					logIt.LineBreak();
					logIt.Line();
					return 1;
				}
				logIt.Write("+ Successfully inserted id into MatchFromAddress");
				break;
			}

			logIt.Write("* Committing transaction");
			dbConn.ExecuteSQL(L"commit transaction");
			dbConn.Disconnect();

			// Insert Routing Rule

		}
		else
		{
			logIt.Write("- Failed to connect to database server");
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function InsertRoutingRule";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// InsertTicketBox function 
//******************************************************************************

int InsertTicketBox(char* dsn, char* name, char* desc, char* email, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wname[256];
	WCHAR wdesc[256];
	WCHAR wemail[256];
	wchar_t  cmd[256];
	wchar_t  temp[256];
	int objID = 0;
	int tbID = 0;

	try
	{
		logIt.Line();
		logIt.Date();
		logIt.Write("* InsertTicketBox function");

		if(!strcmp(name,"Unassigned"))
		{
			strcpy(errString,"Cannot create a TicketBox with the name Unassigned");
			logIt.Write("- Cannot create a TicketBox with the name Unassigned");
			logIt.LineBreak();
			logIt.Line();
			return 0;
		}

		if(!HasValidCharacters(name))
		{
			strcpy(errString,"Cannot create a TicketBox that has double qoutes in the name");
			logIt.Write("- Cannot create a TicketBox that has double qoutes in the name");
			logIt.LineBreak();
			logIt.Line();
			return 0;
		}

		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,desc,-1,wdesc,256);
		MultiByteToWideChar(CP_ACP,0,name,-1,wname,256);
		MultiByteToWideChar(CP_ACP,0,email,-1,wemail,256);

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			// Begin transaction
			logIt.Write("* Beginning transaction");
			ret = dbConn.ExecuteSQL(L"begin transaction");
			if( !SQL_SUCCEEDED( ret ) )
			{
				char buf[256];
				logIt.Write("- Failed to begin transaction");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s", e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();
			
				logIt.LineBreak();
				logIt.Line();
				return 0;
			}

			// Create Object
			logIt.Write("* Attempting to create an Object");
			ret = dbConn.ExecuteSQL(L"insert into Objects (ObjectTypeID,ActualID,DateCreated) values (2,-1,GETDATE())");
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to create an Object");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);
				logIt.Write("* Rolling back transaction");
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s", e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 0;
			}

			logIt.Write("+ Successfully created an Object");
			logIt.Write("* Attempting to get Object's id");
			// Get ObjectID
			ret = dbConn.GetData(L"select ObjectID from Objects where ActualID = -1",temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to get Object's id");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 0;
			}

			logIt.Write("+ Successfully got Object's id");

			objID = _wtoi(temp);
		
			logIt.Write("* Attempting to insert TicketBox");
			// Create TicketBox
			wsprintfW(cmd, L"SET QUOTED_IDENTIFIER OFF insert into TicketBoxes (Name,Description,ObjectID,DefaultEmailAddress,DefaultEmailAddressName) values (\"%s\",\"%s\",%d,'%s',\"%s\") SET QUOTED_IDENTIFIER ON",wname,wdesc,objID,wemail,wname);
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to insert TicketBox");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 0;
			}

			logIt.Write("+ Successfully inserted TicketBox");

			logIt.Write("* Attempting to get TicktBox's id");
			// Get TicketBox Id
			wsprintfW(cmd, L"SET QUOTED_IDENTIFIER OFF select TicketBoxID from TicketBoxes where Name = \"%s\" SET QUOTED_IDENTIFIER ON", wname);
			ret = dbConn.GetData(cmd,temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to get TicketBox's id");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 0;
			}
		
			logIt.Write("+ Successfully got TicketBox's id");
			tbID = _wtoi(temp);

			// Update Object with TicketBox ID
			logIt.Write("* Attempting to Update Object with TicketBox id");
			wsprintfW(cmd, L"update Objects set ActualID = %d where ObjectID = %d", tbID, objID);
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to Update Object with TicketBox id");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);
				dbConn.ExecuteSQL(L"rollback transaction");

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 0;
			}

			logIt.Write("+ Successfully updating the Object with the TicketBox id");
			logIt.Write("* Committing the transaction");
			dbConn.ExecuteSQL(L"commit transaction");
			dbConn.Disconnect();
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 0;
		}
	
		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return tbID;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function InsertTicketBox";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}
	
	return 0;
}

//******************************************************************************

//******************************************************************************
// IsSQLSevenOrHigher function 
//******************************************************************************

int IsSQLSevenOrHigher(char* errString)
{
	try
	{
		UseOle useOle;

		SERVERS::iterator i;
		SERVERS servers;

		sql::Application app;

		long maj;
		app.GetVersionMajor(&maj);
		if(maj < 7)
		{
			strcpy(errString,"");
			return 1;
		}

		app.ListAvailableServers();
		app.EnumServers(&servers);
		
		for (i = servers.begin(); i != servers.end(); ++i)
		{
			string temp;
			temp = *i;
			if(!temp.compare("(local)"))
			{
				strcpy(errString,"");
				return 0;
			}
        }

		strcpy(errString,"");
		return 1;
	}
	catch(char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception occurred in the function IsSQLSevenOrHigher");
	}
	return 1;
}

//******************************************************************************

//******************************************************************************
// ReseedTicketTable function 
//******************************************************************************

int ReseedTicketTable(char* dsn,int nSeedNum,char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	wchar_t cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* ReseedTicketTable function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to reseed TicketID in Tickets");
			wsprintfW(cmd, L"DBCC CHECKIDENT (Tickets, RESEED, %d)", nSeedNum);

			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("* Failed to reseed Tickets");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("* Successfully reseeded Tickets");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();

		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function ReseedTicketTable";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}
	
	return 1;
}

//******************************************************************************

//******************************************************************************
// SetAdminEmail function 
//******************************************************************************

int SetAdminEmail(char* dsn, char* email, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wemail[256];
	wchar_t cmd[1024];
	wchar_t temp[256];

	try
	{
		logIt.Line();
		logIt.Date();
		logIt.Write("* SetAdminEmail function");

		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,email,-1,wemail,256);

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to get AgentId for Admin");
			ret = dbConn.GetData(L"SELECT AgentID FROM Agents where LoginName = 'Admin'", temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to get AgentId for Admin");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully got the AgentId for Admin");
				logIt.Write("* Attempting to set the PersonalData with the AgentId");
				wsprintfW(cmd, L"UPDATE PersonalData SET DataValue = '%s' where AgentID = %s", wemail, temp);
				ret = dbConn.ExecuteSQL(cmd);
				if( !SQL_SUCCEEDED( ret ) )
				{
					logIt.Write("- Failed to set the PersonalData with the AgentId");
					char buf[256];
					dca::String e(dbConn.GetErrorString());
					dca::String c(cmd);
					sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
					strcpy(errString,buf);

					wsprintfA(buf,"- Error : %s",e.c_str());
					logIt.Write(buf);

					dbConn.Disconnect();

					logIt.LineBreak();
					logIt.Line();
					return 1;
				}
				else
				{
					logIt.Write("* Successfully set the PersonalData with the AgentId");
					strcpy(errString,"");
				}
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetAdminEmail";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}
	
	return 1;
}

//******************************************************************************

//******************************************************************************
// SetAdminPassword function 
//******************************************************************************

int SetAdminPassword(char* dsn, char* password, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetAdminPassword function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			std::auto_ptr<wchar_t> temp(new wchar_t[strlen(password) + 1]);

			::MultiByteToWideChar(CP_ACP, 0, password, -1, temp.get(), strlen(password) + 1);

			dca::WString sPass(temp.get());

			dca::WString::size_type pos = dca::WString::npos;
	
			pos = sPass.find_first_of(0x27, 0);

			while(pos != dca::WString::npos)
			{
				sPass.insert(pos, 1, 0x27);

				pos = sPass.find(0x27, pos + 2);
			}

			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to update the password for the Admin");
			wsprintfW(cmd,L"UPDATE Agents SET Password = '%s' where LoginName = 'admin'",sPass.c_str());
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("* Failed to update the password for the Admin");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("* Successfully updated the password for the Admin");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetAdminPassword";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// SetDBUserPassword function 
//******************************************************************************

int SetDBUserPassword(char* dsn, char* oldp, char* newp, char* user, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR woldp[256];
	WCHAR wnewp[256];
	WCHAR wuser[256];
	wchar_t cmd[1024];

	try
	{
		
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,oldp,-1,woldp,256);
		MultiByteToWideChar(CP_ACP,0,newp,-1,wnewp,256);
		MultiByteToWideChar(CP_ACP,0,user,-1,wuser,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetDBUserPassword function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			int len = strlen(oldp);
			if(len != 0)
			{
				wsprintfW(cmd, L"exec sp_password '%s', '%s', '%s'",woldp,wnewp,wuser);
			}
			else
			{
				wsprintfW(cmd, L"exec sp_password NULL, '%s', '%s'",wnewp,wuser);
			}
			logIt.Write("* Attempting to update the DBUser password");
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to update the DBUser password");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully updated the DBUser password");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetDBUserPassword";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// SetDefaultEmail function 
//******************************************************************************

int SetDefaultEmail(char* dsn, char* email, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wemail[256];
	wchar_t cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,email,-1,wemail,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetDefaultEmail function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to update the default email");
			wsprintfW(cmd, L"update ServerParameters set DataValue = '%s' where ServerParameterID = '34'", wemail);
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("* Failed to update default email");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("* Successfully updated the default email");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetDefaultEmail";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// SetLicense function 
//******************************************************************************

int SetLicense(char* dsn, char* license, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wlicense[256];
	wchar_t cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,license,-1,wlicense,256);
		

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetLicense function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			wsprintfW(cmd, L"UPDATE ServerParameters SET DataValue = '%s' WHERE ServerParameterID = '22'",wlicense);
			ret = dbConn.ExecuteSQL(cmd);
			logIt.Write("* Attempting to update the License");
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to update the License");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully updated the License");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetLicense";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// SetMessageDestination function 
//******************************************************************************

int SetMessageDestination(char* dsn, char* srvAddr, int UseSMTPAuth,
						  char* user, char* pass, int port, int active,
						  int freqMins, int useSSL, char* description, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wsrvAddr[256];
	WCHAR wuser[256];
	WCHAR wpass[256];
	WCHAR wdescription[256];
	wchar_t cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,srvAddr,-1,wsrvAddr,256);
		MultiByteToWideChar(CP_ACP,0,user,-1,wuser,256);
		MultiByteToWideChar(CP_ACP,0,pass,-1,wpass,256);
		MultiByteToWideChar(CP_ACP,0,description,-1,wdescription,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetMessageDestination function");
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			wsprintfW(cmd, L"insert into MessageDestinations(ServerAddress,UseSMTPAuth,AuthUser,AuthPass,SMTPPort,IsActive,ProcessFreqMins,Description,IsSSL) values('%s',%d,'%s','%s',%d,%d,%d,'%s',%d)",
					 wsrvAddr,UseSMTPAuth,wuser,wpass,port,active,freqMins,wdescription,useSSL);
			logIt.Write("* Attempting to update the MessageDestination");
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to update the MessageDestination");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully updated the MessageDestination");
				logIt.Write("* Attempting to update the ServerParameters");
				ret = dbConn.ExecuteSQL(L"update ServerParameters set DataValue = 1 where ServerParameterID = 10");
				if( !SQL_SUCCEEDED( ret ) )
				{
					logIt.Write("* Failed to update the ServerParameters");
					char buf[256];
					dca::String e(dbConn.GetErrorString());
					dca::String c(cmd);
					sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
					strcpy(errString,buf);

					wsprintfA(buf,"- Error : %s",e.c_str());
					logIt.Write(buf);

					dbConn.Disconnect();
					logIt.LineBreak();
					logIt.Line();
					return 1;
				}
				logIt.Write("* Successfully updated the ServerParameters");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetMessageDestination";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// SetMessageSources function 
//******************************************************************************

int SetMessageSources(char* dsn, char* remoteAddr, int port, char* user,
					  char* pass, int aPop,int lvmsg, int active, int freqMins, int useSSL,
					  int messId, int messSrcType, char* description, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wremoteAddr[256];
	WCHAR wuser[256];
	WCHAR wpass[256];
	WCHAR wdescription[256];
	wchar_t cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,remoteAddr,-1,wremoteAddr,256);
		MultiByteToWideChar(CP_ACP,0,user,-1,wuser,256);
		MultiByteToWideChar(CP_ACP,0,pass,-1,wpass,256);
		MultiByteToWideChar(CP_ACP,0,description,-1,wdescription,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetMessageSources function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			wsprintfW(cmd, L"insert into MessageSources(RemoteAddress,RemotePort,AuthUserName,AuthPassword,IsAPOP,LeaveCopiesOnServer,IsActive,CheckFreqMins,MessageDestinationID,MessageSourceTypeID,Description,IsSSL) values('%s',%d,'%s','%s',%d,%d,%d,%d,%d,%d,'%s',%d)",
			         wremoteAddr,port,wuser,wpass,aPop,lvmsg,active,freqMins,messId,messSrcType,wdescription,useSSL);
			logIt.Write("* Attempting to update the MessageSources");
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to update the MessageSources");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully updated the MessageSources");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetMessageSources";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}
	
	return 1;
}

//******************************************************************************

//******************************************************************************
// StopOtherWebSites function 
//******************************************************************************

int StopOtherWebSites(char* siteNum,char* errString)
{
	int err;
	char	errBuf[256];

	try
	{
		UseOle	useOle;

		SITES::iterator i;
		SITES	iisSites;
		
		iis::WebServer webServ;

		String	  name = "VisNetic MailFlow";
		ResString comment(g_hInst,IDS_WEB_COMMENT);

		// Attempting to open connection to Local Machine
		err = webServ.Open("/LM/");
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}
		
		// Getting a list of current web sites\n";
		webServ.EnumSites(&iisSites,"W3SVC");

		webServ.Close();

		// Now making a connection to Web Service
		err = webServ.Open();
		if(err != 0)
		{
			throw "Failed to open a connection to the web server";
		}

		err = webServ.WriteTo();
		if(err != 0)
		{
			webServ.Close();
			strcpy(errString,"");
			return 1;
		}

		for(i = iisSites.begin(); i != iisSites.end(); i++)
		{
			string temp = *i;
			if(temp.compare(siteNum))
			{
				if((temp.compare("Info")) && (temp.compare("Filters")) && (temp.compare("AppPools")))
				{
					err = webServ.SetServerAutoStart(temp.c_str());
					if(err != 0)
					{
						webServ.Close();
						strcpy(errString,"");
						return 1;
					}
				}
			}
		}

		// Closing connection to Web Server
		webServ.Close();
	
		return 0;
	}
	catch(const char* msg)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",msg,::GetLastError());
		strcpy(errString,errBuf);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function StopOtherWebSites");
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// TestDBConnection function 
//******************************************************************************

int TestDBConnection(char* dsn, char* errString)
{
	//return 0;
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* TestDBConnection function with DSN");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			strcpy(errString,"");
			dbConn.Disconnect();

			logIt.Write("+ OK");
			logIt.LineBreak();
			logIt.Line();
			return 0;
		}

		char buf[256];
		dca::String e(dbConn.GetErrorString());
		sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
		strcpy(errString,buf);

		wsprintfA(buf,"- Error : %s",e.c_str());
		logIt.Write(buf);
	
		logIt.LineBreak();
		logIt.Line();
		return 1;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function TestDBConnection";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// TESTODBC function
//******************************************************************************

int TestODBC(char* server,int useWia,char* username,char* password,char* domain,char* database,char* errString)
{
	long	lResult;  // To hold our result from COM interface
	CLSID	idDVMF;	// CLSID for ComObject
	char	errBuf[256];

	try
	{
		UseOle	useOle;	// Intitialize COM

		// Declare wide character strings to hold our strings passed in
		// They will then be used with the BString object
		WCHAR wServer[256];
		WCHAR wUserName[256];
		WCHAR wPassword[256];
		WCHAR wDomain[256];
		WCHAR wDatabase[256];

		// Clear the following buffers
		ZeroMemory(wUserName, sizeof(wUserName));
		ZeroMemory(wPassword, sizeof(wPassword));

		// Test for Windows Authentication
		if(useWia)
		{
			// If using WIA then fill parameters 
			MultiByteToWideChar(CP_ACP,0,username,-1,wUserName,256);
			MultiByteToWideChar(CP_ACP,0,password,-1,wPassword,256);
			MultiByteToWideChar(CP_ACP,0,domain,-1,wDomain,256);
		}
		else
		{
			// Else fill them with this data only
			MultiByteToWideChar(CP_ACP,0,username,-1,wUserName,256);
			MultiByteToWideChar(CP_ACP,0,password,-1,wPassword,256);
			wDomain[0] = _T('\0');
		}

		// Fill the rest of wchar parameters
		MultiByteToWideChar(CP_ACP,0,server,-1,wServer,256);
		MultiByteToWideChar(CP_ACP,0,database,-1,wDatabase,256);

		// Declare our BString objects
		BString bServer(wServer);
		BString bUserName(wUserName);
		BString bPassword(wPassword);
		BString bDomain(wDomain);
		BString bDatabase(wDatabase);
		BString bErrString;

		// Get our CLSID for DeerfieldEMS.RoutingEngineAdmin COM object
		HRESULT hr = ::CLSIDFromProgID(L"DeerfieldEMS.RoutingEngineAdmin",&idDVMF);
		if(FAILED(hr))
			throw "Failed to get CLSID from Program ID";

		// Create our vmfEng COM object, set running to true to see if already loaded.
		SmartComObject vmfEng(idDVMF,true);

		// Acquire our IRoutingEngineAdmin interface 
		SmartObjectInterface<IRoutingEngineAdmin,&IID_IRoutingEngineAdmin> vmfAdmin(vmfEng);

		// Test the ODBC connection
		vmfAdmin->TestODBC(bServer,
						   useWia,
						   bUserName,
						   bPassword,
						   bDomain,
						   bDatabase,
						   bErrString.GetPointer(),
						   &lResult);

		// Convert our BString object to a CString
		CString cErrString(bErrString);

		// Copy our CString into our errString buffer
		strcpy(errString,(const char*)cErrString);
	}
	catch(const char* err)
	{
		wsprintfA(errBuf,"The following exception occurred [%s] & Error Result[%d]",err,::GetLastError());
		strcpy(errString,errBuf);
		return 0;
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function TestODBC");
		return 0;
	}

	return lResult;
}

//******************************************************************************

//******************************************************************************
// SetServerParameter function 
//******************************************************************************

int SetServerParameter(char* dsn, char* dataValue, int spID, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wdatavalue[256];
	wchar_t cmd[1024];	

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,dataValue,-1,wdatavalue,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* SetServerParameter function");

		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to set server parameter");
			wsprintfW(cmd, L"update ServerParameters set DataValue = '%s' where ServerParameterID = '%d'", wdatavalue,spID);
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("* Failed to update server parameter");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("* Successfully updated server parameter");
				strcpy(errString,"");
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function SetServerParameter";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// GetServerParameter function 
//******************************************************************************

int GetServerParameter(char* dsn, char* dataValue, int spID, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	char cmd[1024];
	wchar_t temp[256];
	char buf[256];
	dca::WString sqlCmd;

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* GetServerParameter function");
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to retrieve server parameter from database");
			sqlCmd.Format(L"select DataValue from ServerParameters where ServerParameterId = '%d'", spID);
			ret = dbConn.GetData((wchar_t*)sqlCmd.c_str(), temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to retrieve server parameter from database");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully retrieved server parameter from database");
				dca::String s(temp);
				strcpy(dataValue,s.c_str());
			}
		}
		else
		{
			logIt.Write("- Failed to connect to database server");
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function GetServerParameter";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// AddMailFlowServer function 
//******************************************************************************

int AddMailFlowServer(char* dsn, char* description, char* regkey, char* tempfolder, char* spoolfolder, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wdescription[256];
	WCHAR wregkey[256];
	WCHAR wtempfolder[256];
	WCHAR wspoolfolder[256];
	wchar_t cmd[1024];

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,description,-1,wdescription,256);
		MultiByteToWideChar(CP_ACP,0,regkey,-1,wregkey,256);
		MultiByteToWideChar(CP_ACP,0,tempfolder,-1,wtempfolder,256);
		MultiByteToWideChar(CP_ACP,0,spoolfolder,-1,wspoolfolder,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* AddMailFlowServer function");
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			wsprintfW(cmd, L"insert into Servers(Description,RegistrationKey,SpoolFolder,TempFolder) values('%s','%s','%s','%s')",
					 wdescription,wregkey,wspoolfolder,wtempfolder);
			logIt.Write("* Attempting to insert the server");
			ret = dbConn.ExecuteSQL(cmd);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to insert the server");
				char buf[256];
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(),c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully inserted the server, returning the ID");				
				wchar_t temp[256];
				dca::WString sqlCmd;
				int sID;
				sqlCmd.Format(L"select ident_current ('servers')");
				ret = dbConn.GetData((wchar_t*)sqlCmd.c_str(), temp);
				sID = _wtoi(temp);
				return sID;
			}
		}
		else
		{
			char buf[256];
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}
	
		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function AddMailFlowServer";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

//******************************************************************************
// GetNextServerID function 
//******************************************************************************

int GetServerID(char* dsn, char* description, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	WCHAR wdescription[256];
	char cmd[1024];
	wchar_t temp[256];
	char buf[256];
	dca::WString sqlCmd;
	int sID;

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);
		MultiByteToWideChar(CP_ACP,0,description,-1,wdescription,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* GetServerID function");
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to retrieve serverid from database");
			sqlCmd.Format(L"SELECT ServerID FROM Servers WHERE Description='%s'", wdescription);
			ret = dbConn.GetData((wchar_t*)sqlCmd.c_str(), temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to retrieve server ID from database");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 0;
			}
			else
			{
				sID = _wtoi(temp);
				logIt.Write("+ Successfully retrieved server ID [%d] from database", sID);				
			}
		}
		else
		{
			logIt.Write("- Failed to connect to database server");
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 0;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return sID;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function GetServerID";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 0;
}

//******************************************************************************

//******************************************************************************
// GetServerLicense function 
//******************************************************************************

int GetServerLicense(char* dsn, int serverID, char* license, char* errString)
{
	LogFile logIt;
	SQLRETURN ret;
	WCHAR wdsn[256];
	char cmd[1024];
	wchar_t temp[256];
	char buf[256];
	dca::WString sqlCmd;

	try
	{
		MultiByteToWideChar(CP_ACP,0,dsn,-1,wdsn,256);

		logIt.Line();
		logIt.Date();
		logIt.Write("* GetServerLicense function");
		CDBConn dbConn;
		logIt.Write("* Attempting connection to database server");
		ret = dbConn.Connect(wdsn);
		if( SQL_SUCCEEDED( ret ) )
		{
			logIt.Write("+ Successfully connected to database server");
			logIt.Write("* Attempting to retrieve license value from database");
			sqlCmd.Format(L"SELECT RegistrationKey FROM Servers WHERE ServerID=%d", serverID);
			ret = dbConn.GetData((wchar_t*)sqlCmd.c_str(), temp);
			if( !SQL_SUCCEEDED( ret ) )
			{
				logIt.Write("- Failed to retrieve license value from database");
				dca::String e(dbConn.GetErrorString());
				dca::String c(cmd);
				sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str());
				strcpy(errString,buf);

				wsprintfA(buf,"- Error : %s",e.c_str());
				logIt.Write(buf);

				dbConn.Disconnect();

				logIt.LineBreak();
				logIt.Line();
				return 1;
			}
			else
			{
				logIt.Write("+ Successfully retrieved license value from database");
				dca::String s(temp);
				strcpy(license,s.c_str());
			}
		}
		else
		{
			logIt.Write("- Failed to connect to database server");
			dca::String e(dbConn.GetErrorString());
			sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
			strcpy(errString,buf);

			wsprintfA(buf,"- Error : %s",e.c_str());
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
			return 1;
		}

		logIt.Write("+ OK");
		logIt.LineBreak();
		logIt.Line();
		return 0;
	}
	catch(...)
	{
		char* errStr = "Unknown Exception occurred in the function GetServerLicense";
		logIt.Write(errStr);
		logIt.LineBreak();
		logIt.Line();
		strcpy(errString,errStr);
	}

	return 1;
}

//******************************************************************************

////////////////////////////// WebSite Functions //////////////////////////////

//******************************************************************************
// CheckURL function 
//******************************************************************************

int CheckURL(char* url,char* ip,char* errString)
{
	char* pIp;
	WORD wVerReq;
	WSADATA wsaData;
	int err;
	HOSTENT* hEnt = NULL;

	wVerReq = MAKEWORD(2,2);
	try
	{
		err = WSAStartup(wVerReq,&wsaData);
		if(err != 0)
		{
			throw "Could not find a usable Winsock dll";
		}

		hEnt = gethostbyname(url);
		if(hEnt != NULL)
		{
			pIp = new char[80];
			pIp = inet_ntoa(*(in_addr*)hEnt->h_addr_list[0]);
			strcpy(ip,pIp);
		}
		else
		{
			WSACleanup();
			throw "Lookup failed. Either the domain is unknown or your DNS is unreachable or out of service.";
		}

		WSACleanup();
		strcpy(errString,"");
		return 0;
	}
	catch(char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function CheckURL");
	}
	return 1;

}

//******************************************************************************

//******************************************************************************
// GetNextWebSiteId function 
//******************************************************************************

void GetNextWebSiteId(char* Id,char* errString)
{
	try
	{
		char* subKey = "SOFTWARE\\Denny\\WebServer\\CurrentVersion\\MultiHome";
		HKEY  result;
		long  rc;
		unsigned long type = REG_BINARY;
		BYTE* data = new BYTE[1024];
		unsigned long  dataSize = 1024;
		char buf[256];

		long val1;
		long val2;
		long val3;
		long val4;

		rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE,subKey,0,KEY_ALL_ACCESS,&result);
		if(rc != ERROR_SUCCESS)
		{
			throw "Failed to Open Registry Key";
		}

		RegQueryValueExA(result,"NextVirtIndex",NULL,&type,data,&dataSize);
		val1 = data[0];
		val2 = data[1];
		val3 = data[2];
		val4 = data[3];

		sprintf(buf,"%02x%02x%02x%02x",val4,val3,val2,val1);
		strcpy(Id,buf);
		
		RegCloseKey(result);
		strcpy(errString,"");
	}
	catch(const char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function GetNextWebSiteId");
	}
}

//******************************************************************************

DllExport int IsIISInstalled(char* errString)
{
	// Search for IIS Install
	
	// Retrieves the path of the system directory
	dca::WString255 sSysDir;
	ZeroMemory(sSysDir, 256);
	::GetSystemDirectoryW(sSysDir, 255);

	LogFile logIt;

	logIt.Line();
	logIt.Date();
	// Stamping it create because that is what it really does.
	// Ignore the old function.
	dca::String s(sSysDir);
	logIt.Write("* IsIISInstalled found sysdir [ %s ]", s.c_str());

	// Append inetsrv\inetinfo.exe for full path
	dca::WString sFullPath(sSysDir);

	dca::WString::size_type pos = sFullPath.find_last_of('\\');
	if(pos != (sFullPath.size() - 1))
		sFullPath.append(L"\\");

	sFullPath.append(L"inetsrv\\inetinfo.exe");

	s.AssignUnicode(sFullPath.c_str());
	logIt.Write("* IsIISInstalled modified sysdir to [ %s ]", s.c_str());

	// Find executable
	_wfinddata64_t fData;
	intptr_t pHandle = _wfindfirst64((wchar_t*)sFullPath.c_str(), &fData);

	if(pHandle != -1)
	{
		logIt.Write("* IsIISInstalled found the file [ %s ]", s.c_str());
		logIt.LineBreak();
		logIt.Line();
		_findclose(pHandle);
		return 1;
	}
	else
	{
		sFullPath.assign(sSysDir);

		if(pos != (sFullPath.size() - 1))
		sFullPath.append(L"\\");

		sFullPath.append(L"inetsrv\\w3wp.exe");

		pHandle = _wfindfirst64((wchar_t*)sFullPath.c_str(), &fData);

		s.AssignUnicode(sFullPath.c_str());

		if(pHandle != -1)
		{
			logIt.Write("* IsIISInstalled found the file [ %s ]", s.c_str());
			logIt.LineBreak();
			logIt.Line();
			_findclose(pHandle);
			return 1;
		}
	}

	try
	{
		dca::Library lib(L"kernel32");
		dca::LibraryFunction<IsWow64Process_type> isWow64Process(lib,"IsWow64Process");
		BOOL bIs64 = FALSE;
		if((isWow64Process)(GetCurrentProcess(), &bIs64) == TRUE)
		{
			if(bIs64)
			{
				logIt.Write("* Running under WOW64");
				logIt.LineBreak();
				logIt.Line();
				strcpy(errString,"WOW64");
				return 1;
			}
			else
			{
				logIt.Write("* Not running under WOW64");
			}
		}
		else
		{
			dca::WString255 sError;
			ZeroMemory(sError, 256);

			dca::Exception::FormatMsg(::GetLastError(), sError, 255);

			dca::String err(sError);
			logIt.Write("* IsWOW64ProcessFailed - %s", err.c_str());
		}
	}
	catch(...)
	{
	}

	logIt.Write("* Did not find the file [ %s ] - %d ( %s )", sFullPath.c_str(), errno, _tcserror(errno));

	logIt.LineBreak();
	logIt.Line();
	strcpy(errString,"");
	return 0;
}

//******************************************************************************
// IsIPLocal function 
//******************************************************************************

int IsIPLocal(char* ip, char* errString)
{
	int retCode = 1;  // Set Return code to failure

	try
	{
		// Define registry keys
		const wchar_t* sRegLinkage = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage";
		const wchar_t* sRegServices = L"SYSTEM\\CurrentControlSet\\Services";

		// Create string list object
		dca::WSTRINGLIST bindingList;

		// Get Bindings for local machine
		{
			dca::RegKey rkLinkage(HKEY_LOCAL_MACHINE, sRegLinkage, dca::RegKey::OPEN);

			rkLinkage.QueryMultiStringValue(L"bind", bindingList);
		}

		// Create iterator for binding list
		dca::WSTRINGLIST::iterator iterBind;

		// Walk through bindings
		for(iterBind = bindingList.begin();
			iterBind != bindingList.end();
			iterBind++)
		{
			// Copy to temp TString object
			dca::WString sBindings((*iterBind));

			// Erase the word device.
			sBindings = sBindings.erase(0, 7);

			// Check for curly bracket do not need NdisWanIp types
			if(sBindings[1] == '{')
			{
				// Create new registry link
				dca::WString sLinkKey(sRegServices);
				sLinkKey.append(sBindings);
				sLinkKey.append(L"\\Parameters\\Tcpip");

				// Let see if we find the ip
				{
					dca::WSTRINGLIST ipAddressList;	// Address String list

					// Query registry for ips
					{
						dca::RegKey rkTCPIP(HKEY_LOCAL_MACHINE, sLinkKey.c_str(), dca::RegKey::OPEN);

						rkTCPIP.QueryMultiStringValue(L"ipaddress", ipAddressList);
					}

					// Create iterator for ip list
					dca::WSTRINGLIST::iterator iterIP;

					// Walk through ips
					for(iterIP = ipAddressList.begin();
						iterIP != ipAddressList.end();
						iterIP++)
					{
						// Compare one found to the one passed in.
						dca::WString wIP(ip);

						if(!wcscmp((*iterIP).c_str(), wIP.c_str()))
						{
							// set retcode to success and break for loop
							retCode = 0;
							break;
						}
					}
				}

				// if retcode is still failure
				if(retCode)
				{
					// Lets check dhcp ip
					dca::WString sDHCPIP;

					// Query registry
					{
						dca::RegKey rkTCPIP(HKEY_LOCAL_MACHINE, sLinkKey.c_str(), dca::RegKey::OPEN);

						long lRetVal = rkTCPIP.QueryStringValue(L"dhcpipaddress", sDHCPIP);
					}

					// Compare string if match then set retcode and break for loop
					dca::WString wIP(ip);

					if(!wcscmp(sDHCPIP.c_str(), wIP.c_str()))
					{
						retCode = 0;
						break;
					}
				}
			}
		}
	}
	catch(dca::Exception e)
	{
		dca::String x(e.GetMessage());
		strcpy(errString, x.c_str());
	}
	catch(LPCSTR err)
	{
		strcpy(errString,err);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function IsIPLocal");
	}

	return retCode;
}

//******************************************************************************
// ResNextIndex function 
//******************************************************************************

void ResNextIndex(char* next,char* cur,char* errString)
{
	try
	{
		char nonzero[256];
		char zero[256];
		char temp[256];
		int x = 0;
		int i;

		int len = strlen(next);
		for(i = 0;i < len;i++)
		{
			if(next[i] != 48)
			{
				nonzero[x] = next[i];
				x++;
			}
		}
		nonzero[x] = '\0';
		x = 0;

		for(i = 0;i < len;i++)
		{
			if(next[i] == 48)
			{
				zero[x] = next[i];
				x++;
			}
		}
		zero[x] = '\0';

		strcpy(temp,zero);
		strcat(temp,nonzero);
		strcpy(cur,temp);
		strcpy(errString,"");
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function ResNextIndex");
	}
}

//******************************************************************************

//******************************************************************************
// SetNextWebSiteId function 
//******************************************************************************

void SetNextWebSiteId(char* oldId,char* errString)
{
	try
	{
		char* subKey = "SOFTWARE\\Denny\\WebServer\\CurrentVersion\\MultiHome";
		HKEY  result;
		long  rc;
		unsigned long type = REG_BINARY;
		BYTE* data = new BYTE[4];
		const unsigned long  dataSize = 4;
		char val4[3];
		char val3[3];
		char val2[3];
		char val1[3];
		int x1;
		int x2;
		int x3;
		int x4;

		val4[0] = oldId[0];
		val4[1] = oldId[1];
		val4[2] = '\0';
		val3[0] = oldId[2];
		val3[1] = oldId[3];
		val3[2] = '\0';
		val2[0] = oldId[4];
		val2[1] = oldId[5];
		val2[2] = '\0';
		val1[0] = oldId[6];
		val1[1] = oldId[7];
		val1[2] = '\0';

		x1 = MakeHex(val1);
		x2 = MakeHex(val2);
		x3 = MakeHex(val3);
		x4 = MakeHex(val4);

		if(x1 == 255)
		{
			x1 = 0;
			x2++;
			if(x2 == 255)
			{
				x2 = 0;
				x3++;
				
				if(x3 == 255)
				{
					x3 = 0;
					x4++;
					if(x4 == 255)
					{
						x1 = 0;
						x2 = 0;
						x3 = 0;
						x4 = 0;
					}
				}
				else
				{
					x3++;
				}
			}
			else
			{
				x2++;
			}
		}
		else
		{
			x1++;
		}

		data[0] = x1;
		data[1] = x2;
		data[2] = x3;
		data[3] = x3;

		rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE,subKey,0,KEY_ALL_ACCESS,&result);
		if(rc != ERROR_SUCCESS)
		{
			throw "Failed to Open Registry Key";
		}

		
		RegSetValueExA(result,"NextVirtIndex",NULL,type,data,dataSize);

		RegCloseKey(result);
		strcpy(errString,"");
	}
	catch(const char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function GetNextWebSiteId");
	}
}

//******************************************************************************

//******************************************************************************
// TestVirtSite function 
//******************************************************************************

int TestVirtSite(char* site,char* errString)
{
	try
	{
		char* subKey = "SOFTWARE\\Denny\\WebServer\\CurrentVersion\\MultiHome\\HostName";
		HKEY  result;
		long  rc;
		unsigned long count;
		char*	val1 = new char[256];
		BYTE* data = new BYTE[1024];
		unsigned long  dataSize = 1024;
		unsigned long type = REG_SZ;
		
		unsigned long valLen = 256;

		rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE,subKey,0,KEY_ALL_ACCESS,&result);
		if(rc != ERROR_SUCCESS)
		{
			throw "Failed to Open Registry Key";
		}

		rc = RegQueryInfoKey(result,NULL,NULL,NULL,NULL,NULL,NULL,&count,NULL,NULL,NULL,NULL);

		for(unsigned long i = 0;i < count;i++)
		{
			rc = RegEnumValueA(result,i,val1,&valLen,NULL,&type,data,&dataSize);
			char* test = (char*)data;
			if(!strcmp(_strlwr(test),_strlwr(site)))
			{
				RegCloseKey(result);
				strcpy(errString,"");
				return 1;
			}
			valLen = 256;
			dataSize = 1024;
		}

		RegCloseKey(result);
		strcpy(errString,"");
		return 0;
	}
	catch(const char* msg)
	{
		strcpy(errString,msg);
	}
	catch(...)
	{
		strcpy(errString,"Unknown Exception Occurred in function TestVirtSite");
	}
	
	return 0;
}

int GetRegistryInt(char* subKey, char* valName)
{
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey;
	UINT nValue;

	DWORD dwBuffLen = sizeof(UINT);
	DWORD dwType;
	
	// open reg key
	if ((lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, NULL, KEY_EXECUTE|0x0100, &hKey)) != ERROR_SUCCESS)		
		return lRet;

	// query for value
	lRet = RegQueryValueExA(hKey, valName, NULL, &dwType, (BYTE*) &nValue, &dwBuffLen);

	// if the type was wrong
	if (dwType != REG_DWORD)
		lRet = -1;

	// close the registry key
	RegCloseKey(hKey);
	
	return nValue;
}


//******************************************************************************
