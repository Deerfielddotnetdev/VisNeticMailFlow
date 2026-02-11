// KAVMsgScanner.cpp: implementation of the CKAVMsgScanner class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailComponents.h"
#include "KAVMsgScanner.h"
#include "Message.h"
#include "KAVScan.h"
#include "OutboundMessage.h"
#include "DateFns.h"
#include "ServerParameters.h"
#include "Utility.h"
#include ".\kavmsgscanner.h"

extern DWORD g_dwScanThreadID;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKAVMsgScanner::CKAVMsgScanner()
{
}

CKAVMsgScanner::~CKAVMsgScanner()
{
}

int CKAVMsgScanner::ScanMsgAttachments(CMessage *const pMsg)
{
	if (g_dwScanThreadID == 0)
		return ERROR_POSTING_SCAN_JOB;

	int nCount = pMsg->GetAttachments().GetCount();
	for (int i=0; i<nCount; i++)
	{
		CAttachment* pAtt = pMsg->GetAttachments().GetAttachment(i);
		assert(pAtt);
		if (!pAtt)
			return ERROR_MEMORY_ALLOCATION;

		// init the VirusScanInfo struct
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		VirusScanInfo vsinfo;
		ZeroMemory( &vsinfo, sizeof(vsinfo) );

		vsinfo.bFile = true;
		DuplicateHandle(GetCurrentProcess(), hEvent,
						GetCurrentProcess(), &vsinfo.hEvent,
						0, FALSE, DUPLICATE_SAME_ACCESS);

		dca::String f(pAtt->GetFullpath().c_str());
		strncpy( vsinfo.szFile, f.c_str(), MAX_PATH );

		LINETRACE(_T("Scanning message attachment (%s) for viruses...\n"), pAtt->GetFullpath().c_str());

		// post the job to the routing engine scanning thread
		if (!PostThreadMessage(	g_dwScanThreadID, 
								WM_VIRUS_SCAN,
								(WPARAM)&vsinfo, 
								(LPARAM)0))
		{
			LINETRACE(_T("Failed to post AV scanning job to scanning thread\r\n"));
			CloseHandle(hEvent);
			return ERROR_POSTING_SCAN_JOB;
		}

		// should be safe to wait forever here, because the scanning thread that sets
		// the event will always flush its event queue during shutdown
		WaitForSingleObject(hEvent, INFINITE);

		CloseHandle(hEvent);

		dca::WString v(vsinfo.szVirusName);
		pAtt->SetVirusName(v.c_str());

		// evaluate the result
		switch (vsinfo.nRet)
		{
		// free of viruses
		case CKAVScan::Success:

			pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FREE);

			// logpoint: Attachment file (%s) was virus-scanned as is virus-free
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_AV_ATTACH_FILE_CLEAN,
							pAtt->GetFullpath().c_str());

			break;

		// infected but cleaned
		case CKAVScan::Success_Object_Has_Been_Cleaned:

			pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED);

			// logpoint:Infected attachment file found, but was effectively handled (path: %s)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_AV_VIRUS_FOUND,
							vsinfo.szVirusName,
							pAtt->GetFullpath().c_str(), 
							_T("file was repaired") );

			break;

		// infected and deleted
		case CKAVScan::Success_Object_Has_Been_Deleted:

			pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_DELETED);

			// logpoint:Infected attachment file found, but was effectively handled (path: %s)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_AV_VIRUS_FOUND,
							vsinfo.szVirusName,
							pAtt->GetFullpath().c_str(), 
							_T("file was deleted") );

			// file was deleted during scanning process; path is now empty
			pAtt->SetFullPath(_T(""));

			break;

		// infected, but quarantined
		case CKAVScan::Success_Object_Has_Been_Quarantined:
			{
				pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_QUARANTINED);

				// logpoint:Infected attachment file found, but was effectively handled (path: %s)
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_VIRUS_SCANNING,
											ERROR_NO_ERROR),
								EMS_STRING_MAILCOMP_AV_VIRUS_FOUND,
								vsinfo.szVirusName,
								pAtt->GetFullpath().c_str(), 
								_T("file was quarantined") );

				// set attach path to reflect new quarantine location
				dca::WString q(vsinfo.szQuarantineLoc);
				pAtt->SetFullPath(q.c_str());
			}
			break;


		// warning condition - attachment couldn't be scanned, and was passed through
		// the system with a warning label
		case CKAVScan::Err_Object_Was_Not_Scanned:

			pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_UNSCANNABLE);

			// logpoint: Attachment file (%s) could not be virus-scanned
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_ATTACHMENT_CANNOT_BE_AV_SCANNED,
							pAtt->GetFullpath().c_str());

			break;

		// av scanner designated attachment file as suspicious, and we passed it
		// through the system with a warning label
		case CKAVScan::Err_Object_Is_Suspicious:

			pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_SUSPICIOUS);

			// logpoint: Attachment file (%s) was flagged as suspicious by the virus scanner (passed through with warning)
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_ATTACHMENT_SCANNED_SUSPICIOUS,
							pAtt->GetFullpath().c_str());

			break;

		// body content could not be scanned
		// Changed by Mark Mohr on 5/6/2003 because this was causing problems
		// by not quarantine messages that had invalid body.
		case CKAVScan::Err_Unexpected_Failure:
			{
				pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_UNSCANNABLE);

				// logpoint: Body of message from %s could not be scanned for viruses
				CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
											EMSERR_MAIL_COMPONENTS,
											EMS_LOG_VIRUS_SCANNING,
											ERROR_NO_ERROR),
								EMS_STRING_MAILCOMP_MSG_BODY_AV_CANNOT_BE_SCANNED,
								pAtt->GetFullpath().c_str(),
								_T("file was quarantined"));

				int nNoScan = CServerParameters::GetInstance().GetParameter(35,1);

				if(nNoScan == 2)
				{
					tstring sFileName = pMsg->GetMsgFilePath();

					QuarantineMsgBody(pMsg, sFileName);
				}
			}
			break;

		// these error conditions are critical problems; the attachment
		// file was either infected or suspicious, but was not handled
		// properly for some reason
		case CKAVScan::Err_Object_Is_Infected:

			pAtt->SetVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FOUND);
			
			// serious problem; this attachment file is still infected!
			// logpoint: Infected attachment file was found but not handled! Path: %s
			// note: critical errors will automatically fire an alert
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_CRITICAL_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_ENACTING_SCAN),
							EMS_STRING_MAILCOMP_INFECTION_UNHANDLED,
							pAtt->GetFullpath().c_str());

			return ERROR_ENACTING_SCAN;

		// these error conditions are warning-level severity; the AV scanning
		// process failed, but we don't know that the attachment file was
		// (or was not) infected
		//case CKAVScan::Err_Initializing_Old_Plugin:
		//case CKAVScan::Err_Calling_Old_Plugin:
		case CKAVScan::Err_Creating_IStream:
		case CKAVScan::Err_Reading_IStream:
		case CKAVScan::Err_Writing_To_IStream:
		case CKAVScan::Err_Seeking_IStream:
		case CKAVScan::Err_Initializing_New_Plugin:
		case CKAVScan::Err_Product_Not_Registered:
		case CKAVScan::Err_License_Expired:
		case CKAVScan::Err_License_Limited:
		case CKAVScan::Err_Calling_New_Plugin:
		case CKAVScan::Err_Deleting_Infected_Object:
		default:
			
			// logpoint: Unexpected error occured during attachment virus scan: %d
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_ENACTING_SCAN),
							EMS_STRING_MAILCOMP_ATTACH_AVSCAN_UNEXPECTED_RESULT,
							vsinfo.nRet);

			return ERROR_ENACTING_SCAN;
		}
	}

	return 0;
}

