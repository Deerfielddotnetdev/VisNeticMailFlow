// Download3cxPlugin.h: interface for the CDownload3cxPlugin class.
//
//////////////////////////////////////////////////////////////////////


#include "XMLDataClass.h"

class CDownload3cxPlugin : public CXMLDataClass  
{
public:
	CDownload3cxPlugin(CISAPIData& ISAPIData);
	virtual ~CDownload3cxPlugin();

	virtual int Run(CURLAction& action);

};
