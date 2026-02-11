#pragma once

class MailFlowServer
{
public:
	MailFlowServer(void);
	~MailFlowServer(void);
	static bool EnsurePathExists(tstring sPath);
};
