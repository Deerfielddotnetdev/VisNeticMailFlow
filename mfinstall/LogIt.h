#if !defined _LOGIT_H
#define _LOGIT_H

#include <fstream>

using namespace std;

class LogFile
{
public:
	LogFile() 
	{ 
		char tempPath[255];

		GetTempPathA(255,tempPath);
		strcat(tempPath,"vmfinst.txt");
		_logFile.open(tempPath,ios::out | ios::app);
	}
	~LogFile() { _logFile.close(); }
	void Date()
	{
		SYSTEMTIME sysTime;
		char buf[256];

		GetLocalTime(&sysTime);

		wsprintfA(buf, "%02d/%02d/%d - %02d:%02d:%02d",sysTime.wMonth,sysTime.wDay,sysTime.wYear,
		         sysTime.wHour,sysTime.wMinute,sysTime.wSecond);
	
		_logFile << buf << endl;
	}
	void Write(const char* txt, ...)
	{
		char buffer[1025];
		ZeroMemory(buffer, 1025);

		va_list vaList;
		va_start(vaList, txt);
		_vsnprintf(buffer, 1024, txt, vaList);

		_logFile << buffer << endl;
	}
	void LineBreak()
	{
		_logFile << endl;
	}
	void Line()
	{
		for(int i = 0;i < 50;i++)
		{
			_logFile << "*";
		}
		_logFile << endl << endl;
	}
private:
	ofstream _logFile;
};

#endif // _LOGIT_H