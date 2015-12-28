/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.

-------------------------------------------------------------------------
History:

- 09.12.2014   13:49 : Created by AfroStalin(chernecoff)
- 17.04.2015   14:38 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include "WorldState.h"

void CWorldState::Init()
{
	if( gEnv->bServer && !gEnv->IsEditor())
	{
		CryLog("CWorldState::Init()");
		pPak = gEnv->pCryPak;
		pLevel = gEnv->pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel();
		ICVar* pName = gEnv->pConsole->GetCVar("sv_servername");
		worldStateXml==NULL;

		if (pLevel)
			szSavePath = pLevel->GetLevelInfo()->GetPath();

		if(pName)
			serverName = pName->GetString();

		szSaveFile = PathUtil::Make(szSavePath.c_str(), serverName, ".xml");

		ReadWorldStateFile((char*)serverName);
	}
}

void CWorldState::Update()
{
	CryLog("CWorldState::Update()");
}

bool CWorldState::CheckEntityTag(const char * entityName)
{
	if(worldStateXml)
	{
		XmlNodeRef entityNode = worldStateXml->findChild(entityName);
		if(entityNode)
			return true;
	}

	return false;
}

void CWorldState::CreateWorlStateFile(char *xmlName)
{
	CryLog("CWorldState::CreateWorlStateFile()");
	worldStateXml = GetISystem()->CreateXmlNode("WorldState");
	worldStateXml->saveToFile(szSaveFile.c_str());
	ReadWorldStateFile(xmlName);
}

void CWorldState::ReadWorldStateFile(char * xmlName)
{
	CryLog("CWorldState::ReadWorldStateFile()");
	// Check if file already exists
	if (pPak->IsFileExist(szSaveFile.c_str()))
	{
		worldStateXml = GetISystem()->LoadXmlFromFile(szSaveFile);
		if (worldStateXml==NULL)
		{
			CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to load'%s.xml'", xmlName);
			return;
		}
	}
	else
	{
		CreateWorlStateFile(xmlName);
	}
}

void CWorldState::SaveWorldStateFile()
{
	if(worldStateXml)
		worldStateXml->saveToFile(szSaveFile);
}

/*Getters*/

float CWorldState::GetFloat(const char * entityName, char * valueName)
{
//	CryLog("CWorldState::GetFloat()");
	float result = 0.f;

	if(worldStateXml)
	{

		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(entityNode)
		{
			const uint32 Count = entityNode->getChildCount();
			for (uint32 Index = 0; Index < Count; ++Index)
			{
				const XmlNodeRef currentNode = entityNode->getChild(Index);
				if(strcmp(currentNode->getTag(),valueName)==0)
				{
					currentNode->getAttr("value",result);
					break;
				}
			}
		}
		else
			CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");
	}
	else
		CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");

	return result;
}

int CWorldState::GetInt(const char * entityName, char * valueName)
{
//	CryLog("CWorldState::GetInt()");
	int result = 0;

	if(worldStateXml)
	{

		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(entityNode)
		{
			const uint32 Count = entityNode->getChildCount();
			for (uint32 Index = 0; Index < Count; ++Index)
			{
				const XmlNodeRef currentNode = entityNode->getChild(Index);
				if(strcmp(currentNode->getTag(),valueName)==0)
				{
					currentNode->getAttr("value",result);
					break;
				}
			}
		}
		else
			CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");
	}
	else
		CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");

	return result;
}

string CWorldState::GetString(const char * entityName, char * valueName)
{
//	CryLog("CWorldState::GetString()");
	string result = NULL;

	if(worldStateXml)
	{

		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(entityNode)
		{
			const uint32 Count = entityNode->getChildCount();
			for (uint32 Index = 0; Index < Count; ++Index)
			{
				const XmlNodeRef currentNode = entityNode->getChild(Index);
				if(strcmp(currentNode->getTag(),valueName)==0)
				{
					result = currentNode->getAttr("value");
					break;
				}
			}
		}
		else
			CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");
	}
	else
		CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");

	return result;
}

bool CWorldState::GetBool(const char * entityName, char * valueName)
{
//	CryLog("CWorldState::GetBool()");
	bool result = false;

	if(worldStateXml)
	{

		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(entityNode)
		{
			const uint32 Count = entityNode->getChildCount();
			for (uint32 Index = 0; Index < Count; ++Index)
			{
				const XmlNodeRef currentNode = entityNode->getChild(Index);
				if(strcmp(currentNode->getTag(),valueName)==0)
				{
					if(strcmp(currentNode->getAttr("value"),"true")==0)
						result = true;
					break;
				}
			}
		}
		else
			CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");
	}
	else
		CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CWorldState::Failed to get world state value!");

	return result;
}

