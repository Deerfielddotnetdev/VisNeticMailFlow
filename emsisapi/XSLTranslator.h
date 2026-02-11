// XSLTranslator.h: interface for the CXSLTranslator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XSLTRANSLATOR_H__62965B6E_67B3_4E53_B299_D19041593087__INCLUDED_)
#define AFX_XSLTRANSLATOR_H__62965B6E_67B3_4E53_B299_D19041593087__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXSLTranslator  
{
public:
	CXSLTranslator();
	virtual ~CXSLTranslator();

	int CreateComponents(void);
	void ReleaseComponents(void);

	int TranslateXML( tstring& xml, tstring& xslfile, tstring& html );

protected:
	long LoadXSLFromFile( tstring& xslfile );
	long LoadXMLFromString( tstring& xml );
	long Transform( tstring& html );
	void ThrowXMLError( IXMLDOMDocument* pXMLDoc, TCHAR* szOperation );

	IXMLDOMDocument* m_pXML;
    IXMLDOMDocument* m_pXSL;	
public:
	void SetError(IXMLDOMDocument* pXMLDoc);
};

#endif // !defined(AFX_XSLTRANSLATOR_H__62965B6E_67B3_4E53_B299_D19041593087__INCLUDED_)
