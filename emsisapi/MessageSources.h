// MessageSources.h: interface for the CMessageSources class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CMessageSources  : public CXMLDataClass, public TMessageSources
{
public:
	CMessageSources(CISAPIData& ISAPIData );
	virtual ~CMessageSources();

	virtual int Run(CURLAction& action);

protected:
	int ListAll(void);
	int Query(void);
	int Update(void);
	int Delete(void);
	int New(void);
	void DecodeForm(void);
	void GenerateXML(void);
	void EncryptPassword();
	void DecryptPassword();
	void AddServersXML();	
};
