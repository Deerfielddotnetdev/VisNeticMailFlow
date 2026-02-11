// UploadProgress.h: interface for the CUploadProgress class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPLOADPROGRESS_H__6A695FCB_3389_4145_8A81_B8F50F3ED9B4__INCLUDED_)
#define AFX_UPLOADPROGRESS_H__6A695FCB_3389_4145_8A81_B8F50F3ED9B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CUploadProgress : public CXMLDataClass  
{
public:
	CUploadProgress(CISAPIData& ISAPIData);
	virtual ~CUploadProgress();

	int Run(CURLAction& action);
};

#endif // !defined(AFX_UPLOADPROGRESS_H__6A695FCB_3389_4145_8A81_B8F50F3ED9B4__INCLUDED_)
