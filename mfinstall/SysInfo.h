#include <windows.h>

namespace Win
{
	class SysInfo
	{
	public:
		SysInfo()
		{
			_osVer.dwOSVersionInfoSize = sizeof(_osVer);
		}
		~SysInfo(){}
		int GetVersion()
		{
			DWORD buildNumber;
			GetVersionEx(&_osVer);
			if(_osVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				buildNumber = (DWORD)(LOWORD(_osVer.dwBuildNumber));
				if((_osVer.dwMajorVersion == 4) && (_osVer.dwMinorVersion == 10))
				{
					if(buildNumber == 1998)
					{
						return 3;
					}
					else if(buildNumber == 2222)
					{
						return 4;
					}
				}
				else if ((_osVer.dwMajorVersion == 4) && (_osVer.dwMinorVersion == 90))
				{
					return 5;
				}	
				else
				{
					if(buildNumber == 1111)
					{
						return 2;
					}
					else
					{
						return 1;
					}
				}
 
			}
			else if(_osVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				if(_osVer.dwMajorVersion == 4)
				{
					return 6;
				}
				else
				{
					return 7;
				}
			}
			else
			{
				return 0;
			}
			return 0;
		}
	private:
		OSVERSIONINFO	_osVer;
	};
}