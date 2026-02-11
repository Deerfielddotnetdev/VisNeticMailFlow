#ifndef _MAILSTREAM_DEBUG_REPORTER_H_
#define _MAILSTREAM_DEBUG_REPORTER_H_

class DebugReporter
{
public:
	enum TYPE_S{ NO_O = 0, FILE_O, DEBUG_O };
	enum PROGRAM{ NONE = 0, ENGINE, MAIL, ISAPI, LIB };
public:
	~DebugReporter();
	static DebugReporter& Instance() { return _instance; }
	void DisplayMessage(const char* message, int prog, DWORD dwThreadID=0);
	const int GetInitialized()	{ return _initialized; }
	void SetInitialized(const int nVal)			{ _initialized = nVal; }
private:
	DebugReporter();
	void Initialize();
private:
	static DebugReporter _instance;
	dca::WString         _path;
	int                  _howtoshow;
	int                  _initialized;
	int                  _progtype;
};

#endif // _MAILSTREAM_DEBUG_REPORTER_H_
