// SAXErrorHandler.cpp: implementation of the SAXErrorHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAXErrorHandlerImpl.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SAXErrorHandlerImpl::SAXErrorHandlerImpl()
{

}

SAXErrorHandlerImpl::~SAXErrorHandlerImpl()
{

}

HRESULT STDMETHODCALLTYPE SAXErrorHandlerImpl::error( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ const wchar_t * pwchErrorMessage,
			/* [in] */ HRESULT errCode)
{
	int line = 0;
	int column = 0;

	wchar_t buf[256];
	swprintf(buf, L"Error (%x): %s\n", errCode, pwchErrorMessage );
	MessageBox(0, buf, L"Config Database", MB_OK);

	pLocator->getLineNumber( &line );
	pLocator->getColumnNumber( &column );

	swprintf(buf, L"Line %d, Column %d\n", line, column );
	MessageBox(0, buf, L"Config Database", MB_OK);

	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE SAXErrorHandlerImpl::fatalError( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ const wchar_t * pwchErrorMessage,
			/* [in] */ HRESULT errCode)
{
	int line = 0;
	int column = 0;

	wchar_t buf[256];
	swprintf(buf, L"Fatal Error (%x): %s\n", errCode, pwchErrorMessage );
	MessageBox(0,buf, L"Config Database", MB_OK);

	pLocator->getLineNumber( &line );
	pLocator->getColumnNumber( &column );

	swprintf(buf, L"Line %d, Column %d\n", line, column );
	MessageBox(0,buf, L"Config Database", MB_OK);

	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE SAXErrorHandlerImpl::ignorableWarning( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ const wchar_t * pwchErrorMessage,
			/* [in] */ HRESULT errCode)
{
	int line = 0;
	int column = 0;

	wchar_t buf[256];
	swprintf(buf, L"Warning (%x): %s\n", errCode, pwchErrorMessage );
	MessageBox(0,buf, L"Config Database", MB_OK);

	pLocator->getLineNumber( &line );
	pLocator->getColumnNumber( &column );

	swprintf(buf, L"Line %d, Column %d\n", line, column );
	MessageBox(0,buf, L"Config Database", MB_OK);

	return S_OK;
}

long __stdcall SAXErrorHandlerImpl::QueryInterface(const struct _GUID &,void ** )
{
	// hack-hack-hack!
	return 0;
}

unsigned long __stdcall SAXErrorHandlerImpl::AddRef()
{
	// hack-hack-hack!
	return 0;
}

unsigned long __stdcall SAXErrorHandlerImpl::Release()
{
	// hack-hack-hack!
	return 0;
}