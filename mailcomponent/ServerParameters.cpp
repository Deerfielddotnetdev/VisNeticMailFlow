// ServerParameters.cpp: implementation of the CServerParameters class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailComponents.h"
#include "ServerParameters.h"
#include "ODBCConn.h"
#include "ODBCQuery.h"
#include "SingleCrit.h"
#include "RegistryFns.h"
#include <sstream>

extern dca::Mutex g_csDB;
extern CODBCConn g_odbcConn;

dca::Mutex m_cs_1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerParameters& CServerParameters::GetInstance()
{
	static CServerParameters singleton;

	dca::Lock lock(m_cs_1);

	if (!singleton.m_bParamsLoaded)
	{
		if (singleton.LoadParams() != 0)
		{
			DebugReporter::Instance().DisplayMessage("CServerParameters::GetInstance - Unable to load server parameters from database", DebugReporter::ENGINE);
		}
	}

    return singleton;
}

CServerParameters::CServerParameters()
{
	m_bParamsLoaded = false;
}

CServerParameters::~CServerParameters()
{

}

const int CServerParameters::LoadParams()
{
	// note: this function is not locked because it's accessed
	// only by GetInstance(), which applies the local lock

	UINT m_nServerID;
	UINT m_nServerTaskTypeID;
	CEMSString sTemp;

	// if db connection isn't open
	if (!g_odbcConn.IsConnected())
	{
		// try to open it now; if it won't open,
		// wait until the next processing interval
		UINT nRet = OpenDBConn();
		if (nRet != 0)
		{
			// logpoint: Unable to establish connection to database
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_OUTBOUND_MESSAGING,
										nRet),
							EMS_STRING_MAILCOMP_DB_CONN_FAILURE);

			return nRet;
		}
	}

	// clear the map
	if (m_map.size() > 0)
		m_map.clear();
	
	// reset "isloaded" flag
	m_bParamsLoaded = false;

	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		CODBCQuery query(g_odbcConn);
		query.Initialize();

		ServerParameters_t sparam;
		BINDCOL_LONG(query, sparam.m_ServerParameterID);
		BINDCOL_WCHAR(query, sparam.m_DataValue);
		BINDCOL_WCHAR(query, sparam.m_Description);

		query.Execute( _T("SELECT ServerParameterID, DataValue, Description ")
					   _T("FROM	ServerParameters"));

		while (query.Fetch() == S_OK)
		{
			m_map[sparam.m_ServerParameterID] = sparam.m_DataValue;
			
			LINETRACE(_T("Loaded DB parameter: [%s] = %s\n"), sparam.m_Description, sparam.m_DataValue);
		}

		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               EMS_SERVER_ID_VALUE, m_nServerID ) != ERROR_SUCCESS)
		{
			m_nServerID = 1;
		}

		sTemp.Format(_T("%d"),m_nServerID);
		wcscpy( sparam.m_DataValue, sTemp.c_str() );
		m_map[ EMS_SRVPARAM_SERVER_ID ] = sparam.m_DataValue;

		m_map[ EMS_SRVPARAM_ALERT_SENDER ] = _T("0");
		m_map[ EMS_SRVPARAM_DB_MAINT ] = _T("0");
		m_map[ EMS_SRVPARAM_DB_MONITOR ] = _T("0");
		m_map[ EMS_SRVPARAM_MSG_COLLECTOR ] = _T("0");
		m_map[ EMS_SRVPARAM_MSG_ROUTER ] = _T("0");
		m_map[ EMS_SRVPARAM_MSG_SENDER ] = _T("0");
		m_map[ EMS_SRVPARAM_REPORTER ] = _T("0");
		m_map[ EMS_SRVPARAM_TICKET_MONITOR ] = _T("0");
		
		query.Initialize();

		BINDCOL_LONG_NOLEN( query, m_nServerTaskTypeID );
		BINDPARAM_LONG( query, m_nServerID );

		query.Execute( L"SELECT ServerTaskTypeID "
					L"FROM ServerTasks WHERE ServerID=?" );	

		while (  query.Fetch() == S_OK )
		{
			switch ( m_nServerTaskTypeID )
				{			
				case 1:			
					m_map[ EMS_SRVPARAM_ALERT_SENDER ] = _T("1");
					break;
				case 2:
					m_map[ EMS_SRVPARAM_DB_MAINT ] = _T("1");
					break;
				case 3:
					m_map[ EMS_SRVPARAM_DB_MONITOR ] = _T("1");
					break;
				case 4:
					m_map[ EMS_SRVPARAM_MSG_COLLECTOR ] = _T("1");
					break;
				case 5:
					m_map[ EMS_SRVPARAM_MSG_ROUTER ] = _T("1");
					break;
				case 6:
					m_map[ EMS_SRVPARAM_MSG_SENDER ] = _T("1");
					break;
				case 7:
					m_map[ EMS_SRVPARAM_REPORTER ] = _T("1");
					break;
				case 8:
					m_map[ EMS_SRVPARAM_TICKET_MONITOR ] = _T("1");
					break;
				}		
		}
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	m_bParamsLoaded = true;

	return 0; // success
}

const tstring CServerParameters::GetParameter(const UINT nID, const tstring& sDefault)
{
	dca::Lock lock(m_cs_1);

	if (!m_bParamsLoaded)
	{
		DebugReporter::Instance().DisplayMessage("CServerParameters::GetParameter - Database params are not loaded", DebugReporter::ENGINE);
		return sDefault;
	}

	TMAP::iterator iter = m_map.find(nID);
	if(iter != m_map.end())
		return iter->second;

	return sDefault;
}

const int CServerParameters::GetParameter(const UINT nID, const int nDefault)
{
	dca::Lock lock(m_cs_1);

	int nRetValue;

	if (!m_bParamsLoaded)
	{
		DebugReporter::Instance().DisplayMessage("CServerParameters::GetParameter - Database params are not loaded", DebugReporter::ENGINE);
		return nDefault;
	}

	TMAP::iterator iter = m_map.find(nID);
	if(iter != m_map.end())
	{
		nRetValue = _ttoi(iter->second.c_str());
		return nRetValue;
	}

	return nDefault;
}


const BOOL CServerParameters::SetParameter(UINT nID, const int nValue)
{
	TCHAR buf[33];
	_itot(nValue, buf, 10);

	return SetParameter(nID, (tstring)buf);
}

const BOOL CServerParameters::SetParameter(UINT nID, tstring& sValue)
{
	// lock access to the database object
	dca::Lock lock(g_csDB);

	try
	{
		// lock the map
		dca::Lock lock(m_cs_1);

		CODBCQuery query(g_odbcConn);
		query.Initialize();

		BINDPARAM_TCHAR_STRING(query, sValue);
		BINDPARAM_LONG(query, nID);

		query.Execute(	_T("UPDATE	ServerParameters ")
						_T("SET		DataValue = ? ")
						_T("WHERE	ServerParameterID = ?"));

		// update the map
		m_map[nID] = sValue.c_str();
	}
	catch (ODBCError_t oerr)
	{
		HandleDBError(&oerr);
		return ERROR_DATABASE;
	}

	return TRUE; // success
}

const int CServerParameters::Refresh()
{
	// invalidate the map so that the parameters
	// will be re-retrieved during the next pass
	dca::Lock lock(m_cs_1);

	m_bParamsLoaded = false;
	
	return 0;
}
