/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2009.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Checkpoint Save/Load system for Game04

-------------------------------------------------------------------------
History:
- 10:07:2008 : Created By Jan M�ller
- 05:02:2009 : Renamed to CheckpointGame for game-specific usage

*************************************************************************/

#include "StdAfx.h"
#include "CheckpointGame.h"
#include "Game.h"
#include <ICheckPointSystem.h>

#include "GameActions.h"
#include "Player.h"

CCheckpointGame CCheckpointGame::m_Instance;

const static char* PLAYER_DATA_SECTION			= "PlayerData";
const static char* INVENTORY_SECTION			= "PlayerInventory";

//////////////////////////////////////////////////////////////////////////
CCheckpointGame::CCheckpointGame()
{

}

//////////////////////////////////////////////////////////////////////////
CCheckpointGame::~CCheckpointGame()
{

}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::Init()
{
	g_pGame->GetIGameFramework()->GetICheckpointSystem()->SetGameHandler(this);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::OnWriteData(XmlNodeRef parentNode)
{
	// Write out player data
	WritePlayerData(parentNode);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::OnReadData(XmlNodeRef parentNode)
{
	// Read in player data
	ReadPlayerData(parentNode);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::OnPreResetEngine()
{
	// Cleanup game systems

}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::OnPostResetEngine()
{
	//send "game start" event
	SEntityEvent event;
	event.event = ENTITY_EVENT_START_GAME;
	event.nParam[0] = 0;
	gEnv->pEntitySystem->SendEventToAll( event );

}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::OnRestartGameplay()
{

}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::WritePlayerData(XmlNodeRef parentNode)
{
	ICheckpointSystem *pSystem = g_pGame->GetIGameFramework()->GetICheckpointSystem();
	CRY_ASSERT(pSystem);

	XmlNodeRef node = GetISystem()->CreateXmlNode(PLAYER_DATA_SECTION);

	CPlayer *pPlayer = static_cast<CPlayer *>(g_pGame->GetIGameFramework()->GetClientActor());

	CRY_ASSERT(pPlayer);

	//write player pos
	//node->setAttr("PlayerPos", pHero->GetEntity()->GetWorldPos());
	XmlNodeRef playerTM = GetISystem()->CreateXmlNode("PlayerTM");
	node->addChild(playerTM);
	pSystem->SerializeWorldTM(pPlayer->GetEntity(), playerTM, true);

	//serialize player inventory
	WritePlayerInventory(node);

	parentNode->addChild(node);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::WritePlayerInventory(XmlNodeRef parentNode)
{
	//create serialization writer
	XmlNodeRef node = GetISystem()->CreateXmlNode(INVENTORY_SECTION);
	IXmlSerializer *pSerializer = GetISystem()->GetXmlUtils()->CreateXmlSerializer();
	ISerialize *pWriter = pSerializer->GetWriter(node);

	//get serialization data
	TSerialize ser = TSerialize(pWriter);
	CPlayer *pPlayer = static_cast<CPlayer *>(g_pGame->GetIGameFramework()->GetClientActor());
	CRY_ASSERT(pPlayer);
	pPlayer->GetInventory()->FullSerialize(ser);

	//add to parent node
	parentNode->addChild(node);

	pSerializer->Release();
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::ReadPlayerData(XmlNodeRef data)
{
	ICheckpointSystem *pSystem = g_pGame->GetIGameFramework()->GetICheckpointSystem();
	CRY_ASSERT(pSystem);

	XmlNodeRef playerData = data->findChild(PLAYER_DATA_SECTION);
	if(!playerData)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed reading player data from checkpoint, player position won't be set.");
		return;
	}

	CPlayer *pPlayer = static_cast<CPlayer *>(g_pGame->GetIGameFramework()->GetClientActor());
	CRY_ASSERT(pPlayer);

	pPlayer->Revive(CActor::kRFR_Spawn);
	pPlayer->Physicalize(STANCE_STAND);

	//this will set the player to the required location
	XmlNodeRef playerTM = playerData->findChild("PlayerTM");
	if(playerTM)
		pSystem->SerializeWorldTM(pPlayer->GetEntity(), playerTM, false);

	//read player inventory from stream
	ReadPlayerInventory(playerData);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointGame::ReadPlayerInventory(XmlNodeRef parentNode)
{
	//get source node
	XmlNodeRef node = parentNode->findChild(INVENTORY_SECTION);
	if(!node)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Couldn't find Player Inventory section while reading checkpoint.");
		return;
	}

	//create serialization reader
	IXmlSerializer *pSerializer = GetISystem()->GetXmlUtils()->CreateXmlSerializer();
	ISerialize *pReader = pSerializer->GetReader(node);

	//read serialization data
	TSerialize ser = TSerialize(pReader);
	CPlayer *pPlayer = static_cast<CPlayer *>(g_pGame->GetIGameFramework()->GetClientActor());
	CRY_ASSERT(pPlayer);
	pPlayer->GetInventory()->FullSerialize(ser);

	// Make sure all the inventory data changes are applied
	gEnv->pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(pPlayer, pPlayer->GetCurrentItemId(), false);

	pSerializer->Release();
}
