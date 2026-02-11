// SerializeException.cpp: implementation of the CSerializeException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerializeException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerializeException::CSerializeException(const int nErrCode, const int nLine, const string& sFile)
{
	m_nErrorCode = nErrCode;
	m_sFileName = sFile;
	m_nLine = nLine;
}

CSerializeException::~CSerializeException()
{

}

// get the error string associated with m_nErrorCode
const tstring& CSerializeException::GetErrorString()
{
	m_sErrorString = _T("");

	// buffer
	TCHAR* lpszTemp = NULL;
	
	// Let the system allocate a string to describe the error code in dwRet
    FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |	// specifies that the lpBuffer parameter is a pointer to a PVOID pointer, and that the nSize parameter specifies the minimum number of TCHARs to allocate for an output message buffer
					FORMAT_MESSAGE_FROM_SYSTEM |		// specifies that the function should search the system message-table resource(s) for the requested message
					FORMAT_MESSAGE_ARGUMENT_ARRAY,		// specifies that the Arguments parameter is not a va_list structure, but instead is just a pointer to an array of values that represent the arguments
					NULL,								// message source
					m_nErrorCode,						// message identifier					
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 	// default language
					(LPTSTR)&lpszTemp,					// message buffer					
					0,									// maximum size of message buffer
					NULL );								// array of message inserts	
	
    if (lpszTemp)
	{
		// copy to our string and free memory
		m_sErrorString = lpszTemp;
        LocalFree((HLOCAL)lpszTemp);
	}
	
	return m_sErrorString;
}