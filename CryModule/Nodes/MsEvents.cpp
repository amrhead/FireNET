/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 25.09.2014   18:29 : Created by AfroStalin
- 15.05.2015   22:18 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#include "StdAfx.h"
#include "MsEvents.h"
#include "System/Global.h"
 
void CMsEvents::Init()
{
    if (gEnv->pFlashUI)
    {
		gEnv->pLog->Log(TITLE "Init FireNET master server UI events");

		// Chat message received
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc ChatMsg( "ChatMessageReceived", "ChatMessageReceived", "Event when client received chat message" );
		ChatMsg.AddParam<SUIParameterDesc::eUIPT_String>("Message", "Chat message");
        m_EventMap[ eUIGE_MsgResived ] = m_pGameEvents->RegisterEvent( ChatMsg );

		// Server message received
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc ServerMsg( "ServerMessageReceived", "ServerMessageReceived", "Event when client received server message" );
		ServerMsg.AddParam<SUIParameterDesc::eUIPT_String>("Message", "Server message");
        m_EventMap[ eUIGE_ServerMsgResived ] = m_pGameEvents->RegisterEvent( ServerMsg );


		// Server info received
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc ServerInfo( "ServerInfoResived", "ServerInfoResived", "Event when client resived server info" );
		ServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("PlayersAmmount", "Players ammount");
		ServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("GameServersAmmount", "Game servers ammount");
        m_EventMap[ eUIGE_ServerInfoResived ] = m_pGameEvents->RegisterEvent( ServerInfo );


		// Account info received
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc AccountInfo( "AccountInfoReceived", "AccountInfoReceived", "Event when client received account info" );
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_String>("Nickname", "Player nickname");
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Level", "Player level");
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Money", "Ammount game cash");
		AccountInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Xp", "Player xp");
		m_EventMap[ eUIGE_AccountInfoResived ] = m_pGameEvents->RegisterEvent( AccountInfo );


		// Eror received
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc Error( "ErrorReceived", "ErrorReceived", "Event when client received error" );
		Error.AddParam<SUIParameterDesc::eUIPT_String>("Error", "Error");
        m_EventMap[ eUIGE_Error ] = m_pGameEvents->RegisterEvent( Error );

		// Game server info
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc GameServerInfo( "AddOrUpadateGameServer", "AddOrUpadateGameServer", "Event when client resived game servers info" );
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("ServerId", "Server id");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("ServerIP", "Server ip adress");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("ServerPort", "Server port");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("ServerName", "Server name");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("PlayersInServer", "Ammount players in server");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("MapName", "Map name");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_String>("Gamerules", "Server gamerules");
		GameServerInfo.AddParam<SUIParameterDesc::eUIPT_Int>("Ping", "PingToServer");
		m_EventMap[ eUIGE_GameServerInfo ] = m_pGameEvents->RegisterEvent( GameServerInfo );

		// Remove game server
        m_pGameEvents = gEnv->pFlashUI->CreateEventSystem( "FireNET:MasterServer", IUIEventSystem::eEST_SYSTEM_TO_UI );
        SUIEventDesc RemoveServer( "RemoveGameServer", "RemoveGameServer", "Remove game server from server list" );
		RemoveServer.AddParam<SUIParameterDesc::eUIPT_Int>("Id", "Game server id");
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