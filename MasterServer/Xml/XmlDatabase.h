/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 03.03.2015   16:19 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef __XmlDatabase_H__
#define __XmlDatabase_H__

class TiXmlElement;

class CXmlDatabase
{
public:
	CXmlDatabase(void){}
	~CXmlDatabase(void){}

	void Init();

	const char* Register(std::string login, std::string password, std::string nickName);
	const char* Login(std::string login, std::string password);
	SClient GetUserInfo(std::string login);

private:
	/*Getters*/
	int GetInt(const char* childName, const char* valueName);
	std::string GetString(const char* childName, const  char* valueName);
	bool GetBool(const char* childName, const  char* valueName);

	/*Setters*/
	void SetInt(const char* childName, const char* valueName , int value);
	void SetString(const char* childName, const char* valueName , std::string value);
	void SetBool(const char* childName, const char* valueName , bool value);
	//

	bool CreateChild(const char* childName);
	void RemoveChild(const char* childName);
	void CreateXmlFile(const char* xmlName);
	bool ChildExist(const char* childName);
	bool FileExists(const char* fname);

	//TiXmlElement* root;
};

#endif