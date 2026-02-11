// MyContent.cpp: implementation of the MyContent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Table.h"
#include "Database.h"

#include "DBConn.h"

static CDBConn conn;
static char szSQL[10240];
static dca::SAXDBTable* pCurrentTable = NULL;

void MySQLCreateIndex(dca::SAXDBIndex& index )
{
	SQLRETURN ret;

	swprintf( szSQL, L"CREATE INDEX %s on %s(%s)", index.GetName(),
		    pCurrentTable->GetName(), index.GetColumn() );

	ret = conn.ExecuteSQL( szSQL );

	if( !SQL_SUCCEEDED( ret ) )
	{
		char buf[256];
		dca::String e(conn.GetErrorString());
		dca::String c(szSQL);
		sprintf(buf,"Create Index Error: %s\nSQL:%s\n\n", e.c_str(), c.c_str() );
		throw buf;
	}
}

void MySQLCreateColumn(dca::SAXDBColumn& column )
{
	wchar_t szColumn[1024];

	swprintf( szColumn, L"%s %s ", column.GetName(), column.GetType() );

	if( column.GetAllowNulls() == 0 )
		strcat( szColumn, "not null" );

	if( column.GetDefault()[0] )
	{
		char szDefault[256];
		swprintf( szDefault, L" DEFAULT '%s'", column.GetDefault() );
		strcat( szColumn, szDefault );
	}

	if( column.GetIdentity() )
		strcat( szColumn, " AUTO_INCREMENT" );

	if( column.GetPrimaryKey() )
		strcat( szColumn, " PRIMARY KEY" );

	strcat( szColumn, "," );

	strcat( szSQL, szColumn );
}


void MySQLInsertData(dca::SAXDBData& data )
{
	SQLRETURN ret;

	swprintf( szSQL, L"INSERT INTO %s %s VALUES %s", 
		pCurrentTable->GetName(), data.GetColumns(), data.GetValues() );

	ret = conn.ExecuteSQL( szSQL );

	if( !SQL_SUCCEEDED( ret ) )
	{
		char buf[256];
		dca::String e(conn.GetErrorString());
		dca::String c(szSQL);
		sprintf(buf,"Insert Data Error: %s\nSQL:%s\n\n", e.c_str(), c.c_str() );
		throw buf;
	}
}

void MySQLCreateTable(dca::SAXDBTable& table )
{
	SQLRETURN ret;

	swprintf( szSQL, L"create table %s (\n", table.GetName() );

	for_each( table.GetColumnListRef().begin(), table.GetColumnListRef().end(), MySQLCreateColumn );

	// Remove the final comma and replace with parenthesis
	strcpy( szSQL + strlen(szSQL) - 1, ")" );

	ret = conn.ExecuteSQL( szSQL );

	if( !SQL_SUCCEEDED( ret ) )
	{
		char buf[256];
		dca::String e(conn.GetErrorString());
		dca::String c(szSQL);
		sprintf(buf,"Create Table Error: %s\nSQL:%s\n\n", e.c_str(), c.c_str() );
		throw buf;
	}

	pCurrentTable = &table;

	// Create the indices
	for_each( table.GetIndexListRef().begin(), table.GetIndexListRef().end(), MySQLCreateIndex );
}


void MySQLInsertTableData(dca::SAXDBTable& table )
{
	pCurrentTable = &table;
	
	// Insert data
	for_each( table.GetDataListRef().begin(), table.GetDataListRef().end(), MySQLInsertData );
}


void MySQLCreateForeignKey(dca::SAXDBForeignKey& foreignkey )
{
	SQLRETURN ret;

	swprintf( szSQL, L"alter table %s add constraint %s foreign key (%s) references %s(%s)", 
		pCurrentTable->GetName(), foreignkey.GetName, foreignkey.GetColumn(),
		foreignkey.GetForeignTable(), foreignkey.GetForeignColumn() );

	ret = conn.ExecuteSQL( szSQL );

	if( !SQL_SUCCEEDED( ret ) )
	{
		char buf[256];
		dca::String e(conn.GetErrorString());
		dca::String c(szSQL);
		sprintf(buf,"Create Foreign Key Error: %s\nSQL:%s\n\n", e.c_str(), c.c_str() );
		throw buf;
	}
}

void MySQLCreateTableRelationships(dca::SAXDBTable& table )
{
	pCurrentTable = &table;

	for_each( table.GetForeignKeyRef().begin(), table.GetForeignKeyRef().end(), MySQLCreateForeignKey );
}


int CreateMySQLDatabase( CDatabase& database, dca::SAXDBTABLELIST& tableList)
{
	SQLRETURN ret;

	ret = conn.Connect( database.m_wcsDSN );
	
	if( SQL_SUCCEEDED( ret ) )
	{

		if( wcsicmp( database.m_wcsAction, L"CREATE" ) == 0 )
		{
			// This part removes the existing database.
			//sprintf( szSQL, "drop database %S", database.m_wcsName );

			//ret = conn.ExecuteSQL( szSQL );

			//if( !SQL_SUCCEEDED( ret ) )
			//{
				//char buf[256];
				//sprintf(buf,"Fatal Error: %s\nSQL: %s\n\n", conn.GetErrorString(), szSQL );
				//throw buf;
			//}

			// This part creates the new database
			swprintf( szSQL, L"create database %s", database.m_wcsName );

			ret = conn.ExecuteSQL( szSQL );

			if( !SQL_SUCCEEDED( ret ) )
			{
				char buf[256];
				dca::String e(conn.GetErrorString());
				dca::String c(szSQL);
				sprintf(buf, "Fatal Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str() );
				throw buf;
			}
		}

		// Select the new database
		sprintf( szSQL, L"use %s", database.m_wcsName );

		ret = conn.ExecuteSQL( szSQL );
		
		if( !SQL_SUCCEEDED( ret ) )
		{
			char buf[256];
			dca::String e(conn.GetErrorString());
			dca::String c(szSQL);
			sprintf( buf,"Error: %s\nSQL: %s\n\n", e.c_str(), c.c_str() );
			throw buf;
		}

		if( wcsicmp( database.m_wcsAction, L"CREATE" ) == 0 )
		{
			// Create the tables
			for_each( tableList.begin(), tableList.end(), MySQLCreateTable );
		}

		// Insert Data
		for_each( tableList.begin(), tableList.end(), MySQLInsertTableData );

		if( wcsicmp( database.m_wcsAction, L"CREATE" ) == 0 )
		{
			// Create the relationships
			for_each( tableList.begin(), tableList.end(), MySQLCreateTableRelationships );
		}


		ret = conn.Disconnect();
	}
	else
	{
		char buf[256];
		dca::String e(conn.GetErrorString());
		sprintf(buf, "Error connecting to database: %s\n", e.c_str() );
		throw buf;
	}

	return 0;
}