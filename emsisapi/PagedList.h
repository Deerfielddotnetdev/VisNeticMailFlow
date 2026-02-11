/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/PagedList.h,v 1.1 2005/04/18 18:48:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Base class for pages that implement paging
||              
\\*************************************************************************/

#pragma once

class CPagedList : public CXMLDataClass    
{
public:
	CPagedList(CISAPIData& ISAPIData);
	virtual ~CPagedList() {};
	
	void AddPageXML(void);

	void SetCurrentPage(int nCurrentPage);
	int  GetCurrentPage(void) { return m_nCurrentPage; }
	
	void SetItemCount( int nItemCount );
	int  GetItemCount( void ) { return m_nItemCount; }
	
	void SetPageCount( int nPageCount ) { m_nPageCount = nPageCount; }
	int  GetPageCount(void)	{ return m_nPageCount; }
	
	int	 GetStartRow(void)	{ return m_nStartRow; }
	int	 GetEndRow(void)	{ return m_nEndRow; }

private:
	int	m_nCurrentPage;
	int m_nPageCount;
	int m_nItemCount;
	int m_nStartRow;
	int m_nEndRow;
};
