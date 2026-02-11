// FormDump.cpp: implementation of the CFormDump class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormDump.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormDump::CFormDump( CISAPIData& ISAPIData ) : CXMLDataClass( ISAPIData )
{

}

CFormDump::~CFormDump()
{

}


////////////////////////////////////////////////////////////////////////////////
// 
// Run - download a file specified by action.m_String
// 
////////////////////////////////////////////////////////////////////////////////
int CFormDump::Run( CURLAction& action )
{
	TCHAR* buffer;
	map<tstring,ISAPIPARAM>::iterator iter;		// URL parameters map (a.k.a Query String)
	
	for( iter = m_ISAPIData.m_Form.begin(); iter != m_ISAPIData.m_Form.end(); iter++ )
	{
		buffer = new TCHAR[iter->second.nLength+1];
		ZeroMemory( buffer, (iter->second.nLength+1)*sizeof(TCHAR));
		m_ISAPIData.DecodeValue( iter->second, buffer, iter->second.nLength+1 );

		GetXMLGen().AddChildElem( _T("FORM") );
		GetXMLGen().AddChildAttrib( _T("Parameter"), iter->first.c_str() );
		if( iter->second.m_bIsFile )
		{
			GetXMLGen().AddChildAttrib( _T("Filename"), iter->second.m_Filename.c_str() );
		}
		GetXMLGen().SetChildData( buffer, 1 );
		delete[] buffer;
	}
	
	for( iter = m_ISAPIData.m_URL.begin(); iter != m_ISAPIData.m_URL.end(); iter++ )
	{
		buffer = new TCHAR[iter->second.nLength+1];
		ZeroMemory( buffer, (iter->second.nLength+1)*sizeof(TCHAR));
		m_ISAPIData.DecodeValue( iter->second, buffer, iter->second.nLength+1, true );
		GetXMLGen().AddChildElem( _T("URL"), buffer );
		GetXMLGen().AddChildAttrib( _T("Parameter"), iter->first.c_str() );
		delete[] buffer;
	}
	
	return 0;
}
	