/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: 
  
 -------------------------------------------------------------------------
  History:
  - 6:9:2004   12:44 : Created by M�rcio Martins

*************************************************************************/
#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <IViewSystem.h>
#include <IActionMapManager.h>
#include <IGameObject.h>
#include "BitFiddling.h"

class CGameObjectSystem;
class CGameObject;
struct SEntitySchedulingProfiles;

struct SBasicSpawnParams : public ISerializable
{
	string name;
	uint16 classId;
	Vec3 pos;
	Quat rotation;
	Vec3 scale;
	bool bClientActor;
	uint16 nChannelId;
	uint32 flags;

	virtual void SerializeWith( TSerialize ser )
	{
		if(ser.GetSerializationTarget() == eST_Network)
		{
			ser.Value( "name", name, 'sstr' );
			ser.Value( "classId", classId, 'clas');
			ser.Value( "pos", pos, 'spos' );
			bool bHasScale = false;
			Vec3 vScale(1.0f, 1.0f, 1.0f);

			if(ser.IsWriting())
			{
				bHasScale = (scale.x != 1.0f) || (scale.y != 1.0f) || (scale.z != 1.0f);
				vScale = scale;
			}

			//As this is used in an RMI, we can branch on bHasScale and save ourselves 96bits in the
			//	vast majority of cases, at the cost of a single bit.
			ser.Value("hasScale",  bHasScale, 'bool');

			if(bHasScale)
			{
				//We can't just use a scalar here. Some (very few) objects have non-uniform scaling.
				ser.Value("scale", vScale, 'sscl');
			}

			scale = vScale;

			ser.Value( "rotation", rotation, 'srot' );
			ser.Value( "bClientActor", bClientActor, 'bool' );
			ser.Value( "nChannelId", nChannelId, 'schl' );
			ser.Value( "flags", flags, 'ui32' );
		}
		else
		{
			ser.Value( "name", name );
			ser.Value( "classId", classId );
			ser.Value( "pos", pos );
			ser.Value("scale", scale);
			ser.Value( "rotation", rotation );
			ser.Value( "bClientActor", bClientActor );
			ser.Value( "nChannelId", nChannelId );
			ser.Value( "flags", flags, 'ui32' );
		}
	}
};
























struct SDistanceChecker
{
	ILINE void Init(CGameObjectSystem* pGameObjectSystem, EntityId receiverId) {};
	ILINE void Reset() {};
	ILINE void Update(CGameObject& owner, float frameTime) {};

	ILINE EntityId GetDistanceChecker() const { return 0; }
};


struct IGOUpdateDbg;

