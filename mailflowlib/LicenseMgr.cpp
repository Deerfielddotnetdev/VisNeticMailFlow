// LicenseMgr.cpp: implementation of the CLicenseMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LicenseMgr.h"
#include "DebugReporter.h"
#include "AVPKeyStore.h"
#include "AVPKeyStore_i.c"
#include "RegistryFns.h"
#include "DateFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLicenseMgr::CLicenseMgr() :
	m_bKeyInfoValid(false)
{
}

CLicenseMgr::~CLicenseMgr()
{

}

int CLicenseMgr::GetNumAgents( CODBCQuery& query )
{
	int nAgents=0;
	UINT nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
					EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	query.Initialize();
	BINDPARAM_LONG( query, nServerID );
	BINDCOL_LONG_NOLEN( query, nAgents );
	query.Execute( _T("SELECT SUM(NumAgents) FROM [192.168.150.3].VMFHosted.dbo.NumServers WHERE AgentsID = (SELECT TOP 1 AgentsID FROM [192.168.150.3].VMFHosted.dbo.NumServers WHERE ServerName = (SELECT Description COLLATE SQL_Latin1_General_CP1_CI_AI FROM Servers WHERE ServerID=?))") );
	if( query.Fetch() != S_OK )
	{
		return 0;
	}
	return nAgents;
}

int CLicenseMgr::GetMaxAgents( CODBCQuery& query )
{
	int nAgents=0;
	UINT nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
					EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	query.Initialize();
	BINDPARAM_LONG( query, nServerID );
	BINDCOL_LONG_NOLEN( query, nAgents );
	query.Execute( _T("SELECT MaxAgents FROM [192.168.150.3].VMFHosted.dbo.NumAgents WHERE AgentsID = (SELECT TOP 1 AgentsID FROM [192.168.150.3].VMFHosted.dbo.NumServers WHERE ServerName = (SELECT Description COLLATE SQL_Latin1_General_CP1_CI_AI FROM Servers WHERE ServerID=?))") );
	if( query.Fetch() != S_OK )
	{
		return 0;
	}
	return nAgents;
}

int CLicenseMgr::GetMaxServers( CODBCQuery& query )
{
	int nServers=0;
	UINT nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
					EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	query.Initialize();
	BINDPARAM_LONG( query, nServerID );
	BINDCOL_LONG_NOLEN( query, nServers );
	query.Execute( _T("SELECT NumServers FROM [192.168.150.3].VMFHosted.dbo.NumServers WHERE ServerName = (SELECT Description COLLATE SQL_Latin1_General_CP1_CI_AI FROM Servers WHERE ServerID=?)") );
	if( query.Fetch() != S_OK )
	{
		return 0;
	}
	return nServers;
}

void CLicenseMgr::GetAgentEndDate( CODBCQuery& query, TIMESTAMP_STRUCT& EndDate )
{
	long EndDateLen;
	UINT nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
					EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	query.Initialize();
	BINDPARAM_LONG( query, nServerID );
	BINDCOL_TIME( query, EndDate );
	query.Execute( _T("SELECT EndDate FROM [192.168.150.3].VMFHosted.dbo.NumAgents WHERE AgentsID = (SELECT TOP 1 AgentsID FROM [192.168.150.3].VMFHosted.dbo.NumServers WHERE ServerName = (SELECT Description COLLATE SQL_Latin1_General_CP1_CI_AI FROM Servers WHERE ServerID=?))") );
	if( query.Fetch() != S_OK )
	{
		GetTimeStamp( EndDate );
	}	
}

void CLicenseMgr::GetServerEndDate( CODBCQuery& query, TIMESTAMP_STRUCT& EndDate )
{
	long EndDateLen;
	UINT nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
					EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	query.Initialize();
	BINDPARAM_LONG( query, nServerID );
	BINDCOL_TIME( query, EndDate );
	query.Execute( _T("SELECT EndDate FROM [192.168.150.3].VMFHosted.dbo.NumServers WHERE ServerName = (SELECT Description COLLATE SQL_Latin1_General_CP1_CI_AI FROM Servers WHERE ServerID=?)") );
	if( query.Fetch() != S_OK )
	{
		GetTimeStamp( EndDate );
	}
}

