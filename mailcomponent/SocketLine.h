// SocketLine.h: interface for the CSocketLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETLINE_H__50CCC726_CC9E_47EA_A0F6_881F9F186D92__INCLUDED_)
#define AFX_SOCKETLINE_H__50CCC726_CC9E_47EA_A0F6_881F9F186D92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SocketCore.h"
#include <list>
                            
const int SOCK_BLOCK_SIZE = 2048;

class CSocketLine : public CSocketCore  
{
public:
	int GetLine(string& sLine, const UINT nRecvTimeout);
	CSocketLine();
	virtual ~CSocketLine();

private:
	int ReceiveAvail(char* buff, int& nRead, const int nBuffLen, const UINT nRecvTimeout);
	void AddToReceive(const char* buffer, int amt);
	string m_sRemainingRecv;
	list<string> m_ReceiveLines;

};

#endif // !defined(AFX_SOCKETLINE_H__50CCC726_CC9E_47EA_A0F6_881F9F186D92__INCLUDED_)
