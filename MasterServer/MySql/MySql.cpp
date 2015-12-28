/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 15.08.2014   11:16 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------
p.s. SORRY, BUT THIS CODE NEED UPDATE!!!!
*************************************************************************/
#include "StdAfx.h"
#include <mysql.h>

#include "System\md5.h"

MYSQL *connection;
MYSQL_RES *result = 0; 
MYSQL_ROW row;	

int CMySql::MySqlConnect()
{ 
	connection = mysql_init(0);

	Log(LOG_INFO, "Connection to MySql database...");

	if (!connection)
	{
		Log(LOG_ERROR, "Can't create MySQL-descriptor!!!");
		return 0;
	}

	for (int counter=0; counter <= atoi(gEnv->pSettings->GetConfigValue("MySql","mysql_number_connection_attempts")); counter++)
	{
		if (!mysql_real_connect(connection,
			gEnv->pSettings->GetConfigValue("MySql","mysql_host"),     // <-- имя хоста  mysql базы | <-- mysql host
			gEnv->pSettings->GetConfigValue("MySql","mysql_username"), // <-- имя пользователя  mysql базы  | <-- mysql username
			gEnv->pSettings->GetConfigValue("MySql","mysql_password"), // <-- пароль  mysql базы | <-- mysql password
			gEnv->pSettings->GetConfigValue("MySql","mysql_database"),  // <-- имя mysql базы | <-- mysql database name
			atoi(gEnv->pSettings->GetConfigValue("MySql","mysql_port")), // <-- порт | <-- mysql port
			0,0))
		{ 
			Log(LOG_ERROR, "MySql Connection error!!!");
			Log(LOG_INFO, "MySql Retrying connect...");
		}
		else
		{
			Log(LOG_INFO, "MySql Connection success!");
			return 1;
		}
	}
	return 0;
}

int CMySql::SendSqlCommand(char *sql)
{
	gEnv->sqlCounter++;

	Log(LOG_DEBUG, "Send Sql command '%s'", sql);

	// Проверка состояния подключения к mysql , если равно нулю - подключение активно
	// Checking the connection to the mysql, if zero - the connection is active
	if(mysql_ping(connection)!=0) 
	{
		Log(LOG_WARNING,"MySql not connected!");

		MySqlConnect();
		return 0;
	}
	else
	{
		if (mysql_query(connection,sql))
		{
			Log(LOG_ERROR,"Impossible to conduct request!!!");
			return 0;
		}

		result = mysql_store_result(connection);

		if (!result)
		{
			Log(LOG_ERROR,"Retrieving query results will crash!!!");

			return -1;
		}
		else
		{
			row = mysql_fetch_row(result);
			return 1;
		}
	}
}

SClient CMySql::Logining(SOCKET ClientSocket, const char *login,  const char *password)
{
	SClient Player;
	char query[256];

	// При необходимости заменить запрос на свой
	// If necessary, replace request for their
	sprintf(query, "SELECT * FROM %s WHERE Email='%s'" , gEnv->pSettings->GetConfigValue("MySql","mysql_table_name"), login);

	if(SendSqlCommand(query)>0)
	{
		if(row>0)
		{
			Log(LOG_DEBUG, "E-mail <%s> is found!", login);

			// row[7] - ban status, if row[7] = 1 - player banned
			if(strcmp(row[7],"1"))
			{
				Log(LOG_DEBUG, "Password checking ...");

				if(strcmp(row[2], password)==0)
				{
					Log(LOG_DEBUG, "Password correct!");

					// Block dual autorization
					for (auto it = gEnv->pServer->vClients.begin(); it != gEnv->pServer->vClients.end(); ++it)
					{
						/*
						if(pId == block)
						{
							Log(LOG_WARNING, "Attempt dual autorization from : '%s':%s", row[3], row[0]);

							gEnv->pRsp->SendMsg(ClientSocket,"DualAuth","ServerResult");
							return Player;
						}
						*/
					}

					//gEnv->pRsp->SendMsg(ClientSocket,"PasswordCorrect","ServerResult");

					//Player.playerID = atoi(row[0]);
					//Player.nickName = row[3];
					//Player.level = atoi(row[4]);
					//Player.gameCash = atoi(row[5]);
					//Player.realCash = atoi(row[6]);
					//Player.banStatus = atoi(row[6]);

					// Отсылаем информацию о аккаунте и сервере
					// Send information on the account and server
					//gEnv->pRsp->SendAccountInfo(ClientSocket,Player.playerID,Player.nickName.c_str(),Player.level,Player.gameCash,Player.realCash);

					gEnv->aClients++;
					return Player;

				}
				else
				{
					Log(LOG_DEBUG,"Password incorrect!");

					//gEnv->pRsp->SendMsg(ClientSocket,"PasswordIncorrect","ServerResult");

					return Player;
				}
			}
			else
			{

				Log(LOG_DEBUG, "Account <%d> blocked!", atoi(row[0]));

				//gEnv->pRsp->SendMsg(ClientSocket,"@account_blocked","ErrorMsg");

				return Player;
			}
		}
		else
		{
			Log(LOG_DEBUG,"E-mail <%s> not found!", login);

			//gEnv->pRsp->SendMsg(ClientSocket,"LoginNotFound","ServerResult");

			return Player;
		}
	}
	else
	{
		//gEnv->pRsp->SendMsg(ClientSocket,"@server_error","ServerResult");
		return Player;
	}

	return Player;
}

