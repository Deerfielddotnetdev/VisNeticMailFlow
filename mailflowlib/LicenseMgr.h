// LicenseMgr.h: interface for the CLicenseMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LICENSEMGR_H__2B64E7BD_7537_4D15_98E4_B3906FFAAF84__INCLUDED_)
#define AFX_LICENSEMGR_H__2B64E7BD_7537_4D15_98E4_B3906FFAAF84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DCIKey.h"

class CLicenseMgr  
{
public:
	CLicenseMgr();
	virtual ~CLicenseMgr();

	bool m_MFKeyPresent;
	char m_szMFKey[255];
	DCIKeyStruct m_MFKeyInfo;
	bool m_MFKeyExpired;
	int m_MFKeyAgentCount;
	int m_MFKeySerialNum;
	bool m_MFKeyExpiredBuildDate;
	int m_MFKeyServerCount;
	
	bool m_AVKeyPresent;
	char m_szAVKey[255];
	DCIKeyStruct m_AVKeyInfo;
	bool m_AVKeyExpired;

	void GetKeyInfo( CODBCQuery& query, int nServerID=0 );
	void GetKeyInfoFromKey( char* sKey );
	int GetUserCountFromEnum( int UserSize );
	int GetUserCountFromSerial( int m_MFKeySerialNum );
	int GetNumAgents( CODBCQuery& query );
	int GetMaxServers( CODBCQuery& query );
	int GetMaxAgents( CODBCQuery& query );
	void GetAgentEndDate( CODBCQuery& query, TIMESTAMP_STRUCT& EndDate );
	void GetServerEndDate( CODBCQuery& query, TIMESTAMP_STRUCT& EndDate );
	bool UpdateNumAgents( CODBCQuery& query );

	bool m_bKeyInfoValid;
};

#endif // !defined(AFX_LICENSEMGR_H__2B64E7BD_7537_4D15_98E4_B3906FFAAF84__INCLUDED_)
