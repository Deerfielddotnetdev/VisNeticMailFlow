/******************************************************************************
/* MailFlowInstall.DLL - A helper dll for Wise Installer
/* Property of Deerfield.com (copyright 2002)
/*
/* Written by Mark Mohr on 03/19/2002
/* main.h - Declaration file
/*****************************************************************************/
#define INITGUID

#include <windows.h>
#include "resstring.h"
#include "webserver.h"
#include "database.h"
#include "sysinfo.h"
#include "resource.h"
#include "sqlapplication.h"
#include "sqlserver.h"
#include "sqldatabase.h"
#include "stdafx.h"
//#include "MyContent.h"
#include "SAXErrorHandlerImpl.h"
#include <fstream>
#include <memory>
#include <lm.h>
#include <Lmserver.h>

list<CDatabase> g_Databases;

#define DllExport extern "C" __declspec( dllexport )

// External Functions
DllExport int GetRegistryInt(char* subKey, char* valName );
DllExport int ChangeDBOwner(char* dsn, char* type, char* user,char* errString);
DllExport int CreateDBUser(char* dsn, char* type, char* user, char* pass, char* errString);
DllExport int ChangeServiceToAuto(char* errString);
DllExport int ConfigDataBase(char* file,char* dsn,char* type,char* errString);
DllExport int CreateWeb(char* siteNum, char* bindings, char* webDir, char* isapiPath, int reset, int isVirt, char* errString, int servExt);
DllExport int DecryptPassword(char* inPass,char* outPass,char* errString);
DllExport void DecryptString(char* text,char* errString);
DllExport int DeleteDB(char* dsn,char* user,char* password,char* errstring);
DllExport int DeleteWeb(char* siteNum,char* errString);
DllExport int DetectWeb(char* siteNum,char* errString);
DllExport int DoesDBExist(char* dsn,char* dbVer,char* errstring);
DllExport int EncryptPassword(char* inPass,char* outPass,char* errString);
DllExport void EncryptString(char* text,char* errString);
DllExport int GetIISSiteCount(char* errString);							// Added 2/25/2003
DllExport int GetIISSiteID(char* sSiteName,char* errString);			// Added 2/25/2003
DllExport int GetIISSiteNames(char* sSiteName,char* errString);			// Added 2/25/2003
DllExport void GetInsError(char* err,char* errString);
DllExport void GetInsWarning(char* warn,char* errString);
DllExport int GetLicense(char* dsn, char* license, char* errString);
DllExport int GetSQLServers(char* servers,char* errString);
DllExport int InsertRoutingRule(char* dsn,char* desc, int tickBox, char* toEM, char* fromEM, char* errString);
DllExport int InsertTicketBox(char* dsn, char* name, char* desc, char* email, char* errString);
DllExport int IsSQLSevenOrHigher(char* errString);
DllExport int ReseedTicketTable(char* dsn,int nSeedNum,char* errString);
DllExport int SetAdminEmail(char* dsn,char* email, char* errString);
DllExport int SetAdminPassword(char* dsn, char* password, char* errString);
DllExport int SetDBUserPassword(char* dsn, char* oldp, char* newp, char* user, char* errString);
DllExport int SetDefaultEmail(char* dsn,char* email,char* errString);
DllExport int SetLicense(char* dsn, char* license, char* errString);
DllExport int SetMessageDestination(char* dsn, char* srvAddr, int UseSMTPAuth,
									char* user, char* pass, int port, int active,
									int freqMins, int UseSSL, char* description,char* errString);
DllExport int SetMessageSources(char* dsn, char* remoteAddr, int port, char* user,
								char* pass, int aPop,int lvmsg, int active, int freqMins, int UseSSL,
								int messId, int messSrcType, char* description, char* errString);
DllExport int StopOtherWebSites(char* siteNum,char* errString);
DllExport int TestDBConnection(char* dsn, char* errString);
DllExport int TestODBC(char* server,int useWia,char* username,char* password,char* domain,char* database,char* errString);
DllExport int IsIISInstalled(char* errString);
DllExport int SetServerParameter(char* dsn, char* dataValue, int spID, char* errString);
DllExport int GetServerParameter(char* dsn, char* dataValue, int spID, char* errString);
DllExport int AddMailFlowServer(char* dsn, char* description, char* regkey, char* tempfolder, char* spoolfolder, char* errString);
DllExport int GetServerID(char* dsn, char* description, char* errString);
DllExport int GetServerLicense(char* dsn, int serverID, char* license, char* errString);

// WebSitePro External Functions
DllExport int CheckURL(char* url, char* ip, char* errString);
DllExport void GetNextWebSiteId(char* Id,char* errString);
DllExport int IsIPLocal(char* ip,char* errString);
DllExport void ResNextIndex(char* next,char* cur, char* errString);
DllExport void SetNextWebSiteId(char* oldId,char* errString);
DllExport int TestVirtSite(char* site,char* errString);

// Local Functions
void GetRootPath(const char* site,char* rootPath);
void GetAppRoot(const char* site,char* appPath);
void GetIsapiMap1(const char* lnPath, char* mapName);
void GetIsapiMap2(const char* lnPath, char* mapName);
void GetIsapiMap3(const char* lnPath, char* mapName);
void CreateDatabase(CDatabase& database, dca::SAXDBTABLELIST& tableList);
int CreateMSSQLDatabase( CDatabase& database, dca::SAXDBTABLELIST& tableList); // Also works for MSDE
int CreateMySQLDatabase( CDatabase& database, dca::SAXDBTABLELIST& tableList);
int GetHexFromChar(char c);
int MakeHex(char* hex);
void ShiftText(char* in,char* out);
int HasValidCharacters(char* in);
void AddMimeType(iis::WebServer& webServ, const wchar_t* type);

