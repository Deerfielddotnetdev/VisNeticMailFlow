#include "stdafx.h"
#include "sqldatabase.h"

using namespace sql;

Database::Database() : SmartComObject(CLSID_SQLDMODatabase,TRUE),
						_database(SmartComObject(CLSID_SQLDMODatabase,TRUE))
{}

int Database::GetFileGroupByName(ISQLDMOFileGroup** fileGroup, const char* name)
{
	HRESULT hr;
	WCHAR sqlFileGroup [BUF_SIZE];
	MultiByteToWideChar(CP_ACP,0,name,-1,sqlFileGroup,BUF_SIZE);

	hr = _filegroups->GetItemByName(sqlFileGroup,fileGroup);

	return HRESULT_CODE(hr);
}

int Database::Intialize()
{
	HRESULT hr;
	int rc;

	hr = _database->GetFileGroups(_filegroups);

	if(FAILED(hr))
	{
		rc = HRESULT_CODE(hr);
	}

	hr = _database->GetTables(_tables);

	if(FAILED(hr))
	{
		rc = HRESULT_CODE(hr);
	}

	return rc;
}

int Database::SetName(const char* name)
{
	HRESULT hr;
	WCHAR sqlDBName[BUF_SIZE];

	MultiByteToWideChar(CP_ACP,0,name,-1,sqlDBName,BUF_SIZE);

	hr = _database->SetName(sqlDBName);

	return HRESULT_CODE(hr);
}

int Database::SetOwner(const char* name, int alias, int override)
{
	HRESULT hr;
	WCHAR sqlOwner[BUF_SIZE];

	MultiByteToWideChar(CP_ACP,0,name,-1,sqlOwner,BUF_SIZE);

	hr = _database->SetOwner(sqlOwner,alias,override);

	return HRESULT_CODE(hr);
}
