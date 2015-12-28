// Copyright 2001-2015 Crytek GmbH. All rights reserved.

#include "StdAfx.h"
#include "FlowBaseNode.h"
#include <ISystem.h>
#include <IEntityProxy.h>
#include <IDynamicResponseSystem.h>

class CFlowNode_SendDynamicResponseSignal : public CFlowBaseNode<eNCT_Instanced>
{
public:

	explicit CFlowNode_SendDynamicResponseSignal(SActivationInfo* pActInfo) 
	{
	}

	//////////////////////////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SendDynamicResponseSignal(pActInfo);
	}

	//////////////////////////////////////////////////////////////////////////
	enum EInputs
	{
		eIn_Send,
		eIn_Cancel,
		eIn_SignalName,
		eIn_SignalDelay,
		eIn_ContextVariableCollection,
		eIn_AutoReleaseContextCollection,
	};

	enum EOutputs 
	{
		eOut_Done,
	};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = 
		{
			InputPortConfig_Void("Send", _HELP("send dynamic response signal name")),
			InputPortConfig_Void("Cancel", _HELP("cancel dynamic response signal name")),
			InputPortConfig<string>("SignalName", _HELP("dynamic response signal name"), "Name"),
			InputPortConfig<float>("SignalDelay", _HELP("signal delay value"), "Delay"),
			InputPortConfig<string>("ContextCollection", _HELP("the name of the variable collection sent along with the signal as a context"), "ContextCollection"),
			InputPortConfig<bool>("AutoReleaseContextCollection", true, _HELP("should the VariableContextCollection (if specified) be released, after processing the signal? Should always be true for temporary created context collections, otherwise they will never be released!"), "AutoReleaseContextCollection"),
			{0}
		};

		static const SOutputPortConfig outputs[] = 
		{
			OutputPortConfig<string> ("Done", _HELP("Will be triggered if the signal was sent/canceled")),
			{0}
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("This node sends a signal to the Dynamic Response System.");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				break;
			}
		case eFE_Activate:
			{
				if (pActInfo->pEntity)
				{
					const string& signalName = GetPortString(pActInfo, eIn_SignalName);
					const float signalDelay = GetPortFloat(pActInfo, eIn_SignalDelay);
					IEntityDynamicResponseProxyPtr const pIEntityDrsProxy = crycomponent_cast<IEntityDynamicResponseProxyPtr>(pActInfo->pEntity->CreateProxy(ENTITY_PROXY_DYNAMICRESPONSE));

					if (IsPortActive(pActInfo, eIn_Send) || IsPortActive(pActInfo, eIn_ContextVariableCollection))
					{ 
						const string& collectionName = GetPortString(pActInfo, eIn_ContextVariableCollection);
						DRS::IVariableCollection* variableCollection = 0;
						if (!collectionName.empty())
						{
							variableCollection = gEnv->pDynamicResponseSystem->GetCollection(collectionName.c_str());
						}
						pIEntityDrsProxy->QueueSignal(signalName.c_str(), variableCollection, signalDelay, GetPortBool(pActInfo, eIn_AutoReleaseContextCollection));
						ActivateOutput(pActInfo, eOut_Done, true);
					}
					else if (IsPortActive(pActInfo, eIn_Cancel))
					{ 
						gEnv->pDynamicResponseSystem->CancelSignalProcessing(signalName, pIEntityDrsProxy->GetResponseActor(), signalDelay);
						ActivateOutput(pActInfo, eOut_Done, true);
					}
					break;
				}
				else
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CAction:DRS_SendSignal: Cannot send a signal without an specified EntityID.");
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

private:
	//////////////////////////////////////////////////////////////////////////
};


REGISTER_FLOW_NODE("DynamicResponse:SendSignal", CFlowNode_SendDynamicResponseSignal);
