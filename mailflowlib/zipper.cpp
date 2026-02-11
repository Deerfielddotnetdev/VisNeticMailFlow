#include "stdafx.h"
#include "zipper.h"

CZipper::CZipper()
{
    zalloc = 0;  // z_stream member
    zfree = 0;   // z_stream member
    opaque = 0;  // z_stream member

    fin = 0;
    fout = 0;
}

int CZipper::CompressFile(const TCHAR *input, const TCHAR *output, int level)
{
    err = Z_OK;
    avail_in = 0;
    avail_out = output_length;
    next_out = output_buffer;
    m_AbortFlag = 0;

    fin  = _tfopen(input, _T("rb"));
    fout = _tfopen(output, _T("wb"));
    length = _filelength(_fileno(fin));
    deflateInit(this, level);
    while(1)
	{
        if (m_AbortFlag)
            break;
        if (!LoadInput())
            break;
        err = deflate(this, Z_NO_FLUSH);
        FlushOutput();
        if (err != Z_OK)
            break;
        Progress(percent());
    }

    while(1)
	{
        if (m_AbortFlag)
            break;
        err = deflate(this, Z_FINISH);
        if (!FlushOutput())
            break;
        if (err != Z_OK)
            break;
    }

    Progress(percent());
    deflateEnd(this);
    if (m_AbortFlag)
        Status(_T("User Abort"));
    else if (err != Z_OK && err != Z_STREAM_END)
        Status(_T("Zlib Error"));
    else 
	{
        Status(_T("Success"));
        err = Z_OK;
    }

    fclose(fin);
    fclose(fout);
    fin = 0;
    fout = 0;
    if (m_AbortFlag)
        return Z_USER_ABORT;
    else
        return err;
}

int CZipper::DecompressFile(const TCHAR *input, const TCHAR *output)
{
    err = Z_OK;
    avail_in = 0;
    avail_out = output_length;
    next_out = output_buffer;
    m_AbortFlag = 0;

    fin  = _tfopen(input, _T("rb"));
    fout = _tfopen(output, _T("wb"));
    length = _filelength(_fileno(fin));
    inflateInit(this);
    while(1)
	{
        if (m_AbortFlag)
            break;
        if (!LoadInput())
            break;
        err = inflate(this, Z_NO_FLUSH);
        FlushOutput();
        if (err != Z_OK)
            break;
        Progress(percent());
    }

    while(1)
	{
        if (m_AbortFlag)
            break;
        err = inflate(this, Z_FINISH);
        if (!FlushOutput())
            break;
        if (err != Z_OK)
            break;
    }

    Progress(percent());
    inflateEnd(this);
    if (m_AbortFlag)
        Status(_T("User Abort"));
    else if (err != Z_OK && err != Z_STREAM_END)
        Status(_T("Zlib Error"));
    else 
	{
        Status(_T("Success"));
        err = Z_OK;
    }

    if (fin)
        fclose(fin);
    fin = 0;
    if (fout)
        fclose(fout);
    fout = 0;
    if (m_AbortFlag)
        return Z_USER_ABORT;
    else
        return err;
}

//  This function is called so as to provide the progress()
//  virtual function with a reasonable figure to indicate
//  how much processing has been done.  Note that the length
//  member is initialized when the input file is opened.
int CZipper::percent()
{
    if (length == 0)
        return 100;
    else if (length > 10000000L)
        return (total_in / (length / 100));
    else
        return (total_in * 100 / length);
}

//  Every time Zlib consumes all of the data in the
//  input buffer, this function gets called to reload.
//  The avail_in member is part of z_stream, and is
//  used to keep track of how much input is available.
int CZipper::LoadInput()
{
    if (avail_in == 0) 
	{
        next_in = input_buffer;
        avail_in = fread(input_buffer, 1, input_length, fin);
    }
    return avail_in;
}

//  Every time Zlib filsl the output buffer with data,
//  this function gets called.  Its job is to write
//  that data out to the output file, then update
//  the z_stream member avail_out to indicate that more
//  space is now available.
int CZipper::FlushOutput()
{
    unsigned int count = output_length - avail_out;
    if (count) 
	{
        if (fwrite(output_buffer, 1, count, fout) != count) 
		{
            err = Z_ERRNO;
            return 0;
        }
        next_out = output_buffer;
        avail_out = output_length;
    }
    return count;
}
