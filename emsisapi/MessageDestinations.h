// MessageDestinations.h: interface for the CMessageDestinations class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEDESTINATIONS_H__01DC0AC2_E997_4E88_91D6_3CF9A8ACF4DC__INCLUDED_)
#define AFX_MESSAGEDESTINATIONS_H__01DC0AC2_E997_4E88_91D6_3CF9A8ACF4DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"

class CMessageDestinations : public CXMLDataClass , public TMessageDestinations
{
public:
	CMessageDestinations( CISAPIData& ISAPIData );
	virtual ~CMessageDestinations();

	virtual int Run( CURLAction& action );

protected:
	int ListAll(void);
	int Query(void);
	int Update(void);
	int Delete(void);
	int New();
	void DecodeForm(void);
	void SetDefault( CURLAction& action );
	int FailedDelivery( CURLAction& action );
	void GenerateXML(void);
	void RespoolSelected( void );
	void RespoolOlderThan( void );

	void EncryptPassword();
	void DecryptPassword();
	void AddServersXML();
	
	bool m_bDefaultSet;
	int m_nDefaultMsgDest;
};

#endif // !defined(AFX_MESSAGEDESTINATIONS_H__01DC0AC2_E997_4E88_91D6_3CF9A8ACF4DC__INCLUDED_)
