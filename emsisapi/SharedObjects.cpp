// SharedObjects.cpp: implementation of the CSharedObjects class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SharedObjects.h"
#include "RegistryFns.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSharedObjects::CSharedObjects()
{
	m_bDBMaintenanceRunning = false;
	m_bUseATC = false;
	m_bFullText = false;
	m_nIISVer = 6;
	m_nCharSet = 0;
	m_nTimeZone = 0;
}

CSharedObjects::~CSharedObjects()
{

}
