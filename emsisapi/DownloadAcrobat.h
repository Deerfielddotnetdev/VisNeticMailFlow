/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/DownloadAcrobat.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2003 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CDownloadAcrobat : public CXMLDataClass  
{
public:
	CDownloadAcrobat(CISAPIData& ISAPIData);
	virtual ~CDownloadAcrobat() {};

	virtual int Run(CURLAction& action);
};

