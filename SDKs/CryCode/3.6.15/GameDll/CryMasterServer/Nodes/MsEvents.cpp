/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.
-------------------------------------------------------------------------
History:

- 25.09.2014   18:29 : Created by AfroStalin
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#include "StdAfx.h"
#include "MsEvents.h"
 
void CMsEvents::Init()
{
    if (gEnv->pFlashUI)
    {
		// Chat msg resived
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc ChatMsg( "ChatMsg", "ChatMsg", "Event when client resived chat msg" );
		ChatMsg.AddParam<SUIParameterDesc::eUIPT_String>("Msg", "Chat msg");
        m_EventMap[ eUIGE_MsgResived ] = m_pGameEvents->RegisterEvent( ChatMsg );

		// Server msg resived
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc ServerMsg( "ServerMsg", "ServerMsg", "Event when client resived Server Msg" );
		ServerMsg.AddParam<SUIParameterDesc::eUIPT_String>("Msg", "ServerMsg");
        m_EventMap[ eUIGE_ServerMsgResived ] = m_pGameEvents->RegisterEvent( ServerMsg );


		// Server info resived
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc ServerInfo( "ServerInfoResived", "ServerInfoResived", "Event when client resived server info" );
		ServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("PlayersAmmount", "Players ammount");
		ServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("GameServersAmmount", "Game servers ammount");
        m_EventMap[ eUIGE_ServerInfoResived ] = m_pGameEvents->RegisterEvent( ServerInfo );


		// Account info resived
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc AccountInfo( "AccountInfoResived", "AccountInfoInfoResived", "Event when client resived account info" );
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_String>("Nickname", "Player nickname");
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Level", "Player level");
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Money", "Ammount game cash");
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Xp", "Player xp");
		m_EventMap[ eUIGE_AccountInfoResived ] = m_pGameEvents->RegisterEvent( AccountInfo );


		// Eror
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc Error( "Error", "Error", "Event when client resived Error" );
		Error.AddParam<SUIParameterDesc::eUIPT_String>("Error", "Error");
        m_EventMap[ eUIGE_Error ] = m_pGameEvents->RegisterEvent( Error );

		// Game server info
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc GameServerInfo( "AddOrUpadateGameServer", "AddOrUpadateGameServer", "Event when client resived game servers info" );
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("ServerIP", "Server ip adress");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("ServerPort", "Server port");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("ServerName", "Server name");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("PlayersInServer", "Ammount players in server");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("MapName", "Map name");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("Gamerules", "Server gamerules");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Ping", "PingToServer");
		m_EventMap[ eUIGE_GameServerInfo ] = m_pGameEvents->RegisterEvent( GameServerInfo );

		// Remove game server
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc RemoveServer( "Remove", "Remove", "Remove game server from server list" );
		RemoveServer.AddParam<SUIParameterDesc::eUIPT_Int>("ID", "GameServerID");
		m_EventMap[ eUIGE_RemoveServer ] = m_pGameEvents->RegisterEvent( RemoveServer );
    }
}
  
void CMsEvents::SendEvent( EUIGameEvents event, const SUIArguments& args )
{
    // send the event
    if (m_pGameEvents)
    {
        m_pGameEvents->SendEvent( SUIEvent(m_EventMap[event], args) );
    }
}