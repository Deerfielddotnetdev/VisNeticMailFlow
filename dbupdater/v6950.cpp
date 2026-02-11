#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6950(CODBCQuery& m_query, CDBConn& dbConn, reg::Key& rkMailFlow, TCHAR* wDsn)
{
    SQLRETURN retSQL = 0;

    auto LogSQL = [&](TCHAR* sql)
    {
        _tcout << _T("Running SQL:\n") << sql << endl;
        g_logFile.Write(_T("Running SQL: "));
        g_logFile.Write(sql);
    };

    auto ExecSQL = [&](TCHAR* sql)
    {
        LogSQL(sql);
        retSQL = dbConn.ExecuteSQL(sql);
        if (!SQL_SUCCEEDED(retSQL))
        {
            _tcout << _T("FAILED SQL:\n") << sql << endl;
            g_logFile.Write(_T("FAILED SQL: "));
            g_logFile.Write(sql);
            return FALSE;
        }
        else
        {
            g_logFile.Write(_T("SUCCESS"));
            return TRUE;
        }
    };

    //----------------------------------------------------------
    // 6.9.5 Update: Parameter Adjustment
    //----------------------------------------------------------

    if (g_showResults != 0)
        _tcout << _T("Applying database updates for 6.9.5...") << endl;

    // This update is safe and does not modify schema.
    if (!ExecSQL(_T("UPDATE ServerParameters SET DataValue = '1' WHERE ServerParameterID = 139")))
        return ErrorOpt(dbConn, rkMailFlow);

    //----------------------------------------------------------
    // Commented-out operations
    //----------------------------------------------------------

    /*
    // Full-text index removal
    // ExecSQL(_T("IF EXISTS (SELECT 1 FROM sys.fulltext_indexes fi "
    //            "JOIN sys.objects o ON fi.object_id = o.object_id "
    //            "WHERE o.name = 'OutboundMessages') "
    //            "DROP FULLTEXT INDEX ON OutboundMessages;"));

    // Schema modifications
    // ExecSQL(_T("ALTER TABLE AlertMsgs ALTER COLUMN Body VARCHAR(MAX) NULL"));
    // ExecSQL(_T("ALTER TABLE OutboundMessages ALTER COLUMN Body VARCHAR(MAX) NULL"));
    // ExecSQL(_T("ALTER TABLE OutboundMessages ALTER COLUMN EmailTo VARCHAR(MAX) NULL"));
    // ExecSQL(_T("ALTER TABLE OutboundMessages ALTER COLUMN EmailCc VARCHAR(MAX) NULL"));
    // ExecSQL(_T("ALTER TABLE OutboundMessages ALTER COLUMN EmailBcc VARCHAR(MAX) NULL"));
    // ExecSQL(_T("ALTER TABLE OutboundMessages ALTER COLUMN EmailReplyTo VARCHAR(MAX) NULL"));

    // Full-text index recreation
    // ExecSQL(_T("CREATE FULLTEXT INDEX ON OutboundMessages (Body LANGUAGE 1033) "
    //            "KEY INDEX PK_OutboundMessages;"));
    */

    //----------------------------------------------------------
    // Completion
    //----------------------------------------------------------

    if (g_showResults != 0)
        _tcout << _T("Database updates for 6.9.5 complete.") << endl;

    g_logFile.Write(_T("Database updates for 6.9.5 complete."));

    return 0;
}