class CGameObject :
	public IEntityProxy,
	public IGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	static void CreateCVars();

	// IEntityProxy
	virtual EEntityProxy GetType() { return ENTITY_PROXY_USER; };
	virtual bool Init(IEntity *pEntity, SEntitySpawnParams &spawnParams);
	virtual void Reload( IEntity *pEntity,SEntitySpawnParams &params );
	virtual void Done();
	virtual void Release();
	virtual	void Update(SEntityUpdateContext &ctx);
	virtual	void ProcessEvent(SEntityEvent &event);
	virtual void SerializeXML(XmlNodeRef &entityNode, bool loading);
	virtual void Serialize( TSerialize ser );
	virtual bool NeedSerialize();
	virtual bool GetSignature( TSerialize signature );
	// ~IEntityProxy

	// IActionListener
	virtual void OnAction( const ActionId& actionId, int activationMode, float value );
	virtual void AfterAction();
	// ~IActionListener

	// IGameObject
	virtual bool BindToNetwork(EBindToNetworkMode);
	virtual bool BindToNetworkWithParent(EBindToNetworkMode mode, EntityId parentId);
	virtual void ChangedNetworkState( NetworkAspectType aspects );
	virtual void EnableAspect(NetworkAspectType aspects, bool enable);
	virtual	void EnableDelegatableAspect(NetworkAspectType aspects, bool enable);
	virtual IGameObjectExtension *QueryExtension(const char *extension) const;
	virtual bool SetExtensionParams( const char * extension, SmartScriptTable params );
	virtual bool GetExtensionParams( const char * extension, SmartScriptTable params );
	virtual IGameObjectExtension * ChangeExtension( const char * extension, EChangeExtension change );
	virtual void SendEvent( const SGameObjectEvent& );
	virtual void SetChannelId(uint16 id);
	virtual uint16 GetChannelId() const { return m_channelId; }
	virtual INetChannel *GetNetChannel() const;
	virtual bool CaptureView( IGameObjectView * pGOV );
	virtual void ReleaseView( IGameObjectView * pGOV );
	virtual bool CaptureActions( IActionListener * pAL );
	virtual void ReleaseActions( IActionListener * pAL );
	virtual bool CaptureProfileManager( IGameObjectProfileManager * pPH );
	virtual void ReleaseProfileManager( IGameObjectProfileManager * pPH );
	virtual void EnableUpdateSlot( IGameObjectExtension * pExtension, int slot );
	virtual void DisableUpdateSlot( IGameObjectExtension * pExtension, int slot );
	virtual uint8 GetUpdateSlotEnables( IGameObjectExtension * pExtension, int slot );
	virtual void EnablePostUpdates( IGameObjectExtension * pExtension );
	virtual void DisablePostUpdates( IGameObjectExtension * pExtension );
	virtual void PostUpdate(float frameTime);
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int flags );
	virtual NetworkAspectType GetNetSerializeAspects();
	virtual void PostSerialize();
	virtual void SetUpdateSlotEnableCondition( IGameObjectExtension * pExtension, int slot, EUpdateEnableCondition condition );
	virtual bool IsProbablyVisible();
	virtual bool IsProbablyDistant();
	virtual bool SetAspectProfile( EEntityAspects aspect, uint8 profile, bool fromNetwork );
	virtual uint8 GetAspectProfile( EEntityAspects aspect );
	virtual IWorldQuery * GetWorldQuery();
	virtual IMovementController * GetMovementController();
	virtual IGameObjectExtension * GetExtensionWithRMIBase( const void * pBase );
	virtual void AttachDistanceChecker();
	virtual void ForceUpdate(bool force);
	virtual void ForceUpdateExtension( IGameObjectExtension * pExt, int slot );
	virtual void Pulse( uint32 pulse );
	virtual void RegisterAsPredicted();
	virtual void RegisterAsValidated(IGameObject* pGO, int predictionHandle);
	virtual int GetPredictionHandle();
	virtual void RequestRemoteUpdate( NetworkAspectType aspectMask );
	virtual void RegisterExtForEvents( IGameObjectExtension* piExtention, const int* pEvents, const int numEvents );
	virtual void UnRegisterExtForEvents( IGameObjectExtension* piExtention, const int* pEvents, const int numEvents );

	virtual void EnablePhysicsEvent(bool enable, int event)
	{
		if (enable) 
			m_enabledPhysicsEvents = m_enabledPhysicsEvents|event;
		else
			m_enabledPhysicsEvents = m_enabledPhysicsEvents&(~event);
	}
	virtual bool WantsPhysicsEvent(int event) { return (m_enabledPhysicsEvents&event) != 0; };
	virtual void SetNetworkParent( EntityId id );

	virtual bool IsJustExchanging() { return m_justExchanging; };
	virtual bool SetAIActivation( EGameObjectAIActivationMode mode );
	virtual void SetAutoDisablePhysicsMode( EAutoDisablePhysicsMode mode );
	virtual void EnablePrePhysicsUpdate( EPrePhysicsUpdate updateRule );
	// needed for debug
	virtual bool ShouldUpdate();
	// ~IGameObject

	virtual void UpdateView(SViewParams &viewParams);
	virtual void PostUpdateView(SViewParams &viewParams);
	virtual void HandleEvent( const SGameObjectEvent& evt );
	virtual bool CanUpdateView() const { return m_pViewDelegate!=NULL; }
	IGameObjectView* GetViewDelegate() { return m_pViewDelegate; }

	bool IsAspectDelegatable(NetworkAspectType aspect);

	//----------------------------------------------------------------------
	// Network related functions

	// we have gained (or lost) control of this object
	virtual void SetAuthority( bool auth );
	virtual void InitClient(int channelId);
	virtual void PostInitClient(int channelId);

	ISerializableInfoPtr GetSpawnInfo();

	NetworkAspectType GetEnabledAspects() const { return m_enabledAspects; }
	uint8 GetDefaultProfile( EEntityAspects aspect );
	
	// called from CGameObject::BoundObject -- we have become bound on a client
	void BecomeBound() { m_isBoundToNetwork = true; }
	bool IsBoundToNetwork() { return m_isBoundToNetwork; }

	void FlushActivatableExtensions() { FlushExtensions(false); }

	void PostRemoteSpawn();

	void GetMemoryUsage(ICrySizer * s) const;
	
	static void UpdateSchedulingProfiles();

	virtual void DontSyncPhysics() { m_bNoSyncPhysics = true; }

	void AquireMutex();
	void ReleaseMutex();

