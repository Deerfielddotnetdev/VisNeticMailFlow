// EmailText.h: interface for the CEmailText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAILTEXT_H__2258F79C_6341_4EF7_8A5B_916AAEA8099D__INCLUDED_)
#define AFX_EMAILTEXT_H__2258F79C_6341_4EF7_8A5B_916AAEA8099D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEmailText  
{
public:
	CEmailText();
	virtual ~CEmailText();

	CEmailText(const string& text, const string& charset);

	// copy constructor
	CEmailText(const CEmailText&);

	// assignment
	CEmailText& operator = (const CEmailText&);

	const string& GetText() const				{ return m_sText; }
	const string& GetCharset() const			{ return m_sCharset; }
	const string& GetMediaType() const			{ return m_sMediaType; }
	const string& GetMediaSubType() const		{ return m_sMediaSubType; }

	void SetText(const string& sText, const string& charset)	
										{ 
											m_sText = sText; 
											m_sCharset = charset;
										}
	void SetCharset(const string& sVal)	{ m_sCharset = sVal; }
	void SetMediaType(const string& sVal) { m_sMediaType = sVal; }
	void SetMediaSubType(const string& sVal) { m_sMediaSubType = sVal; }

private:
	string m_sText;
	string m_sCharset;
	string m_sMediaType;
	string m_sMediaSubType;

};

#endif // !defined(AFX_EMAILTEXT_H__2258F79C_6341_4EF7_8A5B_916AAEA8099D__INCLUDED_)
