/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.
-------------------------------------------------------------------------
History:

- 25.09.2014   18:29 : Created by AfroStalin
- 07.02.2015   13:50 : Edited by AfroStalin
-------------------------------------------------------------------------


*************************************************************************/

#ifndef __UIGameEvents_H__
#define __UIGameEvents_H__
  
#include <IFlashUI.h>
  
class CMsEvents
{
public:
    // access to instance
    static CMsEvents* GetInstance()
    {
        static CMsEvents inst;
        return &inst;
    }
  
    // init the game events
    void Init();
  
    // events
    enum EUIGameEvents
    {
        eUIGE_MsgResived,
		eUIGE_ServerMsgResived,
		eUIGE_ServerInfoResived,
		eUIGE_AccountInfoResived,
		eUIGE_Error,
		eUIGE_GameServerInfo,
		eUIGE_RemoveServer,
    };
    void SendEvent( EUIGameEvents event, const SUIArguments& args );
  
private:
    CMsEvents() : m_pGameEvents(NULL) {};
    ~CMsEvents() {};
  
    IUIEventSystem* m_pGameEvents;
    std::map<EUIGameEvents, uint> m_EventMap;
};
  
#endif