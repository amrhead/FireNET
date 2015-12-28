/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------
Description: Sending packets
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"

#include "../System/Global.h"

// Connect to master server
class CFlowNode_MasterServerConnect: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Connect = 0,
	};

	enum OUTPUTS
	{
		EOP_Success = 0,
		EOP_Error,
	};

public:
	CFlowNode_MasterServerConnect( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_MasterServerConnect()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_MasterServerConnect(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Connect", _HELP("Connect to master server")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig_Void( "Success", _HELP("Connection success")),
			OutputPortConfig<string>("Error", _HELP("Connection failed")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("CryMasterServer connect");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Connect))
				{
					MasterServerConnect();
				}
			}
			break;
		}
	}

	void MasterServerConnect()
	{
		const char* result = gClientEnv->pMasterServer->Connect();

		if(!strcmp(result,"@ms_connection_established"))
			ActivateOutput(&m_actInfo, EOP_Success, true);
		else
			ActivateOutput(&m_actInfo, EOP_Error, string(result));
	}

protected:
	SActivationInfo m_actInfo;
};

// Try login  
class CFlowNode_MasterServerLogin: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Loggining = 0,
		EIP_Login,
		EIP_Password,
	};

	enum OUTPUTS
	{
		EOP_Success = 0,
		EOP_INCORRECT_PASS,
		EOP_LOGIN_NOT_FOUND,
		EOP_Error,
	};

public:
	CFlowNode_MasterServerLogin( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_MasterServerLogin()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_MasterServerLogin(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "TryLogin", _HELP("Trying login")),
			InputPortConfig<string>("Login", _HELP("Login")),
			InputPortConfig<string>("Password", _HELP("Login")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig_Void( "Success", _HELP("Login success")),
			OutputPortConfig_Void( "Password incorrect", _HELP("Password incorrect")),
			OutputPortConfig_Void( "Login not found", _HELP("Login not found")),
			OutputPortConfig<string>("Fail", _HELP("Login failed")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("CryMasterServer login node");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Loggining))
				{
					const string& login = GetPortString(pActInfo, 1);
					const string& password = GetPortString(pActInfo, 2);
					MasterServerLoggining(login, password);
				}
			}
			break;
		}
	}

	void MasterServerLoggining(string login, string password)
	{
		const char* result = gClientEnv->pMasterServer->Login(login,password);

		if(!strcmp(result,"PasswordCorrect"))
			ActivateOutput(&m_actInfo, EOP_Success, true);
		if(!strcmp(result,"PasswordIncorrect"))
			ActivateOutput(&m_actInfo, EOP_INCORRECT_PASS, true);
		if(!strcmp(result,"LoginNotFound"))
			ActivateOutput(&m_actInfo, EOP_LOGIN_NOT_FOUND, true);
		if(!strcmp(result,"AccountBlocked"))
			ActivateOutput(&m_actInfo, EOP_Error, string(result));
		if(!strcmp(result,"login_timeout"))
			ActivateOutput(&m_actInfo, EOP_Error, string("@ms_connection_timeout"));
	}

protected:
	SActivationInfo m_actInfo;
};

// Try register 
class CFlowNode_MasterServerRegister: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Register = 0,
		EIP_Login,
		EIP_Password,
		EIP_Nickname,
	};

	enum OUTPUTS
	{
		EOP_Success = 0,
		EOP_Error,
	};

public:
	CFlowNode_MasterServerRegister( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_MasterServerRegister()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_MasterServerRegister(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Register", _HELP("Trying register")),
			InputPortConfig<string>("Login", _HELP("Login")),
			InputPortConfig<string>("Password", _HELP("Password")),
			InputPortConfig<string>("Nickname", _HELP("Nickname")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig_Void( "Success", _HELP("Register success")),
			OutputPortConfig<string>("Fail", _HELP("Register failed")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("MasterServerRegister node");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Register))
				{
					const string& login = GetPortString(pActInfo, 1);
					const string& password = GetPortString(pActInfo, 2);
					const string& nickname = GetPortString(pActInfo, 3);
					MasterServerRegister(login, password, nickname);
				}
			}
			break;
		}
	}

	void MasterServerRegister(string login, string password,string nickname)
	{
		const char* result = gClientEnv->pMasterServer->Register(login,password,nickname);

		if(!strcmp(result,"RegSuccess"))
			ActivateOutput(&m_actInfo, EOP_Success, true);
		if(!strcmp(result,"LoginAlReg"))
			ActivateOutput(&m_actInfo, EOP_Error, string("@login_al_reg"));
		if(!strcmp(result,"RegFailed"))
			ActivateOutput(&m_actInfo, EOP_Error, string("@reg_failed"));
		if(!strcmp(result,"register_timeout"))
			ActivateOutput(&m_actInfo, EOP_Error, string("@ms_connection_timeout"));
	}

protected:
	SActivationInfo m_actInfo;
};

// Send chat message
class CFlowNode_SendChatMsg: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Send = 0,
		EIP_Msg,
		EIP_Type,
	};

	enum OUTPUTS
	{
	};

