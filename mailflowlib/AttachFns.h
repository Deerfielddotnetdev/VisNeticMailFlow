/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/AttachFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||
||                                         
||  COMMENTS:   Attachment Functions
||              
\\*************************************************************************/

// only include once
#pragma once

void GetQuarantinePath( tstring& sPath );
void GetInboundAttachPath( tstring& sPath );
void GetOutboundAttachPath( tstring& sPath );
void GetNoteAttachPath( tstring& sPath );
void GetFullQuarantinePath( TCHAR* szFileName, tstring& sFullPath );
void GetFullInboundAttachPath( TCHAR* szFileName, tstring& sFullPath );
void GetFullOutboundAttachPath( TCHAR* szFileName, tstring& sFullPath );
void GetFullNoteAttachPath( TCHAR* szFileName, tstring& sFullPath );
bool VerifyFileExists( LPCTSTR szFileName );
int  GetAttachmentReferenceCount( CODBCQuery& query, int nAttachmentID );
void AttachInlineImages( CODBCQuery& query, TCHAR* szBody, 
						 int nRepliedToMsgID, bool bRepliedToIsInbound, 
						 int nNewMsgID, int nArchiveID = 0 );
void AttachSigImages( CODBCQuery& query, TCHAR* szBody, int nMsgID );
void GetPlugInPath( tstring& sPath );

