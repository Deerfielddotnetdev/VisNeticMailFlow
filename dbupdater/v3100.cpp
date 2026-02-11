#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int	  maxObjectID;

int V3100(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;

	//Add New Tables
	/*
	AgeAlerts
	AgeAlertsSent
	AutoMessages
	AutoMessagesSent
	AutoResponses
	AutoResponsesSent
	TicketBoxOwners
	TicketBoxTicketBoxOwner
	TicketBoxRouting
	TicketCategories
	WaterMarkAlerts
	
	ProcessingRules
	MatchTextP
	MatchFromAddressP
	MatchToAddressP
    
	ReportCustom
	ReportObjects
	ReportTypes
	ReportStandard
	ReportScheduled
	ReportResultRows
	ReportResults
	*/

	//Add AgeAlerts
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgeAlerts table") << endl;
	}

	g_logFile.Write(_T("Create AgeAlerts table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AgeAlerts ( ")
	_T("[AgeAlertID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[Description] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[HitCount] [int] NOT NULL , ")
	_T("[AgeAlerted] [bit] NOT NULL , ")
	_T("[ThresholdMins] [int] NOT NULL , ")
	_T("[ThresholdFreq] [int] NOT NULL , ")
	_T("[IsEnabled] [bit] NOT NULL , ")
	_T("[NoAlertIfRepliedTo] [bit] NOT NULL , ")
	_T("[AlertOnTypeID] [int] NOT NULL , ")
	_T("[AlertOnID] [int] NOT NULL , ")
	_T("[AlertToTypeID] [int] NOT NULL , ")
	_T("[AlertToID] [int] NOT NULL , ")
	_T("[AlertMethodID] [int] NOT NULL , ")
	_T("[EmailAddress] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , ")
	_T("[FromEmailAddress] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , ")
	_T("[DateCreated] [datetime] NOT NULL , ")
	_T("[DateEdited] [datetime] NOT NULL , ")
	_T("[CreatedByID] [int] NOT NULL , ")
	_T("[EditedByID] [int] NOT NULL)"));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AgeAlerts ADD ") 
	_T("CONSTRAINT [DF_AgeAlerts_Description] DEFAULT ('') FOR [Description], ")
	_T("CONSTRAINT [DF_AgeAlerts_HitCount] DEFAULT (0) FOR [HitCount], ")
	_T("CONSTRAINT [DF_AgeAlerts_AgeAlerted] DEFAULT (0) FOR [AgeAlerted], ")
	_T("CONSTRAINT [DF_AgeAlerts_ThresholdMins] DEFAULT (0) FOR [ThresholdMins], ")
	_T("CONSTRAINT [DF_AgeAlerts_ThresholdFreq] DEFAULT (0) FOR [ThresholdFreq], ")
	_T("CONSTRAINT [DF_AgeAlerts_IsEnabled] DEFAULT (1) FOR [IsEnabled], ")
	_T("CONSTRAINT [DF_AgeAlerts_NoAlertIfReplied] DEFAULT (1) FOR [NoAlertIfRepliedTo], ")
	_T("CONSTRAINT [DF_AgeAlerts_AlertOnTypeID] DEFAULT (0) FOR [AlertOnTypeID], ")
	_T("CONSTRAINT [DF_AgeAlerts_AlertOnID] DEFAULT (0) FOR [AlertOnID], ")
	_T("CONSTRAINT [DF_AgeAlerts_AlertToTypeID] DEFAULT (0) FOR [AlertToTypeID], ")
	_T("CONSTRAINT [DF_AgeAlerts_AlertToID] DEFAULT (0) FOR [AlertToID], ")
	_T("CONSTRAINT [DF_AgeAlerts_AlertMethodID] DEFAULT (0) FOR [AlertMethodID], ")
	_T("CONSTRAINT [DF_AgeAlerts_DateCreated] DEFAULT (getdate()) FOR [DateCreated], ")
	_T("CONSTRAINT [DF_AgeAlerts_DateEdited] DEFAULT (getdate()) FOR [DateEdited], ")
	_T("CONSTRAINT [DF_AgeAlerts_CreatedByID] DEFAULT (0) FOR [CreatedByID], ")
	_T("CONSTRAINT [DF_AgeAlerts_EditedByID] DEFAULT (0) FOR [EditedByID], ")
	_T("CONSTRAINT [PK_AgeAlerts] PRIMARY KEY  CLUSTERED ( [AgeAlertID]	)"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add AgeAlertsSent
	if(g_showResults != 0)
	{
		_tcout << _T("Create AgeAlertsSent table") << endl;
	}

	g_logFile.Write(_T("Create AgeAlertsSent table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AgeAlertsSent (	[AgeAlertSentID] [int] IDENTITY (1, 1) NOT NULL ,	[AgeAlertID] [int] NOT NULL ,	[TicketID] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AgeAlertsSent ADD CONSTRAINT [PK_AgeAlertsSent] PRIMARY KEY  CLUSTERED (	[AgeAlertSentID] ) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add AutoMessages
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoMessages table") << endl;
	}

	g_logFile.Write(_T("Create AutoMessages table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AutoMessages ( ")
	_T("[AutoMessageID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[Description] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[HitCount] [int] NOT NULL , ")
	_T("[FailCount] [int] NOT NULL , ")
	_T("[TotalCount] [int] NOT NULL , ")
	_T("[SendToTypeID] [int] NOT NULL , ")
	_T("[SendToID] [int] NOT NULL , ")
	_T("[IsEnabled] [bit] NOT NULL , ")
	_T("[WhenToSendVal] [int] NOT NULL , ")
	_T("[WhenToSendFreq] [int] NOT NULL , ")
	_T("[WhenToSendTypeID] [int] NOT NULL , ")
	_T("[SrToSendID] [int] NOT NULL , ")
	_T("[SendFromTypeID] [int] NOT NULL , ")
	_T("[CreateNewTicket] [bit] NOT NULL , ")
	_T("[ReplyToAddress] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[ReplyToName] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[OwnerID] [int] NOT NULL , ")
	_T("[TicketBoxID] [int] NOT NULL , ")
	_T("[TicketCategoryID] [int] NOT NULL , ")
	_T("[DateCreated] [datetime] NOT NULL , ")
	_T("[DateEdited] [datetime] NOT NULL , ")
	_T("[CreatedByID] [int] NOT NULL , ")
	_T("[EditedByID] [int] NOT NULL , ")
	_T("[SendToPercent] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoMessages ADD ")
	_T("CONSTRAINT [DF_AutoMessages_Description] DEFAULT ('') FOR [Description], ")
	_T("CONSTRAINT [DF_AutoMessages_HitCount] DEFAULT (0) FOR [HitCount], ")
	_T("CONSTRAINT [DF_AutoMessages_FailCount] DEFAULT (0) FOR [FailCount], ")
	_T("CONSTRAINT [DF_AutoMessages_TotalCount] DEFAULT (0) FOR [TotalCount], ")
	_T("CONSTRAINT [DF_AutoMessages_SendToTypeID] DEFAULT (0) FOR [SendToTypeID], ")
	_T("CONSTRAINT [DF_AutoMessages_SendToID] DEFAULT (0) FOR [SendToID], ")
	_T("CONSTRAINT [DF_AutoMessages_IsEnabled] DEFAULT (1) FOR [IsEnabled], ")
	_T("CONSTRAINT [DF_AutoMessages_WhenToSendVal] DEFAULT (0) FOR [WhenToSendVal], ")
	_T("CONSTRAINT [DF_AutoMessages_WhenToSendFreq] DEFAULT (0) FOR [WhenToSendFreq], ")
	_T("CONSTRAINT [DF_AutoMessages_WhenToSendTypeID] DEFAULT (0) FOR [WhenToSendTypeID], ")
	_T("CONSTRAINT [DF_AutoMessages_SrToSendID] DEFAULT (0) FOR [SrToSendID], ")
	_T("CONSTRAINT [DF_AutoMessages_SendFromTypeID] DEFAULT (0) FOR [SendFromTypeID], ")
	_T("CONSTRAINT [DF_AutoMessages_AssociateWithTicket] DEFAULT (0) FOR [CreateNewTicket], ")
	_T("CONSTRAINT [DF_AutoMessages_ReplyToAddress] DEFAULT ('') FOR [ReplyToAddress], ")
	_T("CONSTRAINT [DF_AutoMessages_ReplyToName] DEFAULT ('') FOR [ReplyToName], ")
	_T("CONSTRAINT [DF_AutoMessages_OwnerID] DEFAULT (0) FOR [OwnerID], ")
	_T("CONSTRAINT [DF_AutoMessages_TicketBoxID] DEFAULT (0) FOR [TicketBoxID], ")
	_T("CONSTRAINT [DF_AutoMessages_TicketCategoryID] DEFAULT (1) FOR [TicketCategoryID], ")
	_T("CONSTRAINT [DF_AutoMessages_DateCreated] DEFAULT (getdate()) FOR [DateCreated], ")
	_T("CONSTRAINT [DF_AutoMessages_DateEdited] DEFAULT (getdate()) FOR [DateEdited], ")
	_T("CONSTRAINT [DF_AutoMessages_CreatedByID] DEFAULT (0) FOR [CreatedByID], ")
	_T("CONSTRAINT [DF_AutoMessages_EditedByID] DEFAULT (0) FOR [EditedByID], ")
	_T("CONSTRAINT [DF_AutoMessages_SendToPercent] DEFAULT (100) FOR [SendToPercent], ")
	_T("CONSTRAINT [PK_AutoMessages] PRIMARY KEY  CLUSTERED ( [AutoMessageID] ) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add AutoMessagesSent
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoMessagesSent table") << endl;
	}

	g_logFile.Write(_T("Create AutoMessagesSent table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AutoMessagesSent (	[AutoMessageSentID] [int] IDENTITY (1, 1) NOT NULL ,[AutoMessageID] [int] NOT NULL ,[TicketID] [int] NOT NULL ,[ResultCode] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoMessagesSent ADD CONSTRAINT [DF_AutoMessagesSent_ResultCode] DEFAULT (0) FOR [ResultCode],CONSTRAINT [PK_AutoMessagesSent] PRIMARY KEY CLUSTERED ([AutoMessageSentID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add AutoResponses
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoResponses table") << endl;
	}

	g_logFile.Write(_T("Create AutoResponses table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AutoResponses ( ")
	_T("[AutoResponseID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[Description] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[HitCount] [int] NOT NULL , ")
	_T("[FailCount] [int] NOT NULL , ")
	_T("[TotalCount] [int] NOT NULL , ")
	_T("[SendToTypeID] [int] NOT NULL , ")
	_T("[SendToID] [int] NOT NULL , ")
	_T("[IsEnabled] [bit] NOT NULL , ")
	_T("[SendFromTypeID] [int] NOT NULL , ")
	_T("[ReplyToAddress] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[ReplyToName] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[WhenToSendVal] [int] NOT NULL , ")
	_T("[WhenToSendFreq] [int] NOT NULL , ")
	_T("[WhenToSendTypeID] [int] NOT NULL , ")
	_T("[SrToSendID] [int] NOT NULL , ")
	_T("[AutoReplyQuoteMsg] [bit] NOT NULL , ")
	_T("[AutoReplyCloseTicket] [bit] NOT NULL , ")
	_T("[HeaderID] [int] NOT NULL , ")
	_T("[FooterID] [int] NOT NULL , ")
	_T("[DateCreated] [datetime] NOT NULL , ")
	_T("[DateEdited] [datetime] NOT NULL , ")
	_T("[CreatedByID] [int] NOT NULL , ")
	_T("[EditedByID] [int] NOT NULL , ")
	_T("[SendToPercent] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoResponses ADD ") 
	_T("CONSTRAINT [DF_AutoResponses_Description] DEFAULT ('') FOR [Description], ")
	_T("CONSTRAINT [DF_AutoResponses_HitCount] DEFAULT (0) FOR [HitCount], ")
	_T("CONSTRAINT [DF_AutoResponses_FailCount] DEFAULT (0) FOR [FailCount], ")
	_T("CONSTRAINT [DF_AutoResponses_TotalCount] DEFAULT (0) FOR [TotalCount], ")
	_T("CONSTRAINT [DF_AutoResponses_SendToTypeID] DEFAULT (0) FOR [SendToTypeID], ")
	_T("CONSTRAINT [DF_AutoResponses_SendToID] DEFAULT (0) FOR [SendToID], ")
	_T("CONSTRAINT [DF_AutoResponses_IsEnabled] DEFAULT (1) FOR [IsEnabled], ")
	_T("CONSTRAINT [DF_AutoResponses_SendFromTypeID] DEFAULT (0) FOR [SendFromTypeID], ")
	_T("CONSTRAINT [DF_AutoResponses_ReplyToAddress] DEFAULT ('') FOR [ReplyToAddress], ")
	_T("CONSTRAINT [DF_AutoResponses_ReplyToName] DEFAULT ('') FOR [ReplyToName], ")
	_T("CONSTRAINT [DF_AutoResponses_WhenToSendVal] DEFAULT (0) FOR [WhenToSendVal], ")
	_T("CONSTRAINT [DF_AutoResponses_WhenToSendFreq] DEFAULT (0) FOR [WhenToSendFreq], ")
	_T("CONSTRAINT [DF_AutoResponses_WhenToSendTypeID] DEFAULT (0) FOR [WhenToSendTypeID], ")
	_T("CONSTRAINT [DF_AutoResponses_SrToSendID] DEFAULT (0) FOR [SrToSendID], ")
	_T("CONSTRAINT [DF_AutoResponses] DEFAULT (0) FOR [AutoReplyQuoteMsg], ")
	_T("CONSTRAINT [DF_AutoResponses_AutoReplyCloseTicket] DEFAULT (0) FOR [AutoReplyCloseTicket], ")
	_T("CONSTRAINT [DF_AutoResponses_HeaderID] DEFAULT (0) FOR [HeaderID], ")
	_T("CONSTRAINT [DF_AutoResponses_FooterID] DEFAULT (0) FOR [FooterID], ")
	_T("CONSTRAINT [DF_AutoResponses_DateCreated] DEFAULT (getdate()) FOR [DateCreated], ")
	_T("CONSTRAINT [DF_AutoResponses_DateEdited] DEFAULT (getdate()) FOR [DateEdited], ")
	_T("CONSTRAINT [DF_AutoResponses_CreatedByID] DEFAULT (0) FOR [CreatedByID], ")
	_T("CONSTRAINT [DF_AutoResponses_EditedByID] DEFAULT (0) FOR [EditedByID], ")
	_T("CONSTRAINT [DF_AutoResponses_SendToPercent] DEFAULT (100) FOR [SendToPercent], ")
	_T("CONSTRAINT [PK_AutoResponses] PRIMARY KEY  CLUSTERED ([AutoResponseID])"));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add AutoResponsesSent
	if(g_showResults != 0)
	{
		_tcout << _T("Create AutoResponsesSent table") << endl;
	}

	g_logFile.Write(_T("Create AutoResponsesSent table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE AutoResponsesSent ([AutoResponseSentID] [int] IDENTITY (1, 1) NOT NULL ,[AutoResponseID] [int] NOT NULL ,[TicketID] [int] NOT NULL ,[ResultCode] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AutoResponsesSent ADD CONSTRAINT [DF_AutoResponsesSent_ResultCode] DEFAULT (0) FOR [ResultCode],CONSTRAINT [PK_AutoResponsesSent] PRIMARY KEY  CLUSTERED ([AutoResponseSentID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add TicketBoxOwners
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketBoxOwners table") << endl;
	}

	g_logFile.Write(_T("Create TicketBoxOwners table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE TicketBoxOwners ([TicketBoxOwnerID] [int] IDENTITY (1, 1) NOT NULL ,[Description] [varchar] (50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxOwners ADD CONSTRAINT [DF_TicketBoxOwners_Description] DEFAULT ('') FOR [Description],CONSTRAINT [PK_TicketBoxOwners] PRIMARY KEY  CLUSTERED ([TicketBoxOwnerID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add TicketBoxTicketBoxOwner
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketBoxTicketBoxOwner table") << endl;
	}

	g_logFile.Write(_T("Create TicketBoxTicketBoxOwner table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE TicketBoxTicketBoxOwner ([TbTboID] [int] IDENTITY (1, 1) NOT NULL ,[TicketBoxID] [int] NOT NULL ,[TicketBoxOwnerID] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxTicketBoxOwner ADD CONSTRAINT [DF_TicketBoxTicketBoxOwner_TicketBoxID] DEFAULT (0) FOR [TicketBoxID],CONSTRAINT [DF_TicketBoxTicketBoxOwner_TicketBoxOwnerID] DEFAULT (0) FOR [TicketBoxOwnerID],CONSTRAINT [PK_TicketBoxTicketBoxOwner] PRIMARY KEY  CLUSTERED ([TbTboID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	
	//Add TicketBoxRouting
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketBoxRouting table") << endl;
	}

	g_logFile.Write(_T("Create TicketBoxRouting table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE TicketBoxRouting ([TicketBoxRoutingID] [int] IDENTITY (1, 1) NOT NULL ,[RoutingRuleID] [int] NOT NULL ,[TicketBoxID] [int] NOT NULL ,[Percentage] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxRouting ADD CONSTRAINT [PK_TicketBoxRouting] PRIMARY KEY CLUSTERED ([TicketBoxRoutingID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	


	//Add TicketCategories
	if(g_showResults != 0)
	{
		_tcout << _T("Create TicketCategories table") << endl;
	}

	g_logFile.Write(_T("Create TicketCategories table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE TicketCategories ( ")
	_T("[TicketCategoryID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[Description] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[ObjectID] [int] NOT NULL, ")
	_T("[BuiltIn] [bit] NOT NULL )"));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketCategories ADD ")
	_T("CONSTRAINT [DF_TicketCategories_ObjectID] DEFAULT (0) FOR [ObjectID], ")
	_T("CONSTRAINT [DF_TicketCategories_BuiltIn] DEFAULT (0) FOR [BuiltIn], ")
	_T("CONSTRAINT [PK_TicketCategories] PRIMARY KEY CLUSTERED ([TicketCategoryID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}	

	//Add WaterMarkAlerts
	if(g_showResults != 0)
	{
		_tcout << _T("Create WaterMarkAlerts table") << endl;
	}

	g_logFile.Write(_T("Create WaterMarkAlerts table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE WaterMarkAlerts ( ")
	_T("[WaterMarkAlertID] [int] IDENTITY (1, 1) NOT NULL , ")
	_T("[Description] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , ")
	_T("[HitCount] [int] NOT NULL , ")
	_T("[LowWaterMark] [int] NOT NULL , ")
	_T("[HighWaterMark] [int] NOT NULL , ")
	_T("[IsEnabled] [bit] NOT NULL , ")
	_T("[SendLowAlert] [bit] NOT NULL , ")
	_T("[WaterMarkStatus] [tinyint] NOT NULL , ")
	_T("[AlertOnTypeID] [int] NOT NULL , ")
	_T("[AlertOnID] [int] NOT NULL , ")
	_T("[AlertToTypeID] [int] NOT NULL , ")
	_T("[AlertToID] [int] NOT NULL , ")
	_T("[AlertMethodID] [int] NOT NULL , ")
	_T("[EmailAddress] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , ")
	_T("[FromEmailAddress] [varchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , ")
	_T("[DateCreated] [datetime] NOT NULL , ")
	_T("[DateEdited] [datetime] NOT NULL , ")
	_T("[CreatedByID] [int] NOT NULL , ")
	_T("[EditedByID] [int] NOT NULL ) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE WaterMarkAlerts ADD ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_Description] DEFAULT ('') FOR [Description], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_HitCount] DEFAULT (0) FOR [HitCount], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_LowWaterMark] DEFAULT (0) FOR [LowWaterMark], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_HighWaterMark] DEFAULT (0) FOR [HighWaterMark], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_IsEnabled] DEFAULT (1) FOR [IsEnabled], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_SendLowAlert] DEFAULT (1) FOR [SendLowAlert], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_WaterMarkStatus] DEFAULT (0) FOR [WaterMarkStatus], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_SendToTypeID] DEFAULT (0) FOR [AlertOnTypeID], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_SendToID] DEFAULT (0) FOR [AlertOnID], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_AlertToTypeID] DEFAULT (0) FOR [AlertToTypeID], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_AlertMethodID] DEFAULT (0) FOR [AlertToID], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_AlertMethodID_1] DEFAULT (0) FOR [AlertMethodID], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_DateCreated] DEFAULT (getdate()) FOR [DateCreated], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_DateEdited] DEFAULT (getdate()) FOR [DateEdited], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_CreatedByID] DEFAULT (0) FOR [CreatedByID], ")
	_T("CONSTRAINT [DF_WaterMarkAlerts_EditedByID] DEFAULT (0) FOR [EditedByID], ")
	_T("CONSTRAINT [PK_WaterMarkAlerts] PRIMARY KEY  CLUSTERED ([WaterMarkAlertID]) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ProcessingRules
	if(g_showResults != 0)
	{
		_tcout << _T("Create ProcessingRules table") << endl;
	}

	g_logFile.Write(_T("Create ProcessingRules table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [ProcessingRules]( ")
	_T("[ProcessingRuleID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[RuleDescrip] [varchar](125) COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL, ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_ProcessingRules_IsEnabled] DEFAULT ('1'), ")
	_T("[OrderIndex] [int] NOT NULL CONSTRAINT [DF_ProcessingRules_OrderIndex] DEFAULT (0), ")
	_T("[HitCount] [int] NOT NULL CONSTRAINT [DF_ProcessingRules_HitCount] DEFAULT (0), ")
	_T("[PrePost] [bit] NOT NULL CONSTRAINT [DF_ProcessingRules_PrePost] DEFAULT ((0)), ")
	_T("[ActionType] [int] NOT NULL CONSTRAINT [DF_ProcessingRules_ActionType] DEFAULT ((0)), ")
	_T("[ActionID] [int] NOT NULL CONSTRAINT [DF_ProcessingRules_ActionID] DEFAULT ((0)), ")
	_T("[MessageSourceTypeID] [int] NOT NULL CONSTRAINT [DF_ProcessingRules_MessageSourceTypeID] DEFAULT (0), ")
	_T("CONSTRAINT [PK_ProcessingRules] PRIMARY KEY CLUSTERED ([ProcessingRuleID])) "));
	

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add MatchTextP
	if(g_showResults != 0)
	{
		_tcout << _T("Create MatchTextP table") << endl;
	}

	g_logFile.Write(_T("Create MatchTextP table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [MatchTextP]( ")
	_T("[MatchID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[MatchText] [text] COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL, ")
	_T("[MatchLocation] [tinyint] NOT NULL, ")
	_T("[ProcessingRuleID] [int] NOT NULL CONSTRAINT [DF_Table_1_RoutingRuleID]  DEFAULT (0), ")
	_T("CONSTRAINT [PK_MatchTextP] PRIMARY KEY CLUSTERED ([MatchID])) "));
	

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add MatchFromAddressP
	if(g_showResults != 0)
	{
		_tcout << _T("Create MatchFromAddressP table") << endl;
	}

	g_logFile.Write(_T("Create MatchFromAddressP table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[MatchFromAddressP]( ")
	_T("[MatchID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[EmailAddress] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL, ")
	_T("[ProcessingRuleID] [int] NOT NULL CONSTRAINT [DF_Table_1_RoutingRuleID_1]  DEFAULT (0), ")
	_T("CONSTRAINT [PK_MatchFromAddressP] PRIMARY KEY CLUSTERED ([MatchID])) "));
	

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add MatchToAddressP
	if(g_showResults != 0)
	{
		_tcout << _T("Create MatchToAddressP table") << endl;
	}

	g_logFile.Write(_T("Create MatchToAddressP table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[MatchToAddressP]( ")
	_T("[MatchID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[EmailAddress] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL, ")
	_T("[ProcessingRuleID] [int] NOT NULL CONSTRAINT [DF_Table_1_RoutingRuleID_2]  DEFAULT (0), ")
	_T("CONSTRAINT [PK_MatchToAddressP] PRIMARY KEY CLUSTERED ([MatchID])) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportCustom
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportCustom table") << endl;
	}

	g_logFile.Write(_T("Create ReportCustom table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportCustom]( ")
	_T("[CustomReportID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ReportsCustom_Description]  DEFAULT (''), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_ReportsCustom_IsEnabled]  DEFAULT ((1)), ")
	_T("[ReportTypeID] [int] NOT NULL CONSTRAINT [DF_ReportsCustom_ReportTypeID]  DEFAULT ((0)), ")
	_T("[ReportObjectID] [int] NOT NULL CONSTRAINT [DF_ReportsCustom_ReportObjectID]  DEFAULT ((0)), ")
	_T("[BuiltIn] [bit] NOT NULL CONSTRAINT [DF_ReportsCustom_BuiltIn]  DEFAULT ((0)), ")
	_T("[DateCreated] [datetime] NOT NULL CONSTRAINT [DF_ReportsCustom_DateCreated]  DEFAULT (getdate()), ")
	_T("[CreatedBy] [int] NOT NULL CONSTRAINT [DF_ReportsCustom_CreatedBy]  DEFAULT ((0)), ")
	_T("[DateEdited] [datetime] NOT NULL CONSTRAINT [DF_ReportsCustom_DateEdited]  DEFAULT (getdate()), ")
	_T("[EditedBy] [int] NOT NULL CONSTRAINT [DF_ReportsCustom_EditedBy]  DEFAULT ((0)), ")
	_T("[Query] [text] COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ReportsCustom_Query]  DEFAULT (''), ")
	_T("CONSTRAINT [PK_ReportsCustom] PRIMARY KEY CLUSTERED ([CustomReportID])) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportObjects
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportObjects table") << endl;
	}

	g_logFile.Write(_T("Create ReportObjects table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportObjects]( ")
	_T("[ReportObjectID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ReportObjects_Description]  DEFAULT (''), ")
	_T("[AllowCustom] [bit] NOT NULL CONSTRAINT [DF_ReportObjects_AllowCustom]  DEFAULT ((1)), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_ReportObjects_IsEnabled]  DEFAULT ((1)), ")
	_T("CONSTRAINT [PK_ReportObjects] PRIMARY KEY CLUSTERED ([ReportObjectID])) "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportTypes table") << endl;
	}

	g_logFile.Write(_T("Create ReportTypes table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportTypes]( ")
	_T("[ReportTypeID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ReportTypes_Description]  DEFAULT (''), ")
	_T("[AllowCustom] [bit] NOT NULL CONSTRAINT [DF_ReportTypes_AllowCustom]  DEFAULT ((1)), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_ReportTypes_IsEnabled]  DEFAULT ((1)), ")
	_T("CONSTRAINT [PK_ReportTypes] PRIMARY KEY CLUSTERED ([ReportTypeID])) "));

	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportStandard
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportStandard table") << endl;
	}

	g_logFile.Write(_T("Create ReportStandard table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportStandard]( ")
	_T("[StandardReportID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ReportsStandard_Description]  DEFAULT (''), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_ReportsStandard_IsEnabled]  DEFAULT ((1)), ")
	_T("[CanSchedule] [bit] NOT NULL CONSTRAINT [DF_ReportStandard_CanSchedule]  DEFAULT ((1)), ")
	_T("[ReportTypeID] [int] NOT NULL CONSTRAINT [DF_ReportsStandard_ReportTypeID]  DEFAULT ((0)), ")
	_T("[ReportObjectID] [int] NOT NULL CONSTRAINT [DF_ReportsStandard_ReportObjectID]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_ReportsStandard] PRIMARY KEY CLUSTERED ([StandardReportID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportScheduled
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportScheduled table") << endl;
	}

	g_logFile.Write(_T("Create ReportScheduled table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportScheduled]( ")
	_T("[ScheduledReportID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[Description] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ScheduledReports_Description]  DEFAULT (''), ")
	_T("[ReportID] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_ReportID]  DEFAULT ((0)), ")
	_T("[TargetID] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_TargetID]  DEFAULT ((0)), ")
	_T("[Flag1] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_Flag1]  DEFAULT ((0)), ")
	_T("[Flag2] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_Flag2]  DEFAULT ((0)), ")
	_T("[TicketStateID] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_TicketStateID] DEFAULT ((0)), ")
	_T("[IsCustom] [bit] NOT NULL CONSTRAINT [DF_ScheduledReports_IsCustom]  DEFAULT ((0)), ")
	_T("[IsEnabled] [bit] NOT NULL CONSTRAINT [DF_ScheduledReports_IsEnabled]  DEFAULT ((1)), ")
	_T("[OwnerID] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_OwnerID]  DEFAULT ((0)), ")
	_T("[DateCreated] [datetime] NOT NULL CONSTRAINT [DF_ScheduledReports_DateCreated]  DEFAULT (getdate()), ")
	_T("[DateEdited] [datetime] NOT NULL CONSTRAINT [DF_ScheduledReports_DateEdited]  DEFAULT (getdate()), ")
	_T("[CreatedBy] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_CreatedBy]  DEFAULT ((0)), ")
	_T("[EditedBy] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_EditedBy]  DEFAULT ((0)), ")
	_T("[LastRunTime] [datetime] NULL, ")
	_T("[LastResultCode] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_LastResultCode]  DEFAULT ((0)), ")
	_T("[NextRunTime] [datetime] NULL, ")
	_T("[RunFreq] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_RunFreq]  DEFAULT ((1440)), ")
	_T("[RunOn] [int] NOT NULL CONSTRAINT [DF_ScheduledReports_RunOnDays]  DEFAULT ((0)), ")
	_T("[RunSun] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun]  DEFAULT ((1)), ")
	_T("[RunMon] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun1]  DEFAULT ((1)), ")
	_T("[RunTue] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun1_1]  DEFAULT ((1)), ")
	_T("[RunWed] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun1_2]  DEFAULT ((1)), ")
	_T("[RunThur] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun1_3]  DEFAULT ((1)), ")
	_T("[RunFri] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun1_4]  DEFAULT ((1)), ")
	_T("[RunSat] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_RunSun1_5]  DEFAULT ((1)), ")
	_T("[RunAtHour] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_RunAtTime]  DEFAULT ((0)), ")
	_T("[RunAtMin] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_RunAtMin]  DEFAULT ((1)), ")
	_T("[SumPeriod] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_SumPeriod]  DEFAULT ((1)), ")
	_T("[MaxResults] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_MaxResults]  DEFAULT ((250)), ")
	_T("[SendAlertToOwner] [bit] NOT NULL CONSTRAINT [DF_ScheduledReports_ResultToTypeID]  DEFAULT ((1)), ")
	_T("[SendResultToEmail] [bit] NOT NULL CONSTRAINT [DF_ScheduledReports_ResultToID]  DEFAULT ((0)), ")
	_T("[ResultEmailTo] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL CONSTRAINT [DF_ScheduledReports_ResultEmailTo]  DEFAULT (''), ")
	_T("[IncludeResultFile] [bit] NOT NULL CONSTRAINT [DF_ScheduledReports_IncludeResultFile]  DEFAULT ((1)), ")
	_T("[SaveResultToFile] [bit] NOT NULL CONSTRAINT [DF_ScheduledReports_SaveResultToFile]  DEFAULT ((0)), ")
	_T("[KeepNumResultFile] [int] NOT NULL CONSTRAINT [DF_ReportScheduled_KeepNumResultFile]  DEFAULT ((0)), ")
	_T("[AllowConsolidation] [bit] NOT NULL CONSTRAINT [DF_ReportScheduled_AllowConsolidation]  DEFAULT ((1)), ")
	_T("CONSTRAINT [PK_ScheduledReports] PRIMARY KEY CLUSTERED ([ScheduledReportID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportResultRows
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportResultRows table") << endl;
	}

	g_logFile.Write(_T("Create ReportResultRows table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportResultRows]( ")
	_T("[ReportResultRowsID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[ReportResultID] [int] NOT NULL, ")
	_T("[Col1] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL CONSTRAINT [DF_Table_1_Row1]  DEFAULT (''), ")
	_T("[Col2] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL CONSTRAINT [DF_ReportResultRows_Col2]  DEFAULT (''), ")
	_T("[Col3] [int] NOT NULL CONSTRAINT [DF_Table_1_Row2]  DEFAULT ((0)), ")
	_T("[Col4] [decimal](18, 0) NOT NULL CONSTRAINT [DF_ReportResultRows_Col4]  DEFAULT ((0.00)), ")
	_T("[Col5] [int] NOT NULL CONSTRAINT [DF_ReportResultRows_Col5]  DEFAULT ((0)), ")
	_T("CONSTRAINT [PK_ReportResultRows] PRIMARY KEY CLUSTERED ([ReportResultRowsID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add ReportResults
	if(g_showResults != 0)
	{
		_tcout << _T("Create ReportResults table") << endl;
	}

	g_logFile.Write(_T("Create ReportResults table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[ReportResults]( ")
	_T("[ReportResultID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[ScheduledReportID] [int] NOT NULL CONSTRAINT [DF_ReportResults_ScheduledReportID]  DEFAULT ((0)), ")
	_T("[ResultCode] [int] NOT NULL CONSTRAINT [DF_ReportResults_ResultCode]  DEFAULT ((0)), ")
	_T("[ResultFile] [varchar](255) COLLATE SQL_Latin1_General_CP1_CI_AI NOT NULL CONSTRAINT [DF_ReportResults_ResultFile]  DEFAULT (''), ")
	_T("[DateRan] [datetime] NOT NULL CONSTRAINT [DF_ReportResults_DateRan]  DEFAULT (getdate()), ")
	_T("[DateFrom] [datetime] NULL, ")
	_T("[DateTo] [datetime] NULL, ")
	_T("CONSTRAINT [PK_ReportResults] PRIMARY KEY CLUSTERED ([ReportResultID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	
	//Add Data To New Tables

	//Ticket Categories
	if(g_showResults != 0)
	{
		_tcout << _T("Add row to TicketCategories") << endl;
	}

	g_logFile.Write(_T("Add row to TicketCategories"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT TicketCategories ON ")
	_T("INSERT INTO [TicketCategories]([TicketCategoryID],[Description],[BuiltIn]) VALUES (1,'Unassigned',1) ")
	_T("SET IDENTITY_INSERT TicketCategories OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//ReportObjects
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to ReportObjects") << endl;
	}

	g_logFile.Write(_T("Add rows to ReportObjects"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ReportObjects ON ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (1,'TicketBox',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (2,'Agent',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (3,'Ticket Category',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (4,'Routing Rule',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (5,'Message Source',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (6,'Owner',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (7,'SR Usage',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (8,'SR Category',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (9,'SR Author',1) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (10,'By Hour',0) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (11,'By Day of Week',0) ")
	_T("INSERT INTO [ReportObjects]([ReportObjectID],[Description],[AllowCustom]) VALUES (12,'Contact',1) ")
	_T("SET IDENTITY_INSERT ReportObjects OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//ReportTypes
	if(g_showResults != 0)
	{
		_tcout << _T("Add ReportTypes") << endl;
	}

	g_logFile.Write(_T("Add rows to ReportTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ReportTypes ON ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (1,'Inbound Messages',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (2,'Outbound Messages',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (3,'Open Ticket Age',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (4,'Ticket Distribution',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (5,'Standard Responses',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (6,'Avg Response Time',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (7,'Avg Time to Resolve',1) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (8,'Busiest Time',0) ")
	_T("INSERT INTO [ReportTypes]([ReportTypeID],[Description],[AllowCustom]) VALUES (9,'History',1) ")
	_T("SET IDENTITY_INSERT ReportTypes OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//ReportStandard
	if(g_showResults != 0)
	{
		_tcout << _T("Add ReportStandard") << endl;
	}

	g_logFile.Write(_T("Add rows to ReportStandard"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ReportStandard ON ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (1,'Inbound Messages By TicketBox',1,1,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (2,'Inbound Messages By Agent',1,1,2) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (3,'Inbound Messages By Ticket Category',1,1,3) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (4,'Inbound Messages By Routing Rule',1,1,4) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (5,'Inbound Messages By Message Source',1,1,5) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (6,'Outbound Messages By TicketBox',1,2,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (7,'Outbound Messages By TicketBox',1,2,2) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (8,'Outbound Messages By TicketBox',1,2,3) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (9,'Open Ticket Age By TicketBox',1,3,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (10,'Open Ticket Age By Agent',1,3,2) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (11,'Open Ticket Age By Ticket Category',1,3,3) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (12,'Ticket Distribution By TicketBox',1,4,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (13,'Ticket Distribution By Owner',1,4,6) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (14,'Ticket Distribution By Ticket Category',1,4,3) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (15,'Standard Responses By Usage',1,5,7) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (16,'Standard Responses By Category',1,5,8) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (17,'Standard Responses By Author',1,5,9) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (18,'Avg Response Time By TicketBox',1,6,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (19,'Avg Response Time By Agent',1,6,2) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (20,'Avg Response Time By Ticket Category',1,6,3) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (21,'Avg Time To Resolve By TicketBox',1,7,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (22,'Avg Time To Resolve By Agent',1,7,2) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (23,'Avg Time To Resolve By Ticket Category',1,7,3) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (24,'Busiest Time By Hour',1,8,10) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (25,'Busiest Time By Day Of Week',1,8,11) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (26,'History By Agent',0,9,2) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (27,'History By TicketBox',0,9,1) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (28,'History By Contact',0,9,12) ")
	_T("INSERT INTO [ReportStandard]([StandardReportID],[Description],[CanSchedule],[ReportTypeID],[ReportObjectID]) VALUES (29,'History By Ticket Category',0,9,3) ")
	_T("SET IDENTITY_INSERT ReportStandard OFF "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add New Foreign Key Constraints

	//Update Existing Tables
	
	//Update RoutingRules table
	/*Add AssignToTicketCategory [int] (default 0) to RoutingRules
	Add AlertEnable [bit] (default 0) to RoutingRules
	Add AlertIncludeSubject [bit] (default 0) to RoutingRules
	Add AlertToAgentID [int] (default 0) to RoutingRules
	Add AlertText [varchar] (255) (default '') to RoutingRules
	Add ToOrFrom [bit]  (default 0) to RoutingRules
	Add ConsiderAllOwned [bit]  (default 0) to RoutingRules
	Add DoProcessingRules [bit]  (default 0) to RoutingRules*/
	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to RoutingRules") << endl;
	}

	g_logFile.Write(_T("Add columns to RoutingRules"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE RoutingRules ADD AssignToTicketCategory INT DEFAULT 1 NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD AlertEnable BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD AlertIncludeSubject BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD AlertToAgentID INT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD AlertText VARCHAR(255) DEFAULT ('') NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD ToOrFrom BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD ConsiderAllOwned BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE RoutingRules ADD DoProcessingRules BIT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update Groups table
	/*Add IsEscalationGroup [bit] (default 0) to Groups
	Add UseEscTicketBox [bit] (default 0) to Groups
	Add AssignToTicketBoxID [int]  (default 0) to Groups */

	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to Groups") << endl;
	}

	g_logFile.Write(_T("Add columns to Groups"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Groups ADD IsEscalationGroup BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE Groups ADD UseEscTicketBox BIT DEFAULT 0 NOT NULL ")
							   _T("ALTER TABLE Groups ADD AssignToTicketBoxID INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update AlertMsgs table
	/*Add Subject [varchar] (255) (default '') to AlertMsgs
	Add TicketID [int] (default 0) to AlertMsgs */

	if(g_showResults != 0)
	{
		_tcout << _T("Add columns to AlertMsgs") << endl;
	}

	g_logFile.Write(_T("Add columns to AlertMsgs"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE AlertMsgs ADD Subject VARCHAR(255) DEFAULT ('') NOT NULL ")
							   _T("ALTER TABLE AlertMsgs ADD TicketID INT DEFAULT 0 NOT NULL"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update AlertEvents table
	/*Add Ticket Created as AlertEventID 9 to AlertEvents
	Add Ticket Escalated as AlertEventID 10 to AlertEvents*/

	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AlertEvents") << endl;
	}

	g_logFile.Write(_T("Add rows to AlertEvents"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT AlertEvents ON INSERT INTO AlertEvents (AlertEventID,Description) VALUES (9,'Ticket Created') INSERT INTO AlertEvents (AlertEventID,Description) VALUES (10,'Ticket Escalated') INSERT INTO AlertEvents (AlertEventID,Description) VALUES (11,'Report Ran') SET IDENTITY_INSERT AlertEvents OFF "));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update ObjectTypes table
	//Add TicketCategories to ObjectTypes as ID 6.

	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to ObjectTypes") << endl;
	}

	g_logFile.Write(_T("Add rows to ObjectTypes"));

	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ObjectTypes ON INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (6,'Ticket Category') INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (7,'Standard Reports') INSERT INTO ObjectTypes (ObjectTypeID,Description) VALUES (8,'Scheduled Reports') SET IDENTITY_INSERT ObjectTypes OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	TCHAR buf[256];
	int newObjectID = 0;
	int newObjectID2 = 0;
	int newObjectID3 = 0;
	int newObjectID4 = 0;
		
	if ( maxObjectID == 0 )
	{
		// Get the MAX ObjectID from the  Objects table
		if(g_showResults != 0)
		{
			_tcout << _T("Get the MAX ObjectID from the Objects table and add 1") << endl;
		}
		
		m_query.Initialize();
		BINDCOL_LONG_NOLEN( m_query, maxObjectID );
		m_query.Execute( L"SELECT MAX(ObjectID) FROM Objects" );
		if( !m_query.Fetch() == S_OK )
		{
			_tcout << endl << _T("Failed to get maxObjectID from MailFlow DB") << endl;
			g_logFile.Write(_T("Failed to get maxObjectID from MailFlow DB"));
			g_logFile.LineBreak();
			g_logFile.Line();
			return 99;
		}

		if ( maxObjectID != 0 )
		{
			newObjectID = maxObjectID + 1;
			newObjectID2 = newObjectID + 1;
			newObjectID3 = newObjectID2 + 1;
			newObjectID4 = newObjectID3 + 1;
			if(g_showResults != 0)
			{
				wsprintf(buf,_T("Received MAX ObjectID: %d incrementing to %d"),maxObjectID,newObjectID);
				_tcout << buf << endl;
			}
			maxObjectID = newObjectID4;	
		}
		else
		{
			_tcout << endl << _T("Failed to get maxObjectID from MailFlow DB") << endl;
			g_logFile.Write(_T("Failed to get maxObjectID from MailFlow DB"));
			g_logFile.LineBreak();
			g_logFile.Line();
			return 2;
		}
	}
	else
	{
		newObjectID = maxObjectID + 1;
		newObjectID2 = newObjectID + 1;
		newObjectID3 = newObjectID2 + 1;
		newObjectID4 = newObjectID3 + 1;
		if(g_showResults != 0)
		{
			wsprintf(buf,_T("Received MAX ObjectID: %d incrementing to %d"),maxObjectID,newObjectID);
			_tcout << buf << endl;
		}
		maxObjectID = newObjectID4;	
	}
	
	
	
	//Update Objects table
	//Add TicketCategories to Objects.

	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to Objects") << endl;
	}

	g_logFile.Write(_T("Add rows to Objects"));

	wsprintf(buf,_T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,0,6,1,0) SET IDENTITY_INSERT Objects OFF"),newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	wsprintf(buf,_T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,0,7,1,0) SET IDENTITY_INSERT Objects OFF"),newObjectID2);
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	wsprintf(buf,_T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,0,8,1,0) SET IDENTITY_INSERT Objects OFF"),newObjectID3);
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
		
	//Add an Entry for the Unassigned TicketCategory to Objects
	wsprintf(buf,_T("SET IDENTITY_INSERT Objects ON INSERT INTO Objects (ObjectID,ActualID,ObjectTypeID,BuiltIn,UseDefaultRights) VALUES (%d,1,6,0,1) SET IDENTITY_INSERT Objects OFF"),newObjectID4);
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
		
	//Update AccessControl table
	//Add Entry to AccessControl with GroupID=1, AccessLevel=4 and ObjectID = TicketCategories ID from Objects Table.

	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to AccessControl") << endl;
	}

	g_logFile.Write(_T("Add rows to AccessControl"));
	wsprintf(buf,_T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"),newObjectID);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	wsprintf(buf,_T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"),newObjectID2);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	wsprintf(buf,_T("INSERT INTO AccessControl (AgentID,GroupID,AccessLevel,ObjectID) VALUES (0,1,4,%d)"),newObjectID3);
	retSQL = dbConn.ExecuteSQL(buf);
	if(!SQL_SUCCEEDED(retSQL))																
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update the the Unassigned TicketCategory with the ObjectID
	if(g_showResults != 0)
	{
		_tcout << _T("Updating Unassigned Ticket Category with ObjectID") << endl;
	}

	g_logFile.Write(_T("Updating Unassigned Ticket Category with ObjectID"));

	wsprintf(buf,_T("UPDATE TicketCategories SET ObjectID=%d WHERE TicketCategoryID=1"),newObjectID4);
	retSQL = dbConn.ExecuteSQL(buf);if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Default Routing Rule Default TicketCategoryID to Server Parameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Default TicketCategoryID value to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding Default TicketCategoryID value to Server Parameter table"));

	// Adding value to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(46,'Default Routing Rule TicketCategoryID','1') SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	//Add Scheduled Reports Flag to Server Parameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Scheduled Reports value to Server Parameter table") << endl;
	}

	g_logFile.Write(_T("Adding Scheduled Reports value to Server Parameter table"));

	// Adding value to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON insert into ServerParameters (ServerParameterID,Description,DataValue) values(47,'Scheduled Reports','1') SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update TicketBoxes table
	//Add OwnerID [int] (default 0) to TicketBoxes

	if(g_showResults != 0)
	{
		_tcout << _T("Add OwnerID column to TicketBoxes") << endl;
	}

	g_logFile.Write(_T("Add OwnerID column to TicketBoxes"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE TicketBoxes ADD OwnerID INT DEFAULT (0) NOT NULL"));
							   
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update InboundMessages table
	//Add OriginalTicketCategoryID [int] (default 0) to InboundMessages

	if(g_showResults != 0)
	{
		_tcout << _T("Add OriginalTicketCategoryID column to InboundMessages") << endl;
	}

	g_logFile.Write(_T("Add OriginalTicketCategoryID column to InboundMessages"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE InboundMessages ADD OriginalTicketCategoryID INT DEFAULT (1) NOT NULL"));
							   
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update OutboundMessages table
	//Add TicketCategoryID [int] (default 0) to OutboundMessages

	if(g_showResults != 0)
	{
		_tcout << _T("Add TicketCategoryID column to OutboundMessages") << endl;
	}

	g_logFile.Write(_T("Add TicketCategoryID column to OutboundMessages"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE OutboundMessages ADD TicketCategoryID INT DEFAULT (1) NOT NULL"));
							   
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Update Tickets table
	//Add TicketCategoryID [int] (default 0) to Tickets

	if(g_showResults != 0)
	{
		_tcout << _T("Add TicketCategoryID column to Tickets") << endl;
	}

	g_logFile.Write(_T("Add TicketCategoryID column to Tickets"));

	retSQL = dbConn.ExecuteSQL(_T("ALTER TABLE Tickets ADD TicketCategoryID INT DEFAULT (1) NOT NULL"));
							   
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;

}