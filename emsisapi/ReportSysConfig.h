// ReportSysConfig.h: interface for the CReportSysConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REPORTSYSCONFIG_H__EF5BD25A_E50D_4C67_B74C_3A1CD0BE6F59__INCLUDED_)
#define AFX_REPORTSYSCONFIG_H__EF5BD25A_E50D_4C67_B74C_3A1CD0BE6F59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Reports.h"

class CReportSysConfig : public CXMLDataClass  
{
public:
	CReportSysConfig( CISAPIData& ISAPIData );
	virtual ~CReportSysConfig();

	virtual int Run( CURLAction& action );

		
protected:
	void RunReport( CURLAction& action );
	void RunDBReport( CURLAction& action );
	void GenerateXML( void );
	void DecodeForm( void );
	void GetRights( void );
	void GetVersionInfo( CEMSString& sInstallPath, TCHAR* szBinary, CEMSString& sVersion );
	void BlankOutDBPassword( LPTSTR szDSN );
	void OpenCDF( CURLAction& action );
	void OutputCDF( tstring& sLine );
	void CloseCDF(void);

	int			m_AgentID;
	int			m_GroupID;
	int			m_AgentGroup;
	int			m_ObjectID;
	int			m_AgentOID;
	int			m_GroupOID;
	int			m_TicketBoxID;
	int			m_TicketCategoryID;
	int			m_SRCategoryID;
	int			m_EffectiveRight;
	bool		m_ShowRights;
	int			m_AccessControlID;
	long		m_AccessControlIDLen;
	int			m_MaxRows;

	bool		m_OutputCDF;
	TCHAR		m_szTempFile[MAX_PATH];
	HANDLE		m_hCDFFile;
	
};

inline bool operator<(const TServerParameters& A,const TServerParameters& B)
{ return (_tcscmp(A.m_Description,B.m_Description) < 0); }


#endif // !defined(AFX_REPORTSYSCONFIG_H__EF5BD25A_E50D_4C67_B74C_3A1CD0BE6F59__INCLUDED_)
