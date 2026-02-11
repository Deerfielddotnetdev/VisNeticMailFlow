// SocketCore.cpp : implementation file
//

#include "stdafx.h"
#include "SocketCore.h"
#include <malloc.h>
#include <stdio.h>
#include "Utility.h"
using namespace std;

/*-----------------------------------------------------------\\
|| Constructor
\*----------------------------------------------------------*/
CSocketCore::CSocketCore()
{
	// create local events
	m_hSockHandles[local_event_stop] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hSockHandles[local_event_socket] = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	m_sockActive = INVALID_SOCKET;
}

/*-----------------------------------------------------------\\
|| Destructor
\*-----------------------------------------------------------*/
CSocketCore::~CSocketCore()
{
	if (m_hSockHandles[local_event_stop])
		CloseHandle(m_hSockHandles[local_event_stop]);

	if (m_hSockHandles[local_event_socket])
		CloseHandle(m_hSockHandles[local_event_socket]);

	if (m_sockActive != INVALID_SOCKET)
		closesocket(m_sockActive);
}

/*-----------------------------------------------------------\\
|| Function creates the connection socket
\*-----------------------------------------------------------*/
int CSocketCore::CreateSocket()
{
	m_sockActive = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (m_sockActive == INVALID_SOCKET)
	{
		dca::String f;
		dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
		f.Format("CSocketCore::CreateSocket - Error creating socket: %d (%s)", WSAGetLastError(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return FAILURE_SOCKET_CREATE;
	}
	
	return 0; // success
}

/*-----------------------------------------------------------\\
|| Connect to another host - this version will use the proxy
|| settings if set in the registry
\*-----------------------------------------------------------*/
int CSocketCore::Connect(LPCTSTR szHost, const int nPort, const UINT nTimeoutSecs)
{
	int nRet;
	
	ZeroMemory(&m_sockaddrActive, sizeof(m_sockaddrActive));

	USES_CONVERSION;
	
	// init server sockaddr_in structure
	m_sockaddrActive.sin_family			= AF_INET;
	m_sockaddrActive.sin_port			= htons(nPort);
	m_sockaddrActive.sin_addr.s_addr	= inet_addr(T2CA(szHost));
	
	// do host name resolution if necessary
	if (m_sockaddrActive.sin_addr.s_addr == INADDR_NONE)
	{
		// resolve host to ip
		struct hostent* pHostent = gethostbyname(T2CA(szHost));
		if (pHostent == NULL || pHostent->h_addr_list[0] == NULL)
		{
			dca::String f;
			f.Format("CSocketCore::Connect - Failed to resolve hostname: %s", T2CA(szHost));
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return FAILURE_SERVER_LOOKUP;
		}
		
		CopyMemory(&m_sockaddrActive.sin_addr, pHostent->h_addr_list[0], pHostent->h_length);
	}
	
	// put socket into non-blocking mode
	ResetEvent(m_hSockHandles[local_event_socket]);
	nRet = WSAEventSelect(m_sockActive, m_hSockHandles[local_event_socket], FD_CONNECT);
	if (nRet != 0)
	{
		dca::String f;
		dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
		f.Format("CSocketCore::Connect - Error during WSAEventSelect: %d (%s)", WSAGetLastError(), e.c_str());
		DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
		return FAILURE_SERVER_CONNECT;
	}

	// initiate socket connection
	connect(m_sockActive, (struct sockaddr*)&m_sockaddrActive, sizeof(m_sockaddrActive));
	
	// wait for socket connection to complete (within given timeframe)
	nRet = WaitForMultipleObjects(2, m_hSockHandles, FALSE, nTimeoutSecs * 1000);
	if (nRet == (WAIT_OBJECT_0 + local_event_stop))
		return FAILURE_OPERATION_CANCELLED;
	else if (nRet == WAIT_TIMEOUT) // event never became signaled
		return FAILURE_SERVER_CONNECT_TIMEOUT;
	
	// enumerate and check network events
	WSANETWORKEVENTS networkEvents;
	ZeroMemory(&networkEvents, sizeof(WSANETWORKEVENTS));
	nRet = WSAEnumNetworkEvents(m_sockActive, m_hSockHandles[local_event_socket], &networkEvents);
	if (networkEvents.lNetworkEvents & FD_CONNECT)
	{
		// if there's an error, return the appropriate one
		if (networkEvents.iErrorCode[FD_CONNECT_BIT] != 0)
		{
			// extract and act upon the error code
			switch (networkEvents.iErrorCode[FD_CONNECT_BIT])
			{
			case WSAECONNREFUSED:
				return FAILURE_SERVER_CONNECT_REFUSED;
			case WSAENETUNREACH:
				return FAILURE_SERVER_CONNECT;
			case WSAETIMEDOUT:
				return FAILURE_SERVER_CONNECT_TIMEOUT;
			default:
				return FAILURE_SERVER_CONNECT;
			}
		}
	}
	else
		return FAILURE_SERVER_CONNECT;
	
	return 0; // success
}

/*-----------------------------------------------------------\\
|| Send a buffer
\*-----------------------------------------------------------*/
int CSocketCore::Send(char *buff, const int nBuffLen, const UINT nTransmitTimeout)
{
	assert(buff);
	assert(nBuffLen > 0);
	assert(nTransmitTimeout > 0);

	int nLeft = nBuffLen;
	int nIdx = 0;
	while (nLeft > 0)
	{
		// assume we've gotten FD_WRITE and attempt to send
		m_nRet = send(m_sockActive, ((char*)(buff + nIdx)), nLeft, 0);

		// check for error
		if ((m_nRet == 0) || (m_nRet == SOCKET_ERROR))
		{
			int nWSAError = WSAGetLastError();

			// if we would block on send here, we wait for the earliest
			// opportunity to continue
			if (nWSAError == WSAEWOULDBLOCK)
			{
				ResetEvent(m_hSockHandles[local_event_socket]);

				// select the event for the socket
				m_nRet = WSAEventSelect(m_sockActive, m_hSockHandles[local_event_socket], FD_WRITE);
				if (m_nRet != 0)
				{
					dca::String f;
					dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
					f.Format("CSocketCore::Send - Error during WSAEventSelect: %d (%s)", WSAGetLastError(), e.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					return FAILURE_DATA_TRANSMISSION;
				}

				// wait for an event to become signaled
				m_nRet = WaitForMultipleObjects(2, m_hSockHandles, FALSE, nTransmitTimeout * 1000);
				if (m_nRet == (WAIT_OBJECT_0 + local_event_stop)) 	// if we get a stop event
					return FAILURE_OPERATION_CANCELLED;
				else if (m_nRet == WAIT_TIMEOUT)					// if we timeout
					return FAILURE_OPERATION_TIMEOUT;

				// Let's find out what's up with the socket
				WSANETWORKEVENTS networkEvents;
				m_nRet = WSAEnumNetworkEvents(m_sockActive, m_hSockHandles[local_event_socket], &networkEvents);
				if (m_nRet != 0)
				{
					dca::String f;
					dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
					f.Format("CSocketCore::Send - Error during WSAEnumNetworkEvents: %d (%s)", WSAGetLastError(), e.c_str());
					DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
					return FAILURE_DATA_TRANSMISSION;
				}

				// make sure FD_WRITE is set
				if ((networkEvents.lNetworkEvents & FD_WRITE) == 0)
				{
					DebugReporter::Instance().DisplayMessage("CSocketCore::Send - Failed to get FD_WRTE during send", DebugReporter::MAIL);
					return FAILURE_DATA_TRANSMISSION;
				}
			}
			else
			{
				// an error unrelated to WSAEWOULDBLOCK has occurred
				dca::String f;
				f.Format("CSocketCore::Send - Fatal error occured during Send: %d", nWSAError);
				DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
				return FAILURE_DATA_TRANSMISSION;
			}
		}
		else	// data chunk sent successfully
		{
			// calc remaining packet size, shift index
			nLeft -= m_nRet;
			nIdx += m_nRet;
		}
	}

	// success
	return 0; 
}


/*-----------------------------------------------------------\\
|| Receive data into buffer
\*-----------------------------------------------------------*/
int CSocketCore::Recv(char *buff, const int nBuffLen, const UINT nRecvTimeout)
{
	int nLeft = nBuffLen;
	int nIdx = 0;
	while (nLeft > 0)
	{
		// reset our read event
		ResetEvent(m_hSockHandles[local_event_socket]);

		// Register our interest in the FD_READ event
		m_nRet = WSAEventSelect(m_sockActive, m_hSockHandles[local_event_socket], FD_READ);
		if (m_nRet != 0)
		{
			dca::String f;
			dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
			f.Format("CSocketCore::Recv - Error during WSAEventSelect: %d (%s)", WSAGetLastError(), e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// wait for an event to become signaled
		m_nRet = WaitForMultipleObjects(2, m_hSockHandles, FALSE, nRecvTimeout * 1000);
		if (m_nRet == (WAIT_OBJECT_0 + local_event_stop)) 	// if we get a stop event
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::Recv - Recv operation cancelled",  DebugReporter::MAIL);
			return FAILURE_OPERATION_CANCELLED;
		}
		else if (m_nRet == WAIT_TIMEOUT)					// if we timeout
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::Recv - Recv operation timed out",  DebugReporter::MAIL);
			return FAILURE_OPERATION_TIMEOUT;
		}
		else if (m_nRet != (WAIT_OBJECT_0 + local_event_socket))	// other error
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::Recv - Recv operation - unknown error",  DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// find out what's up with the socket
		WSANETWORKEVENTS networkEvents;
		ZeroMemory(&networkEvents, sizeof(WSANETWORKEVENTS));
		m_nRet = WSAEnumNetworkEvents(m_sockActive, m_hSockHandles[local_event_socket], &networkEvents);
		if (m_nRet != 0)
		{
			dca::String f;
			dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
			f.Format("CSocketCore::Recv - Error during WSAEnumNetworkEvents: %d (%s)", WSAGetLastError(), e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// make sure FD_READ is set
		if ((networkEvents.lNetworkEvents & FD_READ) == 0)
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::ReadLine - Failed to get FD_READ during recv",  DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// receive the packet chunk
		m_nRet = recv(m_sockActive, (buff + nIdx), nLeft, 0);
		if ((m_nRet == 0) || (m_nRet == SOCKET_ERROR))
		{
			dca::String f;
			dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
			f.Format("CSocketCore::ReadLine - Error during recv: %d (%s)", WSAGetLastError(), e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// calc remaining packet size, shift index
		nLeft -= m_nRet;
		nIdx += m_nRet;
	}

	// success
	return 0;
}

/*-----------------------------------------------------------\\
|| Close connection, do cleanup
\*-----------------------------------------------------------*/
void CSocketCore::Close()
{
	if (m_sockActive)
	{
		shutdown(m_sockActive, SD_BOTH);
		closesocket(m_sockActive);
		m_sockActive = INVALID_SOCKET;;
	}	
}

/*-----------------------------------------------------------\\
|| Abort any waits in progress
\*-----------------------------------------------------------*/
void CSocketCore::Stop()
{
	if (!SetEvent(m_hSockHandles[local_event_stop]))
		assert(0);
}

/*-----------------------------------------------------------\\
|| Reads one line via the connected socket and returns
\*-----------------------------------------------------------*/
int CSocketCore::ReadLine(char* buff, const int bufLen, int& nRead, const UINT nTimeout)
{
	// init
	nRead = 0;
	bool bCont = true;
	ZeroMemory(buff, bufLen);
	
	char onechar[1];
	int nLastChar = -1;
	
	// iteration
	while (bCont)
	{
		// reset our read event
		ResetEvent(m_hSockHandles[local_event_socket]);

		// Register our interest in the FD_READ event
		m_nRet = WSAEventSelect(m_sockActive, m_hSockHandles[local_event_socket], FD_READ);
		if (m_nRet != 0)
		{
			dca::String f;
			dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
			f.Format("CSocketCore::ReadLine - Error during WSAEventSelect: %d (%s)", WSAGetLastError(), e.c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// wait for an event to become signaled
		m_nRet = WaitForMultipleObjects(2, m_hSockHandles, FALSE, nTimeout * 1000);
		if (m_nRet == (WAIT_OBJECT_0 + local_event_stop)) 	// if we get a stop event
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::ReadLine - Recv operation cancelled",  DebugReporter::MAIL);
			return FAILURE_OPERATION_CANCELLED;
		}
		else if (m_nRet == WAIT_TIMEOUT)					// if we timeout
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::ReadLine - Recv operation timed out",  DebugReporter::MAIL);
			return FAILURE_OPERATION_TIMEOUT;
		}
		else if (m_nRet != (WAIT_OBJECT_0 + local_event_socket))	// other error
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::ReadLine - Recv operation - unknown error",  DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// make sure that it's safe to read from the socket
		WSANETWORKEVENTS networkEvents;
		m_nRet = WSAEnumNetworkEvents(m_sockActive, m_hSockHandles[local_event_socket], &networkEvents);
		if (m_nRet != 0)
		{
			dca::String f;
			dca::String e(CUtility::GetErrorString(WSAGetLastError()).c_str());
			f.Format("Error during WSAEnumNetworkEvents: %d (%s)", WSAGetLastError(), CUtility::GetErrorString(WSAGetLastError()).c_str());
			DebugReporter::Instance().DisplayMessage(f.c_str(), DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		// make sure FD_READ is set
		if ((networkEvents.lNetworkEvents & FD_READ) == 0)
		{
			DebugReporter::Instance().DisplayMessage("CSocketCore::ReadLine - Failed to get FD_READ during ReadLine",  DebugReporter::MAIL);
			return FAILURE_DATA_TRANSMISSION;
		}

		int nRet = -1;
		while (nRet != 0)
		{
			nRet = recv(m_sockActive, onechar, 1, 0);
			if (nRet == SOCKET_ERROR)
			{
				DWORD dwErr = GetLastError();
				if (dwErr == WSAEMSGSIZE || dwErr == WSAEWOULDBLOCK)
					break;
				else
					return FAILURE_DATA_TRANSMISSION;
			}
			else if (nRet != 0)
			{
				buff[nRead] = onechar[0];
				nRead++;

				if ((nLastChar == '\r' && onechar[0] == '\n')	// found our newline chars
					|| nRead == bufLen)							// exceeded max buffer size
				{
					bCont = false;
					break;
				}

				nLastChar = onechar[0];
			}
		}
	}

	return 0;
}

