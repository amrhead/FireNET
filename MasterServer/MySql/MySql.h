/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef _MySql_
#define _MySql_

class CMySql 
{
public:
	CMySql(void){}
	~CMySql(void){}

	// Connecting to MySql DataBase
	int MySqlConnect();

	// Sql request
	int SendSqlCommand(char *sql);

	// User autorization
	SClient Logining(SOCKET ClientSocket, const char *login, const char *password);

	// User registration
	void Registration(SOCKET ClientSocket, const char *login, const char *password, const char *nickname);

	// Get user account information from MySql DataBase
	SClient GetAccountInfo(SOCKET ClientSocket, const char *login);
};

#endif