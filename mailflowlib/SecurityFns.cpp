/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/SecurityFns.cpp,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:	Security Functions  
||              
\\*************************************************************************/

#include "stdafx.h"
#include "SecurityFns.h"

#include "ErrorCodes.h"
#include "ODBCQuery.h"

/*---------------------------------------------------------------------------\                     
||  Comments:	Returns if the agentID is an admin
\*--------------------------------------------------------------------------*/
bool IsAgentAdmin( CODBCQuery& query, int nAgentID )
{
	// preform the query
	query.Initialize();

	BINDPARAM_LONG( query, nAgentID );

	query.Execute( _T("SELECT AgentGroupingID FROM AgentGroupings ")
				   _T("WHERE GroupID=2 AND AgentID=?") );
	
	// if one record exists, we're in the admin group
	if( query.Fetch() == S_OK )
		return true;

	return false;
}


