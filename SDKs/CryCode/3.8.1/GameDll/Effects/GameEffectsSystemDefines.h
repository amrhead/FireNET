#ifndef _GAME_EFFECTS_SYSTEM_DEFINES_
#define _GAME_EFFECTS_SYSTEM_DEFINES_

#pragma once

// Includes
#include "TypeLibrary.h"

// Defines
#define	GAME_FX_SYSTEM					CGameEffectsSystem::Instance()

#ifndef _RELEASE
	#define DEBUG_GAME_FX_SYSTEM	1
#else
	#define DEBUG_GAME_FX_SYSTEM	0
#endif

#if DEBUG_GAME_FX_SYSTEM
	// Register effect's DebugOnInput and DebugDisplay callback functions
	#define REGISTER_EFFECT_DEBUG_DATA(inputEventCallback,debugDisplayCallback,effectName)	\
		static CGameEffectsSystem::SRegisterEffectDebugData effectName(inputEventCallback,debugDisplayCallback,#effectName)

	// Debug views
	enum EGameEffectsSystemDebugView
	{
		eGAME_FX_DEBUG_VIEW_None = 0,
		eGAME_FX_DEBUG_VIEW_Profiling,
		eGAME_FX_DEBUG_VIEW_EffectList,
		eGAME_FX_DEBUG_VIEW_BoundingBox,
		eGAME_FX_DEBUG_VIEW_BoundingSphere,
		eGAME_FX_DEBUG_VIEW_Particles,
		eMAX_GAME_FX_DEBUG_VIEWS
		// ** If you add/remove a view then remember to update GAME_FX_DEBUG_VIEW_NAMES **
	};

#else
	#define REGISTER_EFFECT_DEBUG_DATA(inputEventCallback,debugDisplayCallback,effectName)
#endif

// FX Asserts
#if DEBUG_GAME_FX_SYSTEM
	#define FX_ASSERT_MESSAGE(condition,message)		CRY_ASSERT_MESSAGE(condition,message); \
																									if(!(condition)) \
																									{\
																										CryLogAlways("\n*************************************************************************************"); \
																										CryLogAlways("FX ASSERT"); \
																										CryLogAlways("Condition: %s",#condition); \
																										CryLogAlways("Message: %s",message); \
																										CryLogAlways("File: %s",__FILE__); \
																										CryLogAlways("Line: %d",__LINE__); \
																										CryLogAlways("*************************************************************************************\n"); \
																									}
#else	
	#define FX_ASSERT_MESSAGE(condition,message)
#endif

// Profile tags
#define ENABLE_GAME_FX_PROFILE_TAGS 0

#if ENABLE_GAME_FX_PROFILE_TAGS
	#define GAME_FX_PROFILE_BEGIN(_TAG_NAME_) \
	{ \
		CryProfile::PushProfilingMarker( #_TAG_NAME_); \
		gEnv->pRenderer->PushProfileMarker(#_TAG_NAME_); \
	}
	#define GAME_FX_PROFILE_END(_TAG_NAME_) \
	{ \
		CryProfile::PopProfilingMarker();  \
		gEnv->pRenderer->PopProfileMarker(#_TAG_NAME_); \
	}
	#define GAME_FX_PROFILE_MARKER(...) { PIXSetMarker(0, __VA_ARGS__ ); }
#else
	#define GAME_FX_PROFILE_BEGIN(_TAG_NAME_) {}
	#define GAME_FX_PROFILE_END(_TAG_NAME_) {}
	#define GAME_FX_PROFILE_MARKER(...) {}
#endif // ENABLE_GAME_FX_PROFILE_TAGS

#define GAME_FX_LISTENER_NAME		"GameEffectsSystem"
#define GAME_FX_LIBRARY_NAME		"GameEffectsLibrary"
#define GAME_RENDER_NODE_LISTENER_NAME		"GameRenderNodeListener"
#define GAME_RENDER_NODE_LIBRARY_NAME			"GameRenderNodeLibrary"
#define GAME_RENDER_ELEMENT_LISTENER_NAME		"GameRenderElementListener"
#define GAME_RENDER_ELEMENT_LIBRARY_NAME		"GameRenderElementLibrary"

// Macro to remove specific code when soft code is enabled
#ifdef SOFTCODE_ENABLED
	#define		REMOVE_IN_SOFT_CODE(_softCodeOnlyCode_)
#else
	#define		REMOVE_IN_SOFT_CODE(_softCodeOnlyCode_) _softCodeOnlyCode_
#endif

// Register effect's data load and release callbacks
#define REGISTER_DATA_CALLBACKS(dataLoadCallback,dataReleaseCallback,dataReloadCallback,effectName)	\
	static CGameEffectsSystem::SRegisterDataCallbacks effectName(dataLoadCallback,dataReleaseCallback,dataReloadCallback)

// Register effect's Game callbacks
#define REGISTER_GAME_CALLBACKS(enteredGameCallback,effectName)	\
	static CGameEffectsSystem::SRegisterGameCallbacks effectName(enteredGameCallback)

// Create Game FX Soft Code instance
#ifdef SOFTCODE_ENABLED
	#define CREATE_GAME_FX_SOFT_CODE_INSTANCE(T)	(static_cast<T*>(GAME_FX_SYSTEM.CreateSoftCodeInstance(#T)))
#else
	#define CREATE_GAME_FX_SOFT_CODE_INSTANCE(T)	(new T)
#endif	

// Safely release and delete effect through macro
#define SAFE_DELETE_GAME_EFFECT(pGameEffect)	\
	if(pGameEffect)															\
	{																						\
		pGameEffect->Release();										\
		SAFE_DELETE(pGameEffect);									\
	}

// Safely delete game render nodes
#define SAFE_DELETE_GAME_RENDER_NODE(pGameRenderNode)					\
	if(pGameRenderNode)																					\
	{																														\
		pGameRenderNode->ReleaseGameRenderNode();									\
		gEnv->p3DEngine->FreeRenderNodeState(pGameRenderNode);		\
		pGameRenderNode = NULL;																		\
	}																														\

// Safely delete game render elements
#define SAFE_DELETE_GAME_RENDER_ELEMENT(pGameRenderElement)				\
	if(pGameRenderElement)																					\
	{																																\
		pGameRenderElement->ReleaseGameRenderElement();								\
		pGameRenderElement = NULL;																		\
	}																																\

// FX input
#define GAME_FX_INPUT_ReleaseDebugEffect				eKI_End
#define GAME_FX_INPUT_ResetParticleManager			eKI_Delete
#define GAME_FX_INPUT_PauseParticleManager			eKI_End
#define GAME_FX_INPUT_ReloadEffectData					eKI_NP_Period
#define GAME_FX_INPUT_IncrementDebugEffectId		eKI_NP_Add
#define GAME_FX_INPUT_DecrementDebugEffectId		eKI_NP_Substract
#define GAME_FX_INPUT_IncrementDebugView				eKI_Right
#define GAME_FX_INPUT_DecrementDebugView				eKI_Left

enum EParticleEmitterGameFlags
{
	ePEGF_NotPartOfLevel = ePEF_Custom << 0,
};

// Forward declares
struct IGameEffect;
struct IGameRenderNode;
struct IGameRenderElement;
class CGameRenderNodeSoftCodeListener;
class CGameRenderElementSoftCodeListener;

// Typedefs
typedef void (*DataLoadCallback)(IItemParamsNode *paramNode);
typedef void (*DataReloadCallback)(IItemParamsNode *paramNode);
typedef void (*DataReleaseCallback)();
typedef void (*EnteredGameCallback)();
typedef void (*DebugOnInputEventCallback)(int);
typedef void (*DebugDisplayCallback)(const Vec2& textStartPos,float textSize,float textYStep);
typedef _smart_ptr<IGameRenderNode> IGameRenderNodePtr;
typedef _smart_ptr<IGameRenderElement> IGameRenderElementPtr;

#endif // _GAME_EFFECTS_SYSTEM_DEFINES_
