#pragma once

#include "XMLDataClass.h"

class CVoipDialingCodes : public CXMLDataClass, public TVoipDialingCodes
{
public:
	CVoipDialingCodes( CISAPIData& ISAPIData );
	virtual ~CVoipDialingCodes();

	virtual int Run( CURLAction& action );

protected:

	int ListAll(void);
	int Query(void);
	int Update();
	int Delete();
	int New();
	
	void DecodeForm(void);
	void GenerateXML(void);
	void CheckForDuplicity( void );
	void ClearDefault( void );
	void SetDefault( void );	
	
};