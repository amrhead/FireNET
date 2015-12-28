/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.

-------------------------------------------------------------------------
History:

- 09.12.2014   13:49 : Created by AfroStalin(chernecoff)
- 05.01.2015   01:38 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef __WorldState_H__
#define __WorldState_H__

class CWorldState
{
public:
	CWorldState(){}
	~CWorldState(){}

	static CWorldState* GetInstance()
	{
		static CWorldState inst;
		return &inst;
	}

	void Init();
	void Update();
	bool CheckEntityTag(const char * entityName);
	void CreateChildToEntity(const char * entityName, const char *childName);
	void RemoveEntity(const char * entityName);
	

	/*Getters*/
	float GetFloat(const char * entityName, char * valueName);
	int GetInt(const char * entityName, char * valueName);
	string GetString(const char * entityName, char * valueName);
	bool GetBool(const char * entityName, char * valueName);

	/*Setters*/
	void SetFloat(const char * entityName,  char * valueName , float value);
	void SetInt(const char * entityName,  char * valueName , int value);
	void SetString(const char * entityName,  char * valueName , string value);
	void SetBool(const char * entityName,  char * valueName , bool value);

private:
	void CreateChild(const char * entityName);
	void CreateWorlStateFile(char * xmlName);
	void ReadWorldStateFile(char * xmlName);
	void SaveWorldStateFile();

	XmlNodeRef worldStateXml;

	ICryPak *pPak;
	string szSavePath;
	ILevel *pLevel;
	string szSaveFile;

	const char* serverName;
};

#endif