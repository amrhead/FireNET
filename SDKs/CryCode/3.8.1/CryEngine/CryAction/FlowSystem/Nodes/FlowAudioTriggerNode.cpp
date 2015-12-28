////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2014.
//
// -------------------------------------------------------------------------
//  File name:   FlowAudioTriggerNode.cpp
//  Version:     v1.00
//  Created:     12/05/2014 by Mikhail Korotyaev
//  Description: FlowGraph Node that executes AudioTriggers.
//
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FlowBaseNode.h"

class CFlowNode_AudioTrigger : public CFlowBaseNode<eNCT_Instanced>
{
public:

	CFlowNode_AudioTrigger(SActivationInfo* pActInfo)
		:	m_bIsPlaying(false)
		, m_nPlayTriggerID(INVALID_AUDIO_CONTROL_ID)
		, m_nStopTriggerID(INVALID_AUDIO_CONTROL_ID)
	{
		gEnv->pAudioSystem->AddRequestListener(&CFlowNode_AudioTrigger::OnAudioTriggerFinished, this, eART_AUDIO_CALLBACK_MANAGER_REQUEST, eACMRT_REPORT_FINISHED_TRIGGER_INSTANCE);
	}

	//////////////////////////////////////////////////////////////////////////
	~CFlowNode_AudioTrigger() 
	{
		gEnv->pAudioSystem->RemoveRequestListener(&CFlowNode_AudioTrigger::OnAudioTriggerFinished, this);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_AudioTrigger(pActInfo);
	}

	//////////////////////////////////////////////////////////////////////////
	enum INPUTS
	{
		eIn_PlayTrigger,
		eIn_StopTrigger,
		eIn_Play,
		eIn_Stop,
	};

	enum OUTPUTS 
	{
		eOut_Done,
	};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = 
		{
			InputPortConfig<string>("audioTrigger_PlayTrigger", _HELP("name of the Play Trigger"), "PlayTrigger"),
			InputPortConfig<string>("audioTrigger_StopTrigger", _HELP("name of the Stop Trigger"), "StopTrigger"),

			InputPortConfig_Void("Play", _HELP("Executes the Play Trigger")),
			InputPortConfig_Void("Stop", _HELP("Executes the Stop Trigger if it is provided, o/w stops all events started by the Start Trigger")),
			{0}
		};

		static const SOutputPortConfig outputs[] = 
		{
			OutputPortConfig_Void( "Done", _HELP("Activated when all of the triggered events have finished playing")),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("This node executes Audio Triggers.");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		bool bPlay = m_bIsPlaying;
		ser.BeginGroup("FlowAudioTriggerNode");
		ser.Value("play", bPlay);
		ser.EndGroup();

		if (ser.IsReading())
		{
			m_bIsPlaying = false;
			Init(pActInfo, bPlay);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				Stop(pActInfo->pEntity);
				Init(pActInfo, false);

				break;
			}
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, eIn_Play))
				{
					m_oPlayActivationInfo = *pActInfo;
					Play(pActInfo->pEntity, pActInfo);
				}

				if (IsPortActive(pActInfo, eIn_Stop))
				{
					Stop(pActInfo->pEntity);
				}

				if (IsPortActive(pActInfo, eIn_PlayTrigger))
				{
					Stop(pActInfo->pEntity);
					GetTriggerID(pActInfo, eIn_PlayTrigger, m_nPlayTriggerID);
				}

				if (IsPortActive(pActInfo, eIn_StopTrigger))
				{
					GetTriggerID(pActInfo, eIn_StopTrigger, m_nStopTriggerID);
				}

				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	void TriggerFinished(TAudioControlID const nTriggerID)
	{
		if (nTriggerID == m_nPlayTriggerID)
		{
			ActivateOutput(&m_oPlayActivationInfo, eOut_Done, true);
		}
	}