public:
	CFlowNode_SendChatMsg( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_SendChatMsg()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SendChatMsg(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Send", _HELP("Send chat msg")),
			InputPortConfig<string>("Message", _HELP("Message")),
			InputPortConfig<string>("Type", _HELP("Message type (Global, Private, etc.)")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Send chat message node");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Send))
				{
					const string& msg = GetPortString(pActInfo, EIP_Msg);
					const string& type = GetPortString(pActInfo, EIP_Type);


					if(!strcmp(type.c_str(),"Global"))
						gClientEnv->pMasterServer->SendGlobalChatMessage(msg.c_str());
				}
			}
			break;
		}
	}


protected:
	SActivationInfo m_actInfo;
};

// Game servers tools
class CFlowNode_GameServersTools: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Connect = 0,
		EIP_Refresh,
		EIP_IP,
		EIP_Port,
	};

	enum OUTPUTS
	{
		EOP_Success = 0,
		EOP_Fail,
	};

public:
	CFlowNode_GameServersTools( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_GameServersTools()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GameServersTools(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "Connect to game server", _HELP("Trying connecting to game server")),
			InputPortConfig_Void( "Refresh server list", _HELP("Refresh server list")),
			InputPortConfig<string>("IP", _HELP("IP")),
			InputPortConfig<int>("Port", _HELP("Port")),
			InputPortConfig<int>("ServerID", _HELP("ServerID")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig_Void("Success", _HELP("If connectection to server success")),
			OutputPortConfig<string>("Fail", _HELP("If connectection to server failed")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Connect to game server");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Connect))
				{
					const string& ip = GetPortString(pActInfo, 1);
					const int& port = GetPortInt(pActInfo, 2);
					
					ConnectToServer(ip,port);

				}


				if(IsPortActive(pActInfo, EIP_Refresh))
				{
					gClientEnv->pMasterServer->SendRequest("GetServers","",0);
				}
			}
			break;
		}
	}

	void ConnectToServer(string ip,int port)
	{
		char cl_ip[256];
		char cl_port[256];
		sprintf(cl_ip,"cl_serveraddr = %s",ip);
		sprintf(cl_port,"cl_serverport = %d", port);
		gEnv->pConsole->ExecuteString(cl_ip);
		gEnv->pConsole->ExecuteString(cl_port);
		gEnv->pConsole->ExecuteString("connect");
	}

protected:
	SActivationInfo m_actInfo;
};

// Get main information
class CFlowNode_GetInfo: public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Get = 0,
	};

	enum OUTPUTS
	{
		EOP_Nickname = 0,
		EOP_Level,
		EOP_GameCash,
		EOP_Xp,


		EOP_PlayersOnline,
		EOP_GameServersAmmount,
		EOP_ClientVersion,
	};

public:
	CFlowNode_GetInfo( SActivationInfo * pActInfo )
	{
	}

	~CFlowNode_GetInfo()
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_GetInfo(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_ports[] = 
		{
			InputPortConfig_Void( "GetInfo", _HELP("Get main info. E.g. Player, server info , etc.")),
			{0}
		};
		static const SOutputPortConfig out_ports[] = 
		{
			OutputPortConfig<string>("Nickname", _HELP("Player nickname")),
			OutputPortConfig<int>("Level", _HELP("Player level")),
			OutputPortConfig<int>("GameCash", _HELP("Player game cash")),
			OutputPortConfig<int>("Xp", _HELP("Player real cash")),

			OutputPortConfig<int>("Players online", _HELP("Ammount player on master server")),
			OutputPortConfig<int>("Game servers ammount", _HELP("Ammount game servers on master server")),
			OutputPortConfig<string>("Client version", _HELP("Client version")),
			{0}
		};
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.sDescription = _HELP("Get main info");
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Get))
				{
					if(!strcmp(gClientEnv->masterPlayer.nickname.c_str(),"Unknown"))
						gClientEnv->pMasterServer->SendRequest("GetPlayer","",0);
					else
					{
						ActivateOutput(pActInfo,EOP_Nickname,string(gClientEnv->masterPlayer.nickname.c_str()));
						ActivateOutput(pActInfo,EOP_Level,gClientEnv->masterPlayer.level);
						ActivateOutput(pActInfo,EOP_GameCash,gClientEnv->masterPlayer.money);
						ActivateOutput(pActInfo,EOP_Xp,gClientEnv->masterPlayer.xp);
					}

					if(gClientEnv->serverInfo.playersOnline == 0)
						gClientEnv->pMasterServer->SendRequest("GetMasterInfo","",0);
					else
					{
						ActivateOutput(pActInfo,EOP_PlayersOnline,gClientEnv->serverInfo.playersOnline);
						ActivateOutput(pActInfo,EOP_GameServersAmmount,gClientEnv->serverInfo.gameServersOnline);
						ActivateOutput(pActInfo,EOP_ClientVersion,string(gClientEnv->clientVersion));
					}
				}
			}
			break;
		}
	}

protected:
	SActivationInfo m_actInfo;
};

// System
REGISTER_FLOW_NODE("MasterServer:ConnectToMasterServer",	CFlowNode_MasterServerConnect);
REGISTER_FLOW_NODE("MasterServer:Login",	CFlowNode_MasterServerLogin);
REGISTER_FLOW_NODE("MasterServer:Register",	CFlowNode_MasterServerRegister);
REGISTER_FLOW_NODE("MasterServer:SendChatMsg",	CFlowNode_SendChatMsg);
REGISTER_FLOW_NODE("MasterServer:GetInfo",	CFlowNode_GetInfo);
// Game servers tools
REGISTER_FLOW_NODE("MasterServer:GameServersTools",	CFlowNode_GameServersTools);