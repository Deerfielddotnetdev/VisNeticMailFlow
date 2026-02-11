/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/StdResponse.h,v 1.2 2005/11/29 21:16:27 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Standard Responses  
||              
\\*************************************************************************/

#if !defined(AFX_STDRESPONSE_H__006302EA_9038_4DC7_88D3_B19A4853795C__INCLUDED_)
#define AFX_STDRESPONSE_H__006302EA_9038_4DC7_88D3_B19A4853795C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLDataClass.h"
#include "Attachment.h"

class CStdResponseList
{
public:
	CStdResponseList() { ZeroMemory( this, sizeof(CStdResponseList)); }
	int m_StandardResponseID;
	long m_StandardResponseIDLen;
	
	int m_UsageCount;
	long m_UsageCountLen;
	
	TCHAR m_Subject[STANDARDRESPONSES_SUBJECT_LENGTH];
	long m_SubjectLen;
	
	TIMESTAMP_STRUCT m_DateModified;
	long m_DateModifiedLen;

};

class CStdResponse : public CXMLDataClass  
{
public:
	CStdResponse( CISAPIData& ISAPIData );
	virtual ~CStdResponse() {};
	
	int Run( CURLAction& action );
	void ListMyFavorites( void );
	void ListMostFrequentUsed( void );
	void ListDrafts( void );
	void ListSearchResults(void);
	void ListByTicketBox( int m_TbID );
	void ListByTicketCategory( int m_TcID );
	
	list<CAttachment>& GetAttachmentList( void ) { return m_AttachList; }

private:
	void DecodeForm( void );

	void GenerateXML( CURLAction& action );
	void GenListXML( int nAccess );

	void Query( CURLAction& action );
	void New( void );
	void Update( CURLAction &action );
	void Insert( CURLAction &action );
	void Delete( CURLAction &action );

	void ProcessAttachments( void );
	void RemoveAttachments( void );

	int GetFirstIDInCategory( void );

	void ListStdResponses( void );
	void UsedResponse( void );

	void AddFavorite( CURLAction &action );
	void RemoveFavorite( CURLAction &action );

	void RequireAccessLevel( unsigned char AccessLevel );

	TStandardResponses m_StandardResponse;
	TCHAR m_CategoryName[STDRESPONSECATEGORIES_CATEGORYNAME_LENGTH];
	long m_CategoryNameLen;
	list<CAttachment> m_AttachList;
	int m_nUsageCount;
	bool m_bDraftAccess;
	unsigned char m_access;
	unsigned char m_IsFavorite; 
	unsigned char m_UpdateStdList;
	unsigned char m_FavoriteMod;
	int m_lastCatID;
	
};

#endif // !defined(AFX_STDRESPONSE_H__006302EA_9038_4DC7_88D3_B19A4853795C__INCLUDED_)
