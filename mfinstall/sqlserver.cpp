#include "stdafx.h"
#include "sqlserver.h"

using namespace sql;

Server::Server() : SmartComObject(CLSID_SQLDMOServer,TRUE),
				   _server(SmartComObject(CLSID_SQLDMOServer,TRUE))
{}

int Server::AddDB(ISQLDMODatabase* db)
{
	HRESULT hr;

	hr = _databases->Add(db);

	return HRESULT_CODE(hr);
}

int Server::Close()
{
	HRESULT hr;

	hr = _server->Close();

	return HRESULT_CODE(hr);
}

int Server::Connect(const char* server,const char* user,const char* password)
{
	HRESULT hr;
	WCHAR	sqlServer[BUF_SIZE];
	WCHAR   sqlUser[BUF_SIZE];
	WCHAR   sqlPassword[BUF_SIZE];

	MultiByteToWideChar(CP_ACP,0,server,-1,sqlServer,BUF_SIZE);
	MultiByteToWideChar(CP_ACP,0,user,-1,sqlUser,BUF_SIZE);
	MultiByteToWideChar(CP_ACP,0,password,-1,sqlPassword,BUF_SIZE);

	hr = _server->Connect(sqlServer,sqlUser,sqlPassword);
	if(SUCCEEDED(hr))
	{
		_server->GetDatabases(_databases);
	}
		
	return HRESULT_CODE(hr);
}

int Server::GetDatabaseCount()
{
	long count;

	_databases->GetCount(&count);

	return (int)count;
}

int Server::GetDatabaseByName(const char* name,ISQLDMODatabase** database)
{
	HRESULT hr;
	WCHAR	dbName[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,name,-1,dbName,BUF_SIZE);

	hr = _server->GetDatabaseByName(dbName,database,NULL);
		
	return HRESULT_CODE(hr);
}

int Server::RemoveDatabaseByName(const char* name)
{
	HRESULT hr;
	WCHAR	dbName[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,name,-1,dbName,BUF_SIZE);

	hr = _server->RemoveDatabaseByName(dbName,NULL);
		
	return HRESULT_CODE(hr);
}

int Server::RemoveLoginByName(const char* name)
{
	HRESULT hr;
	WCHAR	dbName[BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,name,-1,dbName,BUF_SIZE);

	hr = _server->RemoveLoginByName(dbName);
		
	return HRESULT_CODE(hr);
}