/*Setters*/

void CWorldState::SetFloat(const char * entityName, char * valueName , float value)
{
//	CryLog("CWorldState::SetFloat()");
	if(worldStateXml)
	{
		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(!entityNode)
		{
			CreateChild(entityName);
			entityNode = worldStateXml->findChild(entityName);
		}

		const uint32 Count = entityNode->getChildCount();

		for (uint32 Index = 0; Index < Count; ++Index)
		{
			XmlNodeRef child = entityNode->getChild(Index);

			if(strcmp(child->getTag(),valueName)==0)
			{
				child->setAttr("value",value);
				worldStateXml->saveToFile(szSaveFile);
				return;
			}
		}

		//CryLog("CWorldState::CreateFloat()");

		XmlNodeRef child = entityNode->newChild(valueName);
		child->setAttr("value",value);
		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}

void CWorldState::SetInt(const char * entityName,  char * valueName , int value)
{
//	CryLog("CWorldState::SetInt()");
	if(worldStateXml)
	{
		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(!entityNode)
		{
			CreateChild(entityName);
			entityNode = worldStateXml->findChild(entityName);
		}
		const uint32 Count = entityNode->getChildCount();

		for (uint32 Index = 0; Index < Count; ++Index)
		{
			XmlNodeRef child = entityNode->getChild(Index);

			if(strcmp(child->getTag(),valueName)==0)
			{
				child->setAttr("value",value);
				worldStateXml->saveToFile(szSaveFile);
				return;
			}
		}

		//CryLog("CWorldState::CreateInt()");

		XmlNodeRef child = entityNode->newChild(valueName);
		child->setAttr("value",value);
		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}

void CWorldState::SetString(const char * entityName,  char * valueName , string value)
{
//	CryLog("CWorldState::SetString()");
	if(worldStateXml)
	{
		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(!entityNode)
		{
			CreateChild(entityName);
			entityNode = worldStateXml->findChild(entityName);
		}
		const uint32 Count = entityNode->getChildCount();

		for (uint32 Index = 0; Index < Count; ++Index)
		{
			XmlNodeRef child = entityNode->getChild(Index);

			if(strcmp(child->getTag(),valueName)==0)
			{
				child->setAttr("value",value);
				worldStateXml->saveToFile(szSaveFile);
				return;
			}
		}

		//CryLog("CWorldState::CreateString()");

		XmlNodeRef child = entityNode->newChild(valueName);
		child->setAttr("value",value);
		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}

void CWorldState::SetBool(const char * entityName,  char * valueName , bool value)
{
//	CryLog("CWorldState::SetBool()");
	if(worldStateXml)
	{
		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(!entityNode)
		{
			CreateChild(entityName);
			entityNode = worldStateXml->findChild(entityName);
		}
		const uint32 Count = entityNode->getChildCount();

		for (uint32 Index = 0; Index < Count; ++Index)
		{
			XmlNodeRef child = entityNode->getChild(Index);

			if(strcmp(child->getTag(),valueName)==0)
			{
				if(value)
					child->setAttr("value","true");
				else
					child->setAttr("value","false");

				worldStateXml->saveToFile(szSaveFile);
				return;
			}
		}

		//CryLog("CWorldState::CreateBool()");

		XmlNodeRef child = entityNode->newChild(valueName);
		if(value)
			child->setAttr("value","true");
		else
			child->setAttr("value","false");
		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}

/**/

void CWorldState::CreateChild(const char * entityName)
{
	if(worldStateXml)
	{
		CryLog("CWorldState::CreateEntityChild()");
		worldStateXml->newChild(entityName);
		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}


void CWorldState::CreateChildToEntity(const char * entityName, const char *childName)
{
	if(worldStateXml)
	{
		CryLog("CWorldState::CreateChildToEntity()");

		XmlNodeRef entityNode = worldStateXml->findChild(entityName);

		if(!entityNode)
			return;

		entityNode->newChild(childName);
		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}

void CWorldState::RemoveEntity(const char * entityName)
{
	if(worldStateXml)
	{
		CryLog("CWorldState::RemoveEntity()");

		const uint32 Count = worldStateXml->getChildCount();

		for (uint32 Index = 0; Index < Count; ++Index)
		{
			XmlNodeRef child = worldStateXml->getChild(Index);

			if(strcmp(child->getTag(),entityName)==0)
			{
				worldStateXml->deleteChildAt(Index);
			}
		}

		worldStateXml->saveToFile(szSaveFile);
	}
	else
		return;
}