/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/SecurityFns.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2002 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Security Helper Functions
||              
\\*************************************************************************/

#pragma once

#include <asptlb.h>	// For session object definition
#include "ODBCQuery.h"

bool IsAgentAdmin( CODBCQuery& query, int nAgentID );
