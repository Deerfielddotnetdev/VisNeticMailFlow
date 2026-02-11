//******************************************************************************
// $Author: markm $
// $Date: 2005/10/28 17:43:30 $
// $Name:  $
// $Revision: 1.7 $
// $Source: c:/cvsrepo/mailflow/vmfadmintool/vmfadmintool/VMFAdminToolApplication.cpp,v $
//******************************************************************************

#include "prehead.h"
#include ".\VMFAdminToolApplication.h"
#include ".\MailStreamEngine_i.c"
#include ".\TopController.h"
#include ".\LoginController.h"

//******************************************************************************
// Default constructor

VMFAdminToolApplication::VMFAdminToolApplication(HINSTANCE hInst)
	:dca::WinApplication(hInst)  // Initialize base class
{
}

//******************************************************************************

//******************************************************************************
// Destructor
VMFAdminToolApplication::~VMFAdminToolApplication(void)
{
}

//******************************************************************************

//******************************************************************************
// Our overrided Initialize function

int VMFAdminToolApplication::Initialize(int nShow)
{
	try
	{
		// Initialize COM
		dca::UseCom useCom;
		
		// Test to see if we have admin rights to machine
		if(HaveAdminRights())
		{
			DcaTrace(_T("We have admin rights"));

			// We do. Launch main dialog
			return DoMainDialog(1);
		}
		else
		{
			DcaTrace(_T("We do not have admin rights"));

			::MessageBox(0,_T("Elevation is required to run this application! Please\rright click and choose 'Run as an administrator'."),_T("VisNetic MailFlow Admin Tool"),MB_OK);

			/*LoginController loginCtrl;
			dca::ModalDialog loginDlg;

			loginDlg.Create(_hInstance, DLG_LOGIN, &loginCtrl, 0);

			if(loginDlg.GetReturn())
			{
				if(Authenticate(loginCtrl.GetUsername(), loginCtrl.GetPassword()))
				{
					return DoMainDialog(0);
				}
			}*/
		}
	}
	catch(dca::Exception e)  // Catch any DCA library exceptions
	{
		::MessageBox(0,e.GetMessage(),_T("VisNetic MailFlow Admin Tool"),MB_OK);
	}
	catch(...) // Catch all other exceptions
	{
		::MessageBox(0,_T("An unknown or undefined Exception has occurred"),_T("VisNetic MailFlow Admin Tool"),MB_OK);
	}

	return -1;
}

//******************************************************************************

//******************************************************************************
// DoMainDialog function - Creates, Shows dialog

int VMFAdminToolApplication::DoMainDialog(int nWriteAccess)
{
	TopController topCtlr(nWriteAccess);	// The main dialogs controller.
	dca::Dialog topDialog;	// The main dialog
		
	// Create dialog
	topDialog.Create(this->_hInstance, DLG_MAIN, &topCtlr);

	// Set the applications _hMainHwnd handle.  Needed for proper processing of
	// Window message to a dialog
	this->_hMainHwnd = topDialog.GetHwnd();

	// Show dialog.  This is not needed if dialog visible property is set in
	// the resource editor.
	topDialog.Show();

	// Run main thread.  This does not return until the dialog is closed.
	return this->Run();
}

//******************************************************************************

//******************************************************************************
// HaveAdminRights - test to see if the current user has admin rights.