int CKAVMsgScanner::ScanMsgBody(CMessage *const pMsg)
{
	LINETRACE(_T("+ CKAVMsgScanner::ScanMsgBody - test g_dwScanThreadID"));

	if (g_dwScanThreadID == 0)
	{
		LINETRACE(_T("+ CInboundMessage::Save - function failed when testing for g_dwScanThreadID"));

		return ERROR_POSTING_SCAN_JOB;
	}

	USES_CONVERSION;

	// init the VirusScanInfo struct
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	VirusScanInfo vsinfo;
	ZeroMemory( &vsinfo, sizeof(vsinfo) );
	vsinfo.bFile = false;
	vsinfo.szString = (char*) GlobalAlloc( GMEM_FIXED, pMsg->GetMemoText().GetText().size() + 1 );
	vsinfo.szString[pMsg->GetMemoText().GetText().size()] = 0;
	strncpy( vsinfo.szString, pMsg->GetMemoText().GetText().c_str(), pMsg->GetMemoText().GetText().size() );

	DuplicateHandle(GetCurrentProcess(), hEvent,
					GetCurrentProcess(), &vsinfo.hEvent,
					0, FALSE, DUPLICATE_SAME_ACCESS);
	
	// post the job to the routing engine scanning thread
	if (!PostThreadMessage(	g_dwScanThreadID, 
							WM_VIRUS_SCAN,
							(WPARAM)&vsinfo, 
							(LPARAM)0))
	{
		LINETRACE(_T("- CKAVMsgScanner::ScanMsgBody - Failed to post AV scanning job to scanning thread\r\n"));
		CloseHandle(hEvent);
		GlobalFree( vsinfo.szString );
		return ERROR_POSTING_SCAN_JOB;
	}

	// should be safe to wait forever here, because the scanning thread that sets
	// the event will always flush its event queue during shutdown
	WaitForSingleObject(hEvent, INFINITE);

	CloseHandle(hEvent);

	dca::WString vi(vsinfo.szVirusName);
	pMsg->SetVirusName(vi.c_str());

	LINETRACE(_T("+ CKAVMsgScanner::ScanMsgBody - evaluate this result [%d]"),vsinfo.nRet);

	// evaluate the result
	switch (vsinfo.nRet)
	{
	// free of viruses
	case CKAVScan::Success:

		LINETRACE(_T("+ CKAVMsgScanner::ScanMsgBody - Successfull scan\r\n"));

		pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FREE);

		// logpoint: Message body (from: %s) was virus-scanned as is virus-free
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_INFORMATIONAL,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_VIRUS_SCANNING,
									ERROR_NO_ERROR),
						EMS_STRING_MAILCOMP_AV_MSG_BODY_CLEAN,
						A2T(pMsg->GetOriginator().GetInetName().c_str()));

		break;

	// msg body infected but cleaned
	case CKAVScan::Success_Object_Has_Been_Cleaned:

		pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FOUND_FILE_REPAIRED);

		pMsg->GetMemoText().SetText(vsinfo.szString, pMsg->GetMemoText().GetCharset());

		// logpoint: Message from %s had virus-infected body content which was effectively repaired
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_VIRUS_SCANNING,
									ERROR_NO_ERROR),
						EMS_STRING_MAILCOMP_MSG_BODY_INFECTED_HANDLED,
						A2T(pMsg->GetOriginator().GetInetName().c_str()));

		break;


	// body content is suspicious
	case CKAVScan::Err_Object_Is_Suspicious:
		{
			pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_SUSPICIOUS);

			// logpoint: Message from %s has body content considered suspicious by the virus scanner
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_MSG_BODY_AV_SUSPICIOUS,
							A2T(pMsg->GetOriginator().GetInetName().c_str()));

			int nNoScan = CServerParameters::GetInstance().GetParameter(36,1);

			if(nNoScan == 2)
			{
				tstring sFileName = pMsg->GetMsgFilePath();

				QuarantineMsgBody(pMsg, sFileName);
			}
		}

		break;

	// body content could not be scanned
	// Changed by Mark Mohr on 2/6/2003 because this was causing problems
	// by not deleting mail from the users mailbox.
	case CKAVScan::Err_Object_Was_Not_Scanned:
		{
			pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_NOT_SCANNED);

			// logpoint: Body of message from %s could not be scanned for viruses
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_MSG_BODY_AV_CANNOT_BE_SCANNED,
							A2T(pMsg->GetOriginator().GetInetName().c_str()));

			int nNoScan = CServerParameters::GetInstance().GetParameter(35,1);

			if(nNoScan == 2)
			{
				tstring sFileName = pMsg->GetMsgFilePath();

				QuarantineMsgBody(pMsg, sFileName);
			}
		}
		break;

	case CKAVScan::Err_Unexpected_Failure:
		{
			pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_UNSCANNABLE);

			// logpoint: Body of message from %s could not be scanned for viruses
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_MSG_BODY_AV_CANNOT_BE_SCANNED,
							A2T(pMsg->GetOriginator().GetInetName().c_str()));

			int nNoScan = CServerParameters::GetInstance().GetParameter(35,1);

			if(nNoScan == 2)
			{
				tstring sFileName = pMsg->GetMsgFilePath();

				QuarantineMsgBody(pMsg, sFileName);
			}
		}
		break;

	// msg body text is infected or suspicious; we'll flag it in the DB,
	// and the UI should prevent client rendering
	case CKAVScan::Err_Object_Is_Infected:
		{

			pMsg->SetMemoVirusScanState(EMS_VIRUS_SCAN_STATE_VIRUS_FOUND);
			
			// logpoint: Message from %s has virus-infected body content
			CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_WARNING,
										EMSERR_MAIL_COMPONENTS,
										EMS_LOG_VIRUS_SCANNING,
										ERROR_NO_ERROR),
							EMS_STRING_MAILCOMP_MSG_BODY_INFECTED_LOGENTRY,
							A2T(pMsg->GetOriginator().GetInetName().c_str()));

			int nNoScan = CServerParameters::GetInstance().GetParameter(11,1);

			if(nNoScan == 2)
			{
				tstring sFileName = pMsg->GetMsgFilePath();

				QuarantineMsgBody(pMsg, sFileName);
			}
		}
		break;

	// these error conditions are warning-level severity; the AV scanning
	// process failed, but we don't know what the attachment file was
	// (or was not) infected
	//case CKAVScan::Err_Initializing_Old_Plugin:
	case CKAVScan::Success_Object_Has_Been_Deleted:		// shouldn't happen with msg bodies
	case CKAVScan::Err_Deleting_Infected_Object:
	case CKAVScan::Success_Object_Has_Been_Quarantined: // shouldn't happen with msg bodies
	//case CKAVScan::Err_Calling_Old_Plugin:
	case CKAVScan::Err_Creating_IStream:
	case CKAVScan::Err_Reading_IStream:
	case CKAVScan::Err_Writing_To_IStream:
	case CKAVScan::Err_Seeking_IStream:
	case CKAVScan::Err_Initializing_New_Plugin:
	case CKAVScan::Err_Product_Not_Registered:
	case CKAVScan::Err_License_Expired:
	case CKAVScan::Err_License_Limited:
	case CKAVScan::Err_Calling_New_Plugin:
	default:
		
		LINETRACE(_T("+ CKAVMsgScanner::ScanMsgBody - Default was called."));

		// logpoint: Unexpected error occured during msg body virus scan: %d
		CreateLogEntry(	EMSERROR(	EMS_LOG_SEVERITY_ERROR,
									EMSERR_MAIL_COMPONENTS,
									EMS_LOG_VIRUS_SCANNING,
									ERROR_ENACTING_SCAN),
						EMS_STRING_MAILCOMP_MESSAGEBODY_AVSCAN_UNEXPECTED_RESULT,
						vsinfo.nRet);

		GlobalFree(vsinfo.szString);
		return ERROR_ENACTING_SCAN;
	}

	GlobalFree(vsinfo.szString);

	return 0;
}

