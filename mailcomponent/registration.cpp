// Registration.cpp: implementation of the CRegistration class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Registration.h"
#include "DCIKey.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistration::CRegistration()
{

}

CRegistration::~CRegistration()
{

}

int CRegistration::GetRegIsEval(const tstring& sRegKey, bool& bIsEval, bool& bIsExpired)
{
	USES_CONVERSION;

	bIsEval = false;
	bIsExpired = false;

	DCIKeyStruct keyStruct;
	int nErr = DCIKeyAnalyze(T2A(sRegKey.c_str()), &keyStruct);
	if (nErr != DCIKEY_Success)
		return nErr;

	// is eval?
	if (keyStruct.KeyType == DCIKEY_KeyType_Evaluation)
		bIsEval = true;

	// is expired?
	if (DCIKeyExpired(&keyStruct) == DCIKEY_Error_Key_Expired)
		bIsExpired = true;

	return 0;
}