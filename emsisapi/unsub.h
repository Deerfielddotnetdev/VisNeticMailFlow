#pragma once
#include "xmldataclass.h"

class CUnsub :
	public CXMLDataClass
{
public:
	CUnsub(CISAPIData& ISAPIData );
	virtual ~CUnsub(void);

	int virtual Run( CURLAction& action );

private:
	void UnsubGroup(int nContactID, int nGroupID);
	void SubGroup(int nContactID, int nGroupID);
	void GetGroups(int nContactID);
	void ConfirmCode(tstring sCode);

	int nContactID;
	int nOrigGroupID;
	int nGroupID;
	tstring sEmail;
	bool bUnsub;
	bool bSub;
	bool bConfirmed;
	CEMSString sIDs;
	CEMSString szEmail;
};