const int CKAVMsgScanner::SendInfectionAlert(CMessage *const pMsg)
{
	// first determine if the message has any infected parts (body or attachments);
	// if we can identify that one does, we'll only report the first virus (if there's
	// more than one

	tstring sVirusName;
	bool bDoWarn = false;
	if (pMsg->GetMemoVirusScanState() != EMS_VIRUS_SCAN_STATE_VIRUS_FREE && 
		pMsg->GetMemoVirusScanState() != EMS_VIRUS_SCAN_STATE_NOT_SCANNED &&
		pMsg->GetMemoVirusScanState() != EMS_VIRUS_SCAN_STATE_UNSCANNABLE)
	{
		bDoWarn = true;
		sVirusName = pMsg->GetVirusName();
	}
	else
	{
		// check attachments
		CAttachmentList& atts = pMsg->GetAttachments();
		for (int i=0; i<atts.GetCount(); i++)
		{
			CAttachment* pAtt = atts.GetAttachment(i);
			if (pAtt->GetVirusScanState() != EMS_VIRUS_SCAN_STATE_VIRUS_FREE &&
				pAtt->GetVirusScanState() != EMS_VIRUS_SCAN_STATE_NOT_SCANNED &&
				pMsg->GetMemoVirusScanState() != EMS_VIRUS_SCAN_STATE_UNSCANNABLE)
			{
				bDoWarn = true;
				sVirusName = pAtt->GetVirusName();
				break;
			}
		}
	}

	if (bDoWarn)
	{
		// the message is infected, so warn the user
		try
		{
			/*
			------------------------------------------------------------------
			Warning: Virus detection report!

			The following message has been identified by the MailStream
			antivirus system as being either suspicious or infected.
			------------------------------------------------------------------

			Date message received: %s 

			Message sent from: %s

			Message subject: %s

			Virus detected: %s
			*/

			// format the date/time string
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);
			CEMSString sDateTime;
			if (!GetDateTimeString(sysTime, sDateTime))
			{
				assert(0);
				sDateTime.assign(_T("Date/time formatting failed"));
			}
			
			// load and format the warning message (example above)
			USES_CONVERSION;
			CEMSString s;
			s.Format(	EMS_STRING_MAILCOMP_VIRUS_WARNING_EMAIL_TEXT,
						sDateTime.c_str(),
						A2T(pMsg->GetOriginator().GetInetName().c_str()),
						A2T(pMsg->GetSubject().GetText().c_str()),
						sVirusName.c_str());

			// get the system email address from serverparams
			tstring sSystemEmail = CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_DEF_SYSTEM_EMAIL_ADDRESS, _T("EmailSystem@MailFlow"));

			// send out the warning as a new outbound email message
			COutboundMessage out;			
			out.GetDate().SetCurrent();
			out.GetSubject().SetText("Email Message Virus Alert", "ISO-8859-1");
			out.GetToRecipients().Add(pMsg->GetOriginator());
			out.GetOriginator().Set(T2A(sSystemEmail.c_str()), "MailFlow Mail Subsystem", "ISO-8859-1");
			out.GetMemoText().SetText(T2A(s.c_str()), "ISO-8859-1");
			out.GetMemoText().SetMediaType("text");
			out.GetMemoText().SetMediaSubType("plain");
			int nRet = out.SaveQueued();
			if (nRet != 0)
			{
				LINETRACE(_T("Failed to save outbound virus notification message: %d\n"), nRet);
				assert(0);
				return nRet;
			}
		}
		catch (CEMSException EMSException)
		{
			LINETRACE(_T("Error while formatting EMS string (%s)\n"), EMSException.GetErrorString());
			assert(0);
			return EMSException.GetErrorCode();
		}
	}

	return 0;
}