int VMFAdminToolApplication::HaveAdminRights()
{
	// Obtain access token
	if(!::ImpersonateSelf(SecurityImpersonation))
		throw dca::Exception(::GetLastError(),_T("Failed to obtain an access token that impersonates the security context of the calling process."));

	// Create ThreadToken object
	dca::ThreadToken threadToken(::GetCurrentThread(), TOKEN_QUERY, 0);

	BOOL bRetVal = FALSE;

	try
	{
		// Open ThreadToken
		threadToken.Open();

		// Set SecurityID
		SID_IDENTIFIER_AUTHORITY sidIDAuth = SECURITY_NT_AUTHORITY;
		dca::SecurityID secID(sidIDAuth);

		// Create Security Descriptor pointer and allocate memory
		dca::AutoLocalAlloc<SECURITY_DESCRIPTOR> pSecDesc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		
		// Initialize Security Descriptor
		if(!::InitializeSecurityDescriptor(pSecDesc.Get(), SECURITY_DESCRIPTOR_REVISION))
			throw dca::Exception(::GetLastError(),_T("Failed to initialize a new security descriptor."));

		// Compute ACLSize for ACL pointer
		size_t nACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + secID.GetLength() - sizeof(DWORD);

		// Create ACL pointer and allocate memory
		dca::AutoLocalAlloc<ACL> pACL(nACLSize);
		
		// Initialize ACL
		if(!::InitializeAcl(pACL.Get(), (DWORD)nACLSize, ACL_REVISION2))
			throw dca::Exception(::GetLastError(),_T("Failed to initialize a new ACL structure."));

		// Create access mask
		DWORD dwAccessMask = ACCESS_READ | ACCESS_WRITE;

		// Add access-allowed control entry
		if(!::AddAccessAllowedAce(pACL.Get(), ACL_REVISION2, dwAccessMask, secID))
			throw dca::Exception(::GetLastError(),_T("Failed to add an access-allowed access control entry to an access control list."));

		// Set Dacl
		if(!::SetSecurityDescriptorDacl(pSecDesc.Get(), TRUE, pACL.Get(), FALSE))
			throw dca::Exception(::GetLastError(),_T("Failed to set information in a discretionary access control list."));

		// Set Group
		if(!::SetSecurityDescriptorGroup(pSecDesc.Get(), secID, FALSE))
			throw dca::Exception(::GetLastError(),_T("Failed to set the primary group information of an absolute-format security descriptor."));

		// Set Owner
		if(!::SetSecurityDescriptorOwner(pSecDesc.Get(), secID, FALSE))
			throw dca::Exception(::GetLastError(),_T("Failed to set the owner information of an absolute-format security descriptor."));

		// Valid Security Descriptor
		if(!::IsValidSecurityDescriptor(pSecDesc.Get()))
			throw dca::Exception(::GetLastError(),_T("Failed to determine whether the components of a security descriptor are valid."));

		// Setup Access and a generic mapping.
		DWORD dwAccessDesired = ACCESS_READ;
		GENERIC_MAPPING genericMap;
		genericMap.GenericRead = ACCESS_READ;
		genericMap.GenericWrite = ACCESS_WRITE;
		genericMap.GenericExecute = 0;
		genericMap.GenericAll = ACCESS_READ | ACCESS_WRITE;
		DWORD dwStructureSize = sizeof(PRIVILEGE_SET);
		PRIVILEGE_SET privSet;
		DWORD dwStatus;

		// Check Access
		if(!::AccessCheck(pSecDesc.Get()
			              ,threadToken
						  ,dwAccessDesired
						  ,&genericMap
						  ,&privSet
						  ,&dwStructureSize
						  ,&dwStatus
						  ,&bRetVal))
		{
			throw dca::Exception(::GetLastError(),_T("Failed to determine whether a security descriptor grants a specified set of access rights to the client identified by an access token."));
		}

		if(!bRetVal)
			throw dca::Exception(::GetLastError(),_T("Access check failed"));
	}
	catch(dca::Exception e) // Catch any DCA library exceptions
	{
		DcaTrace(e.GetMessage());
		return 0;
	}

	// When done revert to self
	if(!::RevertToSelf())
			DcaTrace(_T("Failed to terminate the impersonation of a client application."));

	return (bRetVal) ? 1 : 0;
}

//******************************************************************************

//******************************************************************************
// HaveAdminRights - test to see if the current user has admin rights.

int VMFAdminToolApplication::Authenticate(LPCTSTR sUsername, LPCTSTR sPassword)
{
	long lReturn = 0;

	{
		dca::SmartComObject comObjMFAdmin(CLSID_RoutingEngineAdmin);
		dca::SmartComInterface<IRoutingEngineAdmin, IID_IRoutingEngineAdmin> iMFAdmin(comObjMFAdmin);

		dca::BString bUsername(sUsername);
		dca::BString bPassword(sPassword);

		iMFAdmin->Authenticate(bUsername, bPassword, &lReturn);

		if(lReturn)
			DcaTrace(_T("Authenticate = 1"));
		else
			DcaTrace(_T("Authenticate = 0"));
	}

	return lReturn;
}

//******************************************************************************

//******************************************************************************
// HaveAdminRights - test to see if the current user has admin rights.

void VMFAdminToolApplication::ReloadMailFlow()
{
	try
	{
		{
			dca::SmartComObject comObjMFComm(CLSID_RoutingEngineComm);
			dca::SmartComInterface<IRoutingEngineComm, IID_IRoutingEngineComm> iMFComm(comObjMFComm);

			iMFComm->ReloadConfig(4);
		}

		dca::WString emsEvent(_T("EMS_RELOAD_ISAPI"));
		dca::MailFlowEvent mfEvent(emsEvent);
		mfEvent.Set();
	}
	catch(...){}
}

//******************************************************************************
