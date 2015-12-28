//==================================================================================================
// Name: CGameEffectsSystem
// Desc: System to handle game effects, render nodes and render elements
// Author: James Chilvers
//==================================================================================================

// Includes
#include "StdAfx.h"
#include "GameEffectsSystem.h"
#include "Effects/GameEffects/GameEffect.h"
#include "GameCVars.h"
#include "ItemSystem.h"
#include "BitFiddling.h"
#include "Effects/RenderNodes/IGameRenderNode.h"
#include "Effects/RenderElements/GameRenderElement.h"
#include "GameRules.h"

//--------------------------------------------------------------------------------------------------
// Desc: Defines
//--------------------------------------------------------------------------------------------------
#define GAME_FX_DATA_FILE	"scripts/effects/gameeffects.xml"
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Desc: Debug data
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
	int	CGameEffectsSystem::s_currentDebugEffectId = 0;

	const char* GAME_FX_DEBUG_VIEW_NAMES[eMAX_GAME_FX_DEBUG_VIEWS] = {	"None",
																																			"Profiling",
																																			"Effect List",
																																			"Bounding Box",
																																			"Bounding Sphere",
																																			"Particles" };

#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Desc: Static data
//--------------------------------------------------------------------------------------------------
CGameEffectsSystem*	CGameEffectsSystem::s_singletonInstance = NULL;
bool								CGameEffectsSystem::s_hasLoadedData = false;
int									CGameEffectsSystem::s_postEffectCVarNameOffset = 0;
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Desc: Game Effect System Static data - contains access to any data where static initialisation
//		   order is critical, this will enforce initialisation on first use
//--------------------------------------------------------------------------------------------------
struct SGameEffectSystemStaticData
{
	static PodArray<DataLoadCallback>&	GetDataLoadCallbackList()
	{
		static PodArray<DataLoadCallback>	dataLoadCallbackList;
		return dataLoadCallbackList;
	}

	static PodArray<DataReleaseCallback>&	GetDataReleaseCallbackList()
	{
		static PodArray<DataReleaseCallback>	dataReleaseCallbackList;
		return dataReleaseCallbackList;
	}

	static PodArray<DataReloadCallback>&	GetDataReloadCallbackList()
	{
		static PodArray<DataReloadCallback>	dataReloadCallbackList;
		return dataReloadCallbackList;
	}

	static PodArray<EnteredGameCallback>&	GetEnteredGameCallbackList()
	{
		static PodArray<EnteredGameCallback>	enteredGameCallbackList;
		return enteredGameCallbackList;
	}

#if DEBUG_GAME_FX_SYSTEM
	static PodArray<CGameEffectsSystem::SEffectDebugData>&	GetEffectDebugList()
	{
		static PodArray<CGameEffectsSystem::SEffectDebugData>	effectDebugList;
		return effectDebugList;
	}
#endif
};
// Easy access macros
#define s_dataLoadCallbackList			SGameEffectSystemStaticData::GetDataLoadCallbackList()
#define s_dataReleaseCallbackList		SGameEffectSystemStaticData::GetDataReleaseCallbackList()
#define s_dataReloadCallbackList		SGameEffectSystemStaticData::GetDataReloadCallbackList()
#define s_enteredGameCallbackList		SGameEffectSystemStaticData::GetEnteredGameCallbackList()
#if DEBUG_GAME_FX_SYSTEM
	#define s_effectDebugList						SGameEffectSystemStaticData::GetEffectDebugList()
#endif
//--------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: CGameRenderNodeSoftCodeListener
// Desc: Game Render Node Soft Code Listener
// Author: James Chilvers
//==================================================================================================
class CGameRenderNodeSoftCodeListener : public ISoftCodeListener
{
public:
	CGameRenderNodeSoftCodeListener()
	{
		if(gEnv->pSoftCodeMgr)
		{
			gEnv->pSoftCodeMgr->AddListener(GAME_RENDER_NODE_LIBRARY_NAME,this,GAME_RENDER_NODE_LISTENER_NAME);
		}
	}
	virtual ~CGameRenderNodeSoftCodeListener()
	{
		if(gEnv->pSoftCodeMgr)
		{
			gEnv->pSoftCodeMgr->RemoveListener(GAME_RENDER_NODE_LIBRARY_NAME,this);
		}
	}

