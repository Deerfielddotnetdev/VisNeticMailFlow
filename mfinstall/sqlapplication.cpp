#include "stdafx.h"
#include "sqlapplication.h"

using namespace sql;

Application::Application() : SmartComObject(CLSID_SQLDMOApplication,TRUE),
						     _application(SmartComObject(CLSID_SQLDMOApplication,TRUE))
{}
