// MainFrame.h: interface for the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRAME_H__F6B6B2BC_7F84_4055_9244_EF15E6DFBA83__INCLUDED_)
#define AFX_MAINFRAME_H__F6B6B2BC_7F84_4055_9244_EF15E6DFBA83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CMainFrame : public CXMLDataClass
{
public:
	CMainFrame( CISAPIData& ISAPIData );
	virtual ~CMainFrame();

	virtual int Run( CURLAction& action );

protected:
	void Translate( tstring& sURL );
};

#endif // !defined(AFX_MAINFRAME_H__F6B6B2BC_7F84_4055_9244_EF15E6DFBA83__INCLUDED_)
