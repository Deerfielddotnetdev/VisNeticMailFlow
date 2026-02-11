// SerialFile.h: interface for the CSerialFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALFILE_H__9A01DA66_0BB7_40C7_83CB_7EEE0223443B__INCLUDED_)
#define AFX_SERIALFILE_H__9A01DA66_0BB7_40C7_83CB_7EEE0223443B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSerialFile  
{
public:
	CSerialFile();
	virtual ~CSerialFile();

public:
	int OpenArchive(const tstring& sArcFileName, const int nDir);
	void CloseArchive();

	enum _serializer_direction
	{
		serializer_dir_in,
		serializer_dir_out
	};

friend class CSerializable;
	
private:
	tstring m_sArcFileName;
	HANDLE m_hArcFile;
};

#endif // !defined(AFX_SERIALFILE_H__9A01DA66_0BB7_40C7_83CB_7EEE0223443B__INCLUDED_)
