#ifndef __HUDInventory_H__
#define __HUDInventory_H__

#include <list>
#include "IGameObject.h"
#include "IFlashPlayer.h"
#include "IHardwareMouse.h"

class CHUDInventory : public IFSCommandHandler, public IHardwareMouseEventListener
{
public:

	struct SInventoryItem
	{
		const char* itemName;
		EntityId itemID;
		int slotID;	
		int itemAmmount;
		bool canStack;
	};

	// IFSCommandHandler
	virtual void HandleFSCommand(const char *strCommand,const char *strArgs, void* pUserData);
	// ~IFSCommandHandler

	// IHardwareMouseEventListener
	virtual void OnHardwareMouseEvent(int iX,int iY,EHARDWAREMOUSEEVENT eHardwareMouseEvent, int wheelDelta);
	// ~IHardwareMouseEventListener

	// IConsole callback
	static void HUD (ICVar *pVar);
	// ~IConsole callback

	CHUDInventory();
	virtual	~CHUDInventory();

	// Cursor handling
	void CursorIncrementCounter();
	void CursorDecrementCounter();

	void ShowInventory();

	

private:
	std::vector <SInventoryItem> inventoryItems;
	bool Inv_loaded;
	bool itemsSaved;
};

#endif