const BOOL CKAVMsgScanner::GetIsScanningEnabled()
{
	return (BOOL)CServerParameters::GetInstance().GetParameter(EMS_SRVPARAM_ANTIVIRUS_ENABLE, 0);
}

bool CKAVMsgScanner::QuarantineMsgBody(CMessage *const pMsg, tstring& sFileName)
{
	if(!sFileName.empty())
	{
		CKAVScan kavScan;
		HKEY hKey = 0;
		TCHAR sBuf[_MAX_PATH + 1];

		//Createfilename
		time_t lTime;
		time(&lTime);

		tm* pLT = localtime(&lTime);

		_tcsftime(sBuf,_MAX_PATH,_T("%Y%m%d%H%M%S"),pLT);

		tstring sNewFileName = sBuf;
		_ltot(pMsg->GetMessageID(),sBuf,10);

		sNewFileName.append(sBuf);
		sNewFileName.append(_T(".tmp"));

		tstring sQuarantinePath;

		// Open registry key
		long lRetVal = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
									  _T("SOFTWARE\\Deerfield.com\\VisNetic MailFlow\\VirusScanner"),
									  0,
									  KEY_ALL_ACCESS,
									  &hKey);

		// Was it successfull
		if(lRetVal == ERROR_SUCCESS)
		{
			// Yes then declare are variables
			TCHAR sFile[_MAX_PATH + 1];
			DWORD dwType;
			DWORD dwSize = _MAX_PATH;

			// Clear buffer
			ZeroMemory(sFile,_MAX_PATH + 1);

			// Get Temp Folder
			lRetVal = ::RegQueryValueEx(hKey,_T("QuarantineFolder"),0,&dwType,(BYTE*)sFile,&dwSize);

			// Was it successfull
			if(lRetVal == ERROR_SUCCESS)
			{
				// If yes then copy it to tstring
				sQuarantinePath = sFile;

				// whackify folder-location if needed
				if (sQuarantinePath.length() > 0 && sQuarantinePath.at(sQuarantinePath.size() - 1) != '\\')
					sQuarantinePath += '\\';

				CUtility util;
				util.EnsurePathExists(sQuarantinePath.c_str());

				sQuarantinePath.append(sNewFileName);

				if(::CopyFile(sFileName.c_str(),sQuarantinePath.c_str(),FALSE))
				{
					return true;
				}
			}
		}
	}

	return false;
}
