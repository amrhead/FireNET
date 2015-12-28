/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 03.03.2015   16:19 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "XmlDatabase.h"
#include "System\md5.h"

#include "tinyxml.h"
#include <io.h>

void CXmlDatabase::Init()
{
	Log(LOG_DEBUG,"CXmlDatabase::Init()");

	if(!FileExists("Database.xml"))
	{
		CreateXmlFile("Database.xml");
		CreateChild("GlobalVariables");
		SetInt("GlobalVariables","playerIDs",10000000);
		SetInt("GlobalVariables","players_count",0);
	}
}

const char* CXmlDatabase::Register(std::string login, std::string password, std::string nickName)
{
	Log(LOG_DEBUG,"CXmlDatabase::Register()");

	MD5 md5;
	char childName[256];

	sprintf(childName,"Player_%s",md5.digestString((char*)login.c_str()));

	if(!CreateChild(childName))
	{
		Log(LOG_WARNING,"CXmlDatabase::Register user <%s> failed! This login alredy registered!", login.c_str());
		return "LoginAlReg";
	}

	if(ChildExist(childName))
	{
		//
		int id = GetInt("GlobalVariables","playerIDs");
		int players_count = GetInt("GlobalVariables","players_count");

		id++; players_count++;

		SetInt("GlobalVariables","playerIDs",id);
		SetInt("GlobalVariables","players_count",players_count);
		//

		SetInt(childName,"id",id);
		SetString(childName,"login",login);
		SetString(childName,"password",password);
		SetString(childName,"nickname",nickName);
		SetInt(childName,"level",0);
		SetInt(childName,"game_money",0);
		SetInt(childName,"xp",0);
		SetBool(childName,"ban",false);

		Log(LOG_DEBUG,"CXmlDatabase::Register user <%s> success!", login.c_str());

		gEnv->rClients++;

		return "RegSuccess";
	}	

	Log(LOG_ERROR,"CXmlDatabase::Register user <%s> failed!", login.c_str());
	return "RegFailed";
}

const char* CXmlDatabase::Login(std::string login, std::string password)
{
	Log(LOG_DEBUG,"CXmlDatabase::Login()");

	MD5 md5;
	char childName[256];

	sprintf(childName,"Player_%s",md5.digestString((char*)login.c_str()));

	if(ChildExist(childName))
	{
		if(!strcmp(password.c_str(),GetString(childName,"password").c_str()))
		{
			if(GetBool(childName,"ban"))
			{
				Log(LOG_WARNING,"CXmlDatabase::Login user <%s> failed! Account banned!",login.c_str());
				return "AccountBlocked";
			}
			else
			{
				Log(LOG_DEBUG,"CXmlDatabase::Login user <%s> success!",login.c_str());
				gEnv->aClients++;
				return "PasswordCorrect";
			}
		}
		else
		{
			Log(LOG_WARNING,"CXmlDatabase::Login user <%s> failed! Incorrect password!",login.c_str());	
			return "PasswordIncorrect";
		}
	}
	else 
	{
		Log(LOG_WARNING,"CXmlDatabase::Login user <%s> failed! Login not found!",login.c_str());
		return "LoginNotFound";
	}

	Log(LOG_ERROR,"CXmlDatabase::Login user <%s> failed!",login.c_str());
	return "LoginFailed";
}

SClient CXmlDatabase::GetUserInfo(std::string login)
{
	Log(LOG_DEBUG,"CXmlDatabase::GetUserInfo()");

	SClient player;
	MD5 md5;
	char childName[256];

	sprintf(childName,"Player_%s",md5.digestString((char*)login.c_str()));

	if(ChildExist(childName))
	{
		player.playerId = GetInt(childName,"id");
		player.nickname = (char*)GetString(childName,"nickname").c_str();
		player.xp = GetInt(childName,"xp");
		player.level = GetInt(childName,"level");
		player.money = GetInt(childName,"game_money");		
		player.banStatus = GetBool(childName,"ban");	

		return player;
	}
	else
		Log(LOG_WARNING,"CXmlDatabase::GetUserInfo::User not found!");

	return player;
}

// Setters

void CXmlDatabase::SetInt(const char* childName, const char* valueName , int value)
{
	Log(LOG_DEBUG,"CXmlDatabase::SetInt()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* child;
	TiXmlElement* root;
	
	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_WARNING, "CXmlDatabase::No root element!");

		child = root->FirstChildElement(childName);

		if(!child)
		{
			Log(LOG_WARNING,"CXmlDatabase::Child <%s> not found!",childName);
			return;
		}

		child->SetAttribute(valueName,value);

		xmlFile.SaveFile();
	}
}

