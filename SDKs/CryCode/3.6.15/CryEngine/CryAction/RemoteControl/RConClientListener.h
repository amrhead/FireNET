/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: IRemoteControlClientListener implementation
-------------------------------------------------------------------------
History:
- Created by Lin Luo, November 09, 2006
*************************************************************************/

#ifndef __RCONCLIENTLISTENER_H__
#define __RCONCLIENTLISTENER_H__

#include <IRemoteControl.h>

#pragma once

class CRConClientListener : public IRemoteControlClientListener
{
public:
	static CRConClientListener& GetSingleton(IRemoteControlClient* rcon_client);

	void OnConnectResult(bool okay, EResultDesc desc);

	void OnSessionStatus(bool connected, EStatusDesc desc);

	void OnCommandResult(uint32 commandId, string command, string result);

	bool IsSessionAuthorized() const;

private:
	CRConClientListener();
	~CRConClientListener();

	bool m_sessionAuthorized;

	static CRConClientListener s_singleton;

	static IRemoteControlClient* s_rcon_client;
};

#endif

