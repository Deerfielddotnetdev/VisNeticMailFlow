// SerializeException.h: interface for the CSerializeException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALIZEEXCEPTION_H__9BCE9B5F_253A_4BD3_8883_E17A03F276EF__INCLUDED_)
#define AFX_SERIALIZEEXCEPTION_H__9BCE9B5F_253A_4BD3_8883_E17A03F276EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;

class CSerializeException  
{
public:
	CSerializeException(const int nErrCode, const int nLine, const string& sFile);
	virtual ~CSerializeException();

public:
	const tstring& GetErrorString();
	const string& GetFileName()			{ return m_sFileName; }
	const int GetErrCode()				{ return m_nErrorCode; }
	const int GetLine()					{ return m_nLine; }

private:
	tstring m_sErrorString;
	int m_nErrorCode;
	string m_sFileName;
	int m_nLine;
};

#endif // !defined(AFX_SERIALIZEEXCEPTION_H__9BCE9B5F_253A_4BD3_8883_E17A03F276EF__INCLUDED_)
