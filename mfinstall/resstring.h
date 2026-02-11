#ifndef RESSTRING_H
#define RESSTRING_H

#include <windows.h>

class ResString
{
    enum { MAX_RESSTRING = 255 };
public:
    ResString (HINSTANCE hInst, int resId)
	{
		if (!::LoadStringA (hInst, resId, _buf, MAX_RESSTRING + 1))
        throw "Load String failed";
	}
    operator char const * () { return _buf; }
private:
    char _buf [MAX_RESSTRING + 1];
};

#endif // RESSTRING_H
