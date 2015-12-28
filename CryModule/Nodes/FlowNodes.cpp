/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 15.05.2015   17:57 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include "../System/Global.h"

CG2AutoRegFlowNodeBase* CG2AutoRegFlowNodeBase::m_pFirst = 0;
CG2AutoRegFlowNodeBase* CG2AutoRegFlowNodeBase::m_pLast = 0;

namespace FNM
{
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
				InputPortConfig_Void( "Connect", _HELP("Connect to FireNet master server")),
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
			config.sDescription = _HELP("Connect to FireNet master server");
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
			EIP_Send = 0,
			EIP_Login,
			EIP_Password,
		};

		enum OUTPUTS
		{
			EOP_Success = 0,
			EOP_Fail,
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
				InputPortConfig_Void( "Send", _HELP("Send login request")),
				InputPortConfig<string>("Login", _HELP("Login")),
				InputPortConfig<string>("Password", _HELP("Login")),
				{0}
			};
			static const SOutputPortConfig out_ports[] = 
			{
				OutputPortConfig_Void( "Success", _HELP("Login success")),
				OutputPortConfig<string>( "Fail", _HELP("Login fail")),
				OutputPortConfig<string>("Error", _HELP("Error")),
				{0}
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send login request to FireNet master server");
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
						const string& login = GetPortString(pActInfo, 1);
						const string& password = GetPortString(pActInfo, 2);
						Loggining(login, password);
					}
				}
				break;
			}
		}

		void Loggining(string login, string password)
		{
			int loginSize = login.size();
			int passwordSize = password.size();

			if(loginSize >= 4 && passwordSize >= 6)
			{
				const char* result = gClientEnv->pMasterServer->Login(login,password);

				if(!strcmp(result,"PasswordCorrect"))
					ActivateOutput(&m_actInfo, EOP_Success, true);
				if(!strcmp(result,"PasswordIncorrect"))
					ActivateOutput(&m_actInfo, EOP_Fail, string("@incorrect_password"));
				if(!strcmp(result,"LoginNotFound"))
					ActivateOutput(&m_actInfo, EOP_Fail, string("@login_not_found"));
				if(!strcmp(result,"AccountBlocked"))
					ActivateOutput(&m_actInfo, EOP_Fail, string("@account_blocked"));
				if(!strcmp(result,"BlockDual"))
					ActivateOutput(&m_actInfo, EOP_Fail, string("@attempt_dual_auth"));

				if(!strcmp(result,"login_timeout"))
					ActivateOutput(&m_actInfo, EOP_Error, string("@ms_connection_timeout"));
			}
			else
			{
				if(loginSize<4)	ActivateOutput(&m_actInfo, EOP_Fail, string("@short_login")); 
				if(passwordSize<6) ActivateOutput(&m_actInfo, EOP_Fail, string("@short_password")); 
			}
		}

	protected:
		SActivationInfo m_actInfo;
	};

	// Try register 
	class CFlowNode_MasterServerRegister: public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Send = 0,
			EIP_Login,
			EIP_Password,
			EIP_Nickname,
		};

		enum OUTPUTS
		{
			EOP_Success = 0,
			EOP_Fail,
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
				InputPortConfig_Void( "Send", _HELP("Send register request")),
				InputPortConfig<string>("Login", _HELP("Login")),
				InputPortConfig<string>("Password", _HELP("Password")),
				InputPortConfig<string>("Nickname", _HELP("Nickname")),
				{0}
			};
			static const SOutputPortConfig out_ports[] = 
			{
				OutputPortConfig_Void( "Success", _HELP("Register success")),
				OutputPortConfig<string>("Fail", _HELP("Register failed")),
				OutputPortConfig<string>("Error", _HELP("Error")),
				{0}
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send register request to FireNet master server");
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
						const string& login = GetPortString(pActInfo, 1);
						const string& password = GetPortString(pActInfo, 2);
						const string& nickname = GetPortString(pActInfo, 3);
						Register(login, password, nickname);
					}
				}
				break;
			}
		}

		void Register(string login, string password,string nickname)
		{
			int loginSize = login.size();
			int passwordSize = password.size();
			int nicknameSize = nickname.size();

			if(loginSize >= 4 && passwordSize >= 6 && nicknameSize >= 4)
			{

				const char* result = gClientEnv->pMasterServer->Register(login,password,nickname);

				if(!strcmp(result,"RegSuccess"))
					ActivateOutput(&m_actInfo, EOP_Success, true);
				if(!strcmp(result,"LoginAlReg"))
					ActivateOutput(&m_actInfo, EOP_Fail, string("@login_al_reg"));
				if(!strcmp(result,"RegFailed"))
					ActivateOutput(&m_actInfo, EOP_Fail, string("@reg_failed"));
				if(!strcmp(result,"register_timeout"))
					ActivateOutput(&m_actInfo, EOP_Error, string("@ms_connection_timeout"));
			}
			else
			{
				if(loginSize<4)	ActivateOutput(&m_actInfo, EOP_Fail, string("@short_login")); 
				if(nicknameSize<4)	ActivateOutput(&m_actInfo, EOP_Fail, string("@short_nickname")); 
				if(passwordSize<6) ActivateOutput(&m_actInfo, EOP_Fail, string("@short_password")); 
			}
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
			EIP_PrivateID, 
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
				InputPortConfig_Void( "Send", _HELP("Send chat message to FireNet master server")),
				InputPortConfig<string>("Message", _HELP("Message")),
				InputPortConfig<string>("Type", _HELP("Message type (Global / Private)")),
				InputPortConfig<int>("PrivatePlayerId", _HELP("Player master id to send a private message (NOT USE IN GLOBAL MESSAGES)")),
				{0}
			};
			static const SOutputPortConfig out_ports[] = 
			{
				{0}
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send chat message to FireNet master server");
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
						{
							string tmp  = msg;
							tmp.replace("#comma#",",");

							if(tmp.size()>=1)
								gClientEnv->pMasterServer->SendGlobalChatMessage(tmp.c_str());
						}
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
			EIP_Id,
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
				InputPortConfig_Void( "Connect to game server", _HELP("Trying connecting to game server by ServerId from server list")),
				InputPortConfig_Void( "Refresh server list", _HELP("Refresh server list")),
				InputPortConfig<int>("ServerID", _HELP("ServerID (Needs to connect by id)")),
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
						const int& id = GetPortInt(pActInfo, EIP_Id);
						const char* ip;
						int port;
						char cl_ip[256];
						char cl_port[256];
						bool result = false;

						// Find server by id
						for(auto it = gClientEnv->pMasterServer->vServers.begin(); it != gClientEnv->pMasterServer->vServers.end(); ++it)
						{
							if(it->id == id)
							{
								ip = it->ip;
								port = it->port;
								result = true;
								break;
							}
						}	
						//

						if(result)
						{
							sprintf(cl_ip,"cl_serveraddr = %s",ip);
							sprintf(cl_port,"cl_serverport = %d", port);

							gEnv->pConsole->ExecuteString(cl_ip);
							gEnv->pConsole->ExecuteString(cl_port);
							gEnv->pConsole->ExecuteString("connect");
						}
						else
						{
							ActivateOutput(pActInfo,EOP_Fail,string("@sv_not_aviable"));
						}
					}


					if(IsPortActive(pActInfo, EIP_Refresh))
					{
						// Remove all servers from server browser and servers vector
						for(auto it = gClientEnv->pMasterServer->vServers.begin(); it != gClientEnv->pMasterServer->vServers.end(); ++it)
						{
							SUIArguments args;
							args.AddArgument(it->id);
							CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_RemoveServer, args);
						}

						gClientEnv->pMasterServer->vServers.clear();

						gClientEnv->pMasterServer->SendRequest("GetServers","",0);
					}
				}
				break;
			}
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
				InputPortConfig_Void( "Get", _HELP("Get main info. E.g. Player, server info , etc.")),
				{0}
			};
			static const SOutputPortConfig out_ports[] = 
			{
				OutputPortConfig<string>("Nickname", _HELP("Player nickname")),
				OutputPortConfig<int>("Xp", _HELP("Player real cash")),
				OutputPortConfig<int>("Level", _HELP("Player level")),
				OutputPortConfig<int>("GameCash", _HELP("Player game cash")),
				OutputPortConfig<int>("Players online", _HELP("Ammount player on master server")),
				OutputPortConfig<int>("Game servers ammount", _HELP("Ammount game servers on master server")),
				OutputPortConfig<string>("Client version", _HELP("Client version")),
				{0}
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Get main info about player/server");
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
}

REGISTER_FLOW_NODE_EX("FireNet:MasterServer:ConnectToMasterServer", FNM::CFlowNode_MasterServerConnect,	CFlowNode_MasterServerConnect);
REGISTER_FLOW_NODE_EX("FireNet:MasterServer:SendLoginRequest", FNM::CFlowNode_MasterServerLogin,	CFlowNode_MasterServerLogin);
REGISTER_FLOW_NODE_EX("FireNet:MasterServer:SendRegisterRequest", FNM::CFlowNode_MasterServerRegister,	CFlowNode_MasterServerRegister);
REGISTER_FLOW_NODE_EX("FireNet:MasterServer:SendChatMessage", FNM::CFlowNode_SendChatMsg,	CFlowNode_SendChatMsg);
REGISTER_FLOW_NODE_EX("FireNet:MasterServer:GetMasterInformation", FNM::CFlowNode_GetInfo,	CFlowNode_GetInfo);
REGISTER_FLOW_NODE_EX("FireNet:MasterServer:GameServersTools", FNM::CFlowNode_GameServersTools,	CFlowNode_GameServersTools);