///////////////////////////////////////////////////////////////////////////////
//
// SQLApplication class header file
// Written by Mark Mohr on 04/03/2002
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SQLAPPLICATION_H
#define SQLAPPLICATION_H

#define INITGUID

#include <list>
#include <string>
#include "olebase.h"
#include "objects.h"
#include "interface.h"
#include "SQLDMOid.h"
#include "SQLDMO.h"

namespace sql
{
	class Application : public SmartComObject
	{
		enum {BUF_SIZE = 256};
	public:
		Application();
		int ListAvailableServers()
		{
			HRESULT hr;
			hr = _application->ListAvailableSQLServers(_namelist);

			return HRESULT_CODE(hr);
		}
		int GetNumberOfServers()
		{
			long count;

			_namelist->GetCount(&count);

			return (int)count;
		}
		int GetVersionMajor(long* verMaj)
		{
			HRESULT hr;

			hr = _application->GetVersionMajor(verMaj);
			
			return HRESULT_CODE(hr);
		}
#pragma warning (disable: 4786)
		void EnumServers(std::list<std::string>* servers)
		{
			long count;
			BString bstr;

			_namelist->GetCount(&count);

			//DEBUGSTRING("sql::Application::EnumServers - Count [ %d ]",count);

			for(int x = 0;x < count;x++)
			{

				_namelist->GetItemByOrd(x,bstr.GetPointer());
				CString cstr(bstr);
				std::string temp = cstr;
				servers->insert(servers->end(),temp);
			}

		}
	private:
		SmartObjectInterface<ISQLDMOApplication,&IID_ISQLDMOApplication> _application;
		SmartCollectionInterface<ISQLDMONameList>						 _namelist;
	};
}

#endif //SQLAPPLICATION_H