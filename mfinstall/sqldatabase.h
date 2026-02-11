///////////////////////////////////////////////////////////////////////////////
//
// SQLDatabase class header file
// Written by Mark Mohr on 03/29/2002
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#define INITGUID

#include "olebase.h"
#include "objects.h"
#include "interface.h"
#include "SQLDMOid.h"
#include "SQLDMO.h"
#include "sqlfilegroup.h"

namespace sql
{
	class Database : public SmartComObject
	{
		enum {BUF_SIZE = 256};
	public:
		Database();
		operator ISQLDMODatabase* () { return _database; }
		operator ISQLDMODatabase** () { return _database; }
		int AddTable(ISQLDMOTable* table)
		{
			HRESULT hr;

			hr = _database->AddTable(table);

			return HRESULT_CODE(hr);
		}
		int Intialize();
		int GetFileGroupByName(ISQLDMOFileGroup** fileGroup, const char* name = "PRIMARY");
		int SetName(const char* name);
		int SetOwner(const char* name,int alias = 0, int override = 0);
	private:
		SmartObjectInterface<ISQLDMODatabase,&IID_ISQLDMODatabase> _database;
		SmartCollectionInterface<ISQLDMOFileGroups>				   _filegroups;
		SmartCollectionInterface<ISQLDMOTables>					   _tables;
	};
}

#endif