	virtual void			InstanceReplaced(void* pOldInstance, void* pNewInstance)
	{
		GAME_FX_SYSTEM.GameRenderNodeInstanceReplaced(pOldInstance,pNewInstance);
	}
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: CGameRenderElementSoftCodeListener
// Desc: Game Render Element Soft Code Listener
// Author: James Chilvers
//==================================================================================================
class CGameRenderElementSoftCodeListener : public ISoftCodeListener
{
public:
	CGameRenderElementSoftCodeListener()
	{
		if(gEnv->pSoftCodeMgr)
		{
			gEnv->pSoftCodeMgr->AddListener(GAME_RENDER_ELEMENT_LIBRARY_NAME,this,GAME_RENDER_ELEMENT_LISTENER_NAME);
		}
	}
	virtual ~CGameRenderElementSoftCodeListener()
	{
		if(gEnv->pSoftCodeMgr)
		{
			gEnv->pSoftCodeMgr->RemoveListener(GAME_RENDER_ELEMENT_LIBRARY_NAME,this);
		}
	}

	virtual void			InstanceReplaced(void* pOldInstance, void* pNewInstance)
	{
		GAME_FX_SYSTEM.GameRenderElementInstanceReplaced(pOldInstance,pNewInstance);
	}
};//------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: CGameEffectsSystem
// Desc: Constructor
//--------------------------------------------------------------------------------------------------
CGameEffectsSystem::CGameEffectsSystem()
{
#if DEBUG_GAME_FX_SYSTEM
	if(gEnv->pInput)
	{
		gEnv->pInput->AddEventListener(this);
	}
#endif

#ifdef SOFTCODE_ENABLED
	if(gEnv->pSoftCodeMgr)
	{
		gEnv->pSoftCodeMgr->AddListener(GAME_FX_LIBRARY_NAME,this,GAME_FX_LISTENER_NAME);
	}

	m_gameRenderNodes.clear();
	m_gameRenderNodeSoftCodeListener = new CGameRenderNodeSoftCodeListener;

	m_gameRenderElements.clear();
	m_gameRenderElementSoftCodeListener = new CGameRenderElementSoftCodeListener;

	RegisterSoftCodeLib(IGameEffect::TLibrary::Instance());
	RegisterSoftCodeLib(IGameRenderNode::TLibrary::Instance());
	RegisterSoftCodeLib(IGameRenderElement::TLibrary::Instance());
#endif

	Reset();
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ~CGameEffectsSystem
// Desc: Destructor
//--------------------------------------------------------------------------------------------------
CGameEffectsSystem::~CGameEffectsSystem()
{
#if DEBUG_GAME_FX_SYSTEM
	if(gEnv->pInput)
	{
		gEnv->pInput->RemoveEventListener(this);
	}
#endif

#ifdef SOFTCODE_ENABLED
	if(gEnv->pSoftCodeMgr)
	{
		gEnv->pSoftCodeMgr->RemoveListener(GAME_FX_LIBRARY_NAME,this);
	}

	SAFE_DELETE(m_gameRenderNodeSoftCodeListener);
	SAFE_DELETE(m_gameRenderElementSoftCodeListener);
	m_softCodeTypeLibs.clear();
	m_gameRenderNodes.clear();
	m_gameRenderElements.clear();
#endif

	// Remove as game rules listener
	if(g_pGame)
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		if(pGameRules)
		{
			pGameRules->RemoveGameRulesListener(this);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Destroy
// Desc: Destroys effects system
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::Destroy()
{
	if(s_singletonInstance)
	{
		s_singletonInstance->AutoReleaseAndDeleteFlaggedEffects(s_singletonInstance->m_effectsToUpdate);
		s_singletonInstance->AutoReleaseAndDeleteFlaggedEffects(s_singletonInstance->m_effectsNotToUpdate);

		FX_ASSERT_MESSAGE(	(s_singletonInstance->m_effectsToUpdate==NULL) && 
												(s_singletonInstance->m_effectsNotToUpdate==NULL),
												"Game Effects System being destroyed even though game effects still exist!");
	}

	SAFE_DELETE(s_singletonInstance);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Reset
// Desc: Resets effects systems data
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::Reset()
{
	m_isInitialised = false;
	m_effectsToUpdate = NULL;
	m_effectsNotToUpdate = NULL;
	m_nextEffectToUpdate = NULL;
	s_postEffectCVarNameOffset = 0;

#if DEBUG_GAME_FX_SYSTEM
	m_debugView = eGAME_FX_DEBUG_VIEW_None;
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Initialise
// Desc: Initialises effects system
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::Initialise()
{
	if(m_isInitialised == false)
	{
		Reset();
		SetPostEffectCVarCallbacks();

		m_isInitialised = true;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GameRulesInitialise
// Desc: Game Rules initialise
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::GameRulesInitialise()
{
	// Add as game rules listener
	if(g_pGame)
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		if(pGameRules)
		{
			pGameRules->AddGameRulesListener(this);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: LoadData
// Desc: Loads data for game effects system and effects
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::LoadData()
{
	if(s_hasLoadedData == false)
	{
		XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFromFile(GAME_FX_DATA_FILE);

		if(!rootNode)
		{
			GameWarning("Could not load game effects data. Invalid XML file '%s'! ", GAME_FX_DATA_FILE);
			return;
		}

		IItemParamsNode* paramNode = g_pGame->GetIGameFramework()->GetIItemSystem()->CreateParams();
		if(paramNode)
		{
			paramNode->ConvertFromXML(rootNode);

			// Pass data to any callback functions registered
			int callbackCount = s_dataLoadCallbackList.size();
			DataLoadCallback dataLoadCallbackFunc = NULL;
			for(int i=0; i<callbackCount; i++)
			{
				dataLoadCallbackFunc = s_dataLoadCallbackList[i];
				if(dataLoadCallbackFunc)
				{
					dataLoadCallbackFunc(paramNode);
				}
			}

			paramNode->Release();
			s_hasLoadedData = true;
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReleaseData
// Desc: Releases any loaded data for game effects system and any effects with registered callbacks
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::ReleaseData()
{
	if(s_hasLoadedData)
	{

#if DEBUG_GAME_FX_SYSTEM
		// Unload all debug effects which rely on effect data
		for(size_t i=0; i<s_effectDebugList.Size(); i++)
		{
			if(s_effectDebugList[i].inputCallback)
			{
				s_effectDebugList[i].inputCallback(GAME_FX_INPUT_ReleaseDebugEffect);
			}
		}
#endif

		// Pass data to any callback functions registered
		int callbackCount = s_dataReleaseCallbackList.size();
		DataReleaseCallback dataReleaseCallbackFunc = NULL;
		for(int i=0; i<callbackCount; i++)
		{
			dataReleaseCallbackFunc = s_dataReleaseCallbackList[i];
			if(dataReleaseCallbackFunc)
			{
				dataReleaseCallbackFunc();
			}
		}
		s_hasLoadedData = false;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReloadData
// Desc: Reloads any loaded data for game effects registered callbacks
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::ReloadData()
{
	XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFromFile(GAME_FX_DATA_FILE);

	if(!rootNode)
	{
		GameWarning("Could not load game effects data. Invalid XML file '%s'! ", GAME_FX_DATA_FILE);
		return;
	}

	IItemParamsNode* paramNode = g_pGame->GetIGameFramework()->GetIItemSystem()->CreateParams();
	if(paramNode)
	{
		paramNode->ConvertFromXML(rootNode);

		// Pass data to any callback functions registered
		int callbackCount = s_dataReloadCallbackList.size();
		DataReloadCallback dataReloadCallbackFunc = NULL;
		for(int i=0; i<callbackCount; i++)
		{
			dataReloadCallbackFunc = s_dataReloadCallbackList[i];
			if(dataReloadCallbackFunc)
			{
				dataReloadCallbackFunc(paramNode);
			}
		}

		paramNode->Release();
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: EnteredGame
// Desc: Called when entering a game
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::EnteredGame()
{
	const int callbackCount = s_enteredGameCallbackList.size();
	EnteredGameCallback enteredGameCallbackFunc = NULL;
	for(int i=0; i<callbackCount; i++)
	{
		enteredGameCallbackFunc = s_enteredGameCallbackList[i];
		if(enteredGameCallbackFunc)
		{
			enteredGameCallbackFunc();
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterDataLoadCallback
// Desc: Registers data load callback
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterDataLoadCallback(DataLoadCallback dataLoadCallback)
{
	if(dataLoadCallback)
	{
		s_dataLoadCallbackList.push_back(dataLoadCallback);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterDataReleaseCallback
// Desc: Registers data release callback
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterDataReleaseCallback(DataReleaseCallback dataReleaseCallback)
{
	if(dataReleaseCallback)
	{
		s_dataReleaseCallbackList.push_back(dataReleaseCallback);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterDataReloadCallback
// Desc: Registers data reload callback
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterDataReloadCallback(DataReloadCallback dataReloadCallback)
{
	if(dataReloadCallback)
	{
		s_dataReloadCallbackList.push_back(dataReloadCallback);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterEnteredGameCallback
// Desc: Registers entered game callback
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterEnteredGameCallback(EnteredGameCallback enteredGameCallback)
{
	if(enteredGameCallback)
	{
		s_enteredGameCallbackList.push_back(enteredGameCallback);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: AutoReleaseAndDeleteFlaggedEffects
// Desc: Calls release and delete on any effects with the these flags set
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::AutoReleaseAndDeleteFlaggedEffects(IGameEffect* effectList)
{
	if(effectList)
	{
		IGameEffect* effect = effectList;
		while(effect)
		{
			m_nextEffectToUpdate = effect->Next();

			bool autoRelease = effect->IsFlagSet(GAME_EFFECT_AUTO_RELEASE);
			bool autoDelete = effect->IsFlagSet(GAME_EFFECT_AUTO_DELETE);

			if(autoRelease || autoDelete)
			{
				SOFTCODE_RETRY(effect,effect->Release());
				if(autoDelete)
				{
					SAFE_DELETE(effect);
				}
			}

			effect = m_nextEffectToUpdate;
		}
		m_nextEffectToUpdate = NULL;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: InstanceReplaced
// Desc: Replaces instance for soft coding
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::InstanceReplaced(void* pOldInstance, void* pNewInstance)
{
#ifdef SOFTCODE_ENABLED
	if(pNewInstance && pOldInstance)
	{
		IGameEffect* pNewGameEffectInstance = static_cast<IGameEffect*>(pNewInstance);
		IGameEffect* pOldGameEffectInstance = static_cast<IGameEffect*>(pOldInstance);

		// Copy over flags and remove registered flag so new effect instance can be registered
		// We haven't used the SOFT macro on the m_flags member because the oldInstance's flags
		// would then be Nulled out, and they are needed for the effect to be deregistered
		uint32 oldGameEffectFlags = pOldGameEffectInstance->GetFlags();
		SET_FLAG(oldGameEffectFlags,GAME_EFFECT_REGISTERED,false);
		pNewGameEffectInstance->SetFlags(oldGameEffectFlags);

		// Register new effect instance, old instance will get unregistered by destructor
		GAME_FX_SYSTEM.RegisterEffect(pNewGameEffectInstance);

		// Reload all data used by effects, then data can be added/removed for soft coding
		ReloadData();

		// Data used by effect will be copied to new effect, so mustn't release it but
		// must set flag so destructor doesn't assert
		pOldGameEffectInstance->SetFlag(GAME_EFFECT_RELEASED,true);
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GameRenderNodeInstanceReplaced
// Desc: Replaces Game Render Node instance for soft coding
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::GameRenderNodeInstanceReplaced(void* pOldInstance, void* pNewInstance)
{
#ifdef SOFTCODE_ENABLED
	if(pOldInstance && pNewInstance)
	{
		IGameRenderNode* pOldGameRenderNodeInstance = (IGameRenderNode*)pOldInstance;
		IGameRenderNode* pNewGameRenderNodeInstance = (IGameRenderNode*)pNewInstance;

		// Unregister old node from engine		
		gEnv->p3DEngine->FreeRenderNodeState(pOldGameRenderNodeInstance);

		// Register new node with engine
		gEnv->p3DEngine->RegisterEntity(pNewGameRenderNodeInstance);

		for(TGameRenderNodeVec::iterator iter(m_gameRenderNodes.begin()); iter != m_gameRenderNodes.end(); ++iter)
		{
			IGameRenderNodePtr* ppRenderNode = *iter;
			if(ppRenderNode)
			{
				IGameRenderNodePtr& pRenderNode = *ppRenderNode;

				if(pRenderNode == pOldGameRenderNodeInstance)
				{
					pRenderNode = pNewGameRenderNodeInstance;
				}
			}
		}
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GameRenderElementInstanceReplaced
// Desc: Replaces Game Render Element instance for soft coding
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::GameRenderElementInstanceReplaced(void* pOldInstance, void* pNewInstance)
{
#ifdef SOFTCODE_ENABLED
	if(pOldInstance && pNewInstance)
	{
		IGameRenderElement* pOldGameRenderElementInstance = (IGameRenderElement*)pOldInstance;
		IGameRenderElement* pNewGameRenderElementInstance = (IGameRenderElement*)pNewInstance;

		pNewGameRenderElementInstance->UpdatePrivateImplementation();

		for(TGameRenderElementVec::iterator iter(m_gameRenderElements.begin()); iter != m_gameRenderElements.end(); ++iter)
		{
			IGameRenderElementPtr* ppRenderElement = *iter;
			if(ppRenderElement)
			{
				IGameRenderElementPtr& pRenderElement = *ppRenderElement;

				if(pRenderElement == pOldGameRenderElementInstance)
				{
					pRenderElement = pNewGameRenderElementInstance;
				}
			}
		}
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetPostEffectCVarCallbacks
// Desc: Sets Post effect CVar callbacks for testing and tweaking post effect values
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::SetPostEffectCVarCallbacks()
{
#if DEBUG_GAME_FX_SYSTEM
	ICVar* postEffectCvar = NULL;
	const char postEffectNames[][64] = {	"g_postEffect.FilterGrain_Amount", 
																				"g_postEffect.FilterRadialBlurring_Amount",
																				"g_postEffect.FilterRadialBlurring_ScreenPosX",
																				"g_postEffect.FilterRadialBlurring_ScreenPosY",
																				"g_postEffect.FilterRadialBlurring_Radius",
																				"g_postEffect.Global_User_ColorC",
																				"g_postEffect.Global_User_ColorM",
																				"g_postEffect.Global_User_ColorY",
																				"g_postEffect.Global_User_ColorK",
																				"g_postEffect.Global_User_Brightness",
																				"g_postEffect.Global_User_Contrast",
																				"g_postEffect.Global_User_Saturation",
																				"g_postEffect.Global_User_ColorHue",
																				"g_postEffect.HUD3D_Interference",
																				"g_postEffect.HUD3D_FOV"};

	int postEffectNameCount = sizeof(postEffectNames)/sizeof(*postEffectNames);

	if(postEffectNameCount > 0)
	{
		// Calc name offset
		const char* postEffectName = postEffectNames[0];
		s_postEffectCVarNameOffset = 0;
		while((*postEffectName) != 0)
		{
			s_postEffectCVarNameOffset++;
			if((*postEffectName) == '.')
			{
				break;
			}
			postEffectName++;
		}

		// Set callback functions
		for(int i=0; i<postEffectNameCount; i++)
		{
			postEffectCvar = gEnv->pConsole->GetCVar(postEffectNames[i]);
			if(postEffectCvar)
			{
				postEffectCvar->SetOnChangeCallback(PostEffectCVarCallback);
			}
		}
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: PostEffectCVarCallback
// Desc: Callback function of post effect cvars to set their values
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::PostEffectCVarCallback(ICVar* cvar)
{
	const char* effectName = cvar->GetName() + s_postEffectCVarNameOffset;
	gEnv->p3DEngine->SetPostEffectParam(effectName,cvar->GetFVal());
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterEffect
// Desc: Registers effect with effect system
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterEffect(IGameEffect* effect)
{
	FX_ASSERT_MESSAGE(m_isInitialised,"Game Effects System trying to register an effect without being initialised");
	FX_ASSERT_MESSAGE(effect,"Trying to Register a NULL effect");

	if(effect)
	{
		// If effect is registered, then unregister first
		if(effect->IsFlagSet(GAME_EFFECT_REGISTERED))
		{
			UnRegisterEffect(effect);
		}

		// Add effect to effect list
		IGameEffect** effectList = NULL;
		bool isActive = effect->IsFlagSet(GAME_EFFECT_ACTIVE);
		bool autoUpdatesWhenActive = effect->IsFlagSet(GAME_EFFECT_AUTO_UPDATES_WHEN_ACTIVE);
		bool autoUpdatesWhenNotActive = effect->IsFlagSet(GAME_EFFECT_AUTO_UPDATES_WHEN_NOT_ACTIVE);
		if((isActive && autoUpdatesWhenActive) || 
			((!isActive) && autoUpdatesWhenNotActive))
		{
			effectList = &m_effectsToUpdate;
		}
		else
		{
			effectList = &m_effectsNotToUpdate;
		}

		if(*effectList)
		{
			(*effectList)->SetPrev(effect);
			effect->SetNext(*effectList);
		}
		(*effectList) = effect;

		effect->SetFlag(GAME_EFFECT_REGISTERED,true);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UnRegisterEffect
// Desc: UnRegisters effect from effect system
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::UnRegisterEffect(IGameEffect* effect)
{
	FX_ASSERT_MESSAGE(m_isInitialised,"Game Effects System trying to unregister an effect without being initialised");
	FX_ASSERT_MESSAGE(effect,"Trying to UnRegister a NULL effect");

	if(effect && effect->IsFlagSet(GAME_EFFECT_REGISTERED))
	{
		// If the effect is the next one to be updated, then point m_nextEffectToUpdate to the next effect after it
		if(effect == m_nextEffectToUpdate)
		{
			m_nextEffectToUpdate = m_nextEffectToUpdate->Next();
		}

		if(effect->Prev())
		{
			effect->Prev()->SetNext(effect->Next());
		}
		else
		{
			if(m_effectsToUpdate == effect)
			{
				m_effectsToUpdate = effect->Next();
			}
			else
			{
				FX_ASSERT_MESSAGE((m_effectsNotToUpdate == effect),"Effect isn't either updating list");
				m_effectsNotToUpdate = effect->Next();
			}
		}

		if(effect->Next())
		{
			effect->Next()->SetPrev(effect->Prev());
		}
	
		effect->SetNext(NULL);
		effect->SetPrev(NULL);

		effect->SetFlag(GAME_EFFECT_REGISTERED,false);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Update
// Desc: Updates effects system and any effects registered in it's update list
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::Update(float frameTime)
{
	FX_ASSERT_MESSAGE(m_isInitialised,"Game Effects System trying to update without being initialised");

	// Get pause state
	bool isPaused = false;
	IGame* pGame = gEnv->pGame;
	if(pGame)
	{
		IGameFramework* pGameFramework = pGame->GetIGameFramework();
		if(pGameFramework)
		{
			isPaused = pGameFramework->IsGamePaused();
		}
	}

	// Update effects
	if(m_effectsToUpdate)
	{
		IGameEffect* effect = m_effectsToUpdate;
		while(effect)
		{
			m_nextEffectToUpdate = effect->Next();
			if((!isPaused) || (effect->IsFlagSet(GAME_EFFECT_UPDATE_WHEN_PAUSED)))
			{
				SOFTCODE_RETRY(effect,effect->Update(frameTime));
			}
			effect = m_nextEffectToUpdate;
		}
	}

	m_nextEffectToUpdate = NULL;

#if DEBUG_GAME_FX_SYSTEM
	DrawDebugDisplay();
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: CreateSoftCodeInstance
// Desc: Creates soft code instance
//--------------------------------------------------------------------------------------------------
#ifdef SOFTCODE_ENABLED
void* CGameEffectsSystem::CreateSoftCodeInstance(const char* pTypeName)
{
	void* pNewInstance = NULL;

	if(pTypeName)
	{
		for(std::vector<ITypeLibrary*>::iterator iter(m_softCodeTypeLibs.begin()); iter != m_softCodeTypeLibs.end(); ++iter)
		{
			ITypeLibrary* pLib = *iter;
			if(pLib)
			{
				if(pNewInstance = pLib->CreateInstanceVoid(pTypeName))
				{
					break;
				}
			}
		}
	}

	return pNewInstance;
}
#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterSoftCodeLib
// Desc: Register soft code lib for creation of instances
//--------------------------------------------------------------------------------------------------
#ifdef SOFTCODE_ENABLED
void CGameEffectsSystem::RegisterSoftCodeLib(ITypeLibrary* pLib)
{
	m_softCodeTypeLibs.push_back(pLib);
};
#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterGameRenderNode
// Desc: Registers game render node
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterGameRenderNode(IGameRenderNodePtr& pGameRenderNode)
{
#ifdef SOFTCODE_ENABLED
	for(TGameRenderNodeVec::iterator iter(m_gameRenderNodes.begin()); iter != m_gameRenderNodes.end(); ++iter)
	{
		IGameRenderNodePtr* ppIterRenderNode = *iter;
		if(ppIterRenderNode == NULL)
		{
			*iter = &pGameRenderNode;
			return;
		}
	}
	m_gameRenderNodes.push_back(&pGameRenderNode);
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UnregisterGameRenderNode
// Desc: Unregisters game render node
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::UnregisterGameRenderNode(IGameRenderNodePtr& pGameRenderNode)
{
#ifdef SOFTCODE_ENABLED
	TGameRenderNodeVec::iterator iter(std::find(m_gameRenderNodes.begin(), m_gameRenderNodes.end(), &pGameRenderNode));
	if(iter != m_gameRenderNodes.end())
	{
		*iter = NULL;
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterGameRenderElement
// Desc: Registers game render element
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::RegisterGameRenderElement(IGameRenderElementPtr& pGameRenderElement)
{
#ifdef SOFTCODE_ENABLED
	for(TGameRenderElementVec::iterator iter(m_gameRenderElements.begin()); iter != m_gameRenderElements.end(); ++iter)
	{
		IGameRenderElementPtr* ppIterRenderElement = *iter;
		if(ppIterRenderElement == NULL)
		{
			*iter = &pGameRenderElement;
			return;
		}
	}
	m_gameRenderElements.push_back(&pGameRenderElement);
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UnregisterGameRenderElement
// Desc: Unregisters game render element
//--------------------------------------------------------------------------------------------------
void CGameEffectsSystem::UnregisterGameRenderElement(IGameRenderElementPtr& pGameRenderElement)
{
#ifdef SOFTCODE_ENABLED
	TGameRenderElementVec::iterator iter(std::find(m_gameRenderElements.begin(), m_gameRenderElements.end(), &pGameRenderElement));
	if(iter != m_gameRenderElements.end())
	{
		*iter = NULL;
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: DrawDebugDisplay
// Desc: Draws debug display
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
void CGameEffectsSystem::DrawDebugDisplay()
{
	static ColorF textCol(1.0f,1.0f,1.0f,1.0f);
	static ColorF controlCol(0.6f,0.6f,0.6f,1.0f);

	static Vec2 textPos(10.0f,10.0f);
	static float textSize = 1.4f;
	static float textYSpacing = 18.0f;

	static float effectNameXOffset = 100.0f;
	static ColorF effectNameCol(0.0f,1.0f,0.0f,1.0f);

	Vec2 currentTextPos = textPos;

	int debugEffectCount = s_effectDebugList.Size();
	if((g_pGameCVars->g_gameFXSystemDebug) && (debugEffectCount > 0))
	{
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Debug view:");
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x+effectNameXOffset,currentTextPos.y,textSize,&effectNameCol.r,false,GAME_FX_DEBUG_VIEW_NAMES[m_debugView]);
		currentTextPos.y += textYSpacing;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&controlCol.r,false,"(Change debug view: Left/Right arrows)");
		currentTextPos.y += textYSpacing;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Debug effect:");
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x+effectNameXOffset,currentTextPos.y,textSize,&effectNameCol.r,false,s_effectDebugList[s_currentDebugEffectId].effectName);
		currentTextPos.y += textYSpacing;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&controlCol.r,false,"(Change effect: NumPad +/-)");
		currentTextPos.y += textYSpacing;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&controlCol.r,false,"(Reload effect data: NumPad .)");
		currentTextPos.y += textYSpacing;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&controlCol.r,false,"(Reset Particle System: Delete)");
		currentTextPos.y += textYSpacing;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&controlCol.r,false,"(Pause Particle System: End)");
		currentTextPos.y += textYSpacing;

		if(s_effectDebugList[s_currentDebugEffectId].displayCallback)
		{
			s_effectDebugList[s_currentDebugEffectId].displayCallback(currentTextPos,textSize,textYSpacing);
		}

		if(m_debugView==eGAME_FX_DEBUG_VIEW_EffectList)
		{
			static Vec2 listPos(350.0f,50.0f);
			static float nameSize = 150.0f;
			static float tabSize = 60.0f;
			currentTextPos = listPos;

			const int EFFECT_LIST_COUNT = 2;
			IGameEffect* pEffectListArray[EFFECT_LIST_COUNT] = {m_effectsToUpdate,m_effectsNotToUpdate};

			gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&effectNameCol.r,false,"Name");
			currentTextPos.x += nameSize;

			const int FLAG_COUNT = 9;

			const char* flagName[FLAG_COUNT] = {"Init","Rel","ARels","ADels","AUWA","AUWnA","Reg","Actv","DBG"};
			const int flag[FLAG_COUNT] = {GAME_EFFECT_INITIALISED,
																		GAME_EFFECT_RELEASED,
																		GAME_EFFECT_AUTO_RELEASE,
																		GAME_EFFECT_AUTO_DELETE,
																		GAME_EFFECT_AUTO_UPDATES_WHEN_ACTIVE,
																		GAME_EFFECT_AUTO_UPDATES_WHEN_NOT_ACTIVE,
																		GAME_EFFECT_REGISTERED,
																		GAME_EFFECT_ACTIVE,
																		GAME_EFFECT_DEBUG_EFFECT};

			for(int i=0; i<FLAG_COUNT; i++)
			{
				gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&effectNameCol.r,false,flagName[i]);
				currentTextPos.x += tabSize;
			}
		
			currentTextPos.y += textYSpacing;

			for(int l=0; l<EFFECT_LIST_COUNT; l++)
			{
				IGameEffect* pCurrentEffect = pEffectListArray[l];
				while(pCurrentEffect)
				{
					currentTextPos.x = listPos.x;

					gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,pCurrentEffect->GetName());
					currentTextPos.x += nameSize;

					for(int i=0; i<FLAG_COUNT; i++)
					{
						gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,pCurrentEffect->IsFlagSet(flag[i])?"1":"0");
						currentTextPos.x += tabSize;
					}

					currentTextPos.y += textYSpacing;
					pCurrentEffect = pCurrentEffect->Next();
				}
			}
		}
	}
}
#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: OnInputEvent
// Desc: Handles any debug input for the game effects system to test effects
//--------------------------------------------------------------------------------------------------
bool CGameEffectsSystem::OnInputEvent(const SInputEvent& inputEvent)
{
#if DEBUG_GAME_FX_SYSTEM

	int debugEffectCount = s_effectDebugList.Size();

	if((g_pGameCVars->g_gameFXSystemDebug) && (debugEffectCount > 0))
	{
		if(inputEvent.deviceType == eIDT_Keyboard && inputEvent.state == eIS_Pressed)
		{
			switch(inputEvent.keyId)
			{
				case GAME_FX_INPUT_IncrementDebugEffectId:
				{
					if(s_currentDebugEffectId < (debugEffectCount-1))
					{
						s_currentDebugEffectId++;
					}
					break;
				}
				case GAME_FX_INPUT_DecrementDebugEffectId:
				{
					if(s_currentDebugEffectId > 0)
					{
						s_currentDebugEffectId--;
					}
					break;
				}
				case GAME_FX_INPUT_DecrementDebugView:
				{
					if(m_debugView > 0)
					{
						OnDeActivateDebugView(m_debugView);
						m_debugView--;
						OnActivateDebugView(m_debugView);
					}
					break;
				}
				case GAME_FX_INPUT_IncrementDebugView:
				{
					if(m_debugView < (eMAX_GAME_FX_DEBUG_VIEWS-1))
					{
						OnDeActivateDebugView(m_debugView);
						m_debugView++;
						OnActivateDebugView(m_debugView);
					}
					break;
				}
				case GAME_FX_INPUT_ReloadEffectData:
				{
					ReloadData();
					break;
				}
				case GAME_FX_INPUT_ResetParticleManager:
				{
					gEnv->pParticleManager->Reset(false);
					break;
				}
				case GAME_FX_INPUT_PauseParticleManager:
				{
#if DEBUG_GAME_FX_SYSTEM
					ICVar* pParticleDebugCVar = gEnv->pConsole->GetCVar("e_ParticlesDebug");
					if(pParticleDebugCVar)
					{
						int flagValue = pParticleDebugCVar->GetIVal();
						if(flagValue & AlphaBit('z'))
						{
							flagValue &= ~AlphaBit('z');
						}
						else
						{
							flagValue |= AlphaBit('z');
						}
						pParticleDebugCVar->Set(flagValue);
					}
#endif
					break;
				}
			}

			// Send input to current debug effect
			if(s_effectDebugList[s_currentDebugEffectId].inputCallback)
			{
				s_effectDebugList[s_currentDebugEffectId].inputCallback(inputEvent.keyId);
			}
		}
	}
#endif

	return false; // Return false so that other listeners will get this event
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GetDebugEffect
// Desc: Gets debug instance of effect
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
IGameEffect* CGameEffectsSystem::GetDebugEffect(const char* pEffectName) const
{
	const int EFFECT_LIST_COUNT = 2;
	IGameEffect* pEffectListArray[EFFECT_LIST_COUNT] = {m_effectsToUpdate,m_effectsNotToUpdate};

	for(int l=0; l<EFFECT_LIST_COUNT;l++)
	{
		IGameEffect* pCurrentEffect = pEffectListArray[l];
		while(pCurrentEffect)
		{
			if(pCurrentEffect->IsFlagSet(GAME_EFFECT_DEBUG_EFFECT) &&
				(strcmp(pCurrentEffect->GetName(),pEffectName) == 0))
			{
				return pCurrentEffect;
			}
			pCurrentEffect = pCurrentEffect->Next();
		}
	}

	return NULL;
}
#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: OnActivateDebugView
// Desc: Called on debug view activation
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
void CGameEffectsSystem::OnActivateDebugView(int debugView)
{
	switch(debugView)
	{
	case eGAME_FX_DEBUG_VIEW_Profiling:
		{
			ICVar* r_displayInfoCVar = gEnv->pConsole->GetCVar("r_DisplayInfo");
			if(r_displayInfoCVar)
			{
				r_displayInfoCVar->Set(1);
			}
			break;
		}
	}
}
#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: OnDeActivateDebugView
// Desc: Called on debug view de-activation
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
void CGameEffectsSystem::OnDeActivateDebugView(int debugView)
{
	switch(debugView)
	{
		case eGAME_FX_DEBUG_VIEW_Profiling:
		{
			ICVar* r_displayInfoCVar = gEnv->pConsole->GetCVar("r_DisplayInfo");
			if(r_displayInfoCVar)
			{
				r_displayInfoCVar->Set(0);
			}
			break;
		}
	}
}
#endif
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RegisterEffectDebugData
// Desc: Registers effect's debug data with the game effects system, which will then call the 
//			 relevant debug callback functions for the for the effect when its selected using the 
//			 s_currentDebugEffectId
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
void CGameEffectsSystem::RegisterEffectDebugData(	DebugOnInputEventCallback inputEventCallback,
																									DebugDisplayCallback displayCallback,
																									const char* effectName)
{
	s_effectDebugList.push_back(SEffectDebugData(inputEventCallback,displayCallback,effectName));
}
#endif
//--------------------------------------------------------------------------------------------------
