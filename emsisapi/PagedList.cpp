/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PagedList.cpp,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Base class for pages that implement paging
||              
\\*************************************************************************/


#include "stdafx.h"
#include "PagedList.h"

/*---------------------------------------------------------------------------\            
||  Comments:	Construction	              
\*--------------------------------------------------------------------------*/
CPagedList::CPagedList(CISAPIData& ISAPIData) : CXMLDataClass(ISAPIData)
{
	m_nCurrentPage = 0;
	m_nStartRow    = 0;
	m_nEndRow      = 0;
	m_nItemCount   = 0;
	m_nPageCount   = 0;

	GetISAPIData().GetURLLong( _T("CurrentPage"), m_nCurrentPage, true);

	SetCurrentPage( m_nCurrentPage );
}

void CPagedList::SetCurrentPage( int nCurrentPage )
{
	m_nCurrentPage = nCurrentPage;

	if (m_nCurrentPage < 1) 
		m_nCurrentPage = 1;

	// calculate the first and last rows of the rowset that we will care about
	m_nStartRow = m_nCurrentPage > 1 ? ((m_nCurrentPage - 1) * GetMaxRowsPerPage()) + 1 : 1;
	m_nEndRow = m_nCurrentPage * GetMaxRowsPerPage();
}

void CPagedList::SetItemCount( int nItemCount )
{
	m_nItemCount = nItemCount;

	// update the page count
	m_nPageCount = m_nItemCount / GetMaxRowsPerPage();
	
	if ((m_nPageCount == 0) || (m_nItemCount % GetMaxRowsPerPage())) 
		m_nPageCount++;

	// if the current page is greater then the page count
	// set the current page to the last page
	if ( m_nCurrentPage > m_nPageCount )
		SetCurrentPage( m_nPageCount );
}

void CPagedList::AddPageXML( void )
{
	GetXMLGen().AddChildElem( _T("Page") );
	GetXMLGen().AddChildAttrib( _T("Current"), GetCurrentPage() );
	GetXMLGen().AddChildAttrib( _T("Count"), GetPageCount() );
	GetXMLGen().AddChildAttrib( _T("OpenItems"), GetItemCount() );
}
