///////////////////////////////////////////////////////////////////////////////
//
// SQLServer class header file
// Written by Mark Mohr on 03/29/2002
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SQLSERVER_H
#define SQLSERVER_H

#define INITGUID

#include "olebase.h"
#include "objects.h"
#include "interface.h"
#include "SQLDMOid.h"
#include "SQLDMO.h"

namespace sql
{
	class Server : public SmartComObject
	{
		enum {BUF_SIZE = 256};
	public:
		Server();
		int AddDB(ISQLDMODatabase* db);
		int Close();
		int Connect(const char* server = "",const char* user = "sa",const char* password = "");
		int GetDatabaseCount();
		int GetDatabaseByName(const char* name,ISQLDMODatabase** database);
		int RemoveDatabaseByName(const char* name);
		int RemoveLoginByName(const char* name);
	private:
		SmartObjectInterface<ISQLDMOServer,&IID_ISQLDMOServer>  _server;
		SmartCollectionInterface<ISQLDMODatabases>				_databases;
	};
}

#endif // SQLSERVER_H