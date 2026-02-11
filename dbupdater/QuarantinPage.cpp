#include "prehead.h"
#include "main.h"

int QuarantinePage(CDBConn& dbConn,reg::Key& rkMailFlow)
{
	SQLRETURN	retSQL;
	TCHAR sData[256];
	
	//*************************************************************************
	
	if(g_showResults != 0)
	{
		_tcout << _T("Upgrading Attachment Tables") << endl;
	}

	// Debug
	dbConn.GetData(_T("select @@TRANCOUNT"),sData);
	OutputDebugString(sData);
	OutputDebugString(_T("\n"));

	g_logFile.Write(_T("Upgrading Attachment Tables"));

	// Modify InboundMessageAttachments since we will be deleting it later
	retSQL = dbConn.ExecuteSQL(_T("alter table InboundMessageAttachments add ContentID varchar(255) default '' not null alter table InboundMessageAttachments add IsInbound bit default 1 not null"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 2705)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	// Debug
	dbConn.GetData(_T("select @@TRANCOUNT"),sData);
	OutputDebugString(sData);
	OutputDebugString(_T("\n"));

	// Modify OutboundMessageAttachments since we will be deleting it later
	retSQL = dbConn.ExecuteSQL(_T("alter table OutboundMessageAttachments add VirusScanStateID int default 0 not null alter table OutboundMessageAttachments add VirusName varchar(125) default '' not null alter table OutboundMessageAttachments add ContentID varchar(255) default '' not null alter table OutboundMessageAttachments add IsInbound bit default 0 not null"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 2705)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	// Modify StdResponseAttachments since we will be deleting it later
	retSQL = dbConn.ExecuteSQL(_T("alter table StdResponseAttachments add VirusScanStateID int default 0 not null alter table StdResponseAttachments add VirusName varchar(125) default '' not null alter table StdResponseAttachments add ContentID varchar(255) default '' not null alter table StdResponseAttachments add IsInbound bit default 0 not null"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 2705)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	// Create TempAttachments table
	retSQL = dbConn.ExecuteSQL(_T("create table TempAttachments( AttachmentID int IDENTITY(1,1) PRIMARY KEY not null,AttachmentLocation VARCHAR(255) not null,MediaType VARCHAR(125) not null,MediaSubType VARCHAR(125) not null,ContentDisposition VARCHAR(125) not null,FileName VARCHAR(255) not null,FileSize int not null,VirusScanStateID int not null,VirusName VARCHAR(125) not null,ContentID VARCHAR(255) not null,IsInbound BIT not null )"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 2714)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
		else
		{
			retSQL = dbConn.ExecuteSQL(_T("DROP TABLE TempAttachments"));

			if(!IsTransUp(dbConn,rkMailFlow))
			{
				// Begin transaction
				retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
				if(!SQL_SUCCEEDED(retSQL))
				{
					OutputDebugString(_T("Failed to begin transaction \n"));
					if(g_showResults != 0)
					{
						_tcout << endl << dbConn.GetErrorString() << endl;
					}

					g_logFile.Write(dbConn.GetErrorString());

					dbConn.Disconnect();
					rkMailFlow.Close();
					g_logFile.LineBreak();
					g_logFile.Line();
					return 4;
				}
			}

			if(SQL_SUCCEEDED(retSQL))
			{
				dbConn.ExecuteSQL(_T("create table TempAttachments( AttachmentID int IDENTITY(1,1) PRIMARY KEY not null,AttachmentLocation VARCHAR(255) not null,MediaType VARCHAR(125) not null,MediaSubType VARCHAR(125) not null,ContentDisposition VARCHAR(125) not null,FileName VARCHAR(255) not null,FileSize int not null,VirusScanStateID int not null,VirusName VARCHAR(125) not null,ContentID VARCHAR(255) not null,IsInbound BIT not null )"));
			}
			else
			{
				return ErrorOpt(dbConn,rkMailFlow);
			}
		}
	}

	// Fill table with inbound add inbound message id
	retSQL = dbConn.ExecuteSQL(_T("alter table TempAttachments add InboundMessageID int default 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Fill TempAttachments with Inbound Messages 
	retSQL = dbConn.ExecuteSQL(_T("insert into TempAttachments(AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound,InboundMessageID) select AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound,InboundMessageID from InboundMessageAttachments"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Create NewInbound table
	retSQL = dbConn.ExecuteSQL(_T("create table NewInboundMessageAttachments (InboundMessageAttachmentID int IDENTITY(1,1) PRIMARY KEY not null, AttachmentID int not null, InboundMessageID int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Insert data into NewInbound table
	retSQL = dbConn.ExecuteSQL(_T("insert into NewInboundMessageAttachments(AttachmentID,InboundMessageID) select AttachmentID,InboundMessageID from TempAttachments"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add OutboundMessageID to TempAttachments
	retSQL = dbConn.ExecuteSQL(_T("alter table TempAttachments add OutboundMessageID int default 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Insert Data into TempAttachments
	retSQL = dbConn.ExecuteSQL(_T("insert into TempAttachments(AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound,OutboundMessageID) select AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound,OutboundMessageID from OutboundMessageAttachments"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Create new table
	retSQL = dbConn.ExecuteSQL(_T("create table NewOutboundMessageAttachments ( OutboundMessageAttachmentID int IDENTITY(1,1) PRIMARY KEY not null,AttachmentID int not null,OutboundMessageID int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Fill Outbound with Data
	retSQL = dbConn.ExecuteSQL(_T("insert into NewOutboundMessageAttachments(AttachmentID,OutboundMessageID) select AttachmentID,OutboundMessageID from TempAttachments where OutboundMessageID is not null"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Setup TempAttachment for StdRespID
	retSQL = dbConn.ExecuteSQL(_T("alter table TempAttachments add StandardResponseID int default 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Fill TempAttachment with data from stdresp
	retSQL = dbConn.ExecuteSQL(_T("insert into TempAttachments(AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound,StandardResponseID) select AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound,StandardResponseID from StdResponseAttachments"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Create new StdResponse Table
	retSQL = dbConn.ExecuteSQL(_T("create table NewStdResponseAttachments ( StdResponseAttachID int IDENTITY(1,1) PRIMARY KEY not null, StandardResponseID int not null, AttachmentID int not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Fill new StdResponse with data
	retSQL = dbConn.ExecuteSQL(_T("insert into NewStdResponseAttachments(StandardResponseID,AttachmentID) select StandardResponseID,AttachmentID from TempAttachments where StandardResponseID is not null"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Create table attachments
	retSQL = dbConn.ExecuteSQL(_T("create table Attachments(AttachmentID int IDENTITY(1,1) PRIMARY KEY not null,AttachmentLocation VARCHAR(255) not null,MediaType VARCHAR(125) not null,MediaSubType VARCHAR(125) not null,ContentDisposition VARCHAR(125) not null,FileName VARCHAR(255) not null,FileSize int not null,VirusScanStateID int not null,VirusName VARCHAR(125) not null,ContentID VARCHAR(255) not null,IsInbound BIT not null)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Fill Attachments with data
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT Attachments ON insert into Attachments(AttachmentID,AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound) select AttachmentID,AttachmentLocation,MediaType,MediaSubType,ContentDisposition,FileName,FileSize,VirusScanStateID,VirusName,ContentID,IsInbound from TempAttachments SET IDENTITY_INSERT Attachments OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// drop temp attachments
	retSQL = dbConn.ExecuteSQL(_T("drop table TempAttachments"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// create index's and constraints
	retSQL = dbConn.ExecuteSQL(_T("create index IX_VirusScanStateID on Attachments(VirusScanStateID) alter table Attachments add constraint DF_AttachmentsFileSize default('0') for FileSize alter table Attachments add constraint DF_AttachmentsVirusScanStateID default('1') for VirusScanStateID alter table Attachments add constraint DF_AttachmentsIsInbound default('0') for IsInbound"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());
			
			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	// drop old table and rename new one
	retSQL = dbConn.ExecuteSQL(_T("drop table InboundMessageAttachments Exec sp_rename 'NewInboundMessageAttachments', 'InboundMessageAttachments'"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// create index and constraints - 1
	retSQL = dbConn.ExecuteSQL(_T("create index IX_AttachmentID on InboundMessageAttachments(AttachmentID) create index IX_InboundMessageID on InboundMessageAttachments(InboundMessageID) alter table InboundMessageAttachments add constraint DF_InboundMessageAttachmentsAttachmentID default ('0') for AttachmentID alter table InboundMessageAttachments add constraint DF_InboundMessageAttachmentsInboundMessageID default ('0') for InboundMessageID"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());
			
			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	// create index and constraints - 2
	retSQL = dbConn.ExecuteSQL(_T("alter table InboundMessageAttachments add constraint FK_InboundMsgAttachmentsInboundMsgs foreign key (InboundMessageID) references InboundMessages(InboundMessageID) alter table InboundMessageAttachments add constraint FK_InboundMsgAttachmentsAttachments foreign key (AttachmentID) references Attachments(AttachmentID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// drop old outbound and rename new
	retSQL = dbConn.ExecuteSQL(_T("drop table OutboundMessageAttachments Exec sp_rename 'NewOutboundMessageAttachments', 'OutboundMessageAttachments'"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// create idx and constraints for outbound - 1
	retSQL = dbConn.ExecuteSQL(_T("create index IX_AttachmentID on OutboundMessageAttachments(AttachmentID) create index IX_OutboundMessageID on OutboundMessageAttachments(OutboundMessageID) alter table OutboundMessageAttachments add constraint DF_OutboundMessageAttachmentsAttachmentID default ('0') for AttachmentID alter table OutboundMessageAttachments add constraint DF_OutboundMessageAttachmentsOutboundMessageID default ('0') for OutboundMessageID"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());
			
			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	// create idx and constraints for outbound - 2
	retSQL = dbConn.ExecuteSQL(_T("alter table OutboundMessageAttachments add constraint FK_OutboundMsgAttachmentsOutboundMsg foreign key (OutboundMessageID) references OutboundMessages(OutboundMessageID) alter table OutboundMessageAttachments add constraint FK_OutboundMsgAttachmentsAttachments foreign key (AttachmentID) references Attachments(AttachmentID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// drop old stdresp table and rename new one
	retSQL = dbConn.ExecuteSQL(_T("drop table StdResponseAttachments Exec sp_rename 'NewStdResponseAttachments', 'StdResponseAttachments'"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// create idx and constraints for stdresp table - 1
	retSQL = dbConn.ExecuteSQL(_T("create index IX_StandardResponseID on StdResponseAttachments(StandardResponseID) create index IX_AttachmentID on StdResponseAttachments(AttachmentID) alter table StdResponseAttachments add constraint DF_StdResponseAttachmentsStandardResponseID default ('0') for StandardResponseID alter table StdResponseAttachments add constraint DF_StdResponseAttachmentsAttachmentID default ('0') for AttachmentID"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		if(dbConn.GetNativeErrorCode() != 1913)
		{
			return ErrorOpt(dbConn,rkMailFlow);
		}
	}

	if(!IsTransUp(dbConn,rkMailFlow))
	{
		// Begin transaction
		retSQL = dbConn.ExecuteSQL(_T("begin transaction"));
		if(!SQL_SUCCEEDED(retSQL))
		{
			OutputDebugString(_T("Failed to begin transaction \n"));
			if(g_showResults != 0)
			{
				_tcout << endl << dbConn.GetErrorString() << endl;
			}

			g_logFile.Write(dbConn.GetErrorString());
			
			dbConn.Disconnect();
			rkMailFlow.Close();
			g_logFile.LineBreak();
			g_logFile.Line();
			return 4;
		}
	}

	// create idx and constraints for stdresp table - 2
	retSQL = dbConn.ExecuteSQL(_T("alter table StdResponseAttachments add constraint FK_StdRespAttStdResp foreign key (StandardResponseID) references StandardResponses(StandardResponseID) alter table StdResponseAttachments add constraint FK_StdRespAttAttachments foreign key (AttachmentID) references Attachments(AttachmentID)"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// update viruscan states set to 0 to a value of 1
	retSQL = dbConn.ExecuteSQL(_T("update Attachments set VirusScanStateID = 1 where VirusScanStateID = 0"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	// Add quarantine restore to virus scan states.
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT VirusScanStates ON insert into VirusScanStates(VirusScanStateID,Description) values(9,'Files restored from quarantine') SET IDENTITY_INSERT VirusScanStates OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	return 0;
}