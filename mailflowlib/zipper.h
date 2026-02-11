#if !defined(_ZIPPER_H)
#define _ZIPPER_H

// this is required for proper linking with the zlib C modules
extern "C" 
{
	#include <zlib/zlib.h>
}

#include <stdio.h>
#include <io.h>

class CZipper : public z_stream 
{
public :
	CZipper();

	// compression levels:
	// 9 = maximum compression
	// 0 = no compression
	int CompressFile(const TCHAR *input, const TCHAR *output, int level = 6);
	int DecompressFile(const TCHAR *input, const TCHAR *output);
	void SetAbortFlag(int i)			{ m_AbortFlag = i; }
	
protected :
	int percent();
	int LoadInput();
	int FlushOutput();

protected :
	// implementations for these can be defined, if desired
	virtual void Progress(int percent)	{};
	virtual void Status(const TCHAR *message)	{};
	int m_AbortFlag;

protected :
	FILE *fin;
	FILE *fout;
	long length;
	int err;
	enum { input_length = 4096 };
	unsigned char input_buffer[input_length];
	enum { output_length = 4096 };
	unsigned char output_buffer[output_length];
};

//
// I define one error code in addition to those
// found in zlib.h
//
#define Z_USER_ABORT (-7)

#endif  // #if !defined(_ZIPPER_H)
