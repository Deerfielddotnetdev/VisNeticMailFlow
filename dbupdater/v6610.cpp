#include "prehead.h"
#include "main.h"

extern BOOL   bIsTrans;
extern BOOL   bIsDBConn;
extern int    maxObjectID;

int V6610(CODBCQuery& m_query,CDBConn& dbConn,reg::Key& rkMailFlow, TCHAR* wDsn)
{
	SQLRETURN	retSQL = 0;
	maxObjectID = 0;

	//Add Row to ServerParameters Table
	if(g_showResults != 0)
	{
		_tcout << _T("Adding Row to ServerParameters table") << endl;
	}

	g_logFile.Write(_T("Adding Row to ServerParameters table"));

	// Adding rows to Server Parameter table
	retSQL = dbConn.ExecuteSQL(_T("SET IDENTITY_INSERT ServerParameters ON ")
							   _T("INSERT INTO ServerParameters (ServerParameterID,Description,DataValue) VALUES (148,'Time Zone ID','0') ")							   
							   _T("SET IDENTITY_INSERT ServerParameters OFF"));
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add TimeZones Table
	if(g_showResults != 0)
	{
		_tcout << _T("Create TimeZones table") << endl;
	}

	g_logFile.Write(_T("Create TimeZones table"));

	retSQL = dbConn.ExecuteSQL(_T("CREATE TABLE [dbo].[TimeZones]( ")
	_T("[TimeZoneID] [int] IDENTITY(1,1) NOT NULL, ")
	_T("[DisplayName] [varchar](100) NOT NULL, ")
	_T("[StandardName] [varchar](100) NOT NULL, ")
	_T("[UTCOffset] [int] NOT NULL , ")
	_T("CONSTRAINT [PK_TimeZones] PRIMARY KEY CLUSTERED ([TimeZoneID])) "));
	
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}

	//Add Rows to TimeZones Table
	if(g_showResults != 0)
	{
		_tcout << _T("Add rows to TimeZones table") << endl;
	}

	g_logFile.Write(_T("Add rows to TimeZones table"));
	
	retSQL = dbConn.ExecuteSQL( _T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+00:00) Casablanca','Morocco Standard Time','0') ")
							   	_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+00:00) Dublin, Edinburgh, Lisbon, London','GMT Standard Time','0') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+00:00) Monrovia, Reykjavik','Greenwich Standard Time','0') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna','W. Europe Standard Time','-60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague','Central Europe Standard Time','-60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+01:00) Brussels, Copenhagen, Madrid, Paris','Romance Standard Time','-60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+01:00) Sarajevo, Skopje, Warsaw, Zagreb','Central European Standard Time','-60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+01:00) West Central Africa','W. Central Africa Standard Time','-60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+01:00) Windhoek','Namibia Standard Time','-60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Amman','Jordan Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Athens, Bucharest','GTB Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Beirut','Middle East Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Cairo','Egypt Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Chisinau','E. Europe Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Damascus','Syria Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Gaza, Hebron','West Bank Gaza Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Harare, Pretoria','South Africa Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius','FLE Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Istanbul','Turkey Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Jerusalem','Jerusalem Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Kaliningrad','Russia TZ 1 Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+02:00) Tripoli','Libya Standard Time','-120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+03:00) Baghdad','Arabic Standard Time','-180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+03:00) Kuwait, Riyadh','Arab Standard Time','-180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+03:00) Minsk','Belarus Standard Time','-180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+03:00) Moscow, St. Petersburg, Volgograd','Russia TZ 2 Standard Time','-180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+03:00) Nairobi','E. Africa Standard Time','-180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+03:30) Tehran','Iran Standard Time','-210') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Abu Dhabi, Muscat','Arabian Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Astrakhan, Ulyanovsk','Astrakhan Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Baku','Azerbaijan Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Izhevsk, Samara','Russia TZ 3 Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Port Louis','Mauritius Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Tbilisi','Georgian Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:00) Yerevan','Caucasus Standard Time','-240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+04:30) Kabul','Afghanistan Standard Time','-270') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+05:00) Ashgabat, Tashkent','West Asia Standard Time','-300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+05:00) Ekaterinburg','Russia TZ 4 Standard Time','-300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+05:00) Islamabad, Karachi','Pakistan Standard Time','-300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+05:30) Chennai, Kolkata, Mumbai, New Delhi','India Standard Time','-330') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+05:30) Sri Jayawardenepura','Sri Lanka Standard Time','-330') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+05:45) Kathmandu','Nepal Standard Time','-345') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+06:00) Astana','Central Asia Standard Time','-360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+06:00) Dhaka','Bangladesh Standard Time','-360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+06:00) Novosibirsk','Russia TZ 5 Standard Time','-360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+06:30) Yangon (Rangoon)','Myanmar Standard Time','-390') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+07:00) Bangkok, Hanoi, Jakarta','SE Asia Standard Time','-420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+07:00) Barnaul, Gorno-Altaysk','Altai Standard Time','-420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+07:00) Hovd','W. Mongolia Standard Time','-420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+07:00) Krasnoyarsk','Russia TZ 6 Standard Time','-420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+07:00) Tomsk','Tomsk Standard Time','-420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:00) Beijing, Chongqing, Hong Kong, Urumqi','China Standard Time','-480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:00) Irkutsk','Russia TZ 7 Standard Time','-480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:00) Kuala Lumpur, Singapore','Malay Peninsula Standard Time','-480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:00) Perth','W. Australia Standard Time','-480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:00) Taipei','Taipei Standard Time','-480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:00) Ulaanbaatar','Ulaanbaatar Standard Time','-480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:30) Pyongyang','North Korea Standard Time','-510') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+08:45) Eucla','Aus Central W. Standard Time','-525') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+09:00) Chita','Transbaikal Standard Time','-540') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+09:00) Osaka, Sapporo, Tokyo','Tokyo Standard Time','-540') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+09:00) Seoul','Korea Standard Time','-540') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+09:00) Yakutsk','Russia TZ 8 Standard Time','-540') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+09:30) Adelaide','Cen. Australia Standard Time','-570') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+09:30) Darwin','AUS Central Standard Time','-570') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+10:00) Brisbane','E. Australia Standard Time','-600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+10:00) Canberra, Melbourne, Sydney','AUS Eastern Standard Time','-600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+10:00) Guam, Port Moresby','West Pacific Standard Time','-600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+10:00) Hobart','Tasmania Standard Time','-600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+10:00) Vladivostok','Russia TZ 9 Standard Time','-600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+10:30) Lord Howe Island','Lord Howe Standard Time','-630') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+11:00) Bougainville Island','Bougainville Standard Time','-660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+11:00) Chokurdakh','Russia TZ 10 Standard Time','-660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+11:00) Magadan','Magadan Standard Time','-660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+11:00) Norfolk Island','Norfolk Standard Time','-660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+11:00) Sakhalin','Sakhalin Standard Time','-660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+11:00) Solomon Is., New Caledonia','Central Pacific Standard Time','-660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+12:00) Anadyr, Petropavlovsk-Kamchatsky','Russia TZ 11 Standard Time','-720') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+12:00) Auckland, Wellington','New Zealand Standard Time','-720') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+12:00) Coordinated Universal Time+12','UTC+12','-720') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+12:00) Fiji','Fiji Standard Time','-720') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+12:00) Petropavlovsk-Kamchatsky - Old','Kamchatka Standard Time','-720') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+12:45) Chatham Islands','Chatham Islands Standard Time','-765') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+13:00) Nuku''alofa','Tonga Standard Time','-780') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+13:00) Samoa','Samoa Standard Time','-780') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC+14:00) Kiritimati Island','Line Islands Standard Time','-840') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-01:00) Azores','Azores Standard Time','60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-01:00) Cabo Verde Is.','Cabo Verde Standard Time','60') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-02:00) Coordinated Universal Time-02','UTC-02','120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-02:00) Mid-Atlantic - Old','Mid-Atlantic Standard Time','120') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Araguaina','Tocantins Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Brasilia','E. South America Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Cayenne, Fortaleza','SA Eastern Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) City of Buenos Aires','Argentina Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Greenland','Greenland Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Montevideo','Montevideo Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Saint Pierre and Miquelon','Saint Pierre Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:00) Salvador','Bahia Standard Time','180') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-03:30) Newfoundland','Newfoundland Standard Time','210') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Asuncion','Paraguay Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Atlantic Time (Canada)','Atlantic Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Caracas','Venezuela Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Cuiaba','Central Brazilian Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Georgetown, La Paz, Manaus, San Juan','SA Western Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Santiago','Pacific SA Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-04:00) Turks and Caicos','Turks and Caicos Standard Time','240') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-05:00) Bogota, Lima, Quito, Rio Branco','SA Pacific Standard Time','300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-05:00) Chetumal','Eastern Standard Time (Mexico)','300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-05:00) Eastern Time (US & Canada)','Eastern Standard Time','300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-05:00) Haiti','Haiti Standard Time','300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-05:00) Havana','Cuba Standard Time','300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-05:00) Indiana (East)','US Eastern Standard Time','300') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-06:00) Central America','Central America Standard Time','360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-06:00) Central Time (US & Canada)','Central Standard Time','360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-06:00) Easter Island','Easter Island Standard Time','360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-06:00) Guadalajara, Mexico City, Monterrey','Central Standard Time (Mexico)','360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-06:00) Saskatchewan','Canada Central Standard Time','360') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-07:00) Arizona','US Mountain Standard Time','420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-07:00) Chihuahua, La Paz, Mazatlan','Mountain Standard Time (Mexico)','420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-07:00) Mountain Time (US & Canada)','Mountain Standard Time','420') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-08:00) Baja California','Pacific Standard Time (Mexico)','480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-08:00) Coordinated Universal Time-08','UTC-08','480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-08:00) Pacific Time (US & Canada)','Pacific Standard Time','480') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-09:00) Alaska','Alaskan Standard Time','540') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-09:00) Coordinated Universal Time-09','UTC-09','540') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-09:30) Marquesas Islands','Marquesas Standard Time','570') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-10:00) Aleutian Islands','Aleutian Standard Time','600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-10:00) Hawaii','Hawaiian Standard Time','600') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-11:00) Coordinated Universal Time-11','UTC-11','660') ")
								_T("INSERT INTO TimeZones (DisplayName,StandardName,UTCOffset) VALUES ('(UTC-12:00) International Date Line West','Dateline Standard Time','720') "));
							  
	if(!SQL_SUCCEEDED(retSQL))
	{
		return ErrorOpt(dbConn,rkMailFlow);
	}
	
	return 0;
}