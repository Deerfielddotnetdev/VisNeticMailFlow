// SocketLine.cpp: implementation of the CSocketLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketLine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketLine::CSocketLine()
{
	m_sRemainingRecv = "";
}

CSocketLine::~CSocketLine()
{

}

int CSocketLine::GetLine(string& sLine, const UINT nRecvTimeout)
{
	// if our line buffer is empty, we must add to it
	while (m_ReceiveLines.size() == 0)
	{
		char szTmp[SOCK_BLOCK_SIZE+1];

		int nRead;
		int nRet = ReceiveAvail(szTmp, nRead, SOCK_BLOCK_SIZE, nRecvTimeout);
		if (nRet != 0)
			return nRet;

		szTmp[nRead] = 0;

		//string sBuffer = (m_sRemainingRecv + szTmp);
		//m_sRemainingRecv = "";

		AddToReceive(szTmp, nRead);
		//AddToReceive(sBuffer.c_str(), sBuffer.size());
	}

	if (m_ReceiveLines.size() != 0)
	{
		sLine = m_ReceiveLines.front();
		m_ReceiveLines.pop_front();
	}
	else
	{
		assert(0);
		return FAILURE_DATA_TRANSMISSION;
	}

	return 0;
}

// waits for a read event on the socket, then receives whatever
// is available
int CSocketLine::ReceiveAvail(char *buff, int& nRead, const int nBuffLen, const UINT nRecvTimeout)
{
	// reset our read event
	ResetEvent(m_hSockHandles[local_event_socket]);

	// Register our interest in the FD_READ event
	m_nRet = WSAEventSelect(m_sockActive, m_hSockHandles[local_event_socket], FD_READ);
	if (m_nRet != 0)
	{
		LINETRACE(_T("Error during WSAEventSelect: %d\n"), WSAGetLastError());
		return FAILURE_DATA_TRANSMISSION;
	}

	// wait for an event to become signaled
	m_nRet = WaitForMultipleObjects(2, m_hSockHandles, FALSE, nRecvTimeout * 1000);
	if (m_nRet == (WAIT_OBJECT_0 + local_event_stop)) 	// if we get a stop event
	{
		LINETRACE(_T("Recv operation cancelled\n"));
		return FAILURE_OPERATION_CANCELLED;
	}
	else if (m_nRet == WAIT_TIMEOUT)					// if we timeout
	{
		LINETRACE(_T("Recv operation timed out\n"));
		return FAILURE_OPERATION_TIMEOUT;
	}
	else if (m_nRet != (WAIT_OBJECT_0 + local_event_socket))	// other error
	{
		LINETRACE(_T("Recv operation - unknown error\n"));
		return FAILURE_DATA_TRANSMISSION;
	}

	// find out what's up with the socket
	WSANETWORKEVENTS networkEvents;
	ZeroMemory(&networkEvents, sizeof(WSANETWORKEVENTS));
	m_nRet = WSAEnumNetworkEvents(m_sockActive, m_hSockHandles[local_event_socket], &networkEvents);
	if (m_nRet != 0)
	{
		LINETRACE(_T("Error during WSAEnumNetworkEvents: %d\n"), WSAGetLastError());
		return FAILURE_DATA_TRANSMISSION;
	}

	// make sure FD_READ is set
	if ((networkEvents.lNetworkEvents & FD_READ) == 0)
	{
		LINETRACE(_T("Failed to get FD_READ during recv\n"));
		return FAILURE_DATA_TRANSMISSION;
	}

	// receive as much as we can in one pass
	nRead = recv(m_sockActive, buff, nBuffLen, 0);
	if ((nRead == 0) || (nRead == SOCKET_ERROR))
	{
		LINETRACE(_T("Error during recv: %d\n"), WSAGetLastError());
		return FAILURE_DATA_TRANSMISSION;
	}

	// success
	return 0;
}

// processes a receive buffer
void CSocketLine::AddToReceive(const char* buffer, int amt)
{
	char temp[SOCK_BLOCK_SIZE+1];

	int length;
	int cur_pos = 0;
	while (cur_pos < amt)
	{
		char* lf = NULL; // = strchr(buffer+cur_pos, '\n');
		char* search = (char*)(buffer+cur_pos);
		for (int i=0; i < (amt-cur_pos); i++)
		{			
			if (*search == '\n')
			{
				lf = search;
				break;
			}

			search++;
		}

		if (lf != NULL)
			length = lf-buffer-cur_pos+1;
		else
			length = (amt-cur_pos);

		strncpy(temp, buffer+cur_pos, length);
		temp[length] = '\0';            

		// we have a linefeed
		if (lf)
		{
			//m_ReceiveLines.push_back(temp);
			if (m_sRemainingRecv.size() == 0)
			{
				m_ReceiveLines.push_back(temp);
			}                            
			else
			{
				m_ReceiveLines.push_back(m_sRemainingRecv + temp);
				m_sRemainingRecv = "";
			}
 		}
 		else
 		{
 			// we don't have a linefeed so add to m_sRemainingRecv buffer
 			m_sRemainingRecv += temp;
 		}

 		cur_pos += length;
	}
}
