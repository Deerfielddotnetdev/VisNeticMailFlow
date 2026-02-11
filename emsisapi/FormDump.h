// FormDump.h: interface for the CFormDump class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMDUMP_H__3DA5872B_B5C9_4909_B2D9_7393A72DCC48__INCLUDED_)
#define AFX_FORMDUMP_H__3DA5872B_B5C9_4909_B2D9_7393A72DCC48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CFormDump : public CXMLDataClass  
{
public:
	CFormDump( CISAPIData& ISAPIData );
	virtual ~CFormDump();

	virtual int Run(CURLAction& action);

};

#endif // !defined(AFX_FORMDUMP_H__3DA5872B_B5C9_4909_B2D9_7393A72DCC48__INCLUDED_)
