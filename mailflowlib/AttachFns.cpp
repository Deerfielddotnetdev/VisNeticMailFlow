/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/AttachFns.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||
||                                         
||  COMMENTS:	Attachment Functions
||              
\\*************************************************************************/

#include "stdafx.h"
#include "AttachFns.h"
#include "RegistryFns.h"
#include "ArchiveFns.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a full path given a Inbound Attachment 
||				location name.	              
\*--------------------------------------------------------------------------*/
void GetFullQuarantinePath( TCHAR* szFileName, tstring& sFullPath )
{
	// get the inbound attachment path
	GetQuarantinePath( sFullPath );
	
	// build the full path
	sFullPath.append( szFileName );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a full path given an Inbound Attachment 
||				location name.	              
\*--------------------------------------------------------------------------*/
void GetFullInboundAttachPath( TCHAR* szFileName, tstring& sFullPath )
{
	// get the inbound attachment path
	GetInboundAttachPath( sFullPath );
	
	// build the full path
	sFullPath.append( szFileName );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a full path given an Outbound Attachment 
||				location name.	              
\*--------------------------------------------------------------------------*/
void GetFullOutboundAttachPath( TCHAR* szFileName, tstring& sFullPath )
{
	// get the inbound attachment path
	GetOutboundAttachPath( sFullPath );

	// build the full path
	sFullPath.append(szFileName);
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns a full path given a Note Attachment 
||				location name.	              
\*--------------------------------------------------------------------------*/
void GetFullNoteAttachPath( TCHAR* szFileName, tstring& sFullPath )
{
	// get the note attachment path
	GetNoteAttachPath( sFullPath );
	
	// build the full path
	sFullPath.append( szFileName );
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the Quarantine Path	              
\*--------------------------------------------------------------------------*/
void GetQuarantinePath( tstring& sPath )
{
	if (GetRegString( EMS_ROOT_KEY, REG_KEY_AV, _T("QuarantineFolder"), sPath ) != ERROR_SUCCESS)
	{
		THROW_EMS_EXCEPTION( E_AttachPathNotInReg, _T("Error reading Quarantine folder from registry") );
	}
	
	// make sure it ends in a back-slash
	if (sPath[sPath.length() - 1] != _T('\\'))
		sPath.append(_T("\\"));
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the Outbound Attachment Path	              
\*--------------------------------------------------------------------------*/
void GetOutboundAttachPath( tstring& sPath )
{
	if (GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("OutboundAttachFolder"), sPath ) != ERROR_SUCCESS)
	{
		THROW_EMS_EXCEPTION( E_AttachPathNotInReg, _T("Error reading Outbound Attachment folder from registry") );
	}
	
	// make sure it ends in a back-slash
	if (sPath[sPath.length() - 1] != _T('\\'))
		sPath.append(_T("\\"));
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the Inbound Attachment Path	              
\*--------------------------------------------------------------------------*/
void GetInboundAttachPath( tstring& sPath )
{
	if (GetRegString( EMS_ROOT_KEY, EMS_REG_LOC_MAIL_BASE, _T("InboundAttachFolder"), sPath ) != ERROR_SUCCESS)
	{
		THROW_EMS_EXCEPTION( E_AttachPathNotInReg, _T("Error reading Inbound Attachment folder from registry") );
	}
	
	// make sure it ends in a back-slash
	if (sPath[sPath.length() - 1] != _T('\\'))
		sPath.append(_T("\\"));
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the Note Attachment Path	              
\*--------------------------------------------------------------------------*/
void GetNoteAttachPath( tstring& sPath )
{
	if (GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("NoteAttachFolder"), sPath ) != ERROR_SUCCESS)
	{
		THROW_EMS_EXCEPTION( E_AttachPathNotInReg, _T("Error reading Note Attachment folder from registry") );
	}
	
	// make sure it ends in a back-slash
	if (sPath[sPath.length() - 1] != _T('\\'))
		sPath.append(_T("\\"));
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Given a path returns if the file exsists	              
\*--------------------------------------------------------------------------*/
bool VerifyFileExists(LPCTSTR szFileName)
{

	HANDLE hFile = INVALID_HANDLE_VALUE;
	
	hFile = CreateFile(	szFileName, 
		GENERIC_READ,			
		(FILE_SHARE_READ | FILE_SHARE_WRITE),	
		NULL,					
		OPEN_EXISTING,			
		FILE_ATTRIBUTE_NORMAL,	
		NULL);
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	
	CloseHandle( hFile );
	return true;
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the number of times an attachment is referenced.	              
\*--------------------------------------------------------------------------*/
int GetAttachmentReferenceCount( CODBCQuery& query, int nAttachmentID )
{
	int nRefCount = 0;
	
	query.Reset();
	BINDCOL_LONG_NOLEN( query, nRefCount );
	BINDPARAM_LONG( query, nAttachmentID );
	BINDPARAM_LONG( query, nAttachmentID );
	BINDPARAM_LONG( query, nAttachmentID );
	BINDPARAM_LONG( query, nAttachmentID );
	
	query.Execute( _T("SELECT((SELECT COUNT(*) FROM InboundMessageAttachments WHERE AttachmentID=?) + ")
		           _T("(SELECT COUNT(*) FROM OutboundMessageAttachments WHERE AttachmentID=?) + ")
				   _T("(SELECT COUNT(*) FROM NoteAttachments WHERE AttachmentID=?) + ")
		           _T("(SELECT COUNT(*) FROM StdResponseAttachments WHERE AttachmentID=?))") );
	
	query.Fetch();
	
	return nRefCount;
}


////////////////////////////////////////////////////////////////////////////////
// 
// AttachInlineImages - check the body for <a href="cid:"> references and
// add the referenced attachments to the new outbound message.
// 
////////////////////////////////////////////////////////////////////////////////
void AttachInlineImages( CODBCQuery& query, TCHAR* szBody, 
						 int nRepliedToMsgID, bool bRepliedToIsInbound,
						 int nNewMsgID, int nArchiveID )
{
	TCHAR* ptr = szBody;
	tstring sContentID;
	int nAttachmentID;
	bool bQuoteFound;
	bool bExists;
	vector<tstring> m_cid;
	vector<tstring>::iterator iter;
	m_cid.clear();

	// search body for content IDs
	try
	{
		while ( *ptr )
		{
			if( *ptr == _T('s') || *ptr == _T('S') )
			{
				if( _tcsnicmp( ptr, _T("src="), 4 ) == 0 )
				{
					ptr += 4;	// skip to start of content ID
					
					// skip the optional quote character
					bQuoteFound= ( *ptr == _T('\"') );
					if( bQuoteFound )
					{
						ptr++;
					}

					if( _tcsnicmp( ptr, _T("cid:"), 4 ) == 0 )
					{
						ptr += 4;

						sContentID.assign( _T("") );

						while( *ptr && ( bQuoteFound ? (*ptr != _T('\"')) : (*ptr != _T(' ')) ) )
						{
							sContentID.append( 1, *ptr );
							ptr++;
						}
						
						if( nArchiveID )
						{
							bool bFoundIt = false;

							// See if we've already restored the attachment for this ContentID
							query.Reset();
							BINDPARAM_LONG( query, nNewMsgID );
							BINDPARAM_TCHAR_STRING( query, sContentID );
							BINDCOL_LONG_NOLEN( query, nAttachmentID );

							query.Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
										_T("INNER JOIN OutboundMessageAttachments ")
										_T("ON Attachments.AttachmentID = OutboundMessageAttachments.AttachmentID ")
										_T("WHERE OutboundMessageID=? AND ContentID=? ") );


							if( query.Fetch() == S_OK )
							{
								bFoundIt = true;
							}
							else
							{
								// Restore the attachment for the contentID in the archive
								if( Arc_Success == 
									RestoreAttachmentFromArchive( query, nArchiveID, 1, 
								 								nAttachmentID, sContentID ) )
								{
									bFoundIt = true;
								}
							}

							if( bFoundIt )
							{
								bExists = false;
								for(iter = m_cid.begin(); iter != m_cid.end(); ++iter)
								{
									if (_tcscmp(sContentID.c_str(), iter->c_str()) == 0 )
									{
										bExists = true;	
									}
								}
								if(!bExists)
								{
									query.Reset();
									BINDPARAM_LONG( query, nAttachmentID );
									BINDPARAM_LONG( query, nNewMsgID );

									// Make a link
									query.Execute( _T("INSERT INTO OutboundMessageAttachments ")
												_T("(AttachmentID,OutboundMessageID) ")
												_T("VALUES ")
												_T("(?,?)") );

									m_cid.push_back(sContentID);
								}
							}
						}
						else
						{
							// look for the ContentID in the database
							query.Reset();
							BINDPARAM_LONG( query, nRepliedToMsgID );
							BINDPARAM_TCHAR_STRING( query, sContentID );
							BINDCOL_LONG_NOLEN( query, nAttachmentID );

							if( bRepliedToIsInbound )
							{
								query.Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
											_T("INNER JOIN InboundMessageAttachments ")
											_T("ON Attachments.AttachmentID = InboundMessageAttachments.AttachmentID ")
											_T("WHERE InboundMessageID=? AND ContentID=? ") );
							}
							else
							{
								query.Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
											_T("INNER JOIN OutboundMessageAttachments ")
											_T("ON Attachments.AttachmentID = OutboundMessageAttachments.AttachmentID ")
											_T("WHERE OutboundMessageID=? AND ContentID=? ") );
							}

							if( query.Fetch() == S_OK )
							{				
								bExists = false;
								for(iter = m_cid.begin(); iter != m_cid.end(); ++iter)
								{
									if (_tcscmp(sContentID.c_str(), iter->c_str()) == 0 )
									{
										bExists = true;	
									}
								}
								if(!bExists)
								{
									// Found it!
									query.Reset();
									BINDPARAM_LONG( query, nAttachmentID );
									BINDPARAM_LONG( query, nNewMsgID );

									// Make a link
									query.Execute( _T("INSERT INTO OutboundMessageAttachments ")
												_T("(AttachmentID,OutboundMessageID) ")
												_T("VALUES ")
												_T("(?,?)") );

									m_cid.push_back(sContentID);
								}
							}
						}										
					}
				}
			}

			ptr++;
		}
	}
	catch (...) 
	{
		//Eat the error and allow the outbound message to proceed without the inline attachments
	}
}

////////////////////////////////////////////////////////////////////////////////
// 
// AttachSigImages - check the body for <a href="cid:"> references and
// add the referenced attachments to the new outbound message.
// 
////////////////////////////////////////////////////////////////////////////////
void AttachSigImages( CODBCQuery& query, TCHAR* szBody, int nMsgID  )
{
	TCHAR* ptr = szBody;
	tstring sContentID;
	int nAttachmentID;
	bool bQuoteFound;

	// search body for content IDs
	while ( *ptr )
	{
		if( *ptr == _T('s') || *ptr == _T('S') )
		{
			if( _tcsnicmp( ptr, _T("src="), 4 ) == 0 )
			{
				ptr += 4;	// skip to start of content ID
				
				// skip the optional quote character
				bQuoteFound= ( *ptr == _T('\"') );
				if( bQuoteFound )
				{
					ptr++;
				}

				if( _tcsnicmp( ptr, _T("cid:"), 4 ) == 0 )
				{
					ptr += 4;

					sContentID.assign( _T("") );

					while( *ptr && ( bQuoteFound ? (*ptr != _T('\"')) : (*ptr != _T(' ')) ) )
					{
						sContentID.append( 1, *ptr );
						ptr++;
					}
					
					if(sContentID.length() > 0)
					{
						// look for the ContentID in the database
						query.Reset();
						BINDPARAM_LONG( query, nMsgID );
						BINDPARAM_TCHAR_STRING( query, sContentID );
						BINDCOL_LONG_NOLEN( query, nAttachmentID );
						
						query.Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
										_T("INNER JOIN OutboundMessageAttachments ")
										_T("ON Attachments.AttachmentID = OutboundMessageAttachments.AttachmentID ")
										_T("WHERE OutboundMessageID=? AND ContentID=? ") );
						

						if( query.Fetch() != S_OK )
						{				
							// Did not find it!
							query.Reset();
							BINDPARAM_TCHAR_STRING( query, sContentID );
							BINDCOL_LONG_NOLEN( query, nAttachmentID );
							
							query.Execute( _T("SELECT TOP 1 Attachments.AttachmentID FROM Attachments ")
										_T("WHERE ContentID=? ") );
							
							
							if( query.Fetch() == S_OK )
							{							
								query.Reset();
								BINDPARAM_LONG( query, nAttachmentID );
								BINDPARAM_LONG( query, nMsgID );

								// Make a link
								query.Execute( _T("INSERT INTO OutboundMessageAttachments ")
												_T("(AttachmentID,OutboundMessageID) ")
												_T("VALUES ")
												_T("(?,?)") );

							}
						}
					}					
				}
			}
		}

		ptr++;
	}
}

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns the PlugIn Path	              
\*--------------------------------------------------------------------------*/
void GetPlugInPath( tstring& sPath )
{
	if (GetRegString( EMS_ROOT_KEY, EMS_LOCATION_IN_REGISTRY, _T("InstallPath"), sPath ) != ERROR_SUCCESS)
	{
		THROW_EMS_EXCEPTION( E_AttachPathNotInReg, _T("Error reading InstallPath folder from registry") );
	}
	
	// make sure it ends in a back-slash
	if (sPath[sPath.length() - 1] != _T('\\'))
		sPath.append(_T("\\"));

	// append the wwwroot plug-in path
	sPath.append(_T("wwwroot\\plug-ins\\"));
}