void CMySql::Registration(SOCKET ClientSocket, const char *login, const char *password, const char *nickname)
{
	char check_login[256];
	char check_nickname[256];
	char register_query[256];

	// При необходимости заменить запросы на свои
	// If necessary, replace requests for their
	sprintf(check_login, "SELECT * FROM %s WHERE Email='%s'", gEnv->pSettings->GetConfigValue("MySql","mysql_table_name"), login);
	sprintf(check_nickname, "SELECT * FROM %s WHERE NickName='%s'", gEnv->pSettings->GetConfigValue("MySql","mysql_table_name") , nickname);
	sprintf(register_query, "INSERT INTO %s(Email,Password,NickName) VALUES ('%s','%s','%s')" , gEnv->pSettings->GetConfigValue("MySql","mysql_table_name"), login, password, nickname);

	if(SendSqlCommand(check_login)>0)
	{
		if(row<=0)
		{
			Log(LOG_DEBUG,"E-mail <%s> not register...");

			if(SendSqlCommand(check_nickname)>0)
			{
				if(row<=0)
				{
					Log(LOG_DEBUG,"Nickname <%s> not register...", nickname);

					if(SendSqlCommand(register_query)<0)
					{
						Log(LOG_DEBUG, "Register <%s> success!", login);

						//gEnv->pRsp->SendMsg(ClientSocket,"RegSuccess","ServerResult");
						gEnv->rClients++;
					}
				}
				else
				{
					Log(LOG_DEBUG, "Nickname <%s> already register!", login);

					//gEnv->pRsp->SendMsg(ClientSocket,"NicknameAlReg","ServerResult");
				}
			}
		}
		else
		{
			Log(LOG_DEBUG,"E-mail <%s> already register!", login);

			//gEnv->pRsp->SendMsg(ClientSocket,"LoginAlReg","ServerResult");

			return;
		}
	}
	//else

	//	gEnv->pRsp->SendMsg(ClientSocket,"@server_error","ErrorMsg");
}

SClient CMySql::GetAccountInfo(SOCKET ClientSocket, const char *login)
{
	SClient Player;
	/*
//	Player.playerID = playerID;
//	Player.nickName = Nick;
	Player.level = Level;
//	Player.gameCash = GameCash;
//	Player.realCash = RealCash;
	Player.banStatus = !!BanStatus;

	char query[256];

	sprintf(query, "SELECT * FROM %s WHERE ID='%d'" , gEnv->pSettings->GetConfigValue("MySql","mysql_table_name"), playerID);

	if(SendSqlCommand(query)>0)
	{
		if(row<=0)
		{
			Log(LOG_DEBUG, "ID <%d> not found!", playerID);

			//gEnv->pRsp->SendMsg(ClientSocket,"IdNotFound","ServerResult");
		}
		else
		{
			Log(LOG_DEBUG, "ID <%d> is found!", playerID);

			// row[7] - ban status, if row[7] = 1 - player banned
			if(strcmp(row[7],"1")) 
			{
				char buffer [33];

				if(strcmp(row[3],Nick) || strcmp(row[4],_itoa(Level, buffer, 10)) || strcmp(row[5],_itoa(GameCash, buffer, 10)) || strcmp(row[6],_itoa(RealCash, buffer, 10)))
				{
				//	Player.playerID = atoi(row[0]);
				//	Player.nickName = row[3];
					Player.level = atoi(row[4]);
				//	Player.gameCash = atoi(row[5]);
				//	Player.realCash = atoi(row[6]);
					Player.banStatus = !!atoi(row[7]);

					Log(LOG_DEBUG, "Player info changed. PLayer ID = %d",playerID);

					//gEnv->pRsp->SendAccountInfo(ClientSocket,atoi(row[0]),row[3],atoi(row[4]),atoi(row[5]),atoi(row[6]));
					return Player;
				}
				else
				{
					Log(LOG_DEBUG, "Player info not changed. PLayer ID = %d",playerID);
					return Player;
				}
			}
			else
			{
				Log(LOG_DEBUG,"Account <%d> blocked!", playerID);
				//gEnv->pRsp->SendMsg(ClientSocket,"@account_blocked","ErrorMsg");
				return Player;
			}
		}
	}
	else
	{
		//gEnv->pRsp->SendMsg(ClientSocket,"@server_error","ErrorMsg");
		return Player;
	}
	*/
	return Player;
}