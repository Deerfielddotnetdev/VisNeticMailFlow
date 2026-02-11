/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/AccessControl.h,v 1.1.4.1 2005/12/13 20:07:21 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

typedef struct AccessControlKey
{
	AccessControlKey(int nAgentID, int nObjectTypeID, int nActualID) :
		m_nAgentID(nAgentID), m_nObjectTypeID(nObjectTypeID), m_nActualID(nActualID)
	{
	};
		
	AccessControlKey(int nAgentID, int nObjectTypeID) :
	m_nAgentID(nAgentID), m_nObjectTypeID(nObjectTypeID), m_nActualID(0)
	{
	};

	int m_nObjectTypeID;
	int m_nAgentID;
	int m_nActualID;
	
} AccessControlKey;

inline bool operator<(const AccessControlKey& A,const AccessControlKey& B)
{
	if (A.m_nObjectTypeID < B.m_nObjectTypeID)
		return true;

	if ((A.m_nObjectTypeID == B.m_nObjectTypeID) && (A.m_nAgentID < B.m_nAgentID))
		return true;

	if ((A.m_nObjectTypeID == B.m_nObjectTypeID) && (A.m_nAgentID == B.m_nAgentID) && (A.m_nActualID < B.m_nActualID))
		return true;

	return false;
}

typedef map<AccessControlKey, unsigned char> AccessMap;


class CAccessControl  
{
public:
	
	// this is the only way to get an instance of this singleton class
	static CAccessControl& GetInstance();
	virtual ~CAccessControl() {};
	
	unsigned char GetAgentRightLevel( CODBCQuery& query, int nAgentID, int nObjectTypeID, int nActualID );

	int ListTicketCategories( CODBCQuery& query, CMarkupSTL& xlmgen, CXMLCache& XMLCache, 
		                 int nAgentID, unsigned char MinRightLevel, int nIncludID = 0 );
	
	int ListTicketBoxes( CODBCQuery& query, CMarkupSTL& xlmgen, CXMLCache& XMLCache, 
		                 int nAgentID, unsigned char MinRightLevel, int nIncludID = 0 );
	
	int ListAgents( CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
			        int nAgentID, unsigned char MinRightLevel, int nIncludeID = 0 );

	int ListTicketBoxesDefaultNames(CODBCQuery& query, CMarkupSTL& xmlgen, CXMLCache& XMLCache, 
									 int nAgentID, unsigned char MinRightLevel, int nIncludeID = 0);

	int ListSRCategories( CODBCQuery& query, CMarkupSTL& xlmgen, CXMLCache& XMLCache, 
		                 int nAgentID, unsigned char MinRightLevel, int nIncludID = 0 );	
	
	void Invalidate( void );

private:
	CAccessControl() {};

	unsigned char QueryRightLevel( CODBCQuery& query, AccessControlKey MapKey );
	void AddTicketBoxesToMap(CODBCQuery& query, int nAgentID);
	void AddTicketCategoriesToMap(CODBCQuery& query, int nAgentID);
	void AddAgentsToMap(CODBCQuery& query, int nAgentID);
	void AddSRCategoriesToMap(CODBCQuery& query, int nAgentID);
	
	AccessMap m_AccessMap;
};
