// SessionInfo.h: interface for the CSessionInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESSIONINFO_H__AF848B6B_22B1_4D40_91C7_0C845F484F1A__INCLUDED_)
#define AFX_SESSIONINFO_H__AF848B6B_22B1_4D40_91C7_0C845F484F1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CSessionInfo : public CXMLDataClass  
{
public:
	CSessionInfo( CISAPIData& ISAPIData );
	virtual ~CSessionInfo();

	virtual int Run(CURLAction& action);

protected:

};

#endif // !defined(AFX_SESSIONINFO_H__AF848B6B_22B1_4D40_91C7_0C845F484F1A__INCLUDED_)
