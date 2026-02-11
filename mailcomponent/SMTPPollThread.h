// SMTPPollThread.h: interface for the CSMTPPollThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMTPPOLLTHREAD_H__BC04BCF7_14CC_4545_8A19_69160D78CE90__INCLUDED_)
#define AFX_SMTPPOLLTHREAD_H__BC04BCF7_14CC_4545_8A19_69160D78CE90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SMTPConnection.h"
#include "SMTPDest.h"
#include <EMSMutex.h>

#include <list>
using namespace std;

class CSMTPSession;

class SMTPMessageDestination{
	public:
		SMTPMessageDestination()
			:_id()
			,_description_list(){
		}

		SMTPMessageDestination(int id, const wchar_t* description)
			:_id(id)
			,_description_list(){

			this->Initialize(description);
		}

		SMTPMessageDestination(const SMTPMessageDestination& smd)
			:_id(smd._id)
			,_description_list(smd._description_list){
		}

		~SMTPMessageDestination(){
		}

		SMTPMessageDestination& operator=(const SMTPMessageDestination& smd){

			_id = smd._id;
			_description_list = smd._description_list;

			return *this;
		}
		
		int GetId() { return _id; }
		std::vector<std::wstring>& GetDescriptionList() { return _description_list; }
	private:
		void Initialize(const wchar_t* description){

			std::wstring temp1(description);
	
			std::wstring::size_type pos = std::wstring::npos;

			do{

				pos = temp1.find(';');
				
				if(pos != std::wstring::npos){

					std::wstring temp2 = temp1.substr(0,pos);
					_description_list.push_back(temp2);
					temp1.erase(0, pos + 1);
				}
				else{

					if(temp1[0] == '@'){

						_description_list.push_back(temp1);
					}
				}

			}while(pos != std::wstring::npos);
		}
	private:
		int                       _id;
		std::vector<std::wstring> _description_list;
};

class CSMTPPollThread  
{
public:
	void SendNow();
	const int Stop();
	const UINT Run();
	CSMTPPollThread();
	virtual ~CSMTPPollThread();
protected:
	CDBMaintenanceMutex _DBMaintenanceMutex;
private:
	bool AssignWorkToSession(CSMTPDest* const pDest);
	bool DrainSessionPool();
	bool FillSessionPool();
	const int ResolveMsgDests();
	const int SpawnConnections();
	UINT	m_nServerID;

	enum 
	{ 
		wait_handle_stop,
		send_right_now
	};

	HANDLE m_hWaitHandles[2];
	UINT m_nSendIntervalSecs;
	BOOL m_bSendNow;

	typedef list<CSMTPSession*> SESSION_LIST;
	SESSION_LIST m_sessions;

	UINT m_nFailureMark;
	UINT m_nSuccessMark;
	
	bool m_bDefMsgDestMissing;
	bool m_bLoggedNoAccountsWarning;

	CWinRegistry m_winReg;	
};

#endif // !defined(AFX_SMTPPOLLTHREAD_H__BC04BCF7_14CC_4545_8A19_69160D78CE90__INCLUDED_)
