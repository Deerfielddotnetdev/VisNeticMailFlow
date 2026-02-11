// Database.h: interface for the CDatabase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABASE_H__7274AB7C_6ACE_47FC_95BA_2A2D2967D19E__INCLUDED_)
#define AFX_DATABASE_H__7274AB7C_6ACE_47FC_95BA_2A2D2967D19E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDatabase  
{
public:
	CDatabase( const wchar_t* wcsName, int cbName );
	virtual ~CDatabase();

	void SetDSN( const wchar_t* wcsValue, int cbValue );
	void SetAction( const wchar_t* wcsValue, int cbValue );
	void SetType( const wchar_t* wcsValue, int cbValue );

	enum 
	{ 
		DATABASE_NAME_LENGTH = 255,
	};


	wchar_t m_wcsName[DATABASE_NAME_LENGTH];
	wchar_t m_wcsDSN[DATABASE_NAME_LENGTH];
	wchar_t m_wcsAction[DATABASE_NAME_LENGTH];
	wchar_t m_wcsType[DATABASE_NAME_LENGTH];
};

#endif // !defined(AFX_DATABASE_H__7274AB7C_6ACE_47FC_95BA_2A2D2967D19E__INCLUDED_)
