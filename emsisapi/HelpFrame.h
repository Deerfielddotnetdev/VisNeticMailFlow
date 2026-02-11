// HelpFrame.h: interface for the CHelpFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPFRAME_H__F6B6B2BC_7F84_4055_9244_EF15E6DFBA83__INCLUDED_)
#define AFX_HELPFRAME_H__F6B6B2BC_7F84_4055_9244_EF15E6DFBA83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CHelpFrame : public CXMLDataClass
{
public:
	CHelpFrame( CISAPIData& ISAPIData );
	virtual ~CHelpFrame();

	virtual int Run( CURLAction& action );

protected:
	void Translate( tstring& sURL );
};

#endif // !defined(AFX_HELPFRAME_H__F6B6B2BC_7F84_4055_9244_EF15E6DFBA83__INCLUDED_)
