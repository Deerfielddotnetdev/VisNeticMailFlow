
#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;

int VExportXML(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	XmlFile g_xmlFile;

	SQLRETURN	retSQL = 0;
	TCHAR m_TableName[256];
	long m_TableNameLen = sizeof(m_TableName);
	TCHAR m_ColumnName[256];
	long m_ColumnNameLen = sizeof(m_ColumnName);
	TCHAR m_ColumnType[256];
	long m_ColumnTypeLen = sizeof(m_ColumnType);
	TCHAR m_ColumnDefault[256];
	long m_ColumnDefaultLen = sizeof(m_ColumnDefault);
	TCHAR m_ColumnNullable[256];
	long m_ColumnNullableLen = sizeof(m_ColumnNullable);
	int m_ColumnSize;
	long m_ColumnSizeLen;
	
	tstring sTemp;
	tstring sColumnType;
	CEMSString columnName;
	CEMSString tableName;
	CEMSString sQuery;
	CEMSString cTemp;
	
	CEMSString columnDefault;
	vector<tstring> m_name;
	vector<tstring>::iterator iter;
	m_name.clear();
	
	g_xmlFile.Write( _T("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>") );
	g_xmlFile.Write( _T("<root>") );
	if(g_showResults != 0)
	{
		_tcout << _T("Getting tables from database") << endl;
	}

	g_logFile.Write(_T("Getting tables from database"));
	
	sQuery.Format( _T("SELECT name FROM sysobjects WHERE type=%s ORDER BY name"), _T("'u'") );
		
	m_query.Initialize();
	BINDCOL_TCHAR( m_query, m_TableName );
	m_query.Execute( sQuery.c_str() );
	
	while( m_query.Fetch() == S_OK )
	{
		m_name.push_back(m_TableName);
	}

	for(iter = m_name.begin(); iter != m_name.end(); ++iter)
	{
		tableName.Format( _T("   <table name=\"%s\">"), iter->c_str() );
		_tcout << tableName.c_str() << endl;
		g_xmlFile.Write(tableName.c_str());
		
		sQuery.Format( _T("SELECT column_name,column_default,is_nullable,data_type,character_maximum_length FROM information_schema.columns WHERE table_name='%s' ORDER BY ordinal_position"), iter->c_str() );
		
		m_query.Initialize();
		BINDCOL_TCHAR( m_query, m_ColumnName );
		BINDCOL_TCHAR( m_query, m_ColumnDefault );
		BINDCOL_TCHAR( m_query, m_ColumnNullable );
		BINDCOL_TCHAR( m_query, m_ColumnType );
		BINDCOL_LONG( m_query, m_ColumnSize );
		m_query.Execute( sQuery.c_str() );
		
		int x=0;
		while( m_query.Fetch() == S_OK )
		{
			sColumnType.clear();
			sTemp.clear();
			sTemp.assign(m_ColumnType);
			
			if ( sTemp == _T("int") )
			{
				sColumnType.assign(_T("INTEGER"));
			}
			else if ( sTemp == _T("bit") )
			{
				sColumnType.assign(_T("BIT"));
			}
			else if ( sTemp == _T("tinyint") )
			{
				sColumnType.assign(_T("TINYINT"));
			}
			else if ( sTemp == _T("datetime") )
			{
				sColumnType.assign(_T("DATETIME"));
			}
			else if ( sTemp == _T("varchar") )
			{
				cTemp.Format( _T("VARCHAR(%d)"), m_ColumnSize );
				sColumnType.assign( cTemp.c_str() );
			}
			else if ( sTemp == _T("text") )
			{
				sColumnType.assign(_T("TEXT"));
			}	
			
			if ( x == 0 )
			{
                columnName.Format( _T("      <column name=\"%s\" type=\"%s\" primarykey=\"1\" identity=\"1\"></column>"), m_ColumnName, sColumnType.c_str() );
			}
			else
			{
				sTemp.clear();
				sTemp.assign(m_ColumnNullable);
				if ( sTemp == _T("YES") )
				{
					columnName.Format( _T("      <column name=\"%s\" type=\"%s\" allownulls=\"1\"></column>"), m_ColumnName, sColumnType.c_str() );
				}
				else
				{
					columnName.Format( _T("      <column name=\"%s\" type=\"%s\"></column>"), m_ColumnName, sColumnType.c_str() );
				}
				
			}
			_tcout << columnName.c_str() << endl;
			g_xmlFile.Write(columnName.c_str());
			
			x++;
		}

		g_xmlFile.Write( _T("   </table>") );
		
	}

	g_xmlFile.Write( _T("</root>") );
	return 0;

}