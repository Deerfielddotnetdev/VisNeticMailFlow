///////////////////////////////////////////////////////////////////////////////
//
// SQLFileGroup class header file
// Written by Mark Mohr on 03/29/2002
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SQLFILEGROUP_H
#define SQLFILEGROUP_H

#define INITGUID

#include "olebase.h"
#include "objects.h"
#include "interface.h"
#include "SQLDMOid.h"
#include "SQLDMO.h"

namespace sql
{
	class FileGroup : public SmartComObject
	{
		enum {BUF_SIZE = 256};
	public:
		FileGroup();
		operator ISQLDMOFileGroup* () { return _filegroup; }
		operator ISQLDMOFileGroup** () { return _filegroup; }
		int AddDBFile(ISQLDMODBFile* dbFile)
		{
			HRESULT hr;

			hr = _dbFiles->Add(dbFile);

			return HRESULT_CODE(hr);
		}
		int Intialize()
		{
			HRESULT hr;
			hr = _filegroup->GetDBFiles(_dbFiles);
			return HRESULT_CODE(hr);
		}
	private:
		SmartObjectInterface<ISQLDMOFileGroup,&IID_ISQLDMOFileGroup> _filegroup;
		SmartCollectionInterface<ISQLDMODBFiles>					 _dbFiles;
	};
}

#endif // SQLFILEGROUP_H