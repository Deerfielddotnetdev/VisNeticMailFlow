// MyContent.cpp: implementation of the MyContent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Table.h"
#include "Database.h"
#include "DBConn.h"
#include "LogIt.h"

static CDBConn conn;
static wchar_t szSQL[10240];
static dca::SAXDBTable* pCurrentTable = NULL;

void CreateTable(dca::SAXDBTable& saxTable)
{
	dca::SAXDBCOLUMNLIST::iterator iter;

	dca::WString sqlCommand(L"CREATE TABLE ");
	sqlCommand.append(saxTable.GetName());
	sqlCommand.append(L" (");

	for(iter = saxTable.GetColumnListRef().begin();
		iter != saxTable.GetColumnListRef().end();
		iter++)
	{
		dca::WString sqlCol(iter->GetName());
		sqlCol.append(L" ");
		sqlCol.append(iter->GetType());

		if(!iter->GetAllowNulls())
			sqlCol.append(L" NOT NULL");

		if(iter->GetIdentity())
			sqlCol.append(L" IDENTITY");

		if(iter->GetPrimaryKey())
			sqlCol.append(L" PRIMARY KEY");

		sqlCol.append(L",");
		sqlCommand.append(sqlCol);
	}

	dca::WString::size_type npos = sqlCommand.find_last_of(',');
	if(npos != dca::WString::npos)
		sqlCommand.at(npos) = ')';

	SQLRETURN ret = conn.ExecuteSQL((wchar_t*)sqlCommand.c_str());

	if( !SQL_SUCCEEDED( ret ) )
	{
		throw dca::Exception(L"Create Table Error: %s\nSQL:%s\n\n", conn.GetErrorString(), szSQL);
	}
}

void CreateIndices(dca::SAXDBTable& saxTable)
{
	dca::SAXDBINDEXLIST::iterator iter;
	LogFile logIt;

	for(iter = saxTable.GetIndexListRef().begin();
		iter != saxTable.GetIndexListRef().end();
		iter++)
	{
		dca::WString sqlCommand(L"CREATE INDEX ");
		sqlCommand.append(iter->GetName());
		sqlCommand.append(L" ON ");
		sqlCommand.append(saxTable.GetName());
		sqlCommand.append(L"(");
		sqlCommand.append(iter->GetColumn());
		sqlCommand.append(L")");

		SQLRETURN ret = conn.ExecuteSQL((wchar_t*)sqlCommand.c_str());

		if( !SQL_SUCCEEDED( ret ) )
		{
			char buf[256];
			dca::String e(conn.GetErrorString());
			dca::String c(szSQL);
			sprintf(buf, "Create Index Error: %s\nSQL:%s\n\n", e.c_str(), c.c_str() );
			logIt.Write(buf);

			logIt.LineBreak();
			logIt.Line();
		}
	}
}

void SetDefaults(dca::SAXDBTable& saxTable)
{
	dca::SAXDBCOLUMNLIST::iterator iter;

	for(iter = saxTable.GetColumnListRef().begin();
		iter != saxTable.GetColumnListRef().end();
		iter++)
	{
		if(lstrcmpW(iter->GetDefault(), L"-1"))
		{
			dca::WString sqlCommand(L"ALTER TABLE ");
			sqlCommand.append(saxTable.GetName());
			sqlCommand.append(L" ADD CONSTRAINT DF_");
			sqlCommand.append(saxTable.GetName());
			sqlCommand.append(iter->GetName());
			if(!lstrcmpW(iter->GetDefault(), L"GetDate()"))
			{
				sqlCommand.append(L" DEFAULT (GetDate()) FOR ");
			}
			else
			{
				sqlCommand.append(L" DEFAULT ('");
				sqlCommand.append(iter->GetDefault());
				sqlCommand.append(L"') FOR ");
			}
			sqlCommand.append(iter->GetName());

			SQLRETURN ret = conn.ExecuteSQL((wchar_t*)sqlCommand.c_str());

			if( !SQL_SUCCEEDED( ret ) )
			{
				throw dca::Exception(L"Set Default Error: %s\nSQL:%s\n\n", conn.GetErrorString(), szSQL );
			}	
		}
	}
}

void InsertData(dca::SAXDBTable& saxTable)
{
	dca::SAXDBDATALIST::iterator iter;

	for(iter = saxTable.GetDataListRef().begin();
		iter != saxTable.GetDataListRef().end();
		iter++)
	{
		dca::WString sqlCommand(L"INSERT INTO ");
		sqlCommand.append(saxTable.GetName());
		sqlCommand.append(L" ");
		sqlCommand.append(iter->GetColumns());
		sqlCommand.append(L" VALUES ");
		sqlCommand.append(iter->GetValues());

		SQLRETURN ret = conn.ExecuteSQL((wchar_t*)sqlCommand.c_str());

		if( !SQL_SUCCEEDED( ret ) )
		{
			throw dca::Exception(L"Insert Data Error: %s\nSQL:%s\n\n", conn.GetErrorString(), szSQL );
		}
	}
}

