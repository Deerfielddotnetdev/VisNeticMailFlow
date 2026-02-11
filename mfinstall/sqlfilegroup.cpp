#include "stdafx.h"
#include "sqlfilegroup.h"

using namespace sql;

FileGroup::FileGroup() : SmartComObject(CLSID_SQLDMOFileGroup,TRUE),
						 _filegroup(SmartComObject(CLSID_SQLDMOFileGroup,TRUE))
{}
