#pragma once

#include "XMLDataClass.h"

class CVoipServers : public CXMLDataClass, public TVoipServers
{
public:
	CVoipServers( CISAPIData& ISAPIData );
	virtual ~CVoipServers();

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