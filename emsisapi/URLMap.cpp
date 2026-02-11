/***************************************************************************\
||             
||  $Header: /root/EMSISAPI/URLMap.cpp,v 1.2.2.2 2006/02/16 16:10:24 markm Exp $
||
||
||                                         
||  COMMENTS:	Maps page names to implementation class
||              
\\*************************************************************************/

#include "stdafx.h"
#include "URLMap.h"

#include "MainFrame.h"
#include "TicketList.h"
#include "MessageList.h"
#include "TicketBoxView.h"
#include "Client.h"
#include "ViewMsg.h"
#include "ContactDetails.h"
#include "MultipleContacts.h"
#include "MessageSources.h"
#include "MessageDestinations.h"
#include "RoutingRules.h"
#include "ProcessingRules.h"
#include "Download.h"
#include "TicketHistory.h"
#include "ContactHistory.h"
#include "LogConfig.h"
#include "AgentAudit.h"
#include "TicketBox.h"
#include "FormDump.h"
#include "VirusConfig.h"
#include "ACLPage.h"
#include "IPRanges.h"
#include "DBMaintenance.h"
#include "AlertConfig.h"
#include "TicketBoxHeaders.h"
#include "TicketBoxFooters.h"
#include "TicketBoxOwners.h"
#include "Agents.h"
#include "TestLogin.h"
#include "Signatures.h"
#include "TicketNotes.h"
#include "ContactNotes.h"
#include "Registration.h"
#include "Trash.h"
#include "StdRespCategories.h"
#include "StdResponse.h"
#include "SessionInfo.h"
#include "SpellCheck.h"
#include "Logout.h"
#include "Groups.h"
#include "AboutPage.h"
#include "PersonalDataTypes.h"
#include "TicketCategories.h"
#include "AutoMessages.h"
#include "AutoResponses.h"
#include "AgeAlerts.h"
#include "WmAlerts.h"
#include "TicketSearch.h"
#include "ContactSearch.h"
#include "ReportOutboundMsgs.h"
#include "ReportHistory.h"
#include "ReportSysConfig.h"
#include "ScheduleReport.h"
#include "AlertList.h"
#include "Alert.h"
#include "ChangeTicket.h"
#include "VMSIntegration.h"
#include "UploadProgress.h"
#include "SecuritySettings.h"
#include "TicketFields.h"
#include "TicketProperties.h"
#include "NewTicket.h"
#include "CustomDictionary.h"
#include "PreferenceTicketboxes.h"
#include "PreferenceAgents.h"
#include "MsgComp.h"
#include "MsgCompAttach.h"
#include "NoteAttach.h"
#include "TicketActions.h"
#include "QuarantinePage.h"
#include "SystemEmailAddress.h"
#include "DownloadSVG.h"
#include "DownloadAcrobat.h"
#include "Download3cxPlugin.h"
#include "Demo.h"
#include "CustomSettings.h"
#include "VoipConfig.h"
#include "VoipServers.h"
#include "VoipDialingCodes.h"
#include "VoipExtensions.h"
#include "VoipCall.h"
#include "CustomTicketBoxViews.h"
#include "UnlockTicketPage.h"
#include "Columns.h"
#include "Servers.h"
#include "MyAlerts.h"
#include "ClientSoft.h"
#include "MsgAttach.h"
#include "RegExTest.h"
#include "MyContacts.h"
#include "TicketLinks.h"
#include "ContactGroups.h"
#include "Unsub.h"
#include "OfficeHours.h"
#include "DateFilters.h"
#include "TicketPrint.h"

// reports
#include "ReportCompareInbound.h"
#include "ReportInboundMsgs.h"
#include "ReportOutboundMsgs.h"
#include "ReportTicketDist.h"
#include "ReportTicketDisp.h"
#include "ReportStdResp.h"
#include "ReportStdRespUsage.h"
#include "ReportRespTime.h"
#include "ReportResolveTime.h"
#include "ReportBusy.h"	
#include "ReportOpenTicketAge.h"
#include "ReportTicketBoxDetails.h"
#include "ReportTicketCountSum.h"
#include "ReportOpenTickets.h"
#include "ViewResults.h"
#include "HelpFrame.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CURLMap::CURLMap()
{

}

