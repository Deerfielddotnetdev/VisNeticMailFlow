/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/ContactSearch.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Contact Search   
||              
\\*************************************************************************/

#pragma once

#include "XMLDataClass.h"

class CContactSearch : public CXMLDataClass  
{
public:
	CContactSearch(CISAPIData& ISAPIData );
	virtual ~CContactSearch();
	
	int Run(CURLAction& action);

private:

	void DecodeForm( void );
	void PreformQuery( void );
	void FormatWhereClause( CEMSString& sWhereClause );
	void Delete( void );
	void Remove( void );
	void AddMyContact( void );
	void Reset( void );

private:

	int m_nMergeContactID;

	int m_nMaxLines;

	CEMSString m_sContactEmail;
	unsigned char m_ExactContactEmail;

	CEMSString m_sContactName;
	unsigned char m_ExactContactName;

	int m_nOwnerID;
	int m_nState;
	int m_nAgentID;
	int m_nSearchType;
	int m_nChkGroups;

	CEMSString m_sCompanyName;
	unsigned char m_ExactCompanyName;
	
	CEMSString m_sWebsite;
	unsigned char m_ExactWebsite;

	CEMSString m_sAddress;
	unsigned char m_ExactAddress;

	CEMSString m_sCity;
	unsigned char m_ExactCity;

	CEMSString m_sState;
	unsigned char m_ExactState;
	
	CEMSString m_sCountry;
	unsigned char m_ExactCountry;
	
	CEMSString m_sZipCode;
	unsigned char m_ExactZipCode;

	int m_nPersonalDataType;
	CEMSString m_sPersonalData;
	unsigned char m_ExactPersonalData;
};
