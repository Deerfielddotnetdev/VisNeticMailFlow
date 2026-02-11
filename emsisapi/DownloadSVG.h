// DownloadSVG.h: interface for the CDownloadSVG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOWNLOADSVG_H__57F6801D_8146_4910_8B2C_A68A0599BD24__INCLUDED_)
#define AFX_DOWNLOADSVG_H__57F6801D_8146_4910_8B2C_A68A0599BD24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CDownloadSVG : public CXMLDataClass  
{
public:
	CDownloadSVG(CISAPIData& ISAPIData);
	virtual ~CDownloadSVG();

	virtual int Run(CURLAction& action);

};

#endif // !defined(AFX_DOWNLOADSVG_H__57F6801D_8146_4910_8B2C_A68A0599BD24__INCLUDED_)