CURLMap::~CURLMap()
{

}
////////////////////////////////////////////////////////////////////////////////
// 
// Build the association map
// 
////////////////////////////////////////////////////////////////////////////////
void CURLMap::Initialize(void)
{
	CURLAction action;
	
	XSLTranslate(action, _T("Main"), _T("main"), _T("main.ems"), Create<CMainFrame>, false );
	XSLTranslate(action, _T("MailFlow Help"), _T("mainhelp"), _T("mainhelp.ems"), Create<CHelpFrame>, false );	
	
	XSLTranslate(action, _T("LeftPane"), _T("ticketboxlist"), _T("ticketboxlist.ems"), Create<CTicketBoxView>, false, true );

	XSLTranslate(action, _T("Ticket List"), _T("ticketlist"), _T("ticketlist.ems"), Create<CTicketList>, true );
	XSLTranslate(action, _T("My Tickets"), _T("mytickets"), _T("mytickets.ems"), Create<CTicketList>, true );	
	XSLTranslate(action, _T("Mark Ticket as Read"), _T("ticketread"), _T("ticketlist.ems"), Create<CTicketList>, false, true );
	XSLTranslate(action, _T("Custom Ticket Box Views"), _T("customticketboxviews"), _T("customticketboxviews.ems"), Create<CustomTicketBoxViews>, false);
	XSLTranslate(action, _T("My Contacts"), _T("mycontacts"), _T("mycontacts.ems"), Create<MyContacts>, false);

	XSLTranslate(action,  _T("Sent Items"), _T("sentitems"), _T("sentitems.ems"), Create<CMessageList>, true );
	XSLTranslate(action, _T("Outbox"), _T("outbox"), _T("outbox.ems"), Create<CMessageList>, true  );
	XSLTranslate(action, _T("Approvals"), _T("approvals"), _T("approvals.ems"), Create<CMessageList>, true  );
	XSLTranslate(action, _T("Outbound Approval"), _T("outboundapproval"), _T("outboundapproval.ems"), Create<CMessageList>, true  );
	XSLTranslate(action, _T("Drafts"), _T("drafts"), _T("drafts.ems"), Create<CMessageList>, true  );
	XSLTranslate(action, _T("Outbound Queue"), _T("outboundqueue"), _T("outboundqueue.ems"), Create<CMessageList>, true  );
	XSLTranslate(action, _T("Inbound Queue"), _T("inboundqueue"), _T("inboundqueue.ems"), Create<CMessageList>, true  );
	XSLTranslate(action, _T("Alerts"), _T("alertlist"), _T("alertlist.ems"), Create<CAlertList>, true  );
	XSLTranslate(action, _T("ViewMsg"), _T("viewmsg"), _T("viewmsg.ems"), Create<CViewMsg>, false );
	XSLTranslate(action, _T("ViewQMsg"), _T("viewqmsg"), _T("viewqmsg.ems"), Create<CViewMsg>, false );
	XSLTranslate(action, _T("ApproveMsg"), _T("approvemsg"), _T("approvemsg.ems"), Create<CViewMsg>, false );
	XSLTranslate(action, _T("Alert"), _T("viewalert"), _T("viewalert.ems"), Create<CAlert>, false );
	XSLTranslate(action, _T("Print Message"), _T("printinboundmsg"), _T("printinboundmsg.ems"), Create<CViewMsg>, false );
	
	XSLTranslate(action,  _T("New Message"), _T("newmessage"), _T("newmessage.ems"), Create<CMsgComp>, false, true );
	XSLTranslate(action, _T("Reply To Message"), _T("replytomessage"), _T("newmessage.ems"), Create<CMsgComp>, false, true );
	XSLTranslate(action, _T("Reply To Message"), _T("replytoall"), _T("newmessage.ems"), Create<CMsgComp>, false, true );
	XSLTranslate(action, _T("Forward Message"), _T("forwardmessage"), _T("newmessage.ems"), Create<CMsgComp>, false, true );
	XSLTranslate(action, _T("Send Again"), _T("sendagain"), _T("newmessage.ems"), Create<CMsgComp>, false, true );
	XSLTranslate(action, _T("Send Draft"), _T("senddraft"), _T("newmessage.ems"), Create<CMsgComp>, false, true );

	XSLTranslate(action, _T("Unlock Ticket"), _T("unlockmsg"), _T("unlockmsg.ems"), Create<CMsgComp>, false );
	XSLTranslate(action, _T("Ticket Properties"), _T("ticketproperties"), _T("ticketproperties.ems"), Create<CTicketProperties>, false );
	XSLTranslate(action, _T("Message Sources"), _T("messagesources"), _T("messagesources.ems"), Create<CMessageSources>, true );
	XSLTranslate(action, _T("Edit Message Source"), _T("messagesource"), _T("messagesource.ems"), Create<CMessageSources>, false );
	XSLTranslate(action, _T("Message Destinations"), _T("messagedestinations"), _T("messagedestinations.ems"), Create<CMessageDestinations>, true );
	XSLTranslate(action, _T("Edit Message Destination"), _T("messagedestination"), _T("messagedestination.ems"), Create<CMessageDestinations>, false );
	XSLTranslate(action, _T("Delivery Failures"), _T("deliveryfailures"), _T("deliveryfailures.ems"), Create<CMessageDestinations>, true );	
	XSLTranslate(action, _T("Custom Routing Rules"), _T("routingrules"), _T("routingrules.ems"), Create<CRoutingRules>, true );
	XSLTranslate(action, _T("Edit Routing Rule"), _T("routingrule"), _T("routingrule.ems"), Create<CRoutingRules>, false );
	XSLTranslate(action, _T("Processing Rules"), _T("processingrules"), _T("processingrules.ems"), Create<CProcessingRules>, true );
	XSLTranslate(action, _T("Edit Processing Rule"), _T("processingrule"), _T("processingrule.ems"), Create<CProcessingRules>, false );
	XSLTranslate(action, _T("DateFilters"), _T("datefilters"), _T("datefilters.ems"), Create<CDateFilters>, true );
	XSLTranslate(action, _T("Edit Date Filter"), _T("datefilter"), _T("datefilter.ems"), Create<CDateFilters>, false );

	XSLTranslate(action, _T("Ticket History"), _T("tickethistory"), _T("tickethistory.ems"), Create<CTicketHistory>, false, true );
	XSLTranslate(action, _T("Update Inbound Message"), _T("updateinbound"), _T("updateinbound.ems"), Create<CInboundMessage>, false, true );

	XSLTranslate(action, _T("Contact History"), _T("contacthistory"), _T("ticketlist.ems"), Create<CContactHistory>, true );
	XSLTranslate(action, _T("Logging Options"), _T("logconfig"), _T("logconfig.ems"), Create<CLogConfig>, true );
	XSLTranslate(action, _T("File System Logs"), _T("showlogs"), _T("showlogs.ems"), Create<CLogConfig>, true );
	XSLTranslate(action, _T("Database Logs"), _T("showdblogs"), _T("showdblogs.ems"), Create<CLogConfig>, true );
	XSLTranslate(action, _T("File System Log Details"), _T("showlogdetails"), _T("showlogdetails.ems"), Create<CLogConfig>, false );
	XSLTranslate(action, _T("Database Log Details"), _T("showdblogdetails"), _T("showdblogdetails.ems"), Create<CLogConfig>, false );
	XSLTranslate(action, _T("Debug Logs"), _T("showdebuglogs"), _T("showdebuglogs.ems"), Create<CLogConfig>, true );
	XSLTranslate(action, _T("Agent Audit"), _T("agentaudit"), _T("agentaudit.ems"), Create<CAgentAudit>, false );
	XSLTranslate(action, _T("TicketBoxes"), _T("ticketboxes"), _T("ticketboxes.ems"), Create<CTicketBox>, true );
	XSLTranslate(action, _T("Edit TicketBox"), _T("editticketbox"), _T("editticketbox.ems"), Create<CTicketBox>, true );
	XSLTranslate(action, _T("Form Dump"), _T("formdump"), _T("formdump.ems"), Create<CFormDump>, false );
	XSLTranslate(action, _T("Multiple Contacts"), _T("multiplecontacts"), _T("multiplecontacts.ems"), Create<CMultipleContacts>, false );
	XSLTranslate(action, _T("Contact Properties"), _T("contact"), _T("contact.ems"), Create<CContactDetails>, true );
	XSLTranslate(action, _T("Virus Scanning Settings"), _T("virusconfig"), _T("virusconfig.ems"), Create<CVirusConfig>, true );	
	XSLTranslate(action, _T("VisNetic AntiVirus Plug-in Registration"), _T("virusreg"), _T("virusreg.ems"), Create<CVirusConfig>, true );		
	XSLTranslate(action, _T("Access Rights"), _T("accessrights"), _T("accessrights.ems"), Create<CACLPage>, false );
	XSLTranslate(action, _T("Object Rights"), _T("objectrights"), _T("objectrights.ems"), Create<CACLPage>, false );
	XSLTranslate(action, _T("Custom Access Control"), _T("accesscontrol"), _T("accesscontrol.ems"), Create<CACLPage>, true );
	XSLTranslate(action, _T("Default Access Control"), _T("default_accesscontrol"), _T("default_accesscontrol.ems"), Create<CACLPage>, true );
	XSLTranslate(action, _T("IP Access Restrictions"), _T("ipaccess"), _T("ipaccess.ems"), Create<CIPRanges>, false );
	XSLTranslate(action, _T("IP Access Restrictions"), _T("agent_ipaccess"), _T("agent_ipaccess.ems"), Create<CIPRanges>, true );
	XSLTranslate(action, _T("IP Access Restrictions"), _T("group_ipaccess"), _T("group_ipaccess.ems"), Create<CIPRanges>, true );
	XSLTranslate(action, _T("IP Access Restrictions"), _T("ipranges"), _T("ipranges.ems"), Create<CIPRanges>, true );
	XSLTranslate(action, _T("Database Maintenance Settings"), _T("dbmaintenance"), _T("dbmaintenance.ems"), Create<CDBMaintenance>, true );
	XSLTranslate(action, _T("Database Settings"), _T("dbsettings"), _T("dbsettings.ems"), Create<CReportSysConfig>, true );
	XSLTranslate(action, _T("Database Maintenance Status"), _T("dbmaintenanceresults"), _T("dbmaintenanceresults.ems"), Create<CDBMaintenance>, false, true );	

	XSLTranslate(action, _T("Database Backups"), _T("dbbackups"), _T("dbbackups.ems"), Create<CDBMaintenance>, true );
	XSLTranslate(action, _T("Database Archives"), _T("dbarchives"), _T("dbarchives.ems"), Create<CDBMaintenance>, true );
	XSLTranslate(action, _T("SQL Query"), _T("sqlquery"), _T("sqlquery.ems"), Create<CDBMaintenance>, true );
	XSLTranslate(action, _T("SQL Query Results"), _T("sqlqueryresults"), _T("sqlqueryresults.ems"), Create<CDBMaintenance>, true );
	XSLTranslate(action, _T("Edit Database Archive"), _T("editdbarchive"), _T("editdbarchive.ems"), Create<CDBMaintenance>, true );
	XSLTranslate(action, _T("Alerts"), _T("alertconfiglist"), _T("alertconfiglist.ems"), Create<CAlertConfig>, true );
	XSLTranslate(action, _T("Alerts"), _T("alertconfig"), _T("alertconfig.ems"), Create<CAlertConfig>, false );
	XSLTranslate(action, _T("TicketBox Headers"), _T("ticketboxheaders"), _T("ticketboxheaders.ems"), Create<CTicketBoxHeaders>, true );
	XSLTranslate(action, _T("Edit TicketBox Header"), _T("editticketboxheader"), _T("editticketboxheader.ems"), Create<CTicketBoxHeaders>, true );
	XSLTranslate(action, _T("TicketBox Owners"), _T("ticketboxowners"), _T("ticketboxowners.ems"), Create<CTicketBoxOwners>, true );
	XSLTranslate(action, _T("Edit TicketBox Owner"), _T("editticketboxowner"), _T("editticketboxowner.ems"), Create<CTicketBoxOwners>, true );
	XSLTranslate(action, _T("Ticket Fields"), _T("ticketfields"), _T("ticketfields.ems"), Create<CTicketFields>, true );
	XSLTranslate(action, _T("Edit Ticket Field"), _T("editticketfield"), _T("editticketfield.ems"), Create<CTicketFields>, true );
	XSLTranslate(action, _T("Edit Columns"), _T("editcolumns"), _T("editcolumns.ems"), Create<CColumns>, true );
	XSLTranslate(action, _T("TocketBox Footers"), _T("ticketboxfooters"), _T("ticketboxfooters.ems"), Create<CTicketBoxFooters>, true );
	XSLTranslate(action, _T("Edit TicketBox Footer"), _T("editticketboxfooter"), _T("editticketboxfooter.ems"), Create<CTicketBoxFooters>, true );
	XSLTranslate(action, _T("Select NT User"), _T("ntagents"), _T("ntagents.ems"), Create<CAgents>, false );
	XSLTranslate(action, _T("Agents"), _T("agents"), _T("agents.ems"), Create<CAgents>, true );
	XSLTranslate(action, _T("Edit Agent"), _T("editagent"), _T("editagent.ems"), Create<CAgents>, false );
	XSLTranslate(action, _T("Agent Preferences"), _T("agentpreferences"), _T("agentpreferences.ems"), Create<CAgents>, true );	
	XSLTranslate(action, _T("Login Test"), _T("testlogin"), _T("testlogin.ems"), Create<CTestLogin>, false );

	XSLTranslate(action, _T("Login Test Results"), _T("testloginresults"), _T("testloginresults.ems"), Create<CTestLogin>, false, true );

	XSLTranslate(action, _T("Signatures"), _T("signatures"), _T("signatures.ems"), Create<CSignatures>, true );
	XSLTranslate(action, _T("Signatures"), _T("signatures_agent"), _T("signatures_agent.ems"), Create<CSignatures>, true );
	XSLTranslate(action, _T("Signatures"), _T("signatures_group"), _T("signatures_group.ems"), Create<CSignatures>, true );
	XSLTranslate(action, _T("Edit Signature"), _T("editsignaturepref"), _T("editsignaturepref.ems"), Create<CSignatures>, false );
	XSLTranslate(action, _T("Ticket Notes"), _T("ticketnotes"), _T("ticketnotes.ems"), Create<CTicketNotes>, false );
	XSLTranslate(action, _T("Update Ticket Note"), _T("updateticketnotes"), _T("updateticketnotes.ems"), Create<CTicketNotes>, false, true );

	XSLTranslate(action, _T("Print Ticket"), _T("printticket"), _T("printticket.ems"), Create<CTicketPrint>, false );
	XSLTranslate(action, _T("Print Ticket Select"), _T("printticketselect"), _T("printticketselect.ems"), Create<CTicketPrint>, false );
	
	XSLTranslate(action, _T("Contact Notes"), _T("contactnotes"), _T("contactnotes.ems"), Create<CContactNotes>, false );
	XSLTranslate(action, _T("VisNetic MailFlow Registration"), _T("registration"), _T("registration.ems"), Create<CRegistration>, true );
	XSLTranslate(action, _T("Deleted Items"), _T("deleteditems"), _T("deleteditems.ems"), Create<CTrash>, true );
	XSLTranslate(action, _T("Standard Responses"), _T("standardresponses"), _T("standardresponses.ems"), Create<CStdRespCategories>, false );
	XSLTranslate(action, _T("Standard Response Categories"), _T("stdrespcategories"), _T("stdrespcategories.ems"), Create<CStdRespCategories>, false );
	XSLTranslate(action, _T("Standard Response Menu"), _T("standardresponse_menu"), _T("standardresponse_menu.ems"), Create<CStdRespCategories>, false );
	XSLTranslate(action, _T("Edit Standard Response Category"), _T("editstdrespcategory"), _T("editstdrespcategory.ems"), Create<CStdRespCategories>, false );
	XSLTranslate(action, _T("Standard Response List"), _T("stdresponselist"), _T("stdresponselist.ems"), Create<CStdResponse>, false );

	XSLTranslate(action, _T("View Standard Response"), _T("stdresponse"), _T("stdresponse.ems"), Create<CStdResponse>, false, true );

	XSLTranslate(action, _T("Edit Standard Response"), _T("editstdresponse"), _T("editstdresponse.ems"), Create<CStdResponse>, false );
	XSLTranslate(action, _T("Standard Response Categories"), _T("liststdrespcategories"), _T("liststdrespcategories.ems"), Create<CStdRespCategories>, false );
	XSLTranslate(action, _T("Active Sessions"), _T("sessioninfo"), _T("sessioninfo.ems"), Create<CSessionInfo>, true );
	XSLTranslate(action, _T("Spell Check"), _T("spellcheck"), _T("spellcheck.ems"), Create<CSpellCheck>, false );
	XSLTranslate(action, _T("Logout"), _T("logout"), _T("logout.ems"), Create<CLogout>, false );
	XSLTranslate(action, _T("Groups"), _T("groups"), _T("groups.ems"), Create<CGroups>, true );
	XSLTranslate(action, _T("Edit Group"), _T("editgroup"), _T("editgroup.ems"), Create<CGroups>, true );
	XSLTranslate(action, _T("Contact Groups"), _T("contactgroups"), _T("contactgroups.ems"), Create<CContactGroups>, true );
	XSLTranslate(action, _T("Edit Contact Group"), _T("editcontactgroup"), _T("editcontactgroup.ems"), Create<CContactGroups>, true );
	XSLTranslate(action, _T("About VisNetic MailFlow"), _T("about"), _T("about.ems"), Create<CAboutPage>, false );
	XSLTranslate(action, _T("Contact Properties"), _T("personaldatatypes"), _T("personaldatatypes.ems"), Create<CPersonalDataTypes>, true );
	XSLTranslate(action, _T("Ticket Categories"), _T("ticketcategories"), _T("ticketcategories.ems"), Create<CTicketCategories>, true );
	XSLTranslate(action, _T("Edit Ticket Category"), _T("editticketcategory"), _T("editticketcategory.ems"), Create<CTicketCategories>, true );
	XSLTranslate(action, _T("Edit Property"), _T("editpersonaldatatype"), _T("editpersonaldatatype.ems"), Create<CPersonalDataTypes>, true );
	XSLTranslate(action, _T("Ticket Search"), _T("ticketsearch"), _T("ticketsearch.ems"), Create<CTicketSearch>, true );
	XSLTranslate(action, _T("Ticket Search Results"), _T("ticketsearchresults"), _T("ticketsearchresults.ems"), Create<CTicketSearch>, true );
	XSLTranslate(action, _T("Contact Search"), _T("contactsearch"), _T("contactsearch.ems"), Create<CContactSearch>, true );
	XSLTranslate(action, _T("Contact Search Results"), _T("contactsearchresults"), _T("contactsearchresults.ems"), Create<CContactSearch>, true );
	XSLTranslate(action, _T("Auto Messages"), _T("automessages"), _T("automessages.ems"), Create<CAutoMessages>, true );
	XSLTranslate(action, _T("Edit Auto Message"), _T("editautomessage"), _T("editautomessage.ems"), Create<CAutoMessages>, true );
	XSLTranslate(action, _T("Auto Responses"), _T("autoresponses"), _T("autoresponses.ems"), Create<CAutoResponses>, true );
	XSLTranslate(action, _T("Edit Auto Response"), _T("editautoresponse"), _T("editautoresponse.ems"), Create<CAutoResponses>, true );
	XSLTranslate(action, _T("Age Alerts"), _T("agealerts"), _T("agealerts.ems"), Create<CAgeAlerts>, true );
	XSLTranslate(action, _T("Edit Age Alert"), _T("editagealert"), _T("editagealert.ems"), Create<CAgeAlerts>, true );
	XSLTranslate(action, _T("Watermark Alerts"), _T("wmalerts"), _T("wmalerts.ems"), Create<CWaterMarkAlerts>, true );
	XSLTranslate(action, _T("Edit Watermark Alert"), _T("editwmalert"), _T("editwmalert.ems"), Create<CWaterMarkAlerts>, true );
	XSLTranslate(action, _T("My Alerts"), _T("myalerts"), _T("myalerts.ems"), Create<CMyAlerts>, true );
	XSLTranslate(action, _T("Edit Alert"), _T("editmyalert"), _T("editmyalert.ems"), Create<CMyAlerts>, true );
	XSLTranslate(action, _T("Configuration Report"), _T("reportsysconfig"), _T("reportsysconfig.ems"), Create<CReportSysConfig>, true );
	XSLTranslate(action, _T("Effective Rights"), _T("effectiverights"), _T("effectiverights.ems"), Create<CReportSysConfig>, true );
	XSLTranslate(action, _T("Move Message"), _T("changeticket"), _T("changeticket.ems"), Create<CChangeTicket>, false );
	XSLTranslate(action, _T("VisNetic MailServer Integration"), _T("vmsconfig"), _T("vmsconfig.ems"), Create<CVMSIntegration>, true );
	XSLTranslate(action, _T("VisNetic MailServer Users"), _T("vmsagents"), _T("vmsagents.ems"), Create<CVMSIntegration>, false );
	XSLTranslate(action, _T("Client Software"), _T("clientsoft"), _T("clientsoft.ems"), Create<CClientSoft>, true );

	XSLTranslate(action, _T("Upload Progress"), _T("uploadprogress"), _T("uploadprogress.ems"), Create<CUploadProgress>, false, true );

	XSLTranslate(action, _T("Other Security Restrictions"), _T("securitysettings"), _T("securitysettings.ems"), Create<CSecuritySettings>, true );
	XSLTranslate(action, _T("New Ticket"), _T("newticket"), _T("newticket.ems"), Create<CNewTicket>, false );
	XSLTranslate(action, _T("Select Contact"), _T("selectcontact"), _T("selectcontact.ems"), Create<CNewTicket>, false );
	XSLTranslate(action, _T("Custom Dictionary"), _T("customdictionary"), _T("customdictionary.ems"), Create<CCustomDictionary>, true );
	XSLTranslate(action, _T("TicketBox Views"), _T("PreferenceTicketboxes"), _T("PreferenceTicketboxes.ems"), Create<CPreferenceTicketboxes>, false );
	XSLTranslate(action, _T("Agents Views"), _T("PreferenceAgents"), _T("PreferenceAgents.ems"), Create<CPreferenceAgents>, false );

	XSLTranslate(action, _T("Ticket History"), _T("ticketactions") , _T("ticketactions.ems"), Create<CTicketActions>, false, true );

	XSLTranslate(action, _T("Quarantined Items"), _T("quarantine"), _T("quarantine.ems"), Create<CQuarantinePage>, true );
	XSLTranslate(action, _T("Default Routing Rule"), _T("rroptions"), _T("rroptions.ems"), Create<CRoutingRules>, false );
	XSLTranslate(action, _T("Note Attachments"), _T("noteattachments"), _T("noteattachments.ems"), Create<CNoteAttach>, false );
	XSLTranslate(action, _T("Message Attachments"), _T("newmessage_attachments"), _T("newmessage_attachments.ems"), Create<CMsgCompAttach>, false );
	XSLTranslate(action, _T("Message Attachments"), _T("newmessageattach"), _T("newmessageattach.ems"), Create<CMsgCompAttach>, false );
	XSLTranslate(action, _T("Message Attachments"), _T("attachments"), _T("attachments.ems"), Create<CMsgAttach>, false );
	XSLTranslate(action, _T("System Email Address"), _T("systememailaddress"), _T("systememailaddress.ems"), Create<CSystemEmailAddress>, false );
	XSLTranslate(action, _T("Download SVG Plug-in"), _T("download_svg"), _T("download_svg.ems"), Create<CDownloadSVG>, false );
	XSLTranslate(action, _T("Download Acrobat Reader"), _T("download_acrobat"), _T("download_acrobat.ems"), Create<CDownloadAcrobat>, false );
	XSLTranslate(action, _T("Download 3CX Plug-in"), _T("download_3cxplugin"), _T("download_3cxplugin.ems"), Create<CDownload3cxPlugin>, false );
	XSLTranslate(action, _T("MailFlow"), _T("mailflow"), _T("mailflow.ems"), Create<CMainFrame>, false );
	XSLTranslate(action, _T("MailFlow Help"), _T("help"), _T("help.ems"), Create<CHelpFrame>, false );	
	XSLTranslate(action, _T("Agent Preferences"), _T("apreferences"), _T("apreferences.ems"), Create<CAgents>, true ); // Don't forget about
	XSLTranslate(action, _T("Left Menu"), _T("leftmenu"), _T("leftmenu.ems"), Create<CTicketBoxView>, false );
	XSLTranslate(action, _T("VisNetic MailFlow"), _T("vmf"), _T("vmf.ems"), Create<CTicketBoxView>, false );
	XSLTranslate(action, _T("Client"), _T("client"), _T("client.ems"), Create<CClient>, false, true );
	XSLTranslate(action, _T("Preferences"), _T("preferencegeneral"), _T("preferencegeneral.ems"), Create<CAgents>, false ); // the change in agents.cpp!
	XSLTranslate(action, _T("Log Entry Details"), _T("showlogentry"), _T("showlogentry.ems"), NULL, false );
	XSLTranslate(action, _T("Signature Preferences"), _T("preferencesignatures"), _T("preferencesignatures.ems"), Create<CAgents>, false );
	XSLTranslate(action, _T("Set Status"), _T("setstatus"), _T("setstatus.ems"), Create<CAgents>, true );

	XSLTranslate(action, _T("CustomSettings"), _T("customsettings"), _T("customsettings.ems"), Create<CustomSettings>, false );
	XSLTranslate(action, _T("BulkSettings"), _T("bulksettings"), _T("bulksettings.ems"), Create<CustomSettings>, false );
	XSLTranslate(action, _T("VoipConfig"), _T("voipconfig"), _T("voipconfig.ems"), Create<VoipConfig>, false );
	XSLTranslate(action, _T("Unlock Ticket"), _T("unlockticket"), _T("unlockticket.ems"), Create<UnlockTicketPage>, true );
	XSLTranslate(action, _T("OfficeHours"), _T("officehours"), _T("officehours.ems"), Create<OfficeHours>, false );
	
	// reports
	XSLTranslate(action, _T(""), _T("reportcompare"), _T("report.ems"), Create<CReportCompare>, true );
	XSLTranslate(action, _T(""), _T("reportinboundmsgs"), _T("report.ems"), Create<CReportInboundMsgs>, true );
	XSLTranslate(action, _T(""), _T("reportoutboundmsgs"), _T("report.ems"), Create<CReportOutboundMsgs>, true );
	XSLTranslate(action, _T(""), _T("reportticketdist"), _T("report.ems"), Create<CReportTicketDist>, true );  
	XSLTranslate(action, _T(""), _T("reportticketboxdetails"), _T("reportticketboxdetails.ems"), Create<CTicketBoxDetails>, true );  
	XSLTranslate(action, _T(""), _T("reportticketcountsum"), _T("reportticketcountsum.ems"), Create<CTicketCountSum>, true );  
	XSLTranslate(action, _T(""), _T("reportticketsopen"), _T("reportticketsopen.ems"), Create<COpenTickets>, true );  
	XSLTranslate(action, _T(""), _T("reportticketdisp"), _T("report.ems"), Create<CReportTicketDisp>, true );  
	XSLTranslate(action, _T(""), _T("reportstdresp"), _T("report.ems"), Create<CReportStdResp>, true );
	XSLTranslate(action, _T(""), _T("reportstdusage"), _T("report.ems"), Create<CReportStdRespUsage>, true );	
	XSLTranslate(action, _T(""), _T("reportresptime"), _T("report.ems"), Create<CReportRespTime>, true );
	XSLTranslate(action, _T(""), _T("reportresolvetime"), _T("report.ems"), Create<CReportResolveTime>, true );
	XSLTranslate(action, _T(""), _T("reportbusy"), _T("report.ems"), Create<CReportBusy>, true );
	XSLTranslate(action, _T(""), _T("reportopenticketage"), _T("report.ems"), Create<CReportOpenTicketAge>, true );
	XSLTranslate(action, _T(""), _T("reporthistory"), _T("reporthistory.ems"), Create<CReportHistory>, true );
	XSLTranslate(action, _T(""), _T("printreport"), _T("printreport.ems"), Create<CPrintReport>, false );
	XSLTranslate(action, _T(""), _T("schedulereport"), _T("schedulereport.ems"), Create<CScheduleReport>, true );
	XSLTranslate(action, _T(""), _T("scheduledreports"), _T("scheduledreports.ems"), Create<CScheduleReport>, true );
	XSLTranslate(action, _T(""), _T("scheduledadminreports"), _T("scheduledadminreports.ems"), Create<CScheduleReport>, true );
	XSLTranslate(action, _T(""), _T("voipservers"), _T("voipservers.ems"), Create<CVoipServers>, true );
	XSLTranslate(action, _T(""), _T("voipserver"), _T("voipserver.ems"), Create<CVoipServers>, true );
	XSLTranslate(action, _T(""), _T("voipdialingcodes"), _T("voipdialingcodes.ems"), Create<CVoipDialingCodes>, true );
	XSLTranslate(action, _T(""), _T("voipdialingcode"), _T("voipdialingcode.ems"), Create<CVoipDialingCodes>, true );
	XSLTranslate(action, _T(""), _T("voipextensions"), _T("voipextensions.ems"), Create<CVoipExtensions>, true );
	XSLTranslate(action, _T(""), _T("voipextension"), _T("voipextension.ems"), Create<CVoipExtensions>, true );
	XSLTranslate(action, _T(""), _T("voipcall"), _T("voipcall.ems"), Create<CVoipCall>, true );
	XSLTranslate(action, _T(""), _T("viewresults"), _T("viewresults.ems"), Create<CViewResults>, true );
	XSLTranslate(action, _T(""), _T("viewreportdetails"), _T("viewreportdetails.ems"), Create<CViewResults>, true );
	XSLTranslate(action, _T(""), _T("servers"), _T("servers.ems"), Create<CServers>, true );
	XSLTranslate(action, _T(""), _T("server"), _T("server.ems"), Create<CServers>, true );
	XSLTranslate(action, _T(""), _T("regextest"), _T("regextest.ems"), Create<CRegExTest>, true );
	XSLTranslate(action, _T(""), _T("ticketlinks"), _T("ticketlinks.ems"), Create<CTicketLinks>, true );
	
	SendFile(action, _T("download"), Create<CDownload>);
			
	// not associated with an object
	XSLTranslate(action, _T("Color Select"), _T("colorselect"), _T("colorselect.ems"), NULL, false, true );
	XSLTranslate(action, _T("View Headers"), _T("viewheaders"), _T("viewheaders.ems"), NULL, false );
	XSLTranslate(action, _T("View Source"), _T("viewsource"), _T("viewsource.ems"), NULL, false );
	XSLTranslate(action, _T("Delete Standard Response Category"), _T("stdeletecat"), _T("stdeletecat.ems"), NULL, false );
	XSLTranslate(action, _T("Spell Check"), _T("spelltemp"), _T("spelltemp.ems"), NULL, false );
	XSLTranslate(action, _T("Print Message"), _T("newmsg_print"), _T("newmsg_print.ems"), NULL, false );
	XSLTranslate(action, _T("Delete TicketBox"), _T("ticketboxdelete"), _T("ticketboxdelete.ems"), NULL, false );
	XSLTranslate(action, _T("Contact Menu"), _T("contactmenu"), _T("contactmenu.ems"), NULL, false );
	XSLTranslate(action, _T("Contact SendTo"), _T("contactsendto"), _T("contactsendto.ems"), NULL, false );
	XSLTranslate(action, _T("Contact Frame"), _T("contactframe"), _T("contactframe.ems"), NULL, true );
	XSLTranslate(action, _T("Modal Frame"), _T("modalframe"), _T("modalframe.ems"), NULL, false );
	XSLTranslate(action, _T("VisNetic MailFlow"), _T("nmsgframe"), _T("nmsgframe.ems"), NULL, false );
	XSLTranslate(action, _T("VisNetic MailFlow"), _T("vmsgframe"), _T("vmsgframe.ems"), NULL, false );
	XSLTranslate(action, _T("Search Standard Responses"), _T("searchstandardresponse"), _T("searchstandardresponse.ems"), NULL, false);
	XSLTranslate(action, _T("VOIP Call Status"), _T("voipcallstatus"), _T("voipcallstatus.ems"), NULL, false );
	XSLTranslate(action, _T("Session Tickler"), _T("sessiontickler"), _T("sessiontickler.ems"), NULL, false );	
	XSLTranslate(action, _T("MailFlow Help"), _T("lefthelp"), _T("lefthelp.ems"), NULL, false );	
	XSLTranslate(action, _T("Manage Subscriptions"), _T("unsub"), _T("unsub.ems"), Create<CUnsub>, false );
	XSLTranslate(action, _T("Manage Subscriptions"), _T("unsub2"), _T("unsub2.ems"), Create<CUnsub>, false );
	XSLTranslate(action, _T("Manage Subscriptions"), _T("unsub3"), _T("unsub3.ems"), Create<CUnsub>, false );

	// support for demo build
#if defined(DEMO_VERSION)
	XSLTranslate(action, _T("Create MailFlow Account"), _T("demo"), _T("demo.ems"), Create<CDemo>, false );
#endif
}

