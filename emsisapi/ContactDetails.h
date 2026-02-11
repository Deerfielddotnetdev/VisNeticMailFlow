/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactDetails.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"
#include "ListClasses.h"

class CContactDetails : public CXMLDataClass  
{
public:
	CContactDetails(CISAPIData& ISAPIData);
	virtual ~CContactDetails() {};

	virtual int Run( CURLAction& action );

// functions
private:
	void GenNewContactXML( void );
	void GenContactXML( void );
	void DecodeForm( void );
	void Query( void );
	void Insert( void );
	void Update( void );
	int	 GetDefaultEmailAddress( void );
	
	void VerifyUniqueEmailAddr( int nContactID = 0);
	
	void Merge( void );

// data members
private:
	unsigned char m_AccessLevel;
	int m_nMaxPersonalDataID;
	int m_nMaxFormPersonalDataID;
	
	TContacts m_Contact;
	CPersonalDataList m_DefaultEmail;
	map<int, CPersonalDataList> m_PersonalDataMap;
};