private:

	struct STriggerNodeInfo
	{
		TFlowGraphId const		nFlowGraphID;
		TFlowNodeId	const			nFlowNodeID;
		TAudioControlID	const	nTriggerID;

		STriggerNodeInfo(TFlowGraphId const nPassedFlowGraphID, TFlowNodeId const nPassedFlowNodeID, TAudioControlID const nPassedTriggerID)
			: nFlowGraphID(nPassedFlowGraphID)
			, nFlowNodeID(nPassedFlowNodeID)
			, nTriggerID(nPassedTriggerID)
		{}

		~STriggerNodeInfo()
		{}
	};

	enum EPlayMode
	{
		ePM_None = 0,
		ePM_Play = 1,
		ePM_PlayStop = 2,
		ePM_ForceStop = 3,
	};

	//////////////////////////////////////////////////////////////////////////
	static void OnAudioTriggerFinished(SAudioRequestInfo const* const pAudioRequestInfo)
	{
		CFlowNode_AudioTrigger* pAudioTrigger = static_cast<CFlowNode_AudioTrigger*>(pAudioRequestInfo->pOwner);
		pAudioTrigger->TriggerFinished(pAudioRequestInfo->nAudioControlID);
	}

	//////////////////////////////////////////////////////////////////////////
	void GetTriggerID(SActivationInfo* const pActInfo, uint32 const nPortIdx, TAudioControlID& rOutTriggerID)
	{
		rOutTriggerID = INVALID_AUDIO_CONTROL_ID;
		string const& rTriggerName = GetPortString(pActInfo, nPortIdx);

		if (!rTriggerName.empty())
		{
			gEnv->pAudioSystem->GetAudioTriggerID(rTriggerName.c_str(), rOutTriggerID);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Init(SActivationInfo* const pActInfo, bool const bPlay)
	{
		m_bIsPlaying = false;
		if (gEnv->pAudioSystem != NULL)
		{
			GetTriggerID(pActInfo, eIn_PlayTrigger, m_nPlayTriggerID);
			GetTriggerID(pActInfo, eIn_StopTrigger, m_nStopTriggerID);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void ExecuteOnProxy(IEntity* const pEntity, TAudioControlID const nTriggerID, EPlayMode const ePlayMode)
	{
		IEntityAudioProxyPtr const pIEntityAudioProxy = crycomponent_cast<IEntityAudioProxyPtr>(pEntity->CreateProxy(ENTITY_PROXY_AUDIO));
		
		if (pIEntityAudioProxy != NULL)
		{
			switch (ePlayMode)
			{
			case ePM_Play:
				{
					SAudioCallBackInfos callbackInfos(this, NULL, this, eARF_PRIORITY_NORMAL | eARF_SYNC_FINISHED_CALLBACK);
					pIEntityAudioProxy->ExecuteTrigger(nTriggerID, eLSM_None, DEFAULT_AUDIO_PROXY_ID, callbackInfos);
					break;
				}
			case ePM_PlayStop:
				{
					pIEntityAudioProxy->ExecuteTrigger(nTriggerID, eLSM_None);

					break;
				}
			case ePM_ForceStop:
				{
					pIEntityAudioProxy->StopTrigger(nTriggerID);

					break;
				}
			default:
				{
					assert(false);// Unknown play mode!
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void ExecuteOnGlobalObject(TAudioControlID const nTriggerID, EPlayMode const ePlayMode)
	{
		switch (ePlayMode)
		{
		case ePM_Play:
			{
				m_oExecuteRequestData.nTriggerID = nTriggerID;
				m_oRequest.pOwner = this;
				m_oRequest.pData = &m_oExecuteRequestData;
				gEnv->pAudioSystem->PushRequest(m_oRequest);

				break;
			}
		case ePM_PlayStop:
			{
				m_oExecuteRequestData.nTriggerID = nTriggerID;
				m_oRequest.pData = &m_oExecuteRequestData;
				gEnv->pAudioSystem->PushRequest(m_oRequest);

				break;
			}
		case ePM_ForceStop:
			{
				m_oStopRequestData.nTriggerID = nTriggerID;
				m_oRequest.pData = &m_oStopRequestData;
				gEnv->pAudioSystem->PushRequest(m_oRequest);

				break;
			}
		default:
			{
				assert(false);// Unknown play mode!
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Play(IEntity* const pEntity, SActivationInfo const* const pActInfo)
	{
		if (m_nPlayTriggerID != INVALID_AUDIO_CONTROL_ID)
		{
			if (pEntity != NULL)
			{
				ExecuteOnProxy(pEntity, m_nPlayTriggerID, ePM_Play);
			}
			else
			{
				ExecuteOnGlobalObject(m_nPlayTriggerID, ePM_Play);
			}

			m_bIsPlaying = true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Stop(IEntity* const pEntity)
	{
		if (m_bIsPlaying)
		{
			if (m_nStopTriggerID != INVALID_AUDIO_CONTROL_ID)
			{
				if (pEntity != NULL)
				{
					ExecuteOnProxy(pEntity, m_nStopTriggerID, ePM_PlayStop);
				}
				else
				{
					ExecuteOnGlobalObject(m_nStopTriggerID, ePM_PlayStop);
				}
			}
			else
			{
				if (pEntity != NULL)
				{
					ExecuteOnProxy(pEntity, m_nPlayTriggerID, ePM_ForceStop);
				}
				else
				{
					ExecuteOnGlobalObject(m_nPlayTriggerID, ePM_ForceStop);
				}
			}

			m_bIsPlaying = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool						m_bIsPlaying;
	TAudioControlID	m_nPlayTriggerID;	
	TAudioControlID	m_nStopTriggerID;

	SActivationInfo m_oPlayActivationInfo;

	SAudioRequest m_oRequest;
	SAudioObjectRequestData<eAORT_EXECUTE_TRIGGER> m_oExecuteRequestData;
	SAudioObjectRequestData<eAORT_STOP_TRIGGER> m_oStopRequestData;
};

REGISTER_FLOW_NODE("Audio:Trigger", CFlowNode_AudioTrigger);
