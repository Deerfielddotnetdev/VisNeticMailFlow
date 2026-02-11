#include "prehead.h"
#include "main.h"

int EncryptMessageDestPasswords(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL;
	
	//*************************************************************************
	
	if(g_showResults != 0)
	{
		_tcout << _T("Encrypting MessageDestination passwords") << endl;
	}

	g_logFile.Write(_T("Encrypting MessageDestination passwords"));

	TCHAR password[256];
	TCHAR test[256];
	TCHAR buf[256];
	int num_rows = 0;
		
	// Add Agents to TicketBoxViews
	retSQL = dbConn.GetData(_T("select count(*) from MessageDestinations"),test);
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	num_rows = _ttoi(test);
	num_rows++;

	retSQL = dbConn.ExecuteSQL(_T("declare i_cursor cursor for select MessageDestinationID from MessageDestinations"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("open i_cursor"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	for(int i = 1; i < num_rows; i++)
	{
		dbConn.GetData(_T("fetch next from i_cursor"),test);

		wsprintf(buf,_T("select AuthPass from MessageDestinations where MessageDestinationID = %s"),test);
		dbConn.GetData(buf,password);

		OutputDebugString(password);
		OutputDebugString(_T("\n"));
		dca::EString EStr(password);
		EStr.Encrypt();
		_tcscpy(password,EStr.c_str());
		OutputDebugString(password);
		OutputDebugString(_T("\n"));

		wsprintf(buf,_T("update MessageDestinations set AuthPass = '%s' where MessageDestinationID = %s"),password,test);
		OutputDebugString(buf);
		OutputDebugString(_T("\n"));
		retSQL = dbConn.ExecuteSQL(buf);
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(dbConn.GetErrorString());
			OutputDebugString(_T("\n"));
		}
	}

	retSQL = dbConn.ExecuteSQL(_T("close i_cursor"));
	retSQL = dbConn.ExecuteSQL(_T("deallocate i_cursor"));

	return 0;
}