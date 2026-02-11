// Signatures.h: interface for the CSignatures class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "XMLDataClass.h"

class CSignatures : public CXMLDataClass, public TSignatures
{
public:
	CSignatures( CISAPIData& ISAPIData );
	virtual ~CSignatures();

	virtual int Run(CURLAction& action);

	int ListAll( CURLAction& action );
	int QueryOne( CURLAction& action );
	int DoUpdate( CURLAction& action );
	int DoInsert( CURLAction& action );
	int DoDelete( CURLAction& action );
	void DecodeForm( void );
	void GenerateXMLList( void);
	void GenerateXML( void );
	void CheckForDuplicates(void);

	int m_refid;	
};