private:
	IActionListener * m_pActionDelegate;
	IGameObjectView * m_pViewDelegate;
	IGameObjectProfileManager * m_pProfileManager;

	uint8							m_profiles[NUM_ASPECTS];

	// Need a mutex to defend shutdown against event handling.
#ifndef PS3
	CryMutex m_mutex;
#endif

	template <class T> bool DoGetSetExtensionParams( const char * extension, SmartScriptTable params );

	// any extensions (extra GameObject functionality) goes here
	struct SExtension
	{
		SExtension()
			: pExtension()
			, id(0)
			, refCount(0)
			, activated(false)
			, sticky(false)
			, postUpdate(false)
			, flagUpdateWhenVisible(0)
			, flagUpdateWhenInRange(0)
			, flagUpdateCombineOr(0)
			, flagDisableWithAI(0)
			, flagNeverUpdate(0)
			, eventReg(0)
		{
			uint32 slotbit = 1;
			for (uint32 i=0; i<MAX_UPDATE_SLOTS_PER_EXTENSION; ++i)
			{
				updateEnables[i] = forceEnables[i] = 0;
				flagDisableWithAI += slotbit;
				slotbit <<= 1;
			}
		}

		// extension by flag event registration
		uint64 eventReg;
		IGameObjectExtensionPtr pExtension;
		IGameObjectSystem::ExtensionID id;
		// refCount is the number of AcquireExtensions pending ReleaseExtensions
		uint8 refCount;
		uint8 updateEnables[MAX_UPDATE_SLOTS_PER_EXTENSION];
		uint8 forceEnables[MAX_UPDATE_SLOTS_PER_EXTENSION];
		// upper layers only get to activate/deactivate extensions
		uint8 flagUpdateWhenVisible : MAX_UPDATE_SLOTS_PER_EXTENSION;
		uint8 flagUpdateWhenInRange : MAX_UPDATE_SLOTS_PER_EXTENSION;
		uint8 flagUpdateCombineOr : MAX_UPDATE_SLOTS_PER_EXTENSION;
		uint8 flagDisableWithAI : MAX_UPDATE_SLOTS_PER_EXTENSION;
		uint8 flagNeverUpdate : MAX_UPDATE_SLOTS_PER_EXTENSION;
		bool activated : 1;
		bool sticky : 1;
		bool postUpdate : 1;

		bool operator<( const SExtension& rhs ) const
		{
			return id < rhs.id;
		}

		void GetMemoryUsage(ICrySizer *pSizer) const
		{		
			pSizer->AddObject(pExtension);
		}
	};

	static const int MAX_ADDING_EXTENSIONS = 8;
	static SExtension m_addingExtensions[MAX_ADDING_EXTENSIONS];
	static int m_nAddingExtension;

	typedef std::vector<SExtension> TExtensions;
	TExtensions m_extensions;
	uint16 m_channelId;
	NetworkAspectType m_enabledAspects;
	NetworkAspectType m_delegatableAspects;
	bool m_inRange : 1;
	bool m_isBoundToNetwork : 1;
	bool m_justExchanging : 1;
	bool m_bVisible : 1;
	bool m_bPrePhysicsEnabled : 1;
	bool m_bPhysicsDisabled : 1;
	bool m_bNoSyncPhysics : 1;
	bool m_bNeedsNetworkRebind : 1;
	enum EUpdateState
	{
		eUS_Visible_Close = 0,
		eUS_Visible_FarAway,
		eUS_NotVisible_Close,
		eUS_NotVisible_FarAway,
		eUS_CheckVisibility_Close,
		eUS_CheckVisibility_FarAway,
		eUS_COUNT_STATES,
		eUS_INVALID = eUS_COUNT_STATES
	};
	uint m_updateState : CompileTimeIntegerLog2_RoundUp<eUS_COUNT_STATES>::result;
	uint m_aiMode : CompileTimeIntegerLog2_RoundUp<eGOAIAM_COUNT_STATES>::result;
	uint m_physDisableMode : CompileTimeIntegerLog2_RoundUp<eADPM_COUNT_STATES>::result;

	IGameObjectExtensionPtr m_pGameObjectExtensionCachedKey;
	SExtension * m_pGameObjectExtensionCachedValue;
	void ClearCache() { m_pGameObjectExtensionCachedKey = IGameObjectExtensionPtr(); m_pGameObjectExtensionCachedValue = 0; }
	SExtension * GetExtensionInfo( IGameObjectExtension * pExt )
	{
		CRY_ASSERT( pExt );
		if (m_pGameObjectExtensionCachedKey.get() == pExt)
		{
			CRY_ASSERT(m_pGameObjectExtensionCachedValue->pExtension.get() == pExt);
			return m_pGameObjectExtensionCachedValue;
		}
		for (TExtensions::iterator iter = m_extensions.begin(); iter != m_extensions.end(); ++iter)
		{
			if (iter->pExtension.get() == pExt)
			{
				m_pGameObjectExtensionCachedKey = iter->pExtension;
				m_pGameObjectExtensionCachedValue = &*iter;
				return &*iter;
			}
		}
		return 0;
	}

	enum EUpdateStateEvent
	{
		eUSE_BecomeVisible = 0,
		eUSE_BecomeClose,
		eUSE_BecomeFarAway,
		eUSE_Timeout,
		eUSE_COUNT_EVENTS,
	};
	float m_updateTimer;

	SDistanceChecker m_distanceChecker;

	int	m_enabledPhysicsEvents;
	int m_forceUpdate;
	int m_predictionHandle;

	EPrePhysicsUpdate m_prePhysicsUpdateRule;

	const SEntitySchedulingProfiles * m_pSchedulingProfiles;
	uint32 m_currentSchedulingProfile;
	EntityId m_cachedParentId;

	void FlushExtensions(bool includeStickyBits);
	void DoInvokeRMI(_smart_ptr<CRMIBody> pBody, unsigned, int);
	bool ShouldUpdateSlot( const SExtension * pExt, uint32 slot, uint32 slotbit, bool checkAIDisable );
	void EvaluateUpdateActivation();
	void DebugUpdateState();
	bool ShouldUpdateAI();
	void UpdateLOD();
	void RemoveExtension( const TExtensions::iterator& iter );
	void UpdateStateEvent( EUpdateStateEvent evt );
	bool TestIsProbablyVisible( uint state );
	bool TestIsProbablyDistant( uint state );
	bool DoSetAspectProfile( EEntityAspects aspect, uint8 profile, bool fromNetwork );
	void SetActivation( bool activate );
	void SetPhysicsDisable( bool disablePhysics );

	void UpdateSchedulingProfile();

	static const float UpdateTimeouts[eUS_COUNT_STATES];
	static const EUpdateState UpdateTransitions[eUS_COUNT_STATES][eUSE_COUNT_EVENTS];
	static const char * UpdateNames[eUS_COUNT_STATES];
	static const char * EventNames[eUSE_COUNT_EVENTS];

	static CGameObjectSystem * m_pGOS;
};

DECLARE_COMPONENT_POINTERS( CGameObject );

#endif //__GAMEOBJECT_H__