void CXmlDatabase::SetString(const char* childName, const char* valueName , std::string value)
{
	Log(LOG_DEBUG,"CXmlDatabase::SetString()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* child;
	TiXmlElement* root;
	
	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_WARNING, "CXmlDatabase::No root element!");

		child = root->FirstChildElement(childName);

		if(!child)
		{
			Log(LOG_WARNING,"CXmlDatabase::Child <%s> not found!",childName);
			return;
		}

		child->SetAttribute(valueName,value.c_str());

		xmlFile.SaveFile();
	}
}

void CXmlDatabase::SetBool(const char* childName, const char* valueName , bool value)
{
	Log(LOG_DEBUG,"CXmlDatabase::SetBool()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* child;
	TiXmlElement* root;
	
	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_WARNING, "CXmlDatabase::No root element!");

		child = root->FirstChildElement(childName);

		if(!child)
		{
			Log(LOG_WARNING,"CXmlDatabase::Child <%s> not found!",childName);
			return;
		}

		child->SetAttribute(valueName,value);

		xmlFile.SaveFile();
	}
}

// Getters

int CXmlDatabase::GetInt(const char* childName, const char* valueName)
{
	Log(LOG_DEBUG,"CXmlDatabase::GetInt()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* child;
	TiXmlElement* root;
	int result = 0;
	
	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_WARNING, "CXmlDatabase::No root element!");

		child = root->FirstChildElement(childName);

		if(!child)
		{
			Log(LOG_WARNING,"CXmlDatabase::Child <%s> not found!",childName);
			return 0;
		}

		
		child->QueryIntAttribute(valueName, &result);
		return result;
	}

	return 0;
}

std::string CXmlDatabase::GetString(const char* childName, const  char* valueName)
{
	Log(LOG_DEBUG,"CXmlDatabase::GetString()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* child;
	TiXmlElement* root;
	
	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_WARNING, "CXmlDatabase::No root element!");

		child = root->FirstChildElement(childName);

		if(!child)
			Log(LOG_WARNING,"CXmlDatabase::Child <%s> not found",childName);

		return child->Attribute(valueName);	
	}
	else
		Log(LOG_WARNING,"CXmlDatabase::Failed load xml");

	return NULL;
}

bool CXmlDatabase::GetBool(const char* childName, const  char* valueName)
{
	Log(LOG_DEBUG,"CXmlDatabase::GetBool()");
	return !!GetInt(childName,valueName);
}

// Tools
void CXmlDatabase::CreateXmlFile(const char *xmlName)
{
	Log(LOG_DEBUG,"CXmlDatabase::CreateXmlFile()");

	TiXmlDocument xmlFile(xmlName);

	const char* startNode = "<Database/"">\n";
	xmlFile.Parse(startNode);
	xmlFile.SaveFile();
}

bool CXmlDatabase::CreateChild(const char* childName)
{
	Log(LOG_DEBUG,"CXmlDatabase::CreateChild()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement child(childName);
	TiXmlElement* root;

	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
		{
			Log(LOG_DEBUG, "CXmlDatabase::No root element!");
			return false;
		}

		TiXmlElement *oldChild = root->FirstChildElement(childName);

		if(oldChild)
		{
			Log(LOG_DEBUG,"CXmlDatabase::Child <%s> alredy created!",childName);
			return false;
		}

		root->InsertEndChild(child);


		xmlFile.SaveFile();
		return true;
	}
	else
	{
		Log(LOG_ERROR, "CXmlDatabase::Error loading xml!!!");
		return false;
	}

	return false;
}

void CXmlDatabase::RemoveChild(const char* childName)
{
	Log(LOG_DEBUG,"CXmlDatabase::RemoveChild()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* root;

	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_DEBUG, "CXmlDatabase::No root element!");

		root->RemoveChild(root->FirstChildElement(childName));

		xmlFile.SaveFile();
	}
	else
		Log(LOG_ERROR, "CXmlDatabase::Error loading xml!!!");
}

bool CXmlDatabase::FileExists(const char *fname)
{
	return _access(fname, 0) != -1;
}

bool CXmlDatabase::ChildExist(const char* childName)
{
	Log(LOG_DEBUG,"CXmlDatabase::ChildExist()");
	TiXmlDocument xmlFile("Database.xml");
	TiXmlElement* root;

	if (xmlFile.LoadFile())
	{
		root = xmlFile.FirstChildElement("Database");
		if (!root)
			Log(LOG_WARNING, "CXmlDatabase::No root element!");

		TiXmlElement *Child = root->FirstChildElement(childName);

		if(Child)
			return true;
	}	
	else
	{
		Log(LOG_ERROR, "CXmlDatabase::Error loading xml!!!");
		return false;
	}

	return false;
}