bool CLicenseMgr::UpdateNumAgents( CODBCQuery& query )
{
	UINT nServerID;
	if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
					EMS_SERVER_ID_VALUE, nServerID ) != ERROR_SUCCESS)
	{
		nServerID = 1;
	}
	query.Initialize();
	BINDPARAM_LONG( query, nServerID );
	query.Execute( _T("UPDATE [192.168.150.3].VMFHosted.dbo.NumServers SET NumAgents = ((SELECT COUNT(*) FROM Agents WHERE IsEnabled=1 AND IsDeleted=0)-1) WHERE ServerName = (SELECT Description COLLATE SQL_Latin1_General_CP1_CI_AI FROM Servers WHERE ServerID=?)") );
	if ( query.GetRowCount() != 1)
	{
		return false;
	}
	return true;
}

void CLicenseMgr::GetKeyInfo( CODBCQuery& query, int nServerID )
{
	dca::String f;
		
	try
	{
		int nParamID = EMS_SRVPARAM_LICENSE_KEY;
		HRESULT hres;
		long nError;
		dca::BString str("3CI55");
		CEMSString string;

		m_MFKeyPresent = false;
		ZeroMemory( &m_MFKeyInfo, sizeof(m_MFKeyInfo) );
		m_szMFKey[0] = _T('\0');
		m_MFKeyAgentCount = 0;

		m_AVKeyPresent = false;
		ZeroMemory( &m_AVKeyInfo, sizeof(m_AVKeyInfo) );
		m_szAVKey[0] = _T('\0');

		if ( nServerID != 0 )
		{
			query.Initialize();
			BINDCOL_CHAR_NOLEN( query, m_szMFKey );
			BINDPARAM_LONG( query, nServerID );
			query.Execute( _T("SELECT RegistrationKey ")
						_T("FROM Servers ")
						_T("WHERE ServerID=?") );
		}
		else
		{
			query.Initialize();
			BINDCOL_CHAR_NOLEN( query, m_szMFKey );
			BINDPARAM_LONG( query, nParamID );
			query.Execute( _T("SELECT DataValue ")
						_T("FROM ServerParameters ")
						_T("WHERE ServerParameterID=?") );
		}

		if( query.Fetch() == S_OK )
		{
			if( (DCIKeyAnalyze( m_szMFKey, &m_MFKeyInfo ) == 0)
				&& (strcmp( m_MFKeyInfo.szAppID, "3WGFQ" ) == 0) )
			{
				m_MFKeyPresent = true;
				m_MFKeyExpired = ( DCIKeyExpired( &m_MFKeyInfo ) == DCIKEY_Error_Key_Expired );
				m_MFKeyExpiredBuildDate = false;

				SYSTEMTIME buildDate;
				buildDate.wYear = 2022;
				buildDate.wMonth = 2;
				buildDate.wDay = 14;
				
				// check Expire date against Build date
				if( m_MFKeyInfo.ExpireYear < buildDate.wYear)
				{
					m_MFKeyExpiredBuildDate = true;
				}
				else if( m_MFKeyInfo.ExpireYear == buildDate.wYear)
				{
					if( m_MFKeyInfo.ExpireMonth < buildDate.wMonth )
					{
						m_MFKeyExpiredBuildDate = true;
					}
					else if( m_MFKeyInfo.ExpireMonth == buildDate.wMonth )
					{
						if( m_MFKeyInfo.ExpireDay < buildDate.wDay )
						{
							m_MFKeyExpiredBuildDate = true;
						}
					}
				}
				
				m_MFKeyAgentCount = GetUserCountFromEnum( m_MFKeyInfo.UserSize );
				m_MFKeyServerCount = m_MFKeyInfo.ResellerID;
				if(m_MFKeyServerCount == 1153){m_MFKeyServerCount = 1;}

				/*f.Format("CLicenseMgr::GetKeyInfo - ServerID:[%d] Key:[%s] Agents:[%d] Type:[%d] GenDate:[%d/%d/%d] ExpireDate:[%d/%d/%d] ", nServerID,m_szMFKey,m_MFKeyAgentCount,m_MFKeyInfo.KeyType,m_MFKeyInfo.CreateMonth,m_MFKeyInfo.CreateDay,m_MFKeyInfo.CreateYear,m_MFKeyInfo.ExpireMonth,m_MFKeyInfo.ExpireDay,m_MFKeyInfo.ExpireYear);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);*/				
			}
			else
			{
				/*f.Format("CLicenseMgr::GetKeyInfo - Key analyzer found invalid key or appid for key %s", m_szMFKey);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);*/
			}
		}
		else
		{
			f.Format("CLicenseMgr::GetKeyInfo - Failed to get the MailFlow license key from the database for ServerID:[%d], trying again", nServerID);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);

			if ( nServerID != 0 )
			{
				query.Initialize();
				BINDCOL_CHAR_NOLEN( query, m_szMFKey );
				BINDPARAM_LONG( query, nServerID );
				query.Execute( _T("SELECT RegistrationKey ")
							_T("FROM Servers ")
							_T("WHERE ServerID=?") );
			}
			else
			{
				query.Initialize();
				BINDCOL_CHAR_NOLEN( query, m_szMFKey );
				BINDPARAM_LONG( query, nParamID );
				query.Execute( _T("SELECT DataValue ")
							_T("FROM ServerParameters ")
							_T("WHERE ServerParameterID=?") );
			}
			if( query.Fetch() == S_OK )
			{
				if( (DCIKeyAnalyze( m_szMFKey, &m_MFKeyInfo ) == 0)
					&& (strcmp( m_MFKeyInfo.szAppID, "3WGFQ" ) == 0) )
				{
					m_MFKeyPresent = true;
					m_MFKeyExpired = ( DCIKeyExpired( &m_MFKeyInfo ) == DCIKEY_Error_Key_Expired );
					m_MFKeyExpiredBuildDate = false;

					SYSTEMTIME buildDate;
					buildDate.wYear = 2022;
					buildDate.wMonth = 2;
					buildDate.wDay = 14;
					
					// check Expire date against Build date
					if( m_MFKeyInfo.ExpireYear < buildDate.wYear)
					{
						m_MFKeyExpiredBuildDate = true;
					}
					else if( m_MFKeyInfo.ExpireYear == buildDate.wYear)
					{
						if( m_MFKeyInfo.ExpireMonth < buildDate.wMonth )
						{
							m_MFKeyExpiredBuildDate = true;
						}
						else if( m_MFKeyInfo.ExpireMonth == buildDate.wMonth )
						{
							if( m_MFKeyInfo.ExpireDay < buildDate.wDay )
							{
								m_MFKeyExpiredBuildDate = true;
							}
						}
					}
					
					m_MFKeyAgentCount = GetUserCountFromEnum( m_MFKeyInfo.UserSize );

					f.Format("CLicenseMgr::GetKeyInfo - ServerID:[%d] Key:[%s] Agents:[%d] Type:[%d] GenDate:[%d/%d/%d] ExpireDate:[%d/%d/%d] ", nServerID,m_szMFKey,m_MFKeyAgentCount,m_MFKeyInfo.KeyType,m_MFKeyInfo.CreateMonth,m_MFKeyInfo.CreateDay,m_MFKeyInfo.CreateYear,m_MFKeyInfo.ExpireMonth,m_MFKeyInfo.ExpireDay,m_MFKeyInfo.ExpireYear);
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);			
				}
				else
				{
					f.Format("CLicenseMgr::GetKeyInfo - Key analyzer found invalid key or appid for key %s", m_szMFKey);
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
				}
			}
			else
			{
				f.Format("CLicenseMgr::GetKeyInfo - Failed to get the MailFlow license key from the database for ServerID:[%d]", nServerID);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
			}
		}

		try
		{
			nParamID = EMS_SRVPARAM_ANTIVIRUS_ENABLE;

			query.Initialize();
			BINDCOL_CHAR_NOLEN( query, m_szAVKey );
			BINDPARAM_LONG( query, nParamID );
			query.Execute( _T("SELECT DataValue ")
						_T("FROM ServerParameters ")
						_T("WHERE ServerParameterID=?") );

			if( query.Fetch() == S_OK )
			{
				int nAVEnabled = atoi(m_szAVKey);
				
				if ( nAVEnabled == 1 )
				{
					dca::SmartComObject objAVPKeyStore(CLSID_AVPKeyStore);
					dca::SmartComInterface<IAVPKeyStore, IID_IAVPKeyStore> iAVKey(objAVPKeyStore);

					hres = iAVKey->put_AppID(str);

					if( SUCCEEDED( hres ) )
					{
						hres = iAVKey->CheckKey( &nError );

						if( SUCCEEDED( hres ) && (nError == 0 || nError == DCIKEY_Error_Key_Expired) )
						{				
							dca::BString bKey;
							dca::BString bProductCode;

							m_AVKeyPresent = true;
							// Query the COM object for all info
							
							iAVKey->get_Key(bKey.GetRef());

							dca::String sKey(bKey);
							strcpy(m_szAVKey,sKey.c_str());

							iAVKey->get_ExpireDay( (long*) &m_AVKeyInfo.ExpireDay );
							iAVKey->get_ExpireMonth( (long*) &m_AVKeyInfo.ExpireMonth );
							iAVKey->get_ExpireYear( (long*) &m_AVKeyInfo.ExpireYear );
							iAVKey->get_CreateDay( (long*) &m_AVKeyInfo.CreateDay );
							iAVKey->get_CreateMonth( (long*) &m_AVKeyInfo.CreateMonth );
							iAVKey->get_CreateYear( (long*) &m_AVKeyInfo.CreateYear );
							iAVKey->get_KeyType( (long*) &m_AVKeyInfo.KeyType );
							iAVKey->get_UserType( (long*) &m_AVKeyInfo.UserType );
							iAVKey->get_UserSize( (long*) &m_AVKeyInfo.UserSize );
							
							
							iAVKey->get_ProductCode(bProductCode.GetRef());
							dca::String sProductCode(bProductCode);
							strcpy(m_AVKeyInfo.szProductCode,sProductCode.c_str());

							iAVKey->get_ResellerID( (long*) &m_AVKeyInfo.ResellerID );
							iAVKey->get_SerialNum( (long*) &m_AVKeyInfo.SerialNum );

							m_AVKeyExpired = ( DCIKeyExpired( &m_AVKeyInfo ) == DCIKEY_Error_Key_Expired );

						}					
					}
				}
				else
				{
					//DebugReporter::Instance().DisplayMessage("CLicenseMgr::GetKeyInfo - AntiVirus scanning disabled - AntiVirus License not checked", DebugReporter::LIB);
				}
			}
		}
		catch(dca::Exception e)
		{
			/*dca::String er(e.GetMessage());
			f.Format("CLicenseMgr::GetKeyInfo - %s", er.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);*/
		}
		catch(...)
		{
			//DebugReporter::Instance().DisplayMessage("CLicenseMgr::GetKeyInfo - An unknown or undefined exception has occurred", DebugReporter::LIB);
		}

		m_bKeyInfoValid = true;
	}
	catch(dca::Exception e)
	{
		/*dca::String er(e.GetMessage());
		f.Format("CLicenseMgr::GetKeyInfo - %s", er.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);*/
	}
}


