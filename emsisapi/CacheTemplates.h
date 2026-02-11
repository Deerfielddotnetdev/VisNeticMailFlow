/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/CacheTemplates.h,v 1.2 2005/11/29 21:16:25 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#include <boost\thread.hpp>

#pragma once

template <class T>
class CMapCache
{
public:

	///////////////////////////////////////////
	// constructor
	///////////////////////////////////////////
	CMapCache()
	{
		m_bListValid = false;
	}

	///////////////////////////////////////////
	// destructor
	///////////////////////////////////////////
	~CMapCache() { }

	//////////////////////////////////////////////////////////////
	// Build the map
	//////////////////////////////////////////////////////////////
	void BuildMap( CODBCQuery& query )
	{
		boost::unique_lock< boost::shared_mutex > lock(m_MapCacheLock);
		List( query );
	}

	///////////////////////////////////////////////////////////////////
	// Search for a record matching the ID and return true if found
	////////////////////////////////////////////////////////////////////
	bool Query( unsigned int ID, T& tobject )
	{
		map<unsigned int,T>::iterator iter;
		bool retval = false;

		boost::shared_lock< boost::shared_mutex > lock(m_MapCacheLock);
	
		// Search the list for the ID of interest
		iter = m_Map.find( ID );
		if( iter != m_Map.end() )
		{
			tobject = iter->second;
			retval = true;
		}
	
		// Return true if we got a hit
		return retval;
	}

	//////////////////////////////////////////////////////////////
	// Return the list in an XML string
	//////////////////////////////////////////////////////////////
	int GetXML( tstring& str ) 
	{ 
		tstring xml;

		boost::shared_lock< boost::shared_mutex > lock(m_MapCacheLock);
	
		str = m_XMLList;
	
		return GetListSize();
	}

	void GetMap( map<unsigned int, T>& map_cpy )
	{
		boost::shared_lock< boost::shared_mutex > lock(m_MapCacheLock);
		map_cpy = m_Map;
	}
	
	void Invalidate(void) { m_bListValid = false; }
	bool ListValid(void) { return m_bListValid; }
	int GetListSize(void) { return m_Map.size(); }	

protected:
	
	///////////////////////////////////////////
	// Initialize (or reset) and fill the list
	///////////////////////////////////////////
	void List( CODBCQuery& query )
	{
		CMarkupSTL xmlgen;
		T tobject;
		
		m_bListValid = false;
		
		// initialize
		m_Map.clear();
		
		// Load the map and generate XML
		tobject.LoadMap( query, m_Map, xmlgen );
		
		// Save the XML as a string
		m_XMLList = xmlgen.GetDoc();
		
		m_bListValid = true;
	}
	
	map<unsigned int,T>		m_Map;
	boost::shared_mutex		m_MapCacheLock;
	bool					m_bListValid;
	tstring					m_XMLList;
};



/*---------------------------------------------------------------------------\                     
||  Comments:	CIDMapCache template - This is used to maintain caches 
||				of data that is stored in a map using the ID, 
||				e.g.: signatures and email addressses	              
\*--------------------------------------------------------------------------*/
template <class T>
class CIDMapCache
{
public:

	///////////////////////////////////////////
	// constructor
	///////////////////////////////////////////
	CIDMapCache()
	{
		m_bListValid = false;
	}

	///////////////////////////////////////////
	// destructor
	///////////////////////////////////////////
	~CIDMapCache() {}
	
	//////////////////////////////////////////////////////////////
	// Build the map
	//////////////////////////////////////////////////////////////
	void BuildMap( CODBCQuery& query )
	{
		boost::unique_lock< boost::shared_mutex > lock(m_IdMapCacheLock);
		List( query );
	}
	
	///////////////////////////////////////////////////////////////////
	// Search for a record matching the ID and return true if found
	////////////////////////////////////////////////////////////////////
	bool Query( unsigned int ID, T& tobject )
	{
		multimap<unsigned int,T>::iterator iter;
		bool retval = false;
		
		boost::shared_lock< boost::shared_mutex > lock(m_IdMapCacheLock);
	
		retval = tobject.Query( m_Map, ID );
		
		// Return true if we got a hit
		return retval;
	}

	//////////////////////////////////////////////////////////////
	// Return the list in an XML string
	//////////////////////////////////////////////////////////////
	void GetXML( tstring& str, int ID ) 
	{ 
		tstring xml;

		boost::shared_lock< boost::shared_mutex > lock(m_IdMapCacheLock);

		CMarkupSTL xmlgen;
		T tobject;
		tobject.GetXMLByID( m_Map, ID, xmlgen );
		str = xmlgen.GetDoc();
	}
	
	void Invalidate(void) { m_bListValid = false; }
	bool ListValid(void) { return m_bListValid; }
	int GetListSize(void) { return m_Map.size(); }
	multimap<unsigned int,T>& GetMap(void) { return m_Map; }
	
protected:
	
	///////////////////////////////////////////
	// Initialize (or reset) and fill the list
	///////////////////////////////////////////
	void List( CODBCQuery& query )
	{
		T tobject;
		
		m_bListValid = false;
		
		// init
		m_Map.clear();
		
		// load the map
		tobject.LoadMap( query, m_Map );
		
		m_bListValid = true;
	}
	
	multimap<unsigned int,T>	m_Map;
	boost::shared_mutex			m_IdMapCacheLock;
	bool						m_bListValid;
};


/*---------------------------------------------------------------------------\                     
||  Comments:	Template for a class which contains a list of ID's which
||				are not sorted.  Used to hold AgentIDs and TicketBoxIDs in
||				the order which they were returned from the database.             
\*--------------------------------------------------------------------------*/
template <class T>
class COrderedListCache
{
public:

	COrderedListCache()
	{
		m_bListValid = false;
	}

	~COrderedListCache() {}

	// build the list
	void Build( CODBCQuery& query )
	{
		boost::unique_lock< boost::shared_mutex > lock(m_OrderedListMapCacheLock);
		List( query );
	}

	void Invalidate(void) { m_bListValid = false; }
	bool ListValid(void) { return m_bListValid; }
	int GetListSize(void) { return m_List.size(); }
	list<unsigned int>& GetList(void) { return m_List; }

protected:

	///////////////////////////////////////////
	// Initialize (or reset) and fill the list
	///////////////////////////////////////////
	void List( CODBCQuery& query )
	{
		T tobject;
		m_bListValid = false;
		
		// Initialize
		m_List.clear();
		
		// Load the map and generate XML
		tobject.LoadList( query, m_List );
		
		m_bListValid = true;
	}
	
	list<unsigned int>		m_List;
	boost::shared_mutex		m_OrderedListMapCacheLock;
	bool					m_bListValid;
};

