/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/EMSString.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||
||                                         
||  COMMENTS:   
||              
\\*************************************************************************/

#pragma once

class CEMSString : public tstring
{
public:
	
	CEMSString();
	CEMSString( LPCTSTR szString );
	CEMSString( unsigned int nResourceID);

	virtual ~CEMSString() {};

	static int LoadResourceDLL();
	static int UnloadResourceDLL();
	void LoadString(unsigned int nStringID);
	void Format(unsigned int nStringID, ...);
	void Format(LPCTSTR szFormat, ...);
	void FormatArgList(unsigned int  nStringID, va_list& va);	
	void FormatArgList(LPCTSTR szFormat, va_list& va);
	void TrimWhiteSpace(void);
	void EscapeJavascript(bool bAllowNewLine = true);
	void EscapeCSV(void);
	void EscapeSQL(void);
	void EscapeHTML(void);
	void ConvertHTMLCharCodes(void);
	bool ValidateEmailAddr(void);
	bool ValidateHostName(void);
	void FormatBytes( int BytesLow, int BytesHigh );
	void EscapeBody( int nIsHTML, int nMsgID, bool bIsInbound, 
		             int nArchiveID = 0, bool bForEdit = false );
	void HyperLink( int& nPos );
	void EscapeHTMLAndNewLines( void );
	void FormatHTMLMsgForSend( int nOutboundMsgID );
	void FormatHTMLSigForSend( void );
	void FixInlineTags( bool bForSig=false );
	void SetVariables( LPCTSTR szEmail );
	
	void FixLineEnds( void );
	void RemoveScriptTags( void );
	void FixBodyTag( void );
	void FixCommentTag( void );
	bool IsStringValid( void );

	void PrefixBody( LPCTSTR szPrefix );
	void RemoveSubjectPrefixes( void );
	
	void Encrypt();
	void Decrypt();

	bool ToUTF8( void );
	bool ToAscii( void );

	void DoubleQuoteRealName( void );

	///////////////////////////////////////////////////////////////////////
	// These methods are for iterating through comma-delimited lists of IDs
	// Initialize the iteration - call this first
	void CDLInit( void );
	// The following 2 functions return false when the list is exhausted
	// Gets the next int in the list
	bool CDLGetNextInt( int& nInt );
	// Returns the next nChunkSize IDs in a string (w/o the comma at the end)
	bool CDLGetNextChunk( int nChunkSize, tstring& sChunk );	
	
	// this function returns the next string in a comma-delimited list of strings
	bool CDLGetNextString( tstring& sString );
	
	// this function populates a TIMESTAMP_STRUCT from a comma-delimited string in
	// the format of mm,dd,yyyy
	void CDLGetTimeStamp( TIMESTAMP_STRUCT& time );

private:
	static HMODULE m_hResourceDLL;

	size_type m_nCDLPos;		// Stores state for CDL functions
};