void CLicenseMgr::GetKeyInfoFromKey( char* sKey )
{
	dca::String f;
		
	try
	{
		dca::BString str("3CI55");
		CEMSString string;

		m_MFKeyPresent = false;
		m_bKeyInfoValid = false;
		ZeroMemory( &m_MFKeyInfo, sizeof(m_MFKeyInfo) );
		m_szMFKey[0] = _T('\0');
		m_MFKeyAgentCount = 0;

		if( (DCIKeyAnalyze( sKey, &m_MFKeyInfo ) == 0)
				&& (strcmp( m_MFKeyInfo.szAppID, "3WGFQ" ) == 0) )
		{
			m_MFKeyPresent = true;
			m_MFKeyExpired = ( DCIKeyExpired( &m_MFKeyInfo ) == DCIKEY_Error_Key_Expired );
			m_MFKeyExpiredBuildDate = false;

			SYSTEMTIME buildDate;
			buildDate.wYear = 2022;
			buildDate.wMonth = 2;
			buildDate.wDay = 14;
			
			// check Expire date against Build date
			if( m_MFKeyInfo.ExpireYear < buildDate.wYear)
			{
				m_MFKeyExpiredBuildDate = true;
			}
			else if( m_MFKeyInfo.ExpireYear == buildDate.wYear)
			{
				if( m_MFKeyInfo.ExpireMonth < buildDate.wMonth )
				{
					m_MFKeyExpiredBuildDate = true;
				}
				else if( m_MFKeyInfo.ExpireMonth == buildDate.wMonth )
				{
					if( m_MFKeyInfo.ExpireDay < buildDate.wDay )
					{
						m_MFKeyExpiredBuildDate = true;
					}
				}
			}
			
			m_MFKeyAgentCount = GetUserCountFromEnum( m_MFKeyInfo.UserSize );
			m_MFKeyServerCount = m_MFKeyInfo.ResellerID;
			if(m_MFKeyServerCount == 1153){m_MFKeyServerCount = 1;}
			m_bKeyInfoValid = true;
			
			f.Format("CLicenseMgr::GetKeyInfoFromKey - Key:[%s] Agents:[%d] Type:[%d] GenDate:[%d/%d/%d] ExpireDate:[%d/%d/%d] ", sKey,m_MFKeyAgentCount,m_MFKeyInfo.KeyType,m_MFKeyInfo.CreateMonth,m_MFKeyInfo.CreateDay,m_MFKeyInfo.CreateYear,m_MFKeyInfo.ExpireMonth,m_MFKeyInfo.ExpireDay,m_MFKeyInfo.ExpireYear);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
		}
		else
		{
			f.Format("CLicenseMgr::GetKeyInfoFromKey - Key analyzer found invalid key or appid for key %s", sKey);
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);
		}
	}
	catch(dca::Exception e)
	{
		dca::String er(e.GetMessage());
		f.Format("CLicenseMgr::GetKeyInfoFromKey - %s", er.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::LIB);		
	}
	catch(...){}
}


int CLicenseMgr::GetUserCountFromEnum( int UserSize )
{
	CEMSString sUserSize;
	
	switch( UserSize )
	{
	case DCIKEY_UserSize_1:
		return 1;

	case DCIKEY_UserSize_3:
		return 3;

	case DCIKEY_UserSize_6:
		return 6;

	case DCIKEY_UserSize_12:
		return 12;

	case DCIKEY_UserSize_25:
		return 25;

	case DCIKEY_UserSize_50:
		return 50;

	case DCIKEY_UserSize_100:
		return 100;

	case DCIKEY_UserSize_250:
		return 250;

	case DCIKEY_UserSize_500:
		return 500;
		
	case DCIKEY_UserSize_1000:
		return 1000;

	case DCIKEY_UserSize_2500:
		return 2500;

	case DCIKEY_UserSize_Unlimited:
	case DCIKEY_UserSize_Site:
	case DCIKEY_UserSize_Enterprise:
		return 1000000000;

	case DCIKEY_UserSize_Custom:
		int nNumUsers = m_MFKeyInfo.SerialNum%10000;
		return nNumUsers;

	}

	return 0;
}