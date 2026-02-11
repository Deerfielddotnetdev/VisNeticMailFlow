#pragma once

#include "XMLDataClass.h"

class CVoipExtensions : public CXMLDataClass, public TVoipExtensions
{
public:
	CVoipExtensions( CISAPIData& ISAPIData );
	virtual ~CVoipExtensions();

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