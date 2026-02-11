// QuarantinePage.h: interface for the CQuarantinePage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUARANTINEPAGE_H__84A3F150_845C_40F2_9F01_54D3C23B4F1E__INCLUDED_)
#define AFX_QUARANTINEPAGE_H__84A3F150_845C_40F2_9F01_54D3C23B4F1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CAnyAttachment
{
public:
	int m_AttachID;
	long m_AttachIDLen;

	TCHAR m_AttachmentLocation[ATTACHMENTS_ATTACHMENTLOCATION_LENGTH];
	long m_AttachmentLocationLen;

	int m_VirusScanStateID;	
	long m_VirusScanStateIDLen;

	int m_IsInbound;
	long m_IsInboundLen;
};


class CQuarantinePage : public CXMLDataClass  
{
public:
	CQuarantinePage( CISAPIData& ISAPIData );
	virtual ~CQuarantinePage();

	virtual int Run( CURLAction& action );

protected:
	
	void Delete( CURLAction& action );
	void DeleteAll( CURLAction& action );
	void Repair( CURLAction& action );
	void RepairAll( CURLAction& action );
	void Restore( CURLAction& action );
	void ListAll( void );
	
	bool DoesFileExist( tstring& sFileName );
	void RestoreAttachment( int nID, LPTSTR szLocation, unsigned char IsInbound, 
		                    int nNewVirusScanState = EMS_VIRUS_SCAN_STATE_RESTORED );
	void DeleteAttachment( int nID, LPTSTR szLocation, unsigned char IsInbound );
	void RepairAttachment( int nID, LPTSTR szLocation, unsigned char IsInbound );

	int m_nCurrentPage;
	int m_nFilesRepaired;
};

#endif // !defined(AFX_QUARANTINEPAGE_H__84A3F150_845C_40F2_9F01_54D3C23B4F1E__INCLUDED_)
