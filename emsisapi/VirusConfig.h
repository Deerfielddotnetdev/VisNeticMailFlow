// VirusConfig.h: interface for the CVirusConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIRUSCONFIG_H__B094551A_E591_4314_875F_68F20BF407C9__INCLUDED_)
#define AFX_VIRUSCONFIG_H__B094551A_E591_4314_875F_68F20BF407C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CVirusConfig : public CXMLDataClass  
{
public:
	CVirusConfig(CISAPIData& ISAPIData);
	virtual ~CVirusConfig();

	int virtual Run( CURLAction& action );

protected:
	void Update(void);
	void QueryDB(void);
	void DecodeForm(void);
	void GenerateXML(void);

	int UpdateAVP( CURLAction& action );
	void GenerateAVPXML(void);
	void AddUserSize( long nUserSize );

	void GenerateKey( CURLAction& action );
	
	unsigned int m_nAVEnabled;
	unsigned int m_nAVAction;
	tstring m_sQuarantineFolder;
	unsigned int m_nUnscannableAction;
	unsigned int m_nSuspiciousAction;
};


#endif // !defined(AFX_VIRUSCONFIG_H__B094551A_E591_4314_875F_68F20BF407C9__INCLUDED_)