void CreateForeignKeys(dca::SAXDBTable& saxTable)
{
	dca::SAXDBFOREIGNKEYLIST::iterator iter;

	for(iter = saxTable.GetForeignKeyRef().begin();
		iter != saxTable.GetForeignKeyRef().end();
		iter++)
	{
		dca::WString sqlCommand(L"ALTER TABLE ");
		sqlCommand.append(saxTable.GetName());
		sqlCommand.append(L" ADD CONSTRAINT ");
		sqlCommand.append(iter->GetName());
		sqlCommand.append(L" FOREIGN KEY (");
		sqlCommand.append(iter->GetColumn());
		sqlCommand.append(L") REFERENCES ");
		sqlCommand.append(iter->GetForeignTable());
		sqlCommand.append(L"(");
		sqlCommand.append(iter->GetForeignColumn());
		sqlCommand.append(L")");

		SQLRETURN ret = conn.ExecuteSQL((wchar_t*)sqlCommand.c_str());

		if( !SQL_SUCCEEDED( ret ) )
		{
			throw dca::Exception(L"Create Foreign Key Error: %s\nSQL:%s\n\n", conn.GetErrorString(), szSQL );
		}
	}
}


int CreateMSSQLDatabase( CDatabase& database, dca::SAXDBTABLELIST& tableList)
{
	SQLRETURN ret;

	ret = conn.Connect( database.m_wcsDSN );
	
	if( SQL_SUCCEEDED( ret ) )
	{

		if( wcsicmp( database.m_wcsAction, L"CREATE" ) == 0 )
		{
			// This part removes the existing database.
			ret = conn.ExecuteSQL( L"use master" );

			if( !SQL_SUCCEEDED( ret ) )
			{
				char buf[256];
				dca::String e(conn.GetErrorString());
				sprintf(buf,"Error: %s\nSQL: use master\n\n", e.c_str());
				throw buf;
			}

			wchar_t sBuf[MAX_PATH];
			ret = conn.GetData(L"sp_server_info 500", sBuf,3);

			int ver = _wtoi(sBuf);

			// This part creates the new database
			if(ver >= 8)
			{
				swprintf( szSQL, L"create database %s collate SQL_Latin1_General_CP1_CI_AI", database.m_wcsName );
			}
			else
			{
				swprintf( szSQL, L"create database %s", database.m_wcsName );
			}

			ret = conn.ExecuteSQL( szSQL );

			if( !SQL_SUCCEEDED( ret ) )
			{
				char buf[256];
				dca::String e(conn.GetErrorString());
				dca::String c(szSQL);
				sprintf(buf,"Fatal Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str() );
				throw buf;
			}
		}


		// Select the new database
		swprintf( szSQL, L"use %s", database.m_wcsName );

		ret = conn.ExecuteSQL( szSQL );
		
		if( !SQL_SUCCEEDED( ret ) )
		{
			char buf[256];
			dca::String e(conn.GetErrorString());
			dca::String c(szSQL);
			sprintf(buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str() );
			throw buf;
		}

		dca::SAXDBTABLELIST::iterator i;

		if( wcsicmp( database.m_wcsAction, L"CREATE" ) == 0 )
		{
			// Create the tables
			//for_each( tableList.begin(), tableList.end(), MSSQLCreateTable );

			for(i = tableList.begin();
				i != tableList.end();
				i++)
			{
				CreateTable((*i));
				CreateIndices((*i));
				SetDefaults((*i));
			}
		}

		for(i = tableList.begin();
			i != tableList.end();
			i++)
		{
			InsertData((*i));
		}


		// Insert Data
		//for_each( tableList.begin(), tableList.end(), MSSQLInsertTableData );

		if( wcsicmp( database.m_wcsAction, L"CREATE" ) == 0 )
		{
			// Create the relationships
			//for_each( tableList.begin(), tableList.end(), MSSQLCreateTableRelationships );
			for(i = tableList.begin();
				i != tableList.end();
				i++)
			{
				CreateForeignKeys((*i));
			}
		}

		ret = conn.Disconnect();
	}
	else
	{
		char buf[256];
		dca::String e(conn.GetErrorString());
		sprintf(buf,"Error connecting to database: %s\n", e.c_str() );
		throw buf;
	}

	return 0;
}