void CURLMap::AddToMap( TCHAR* szPage, CURLAction& action )
{
	TCHAR szPageName[MAX_PAGE_NAME_LENGTH];
	TCHAR *p,*q,*e;
	
	// Convert page names to upper case and limit the length
	p = szPage;
	q = szPageName;
	e = szPageName + MAX_PAGE_NAME_LENGTH - 1;

	while( *p != _T('\0') && q < e )
	{
		*q++ = toupper( *p++ );
	}

	// zero terminate
	*q = 0;

	// Put it in the map
	m_Map[szPageName] = action;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Search the map for an action matching the requested file.
// The file extension is removed prior to the comparison
// 
////////////////////////////////////////////////////////////////////////////////
void CURLMap::GetAction( CISAPIData* pISAPIData, CURLAction& action )
{
	//DEBUGPRINT(_T("CURLMap::GetAction - %s\n"), pISAPIData->m_sPage.c_str());
	TCHAR szPageName[MAX_PAGE_NAME_LENGTH];
	TCHAR *p,*q,*e;

	p = (TCHAR*) pISAPIData->m_sPage.c_str();
	
	q = szPageName;
	e = szPageName + MAX_PAGE_NAME_LENGTH - 1;
	
	// advance past the leading '/'
	if( *p == _T('/') )
	{
		p++;
	}

	// copy up to the first period, translate case, and limit length
	while( *p != _T('.') && *p != _T('\0') && q < e )
	{
		*q++ = toupper( *p++ );
	}

	// zero terminate
	*q = 0;
	
	// search the map
	map<tstring,CURLAction>::iterator iter = m_Map.find( szPageName );

	if( iter != m_Map.end() )
		action = (iter->second);

	// always send back XML for XML posts
	if ( pISAPIData->GetXMLPost() )
		action.m_nAction = CURLAction::SEND_XML;
}
