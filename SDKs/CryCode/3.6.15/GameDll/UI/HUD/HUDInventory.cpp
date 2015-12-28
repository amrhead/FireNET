#include "StdAfx.h"
#include "HUDInventory.h"
#include "UI/UICVars.h"
#include "UI/UIManager.h"
#include "UI/UIInput.h"
#include "IUIDraw.h"

#include "IFlashPlayer.h"
#include "EntityUtility/EntityScriptCalls.h"
#include "IAIActor.h"
#include "AI/GameAISystem.h"
#include "Actor.h"
#include "Item.h"

#include <GameUtils.h>
#include <IGameFramework.h>
#include "Player.h"
#include "Game.h"
#include "GameCVars.h"
#include "GameActions.h"


void CHUDInventory::HUD(ICVar *pVar)
{

}

CHUDInventory::CHUDInventory() 
{

	if(gEnv->pHardwareMouse)
	{
		gEnv->pHardwareMouse->AddListener(this);
	}
	Inv_loaded = false;
	itemsSaved = false;
}

CHUDInventory::~CHUDInventory()
{
	inventoryItems.clear();
}

//-----------------------------------------------------------------------------------------------------

void CHUDInventory::HandleFSCommand(const char *strCommand,const char *strArgs, void* pUserData)
{
	if(!strcmp(strCommand,"itemPressed"))
	{
		EntityId id = static_cast<EntityId>(atoi(strArgs));

		if(id>=1)
		{
			CActor *pActor=static_cast<CActor *>(gEnv->pGame->GetIGameFramework()->GetClientActor());

			if(IItem *pItem = gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(id))
			{
				if (pItem->CanSelect())
				{
					CItem *pItemii = static_cast<CItem*>(gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(id));
					if(!pItemii->IsSelected())
					{
						pActor->SelectItem(id,true,true);
					}
				}
			}
		}
	}

	if(!strcmp(strCommand,"slotChanged"))
	{
		SUIArguments args;
		args.SetDelimiter(",");
		args.SetArguments(strArgs);

		int oldSlotID;
		int slotID;
		int itemID;

		args.GetArg(0, oldSlotID);
		args.GetArg(1, slotID);
		args.GetArg(2, itemID);

		for(int i = 0; i<inventoryItems.size(); i++)
		{
			if(inventoryItems[i].itemID == itemID)
			{
				//CryLog("ID find in items, changing slotID....");
				inventoryItems[i].slotID = slotID;
			}
		}	
	}

	if(!strcmp(strCommand,"dropItem"))
	{
		EntityId itemID = static_cast<EntityId>(atoi(strArgs));
		CItem *curItem = static_cast<CItem*>(gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemID));
		curItem->Drop(1.f,false,false);
		//CryLog("Drop item = %d", itemID);

		for(int i = 0; i<inventoryItems.size(); i++)
		{
			if(inventoryItems[i].itemID == itemID)
			{
				SFlashVarValue args[1] = {inventoryItems[i].slotID};
				IUIElement* pElement = gEnv->pFlashUI->GetUIElement("Inventory");
				pElement->GetFlashPlayer()->Invoke("removeInventoryItem", args, 1);
			}
		}
	
	}
}

void CHUDInventory::OnHardwareMouseEvent(int iX,int iY,EHARDWAREMOUSEEVENT eHardwareMouseEvent, int wheelDelta = 0)
{

}
//-----------------------------------------------------------------------------------------------------

void CHUDInventory::CursorIncrementCounter()
{
	if(gEnv->pHardwareMouse)
	{
		gEnv->pHardwareMouse->IncrementCounter();
	}
}

//-----------------------------------------------------------------------------------------------------

void CHUDInventory::CursorDecrementCounter()
{
	if(gEnv->pHardwareMouse)
	{
		gEnv->pHardwareMouse->DecrementCounter();
	}
}

//-----------------------------------------------------------------------------------------------------

void CHUDInventory::ShowInventory()
{
	IUIElement* pElement = gEnv->pFlashUI->GetUIElement("Inventory");

	if(pElement)
	{
		if(!Inv_loaded)
		{
			pElement->Reload();
			pElement->GetFlashPlayer()->SetVisible(true);
			pElement->SetVisible(true);

			CActor *pActor = static_cast<CActor *>(gEnv->pGame->GetIGameFramework()->GetClientActor());
			IInventory *pInventory = pActor->GetInventory();
			g_pGame->Actions().FilterNoMouse()->Enable(true);

			for (int i = 0; i < pInventory->GetCount(); i++)
			{
				EntityId ide = pInventory->GetItem(i);
				EntityId slotID = pActor->GetInventory()->FindItem(ide);
				CItem *curItem = static_cast<CItem*>(gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(ide));
				IItem *pItem = static_cast<IItem*>(gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(ide));

				CItem::SInventotyInfo info = curItem->GetInventoryInfo();
				
				//int slotID;
				EntityId itemID = pItem->GetEntityId();

				itemsSaved = false;

				for(int i = 0; i<inventoryItems.size(); i++)
				{
					if(inventoryItems[i].itemID == itemID)
					{
						//CryLog("Items finded in saved items");
						slotID = inventoryItems[i].slotID;
						itemsSaved = true;
						break;
					}
				}

				if(!itemsSaved)
					slotID--;

				SFlashVarValue args[8] = {slotID, info.iconName.c_str(), itemID, info.itemName.c_str(), info.itemDescription.c_str(), 0, info.canStack, info.canQuick};

				if(info.itemClass)
				{
					pElement->GetFlashPlayer()->Invoke("addInventoryItem", args, 8);

					if(!itemsSaved)
					{
						SInventoryItem inventoryItem;
						inventoryItem.itemName = info.itemName.c_str();
						inventoryItem.itemID = itemID;
						inventoryItem.slotID = slotID++;
						inventoryItem.itemAmmount = 0;
						inventoryItem.canStack = info.canStack;

						inventoryItems.push_back(inventoryItem);
					}
				}


			}

			pElement->GetFlashPlayer()->SetFSCommandHandler(this);
			Inv_loaded = true;
		}
		else
		{
			g_pGame->Actions().FilterNoMouse()->Enable(false);
			pElement->SetVisible(false);
			pElement->GetFlashPlayer()->SetVisible(false);
			pElement->GetFlashPlayer()->SetFSCommandHandler(NULL);
			Inv_loaded = false;
		}

	}
	else
		CryLog("CHUDInventory::Error open inventory!");
}