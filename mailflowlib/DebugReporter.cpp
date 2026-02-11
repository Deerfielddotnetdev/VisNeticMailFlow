#include "stdafx.h"
#include "DebugReporter.h"
#include "RegistryFns.h"
#include <fstream>
#include <ctime>

using namespace std;

DebugReporter DebugReporter::_instance;

DebugReporter::DebugReporter()
	:_path()
	,_howtoshow(-1)
	,_initialized(0)
	,_progtype(0)
{
}

DebugReporter::~DebugReporter()
{
}

void DebugReporter::Initialize()
{
	LONG rc = GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, EMS_LOG_PATH_VALUE, _path );
	
	if(rc == ERROR_SUCCESS)
	{
		dca::WString::size_type pos = _path.find_last_of(0x5C);
		if(pos != (_path.length() - 1))
			_path.append(L"\\");
	}
	
	UINT nDebugLogValue = 0;

	switch(_progtype)
	{
	case 1:
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogEngine"), nDebugLogValue ) == ERROR_SUCCESS)
		{
			_howtoshow = nDebugLogValue;
		}
		break;
	case 2:
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogMail"), nDebugLogValue ) == ERROR_SUCCESS)
		{
			_howtoshow = nDebugLogValue;
		}
		break;
	case 3:
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogWeb"), nDebugLogValue ) == ERROR_SUCCESS)
		{
			_howtoshow = nDebugLogValue;
		}
		break;
	case 4:
		if ( GetRegInt( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, 
		               _T("DebugLogLib"), nDebugLogValue ) == ERROR_SUCCESS)
		{
			_howtoshow = nDebugLogValue;
		}
		break;
	default:
		_howtoshow = 0;
		return;
	}

	_initialized = 1;
}

void DebugReporter::DisplayMessage(const char* message, int prog, DWORD dwThreadID)
{
	if(!_initialized)
	{
		_progtype = prog;
		this->Initialize();
	}

	if(_howtoshow == NO_O)
	{
		return;
	}

	char szVal[25];					
	if ( dwThreadID == 0 )
	{
		DWORD threadID = GetCurrentThreadId();
		sprintf( szVal, " [ %d ] ", threadID );
	}
	else
	{
		sprintf( szVal, " [ %d ] ", dwThreadID );
	}
	
	char szID[25];
	sprintf( szID, "Isapi_%d_",  GetCurrentProcessId() );
	
	switch(_howtoshow)
	{
	case DEBUG_O:
		{
			dca::String sDebug;			
			switch(prog)
			{
			case 1:
				sDebug.Format("[E] %s %s",szVal,message);
				break;
			case 2:
				sDebug.Format("[M] %s %s",szVal,message);
				break;
			case 3:
				sDebug.Format("[I] %s %s",szVal,message);
				break;
			case 4:
				sDebug.Format("[L] %s %s",szVal,message);
				break;			
			}
			message = sDebug.c_str();
			::OutputDebugStringA(message);			
		}
		break;
	case FILE_O:
		{
			if(_path.size() != 0)
			{
				dca::String s(_path.c_str());
				char filename[81];
				memset(filename, 0x00, 81);

				time_t l;
				time(&l);
				tm* t = localtime(&l);
				switch(prog)
				{
				case 1:
					strftime(filename,80,"Engine_%Y%m%d.log",t);
					break;
				case 2:
					strftime(filename,80,"Mail_%Y%m%d.log",t);
					break;
				case 3:
					strftime(filename,80,"%Y%m%d.log",t);
					s.append(szID);
					break;
				case 4:
					strftime(filename,80,"Library_%Y%m%d.log",t);
					break;
				default:
					return;
				}

				char timestamp[81];
				memset(timestamp, 0x00, 81);
				strftime(timestamp, 80, "[ %H:%M:%S ] ", t);
				
				s.append(filename);

				std::ofstream f;
				dca::String m(timestamp);
				m.append(szVal);
				m.append(message);
				m.append("\r\n");
				f.open(s.c_str(), ios_base::in | ios_base::out | ios_base::app| ios_base::binary);

				f << m.c_str();

				f.close();
			}
		}
		break;
	}
}