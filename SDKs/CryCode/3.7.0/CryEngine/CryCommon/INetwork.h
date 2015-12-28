/*************************************************************************
 Crytek Source File.
 Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
 $Id$
 $DateTime$
 Description:  Message definition to id management
 -------------------------------------------------------------------------
 History:
 - 07/25/2001   : Alberto Demichelis, Created
 - 07/20/2002   : Martin Mittring, Cleaned up
 - 09/08/2004   : Craig Tiller, Refactored
 - 17/09/2004   : Craig Tiller, Introduced contexts
*************************************************************************/
#include DEVIRTUALIZE_HEADER_FIX(INetwork.h)

#ifndef __INETWORK_H__
#define __INETWORK_H__

#pragma once

#ifdef CRYNETWORK_EXPORTS
	#define CRYNETWORK_API DLL_EXPORT
#else
	#define CRYNETWORK_API DLL_IMPORT
#endif

#if _MSC_VER > 1000
#pragma warning(disable:4786)
#endif

#include <ISerialize.h> // <> required for Interfuscator
#include "TimeValue.h"
#include <ITimer.h> // <> required for Interfuscator
#include <ICryMatchMaking.h> // <> required for Interfuscator
#include <INetworkService.h>

#define SERVER_DEFAULT_PORT 64087
#define SERVER_DEFAULT_PORT_STRING "64087"

#define UNSAFE_NUM_ASPECTS		32							// 8,16 or 32
#define NUM_ASPECTS				(UNSAFE_NUM_ASPECTS)

#define NEW_BANDWIDTH_MANAGEMENT 1
#if NEW_BANDWIDTH_MANAGEMENT
#define ENABLE_PACKET_PREDICTION	1
#define USE_ACCURATE_NET_TIMERS 1
#else
#define ENABLE_PACKET_PREDICTION	0		// NOTE not supported by old message queue!
#define USE_ACCURATE_NET_TIMERS 0
#endif // !NEW_BANDWIDTH_MANAGEMENT

// Enable the 'hack' scheduling policy group to be used for the aspects in HIGH_PRIORITY_ASPECT_MASK
// HIGH_PRIORITY_SCHEDULING_POLICY_GROUP is calculated by turning on the logging, running the game and
// seeing which index the generated hash for the policy group gets sorted to.  The hash for 'hack' is
// 68617368 and currently gets sorted to index 13.
#define USE_HIGH_PRIORITY_ASPECT_HACK 0
#define HIGH_PRIORITY_LOGGING (1 && USE_HIGH_PRIORITY_ASPECT_HACK)
#define HIGH_PRIORITY_SCHEDULING_POLICY_GROUP 13
#define HIGH_PRIORITY_ASPECT_MASK 0x80000000

// Allows RMIs marked as urgent to get sent at the next sync with game
#define ENABLE_URGENT_RMIS 1				// NOTE not supported by old message queue!
#define ENABLE_INDEPENDENT_RMIS 1		// For RMIs independent of the object update

////////////////////////////////////////////////////////////////////////////////////////
// Profiling
////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_RELEASE)
#  define NET_PROFILE_ENABLE 1
#  define NET_MINI_PROFILE 1
#else
// N.B. if you turn this on in release, you need to make dev only cvars available in release
// as well - see ISystem.h, REGISTER_CVAR2_DEV_ONLY and netProfileRegisterCVars()
#  define NET_PROFILE_ENABLE 0
#  define NET_MINI_PROFILE 0
#endif

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD) || PROFILE_PERFORMANCE_NET_COMPATIBLE
#  define ENABLE_RMI_BENCHMARK 1
#else
#  define ENABLE_RMI_BENCHMARK 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
// EntityID reservation for netIDs without bound entities
////////////////////////////////////////////////////////////////////////////////////////
#define RESERVE_UNBOUND_ENTITIES 1

////////////////////////////////////////////////////////////////////////////////////////
// Interfaces
////////////////////////////////////////////////////////////////////////////////////////

struct IRConSystem;
class ICrySizer;

class INetMessage;
struct INetMessageSink;

struct IGameNub;
struct IGameQuery;
struct IGameSecurity;
struct IGameChannel;
struct IGameContext;
struct IServerSnooperSink;
struct INetNub;
struct INetChannel;
struct INetContext;
struct IServerSnooper;
struct ICVar;
struct IRMIMessageBody;
struct ILanQueryListener;
struct IGameQueryListener;
struct IRMIListener;
struct INetAtSyncItem;
struct IRMICppLogger;
struct IContextEstablishTask;
struct IContextEstablisher;
struct INetSendable;
struct SNetProfileStackEntry;
struct ICryLobby;

#if defined(DEDICATED_SERVER) 
struct IDefenceContext;
#endif

class CNetSerialize;

typedef _smart_ptr<IRMIMessageBody> IRMIMessageBodyPtr;
typedef uint32 TPacketSize;

// Description:
//	 This enum describes different reliability mechanisms for
//	 packet delivery.
// Remarks:
//	 Do not change ordering here without updating ServerContextView, ClientContextView.
// Note:
//  This is pretty much deprecated; new code should use the message queue facilities
//  to order messages, and be declared either ReliableUnordered or UnreliableUnordered.
// See also:
//	 ServerContextView,ClientContextView
enum ENetReliabilityType
{
	eNRT_ReliableOrdered,
	eNRT_ReliableUnordered,
	eNRT_UnreliableOrdered,
	eNRT_UnreliableUnordered,
	// Must be last.
	eNRT_NumReliabilityTypes
};

// Description:
//	 Implementation of CContextView relies on the first two values being
//	 as they are.
enum ERMIAttachmentType
{
	eRAT_PreAttach = 0,
	eRAT_PostAttach = 1,
	eRAT_NoAttach,
	// urgent RMIs will be sent at the next sync with game
	// N.B. use with caution as this will increase bandwidth
	eRAT_Urgent,
	// If an RMI doesn't care about the object update, then
	// use independent RMIs as they can be sent in the urgent
	// RMI flush
	eRAT_Independent,

	// Must be last.
	eRAT_NumAttachmentTypes
};

enum EContextViewState
{
	// We use this initially to ensure that eCVS_Begin is a real state 
	// change.
	eCVS_Initial = 0,
	// Starts setting up a context.
	eCVS_Begin,
	// Establishes the context (load any data files).
	eCVS_EstablishContext,
	// Configure any data-dependent things before binding.
	eCVS_ConfigureContext,
	// Spawns (bind) any objects.
	eCVS_SpawnEntities,
	// Post-spawns initialization for any entities.
	eCVS_PostSpawnEntities,
	// Now play the game (finally!).
	eCVS_InGame,
	// Resets some game state without completely destroying the context.
	eCVS_NUM_STATES
};

enum EChannelConnectionState
{
	eCCS_WaitingForResponse,
	eCCS_StartingConnection,
	eCCS_InContextInitiation,
	eCCS_InGame,
	eCCS_Disconnecting,
};

enum EPunkType
{
	ePT_NotResponding,
	ePT_ChangedVars,
	ePT_ModifiedCode,
	ePT_ModifiedFile,
	ePT_NetProtocol
};

enum ECheatProtectionLevel
{
	eCPL_Default = 0,
	eCPL_RandomChecks,
	eCPL_Code,
	eCPL_High
};

enum EAspectFlags
{
	// aspect will not be sent to clients that don't control the entity
	eAF_ServerControllerOnly	= 0x04,
	// aspect is serialized without using compression manager (useful for data that is allready well quantised/compressed)
	eAF_NoCompression					=	0x08,
	// aspect can be client controlled (delegated to the client)
	eAF_Delegatable						= 0x10,
	// aspect has more than one profile (serialization format)
	eAF_ServerManagedProfile	= 0x20,
	// client should periodically send a hash of what it thinks the current state of an aspect is
	// this hash is compared to the server hash and forces a server update if there's a mismatch
	eAF_HashState							= 0x40,
	// aspect needs a timestamp to make sense (i.e. physics)
	eAF_TimestampState				= 0x80,
};

enum EMessageParallelFlags
{
	// don't change the context state until this message has been sent & acknowledged
	// PROBABLY SHOULD BE DEALT WITH BY MESSAGE DEPENDENCIES
	eMPF_BlocksStateChange   = 0x0001,
	// decoding of this message needs to occur on the main thread (legacy support mainly)
	// means that the entire packet will need to be decoded in the main thread, slowing things down
	eMPF_DecodeInSync        = 0x0002,
	// lock the network thread and get this message sent as quickly as possible
	eMPF_NoSendDelay         = 0x0004, 
	// message makes no sense if the entity is not bound, so discard it in that case (a bit hacky!)
	eMPF_DiscardIfNoEntity   = 0x0008,
	// message represents part of a context state change
	// PROBABLY SHOULD BE DEALT WITH BY MESSAGE DEPENDENCIES
	eMPF_StateChange         = 0x0010,
	// not a critical message - don't wake up sending
	eMPF_DontAwake           = 0x0020, 
	// delay sending this message until spawning is complete
	// PROBABLY SHOULD BE DEALT WITH BY MESSAGE DEPENDENCIES
	eMPF_AfterSpawning       = 0x0040
};

// At which point in the frame are we.
enum ENetworkGameSync
{
	eNGS_FrameStart = 0,
	eNGS_FrameEnd,
	eNGS_Shutdown_Clear,
	eNGS_Shutdown,
	eNGS_MinimalUpdateForLoading, // Internal use - workaround for sync loading problems
	eNGS_AllowMinimalUpdate,
	eNGS_DenyMinimalUpdate,
	eNGS_SleepNetwork,						// Should only be called once per frame!
	eNGS_WakeNetwork,							// Should only be called once per frame!
	eNGS_ForceChannelTick,				// Forces the netchannels to tick when the network ticks (for next tick only)
	eNGS_DisplayDebugInfo,				// update and display network debug info.
	// must be last
	eNGS_NUM_ITEMS
};

enum EMessageSendResult
{
	eMSR_SentOk,			// Message was successfully sent.
	eMSR_NotReady,			// Message wasn't ready to be sent (try again later).
	eMSR_FailedMessage,		// Failed sending the message, but it's ok.
	eMSR_FailedConnection,	// Failed sending so badly that we need to disconnect!
	eMSR_FailedPacket,		// Failed sending the message; don't try to send anything more this packet.
};

// this function ranks EMessageSendResults in order of severity
ILINE EMessageSendResult WorstMessageSendResult( EMessageSendResult r1, EMessageSendResult r2 )
{
	if (r1 < r2)
		return r2;
	else
		return r1;
}

#define _CRYNETWORK_CONCAT(x,y)	x ## y
#define CRYNETWORK_CONCAT(x,y)		_CRYNETWORK_CONCAT(x,y)
#define ASPECT_TYPE		CRYNETWORK_CONCAT(uint,UNSAFE_NUM_ASPECTS)

#if NUM_ASPECTS > 32
#error Aspects > 32 Not supported at this time
#endif

typedef uint8 ChannelMaskType;
typedef ASPECT_TYPE NetworkAspectType;
typedef uint8 NetworkAspectID;

#define NET_ASPECT_ALL (NetworkAspectType(0xFFFFFFFF))

typedef uint32 TNetChannelID;
static const char * LOCAL_CONNECTION_STRING = "<local>";
static const char * NULL_CONNECTION_STRING = "<null>";
static const size_t MaxProfilesPerAspect = 8;

// represents a message that has been added to the message queue
// these cannot be shared between channels
struct SSendableHandle
{
	SSendableHandle() : id(0), salt(0) {}

	uint32 id;
	uint32 salt;

	ILINE bool operator!() const
	{
		return id==0 && salt==0;
	}
	typedef uint32 (SSendableHandle::*unknown_bool_type);
	ILINE operator unknown_bool_type() const
	{
		return !!(*this)? &SSendableHandle::id : NULL;
	}
	ILINE bool operator!=( const SSendableHandle& rhs ) const
	{
		return !(*this == rhs);
	}
	ILINE bool operator==( const SSendableHandle& rhs ) const
	{
		return id==rhs.id && salt==rhs.salt;
	}
	ILINE bool operator<( const SSendableHandle& rhs ) const
	{
		return id<rhs.id || (id==rhs.id && salt<rhs.salt);
	}

	const char * GetText( char * tmpBuf = 0 )
	{
		static char singlebuf[64];
		if (!tmpBuf)
			tmpBuf = singlebuf;
		sprintf(tmpBuf, "%.8x:%.8x", id, salt);
		return tmpBuf;
	}
};

typedef std::pair<bool,INetAtSyncItem*> TNetMessageCallbackResult;

// Description:
//	 This structure describes the meta-data we need to be able
//	 to dispatch/handle a message.
struct SNetMessageDef
{
	typedef TNetMessageCallbackResult (*HandlerType)( 
		uint32 nUser,
		INetMessageSink*, 
		TSerialize,
		uint32 curSeq,
		uint32 oldSeq,
		EntityId* pRmiObject,
		INetChannel * pChannel
		);
	HandlerType handler;
	const char * description;
	ENetReliabilityType reliability;
	uint32 nUser;
	uint32 parallelFlags;
	bool CheckParallelFlag( EMessageParallelFlags flag ) const
	{
		return (parallelFlags & flag) != 0;
	}
	struct ComparePointer
	{
		bool operator () (const SNetMessageDef *p1, const SNetMessageDef *p2)
		{
			return strcmp(p1->description, p2->description) < 0;
		}
	};
};

// Description:
//	 If we are playing back a demo, and simulating packets, oldSeq == curSeq == DEMO_PLAYBACK_SEQ_NUMBER.
static const uint32 DEMO_PLAYBACK_SEQ_NUMBER = ~uint32(0);

// Description:
//	 This structure is a nice cross-dll way of passing a table
//	 of SNetMessageDef's.
struct SNetProtocolDef
{
	size_t nMessages;
	SNetMessageDef * vMessages;
};

UNIQUE_IFACE struct INetBreakagePlayback : public CMultiThreadRefCount
{

	virtual void SpawnedEntity( int idx, EntityId id ) = 0;
	virtual EntityId GetEntityIdForIndex( int idx ) = 0;

};
typedef _smart_ptr<INetBreakagePlayback> INetBreakagePlaybackPtr;

// Experimental, alternative break-system
UNIQUE_IFACE struct INetBreakageSimplePlayback : public CMultiThreadRefCount
{

	virtual void BeginBreakage() = 0;                                  // Used to tell the network that breakage replication has started
	virtual void FinishedBreakage() = 0;                               // Used to tell the network that breakage replication has finished
	virtual void BindSpawnedEntity(EntityId id, int spawnIdx) = 0;     // Use to net bind the "collected" entities from the break

};
typedef _smart_ptr<INetBreakageSimplePlayback> INetBreakageSimplePlaybackPtr;

struct ISerializableInfo : public CMultiThreadRefCount, public ISerializable {};
typedef _smart_ptr<ISerializableInfo> ISerializableInfoPtr;

UNIQUE_IFACE struct INetSendableSink
{

	virtual ~INetSendableSink(){}
	virtual void NextRequiresEntityEnabled( EntityId id ) = 0;
	virtual void SendMsg( INetSendable * pSendable ) = 0;

};

struct IBreakDescriptionInfo : public CMultiThreadRefCount
{

	virtual void GetAffectedRegion(AABB& aabb) = 0;
	virtual void AddSendables( INetSendableSink * pSink, int32 brkId ) = 0;
	
	// Experimental, alternative break-system
	virtual void SerialiseSimpleBreakage( TSerialize ser ) {}

};
typedef _smart_ptr<IBreakDescriptionInfo> IBreakDescriptionInfoPtr;

enum ENetBreakDescFlags
{
	eNBF_UseDefaultSend          = 0,
	eNBF_UseSimpleSend           = 1<<0,
	eNBF_SendOnlyOnClientJoin    = 1<<1,
};

struct SNetBreakDescription
{
	SNetBreakDescription() { flags = eNBF_UseDefaultSend; breakageEntity=0; }
	IBreakDescriptionInfoPtr pMessagePayload;
	ENetBreakDescFlags flags;
	EntityId * pEntities;
	int nEntities;
	EntityId breakageEntity;
};

enum ENetworkServiceInterface
{
	eNSI_CDKeyValidation,
};

// sendables get callbacks depending on 'how sent they are'
enum ENetSendableStateUpdate
{
	// message has been acknowledged
	eNSSU_Ack,
	// message was dropped, but since it's reliable (and all other conditions are right), packet has been requeued in the message queue
	eNSSU_Requeue,
	// message was dropped and will not be resent
	eNSSU_Nack,
	// message was not allowed into the message queue for some reason
	eNSSU_Rejected
};

// Description:
//    An ancillary service provided by the network system.
typedef _smart_ptr<INetworkService> INetworkServicePtr;

struct IRemoteControlSystem;
struct ISimpleHttpServer;

struct SNetGameInfo
{
	SNetGameInfo() : maxPlayers(-1) {}
	int maxPlayers;
};

/////////////////////////////////////////////////////////////////////////////
// Host Migration
#define HOST_MIGRATION_MAX_SERVER_NAME_SIZE (32)
#define HOST_MIGRATION_MAX_PLAYER_NAME_SIZE (32)
#define HOST_MIGRATION_LISTENER_NAME_SIZE (64)

typedef uint32		HMStateType;

struct SHostMigrationEventListenerInfo
{
	SHostMigrationEventListenerInfo(IHostMigrationEventListener* pListener, const char* pWho)
		: m_pListener(pListener)
	{
		for (uint32 index = 0; index < MAX_MATCHMAKING_SESSIONS; ++index)
		{
			Reset(index);
		}
#if !defined(_RELEASE)
		m_pWho = pWho;
#endif
	}

	void Reset(uint32 sessionIndex)
	{
		CRY_ASSERT(sessionIndex < MAX_MATCHMAKING_SESSIONS);
		m_done[sessionIndex] = false;
		m_state[sessionIndex] = 0;
	}

	IHostMigrationEventListener* m_pListener;
	bool m_done[MAX_MATCHMAKING_SESSIONS];
	HMStateType m_state[MAX_MATCHMAKING_SESSIONS];
#if !defined(_RELEASE)
	CryFixedStringT<HOST_MIGRATION_LISTENER_NAME_SIZE> m_pWho;
#endif
};

struct SHostMigrationInfo
{
	SHostMigrationInfo(void) : m_pServerChannel(NULL), m_pGameSpecificData(NULL), m_taskID(CryLobbyInvalidTaskID), m_state(eHMS_Idle), m_playerID(0), m_isHost(false), m_logProgress(false), m_shouldMigrateNub(false)
	{
		m_newServer.clear();
		m_migratedPlayerName.clear();
	}

	void Initialise(CrySessionHandle h, bool shouldMigrateNub)
	{
		m_newServer.clear();
		m_migratedPlayerName.clear();
		m_pServerChannel = NULL;
		m_pGameSpecificData = NULL;
		m_session = h;
		m_taskID = CryLobbyInvalidTaskID;
		m_state = eHMS_Idle;
		m_playerID = 0;
		m_isHost = false;
		m_logProgress = false;
		m_shouldMigrateNub = shouldMigrateNub;
	}

	void SetNewServer(const char* newServer)
	{
		m_newServer = newServer;
	}

	void SetMigratedPlayerName(const char* migratedPlayerName)
	{
		m_migratedPlayerName = migratedPlayerName;
	}

	void SetIsNewHost(bool set) { m_isHost = set; }
	bool IsNewHost(void) { return m_isHost; }
	bool ShouldMigrateNub() { return m_shouldMigrateNub; }

	CryFixedStringT<HOST_MIGRATION_MAX_SERVER_NAME_SIZE> m_newServer;
	CryFixedStringT<HOST_MIGRATION_MAX_PLAYER_NAME_SIZE> m_migratedPlayerName;
	INetChannel* m_pServerChannel;
	void* m_pGameSpecificData;
	CrySessionHandle m_session;
	CryLobbyTaskID m_taskID;
	eHostMigrationState m_state;
	EntityId m_playerID;
	bool m_isHost;
	bool m_logProgress;
	bool m_shouldMigrateNub;
};

struct IHostMigrationEventListener
{
	enum EHostMigrationReturn
	{
		Listener_Done,
		Listener_Wait,
		Listener_Terminate
	};

	virtual ~IHostMigrationEventListener(){}
	virtual EHostMigrationReturn OnInitiate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual EHostMigrationReturn OnDisconnectClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual EHostMigrationReturn OnDemoteToClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual EHostMigrationReturn OnPromoteToServer(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual EHostMigrationReturn OnReconnectClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual EHostMigrationReturn OnFinalise(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual EHostMigrationReturn OnTerminate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;
	virtual void OnComplete(SHostMigrationInfo& hostMigrationInfo) = 0;
	virtual EHostMigrationReturn OnReset(SHostMigrationInfo& hostMigrationInfo, HMStateType& state) = 0;

};

// Must be at least the same as CMessageQueue::MAX_ACCOUNTING_GROUPS
#define STATS_MAX_MESSAGEQUEUE_ACCOUNTING_GROUPS (64)
struct SAccountingGroupStats 
{
	SAccountingGroupStats()
	: m_sends(0)
	, m_bandwidthUsed(0.0f)
	, m_totalBandwidthUsed(0.0f)
	, m_priority(0)
	, m_maxLatency(0.0f)
	, m_discardLatency(0.0f)
	, m_inUse(false)
	{
		memset(m_name, 0, sizeof(m_name));
	}

	char m_name[8];
	uint32 m_sends;
	float m_bandwidthUsed;
	float m_totalBandwidthUsed;
	uint32 m_priority;
	float m_maxLatency;
	float m_discardLatency;
	bool m_inUse;
};

struct SMessageQueueStats
{
	SMessageQueueStats()
	: m_usedPacketSize(0)
	, m_sentMessages(0)
	, m_unsentMessages(0)
#if ENABLE_URGENT_RMIS
	, m_urgentRMIs(0)
#endif // ENABLE_URGENT_RMIS
	{}

	SAccountingGroupStats m_accountingGroup[STATS_MAX_MESSAGEQUEUE_ACCOUNTING_GROUPS];
	uint32 m_usedPacketSize;
	uint16 m_sentMessages;
	uint16 m_unsentMessages;
#if ENABLE_URGENT_RMIS
	uint16 m_urgentRMIs;
#endif // ENABLE_URGENT_RMIS
};

// Max channels for peer hosted games = max server channels + local client
#define STATS_MAX_NUMBER_OF_CHANNELS (32 + 1)
#define STATS_MAX_NAME_SIZE (32)
struct SNetChannelStats
{
	SNetChannelStats()
	: m_ping(0)
	, m_pingSmoothed(0)
	, m_bandwidthInbound(0.0f)
	, m_bandwidthOutbound(0.0f)
	, m_bandwidthShares(0)
	, m_desiredPacketRate(0)
	, m_currentPacketRate(0.0f)
	, m_packetLossRate(0.0f)
	, m_maxPacketSize(0)
	, m_idealPacketSize(0)
	, m_sparePacketSize(0)
	, m_idle(false)
	, m_inUse(false)
	{
		memset(m_name, 0, sizeof(m_name));
	}

	SMessageQueueStats m_messageQueue;

	char m_name[STATS_MAX_NAME_SIZE];
	uint32 m_ping;
	uint32 m_pingSmoothed;
	float m_bandwidthInbound;
	float m_bandwidthOutbound;
	uint32 m_bandwidthShares;
	uint32 m_desiredPacketRate;
	float m_currentPacketRate;
	float m_packetLossRate;
	uint32 m_maxPacketSize;
	uint32 m_idealPacketSize;
	uint32 m_sparePacketSize;
	bool m_idle;
	bool m_inUse;
};

struct SBandwidthStatsSubset
{
	SBandwidthStatsSubset()
	: m_totalBandwidthSent(0),
		m_lobbyBandwidthSent(0),
		m_seqBandwidthSent(0),
		m_fragmentBandwidthSent(0),
		m_totalBandwidthRecvd(0),
		m_totalPacketsSent(0),
		m_lobbyPacketsSent(0),
		m_seqPacketsSent(0),
		m_fragmentPacketsSent(0),
		m_totalPacketsRecvd(0)
	{
	}

	uint64	m_totalBandwidthSent;
	uint64	m_lobbyBandwidthSent;
	uint64	m_seqBandwidthSent;
	uint64	m_fragmentBandwidthSent;
	uint64	m_totalBandwidthRecvd;
	int			m_totalPacketsSent;
	int			m_lobbyPacketsSent;
	int			m_seqPacketsSent;
	int			m_fragmentPacketsSent;
	int			m_totalPacketsRecvd;
};

struct SBandwidthStats 
{
	SBandwidthStats()
	: m_total(), m_prev(), m_1secAvg(), m_10secAvg()
	{
	}

	SBandwidthStatsSubset TickDelta()
	{
		SBandwidthStatsSubset ret;
		ret.m_totalBandwidthSent = m_total.m_totalBandwidthSent - m_prev.m_totalBandwidthSent;
		ret.m_lobbyBandwidthSent = m_total.m_lobbyBandwidthSent - m_prev.m_lobbyBandwidthSent;
		ret.m_seqBandwidthSent = m_total.m_seqBandwidthSent - m_prev.m_seqBandwidthSent;
		ret.m_fragmentBandwidthSent = m_total.m_fragmentBandwidthSent - m_prev.m_fragmentBandwidthSent;
		ret.m_totalBandwidthRecvd = m_total.m_totalBandwidthRecvd - m_prev.m_totalBandwidthRecvd;
		ret.m_totalPacketsSent = m_total.m_totalPacketsSent - m_prev.m_totalPacketsSent;
		ret.m_lobbyPacketsSent = m_total.m_lobbyPacketsSent - m_prev.m_lobbyPacketsSent;
		ret.m_seqPacketsSent = m_total.m_seqPacketsSent - m_prev.m_seqPacketsSent;
		ret.m_fragmentPacketsSent = m_total.m_fragmentPacketsSent - m_prev.m_fragmentPacketsSent;
		ret.m_totalPacketsRecvd = m_total.m_totalPacketsRecvd - m_prev.m_totalPacketsRecvd;
		return ret;
	}
	
	SBandwidthStatsSubset		m_total;
	SBandwidthStatsSubset		m_prev;
	SBandwidthStatsSubset		m_1secAvg;
	SBandwidthStatsSubset		m_10secAvg;

	SNetChannelStats	m_channel[STATS_MAX_NUMBER_OF_CHANNELS];
	uint32						m_numChannels;
};

struct SInternetSimulatorStats
{
	SInternetSimulatorStats()
	: m_packetSends(0),
		m_packetDrops(0),
		m_lastPacketLag(0)
	{
	}
	uint32								m_packetSends;
	uint32								m_packetDrops;
	uint32								m_lastPacketLag;
};

struct SProfileInfoStat
{
	SProfileInfoStat()
	: m_name(""),
		m_totalBits(0),
		m_calls(0),
		m_rmi(false)
	{
	}

	string						m_name;
	uint32						m_totalBits;
	uint32						m_calls;
	bool							m_rmi;
};

typedef DynArray<SProfileInfoStat> ProfileLeafList;

struct SNetworkProfilingStats
{
	SNetworkProfilingStats()
	: m_ProfileInfoStats(),
		m_InternetSimulatorStats(),
		m_numBoundObjects( 0 ),
		m_maxBoundObjects( 0 )
	{
	}

	ProfileLeafList							m_ProfileInfoStats;
	SInternetSimulatorStats			m_InternetSimulatorStats;
	uint												m_numBoundObjects;
	uint												m_maxBoundObjects;
};

typedef void* TCipher;

struct SNetworkPerformance
{
	uint64 m_nNetworkSync;
	float m_threadTime;
};

enum EListenerPriorityType
{
	ELPT_PreEngine	=	0x00000000,
	ELPT_Engine			= 0x00010000,
	ELPT_PostEngine	=	0x00020000,
};

/////////////////////////////////////////////////////////////////////////////

// Description:
//    Main access point for creating Network objects.
UNIQUE_IFACE struct INetwork
{
	enum ENetwork_Multithreading_Mode
	{
		NETWORK_MT_OFF = 0,
		NETWORK_MT_PRIORITY_NORMAL,
		NETWORK_MT_PRIORITY_HIGH,
	};

	enum ENetContextCreationFlags
	{
		eNCCF_Multiplayer = BIT(0)
	};

	virtual ~INetwork(){}

	virtual bool IsPbInstalled() = 0;

	virtual void SetNetGameInfo( SNetGameInfo ) = 0;
	virtual SNetGameInfo GetNetGameInfo() = 0;

	// Description:
	//    Retrieves RCON system interface.
	virtual IRemoteControlSystem* GetRemoteControlSystemSingleton() = 0;

	// Description:
	//    Retrieves HTTP server interface.
	virtual ISimpleHttpServer* GetSimpleHttpServerSingleton() = 0;

	// Description:
	//    Disconnects everything before fast shutdown.
	virtual void FastShutdown() = 0;

	// Description:
	//    Enables/disables multi threading.
	virtual void SetMultithreadingMode( ENetwork_Multithreading_Mode threadMode ) = 0;

	// Description:
	//    Initializes some optional service.
	virtual INetworkServicePtr GetService( const char * name ) = 0;

	// Description:
	//    Allocates a nub for communication with another computer.
	// See Also: 
	//	 INetNub, IGameNub
	// Arguments:
	//    address		- specify an address for the nub to open, or NULL.
	//    pGameNub		- the game half of this nub, must be non-null (will be released by the INetNub).
	//    pSecurity		- security callback interface (for banning/unbanning ip's, etc, can be null).
	//    pGameQuery	- interface for querying information about the nub (for server snooping, etc, can be null).
	// Return:
	//    Pointer to the new INetNub, or NULL on failure.
	virtual INetNub * CreateNub( const char * address, IGameNub * pGameNub, IGameSecurity * pSecurity, IGameQuery * pGameQuery ) = 0;

	// Description:
	//    Queries the local network for games that are running.
	// Arguments:
	//    pGameQueryListener - the game half of the query listener, must be non-null (will be released by INetQueryListener).
	// Return:
	//    Pointer to the new INetQueryListener, or NULL on failure.
	virtual ILanQueryListener * CreateLanQueryListener( IGameQueryListener * pGameQueryListener ) = 0;

	// Description:
	//    Creates a server context with an associated game context.
	virtual INetContext * CreateNetContext( IGameContext * pGameContext, uint32 flags ) = 0;

	// Description:
	//    Releases the interface (and delete the object that implements it).
  virtual void Release() = 0;
	// Description:
	//    Gathers memory statistics for the network module.
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;

	// Description:
	//    Gets the socket level bandwidth statistics 
	virtual void GetBandwidthStatistics(SBandwidthStats* const pStats) = 0;

	// Description:
	//    Gathers performance statistics for the network module.
	virtual void GetPerformanceStatistics(SNetworkPerformance *pSizer) = 0;

	// Description:
	//    Gets debug and profiling statistics from network members
	virtual void GetProfilingStatistics(SNetworkProfilingStats* const pStats) = 0;

	// Description:
	//    Updates all nubs and contexts.
	// Arguments:
	//    blocking - time to block for network input (zero to not block).
	virtual void SyncWithGame( ENetworkGameSync syncType ) = 0; 

	// Description:
	//    Gets the local host name.
	virtual const char * GetHostName() = 0;

  // Description:
  //    Sets CD key string for online validation.
  virtual void SetCDKey(const char*) = 0;
	virtual bool HasNetworkConnectivity() = 0;
	
	// Description:
	//     Sends a console command event to PunkBuster.
	virtual bool PbConsoleCommand(const char*, int length) = 0;				// EvenBalance - M. Quinn
	virtual void PbCaptureConsoleLog(const char* output, int length) = 0;	// EvenBalance - M. Quinn
	virtual void PbServerAutoComplete(const char*, int length) = 0;			// EvenBalance - M. Quinn
	virtual void PbClientAutoComplete(const char*, int length) = 0;			// EvenBalance - M. Quinn

	virtual bool IsPbSvEnabled() = 0;

	virtual void StartupPunkBuster(bool server) = 0;
	virtual void CleanupPunkBuster() = 0;

	virtual bool IsPbClEnabled() = 0;

	virtual	ICryLobby* GetLobby() = 0;

	virtual void NpCountReadBits(bool count) = 0;
	virtual bool NpGetChildFromCurrent(const char *name, SNetProfileStackEntry **entry, bool rmi) = 0;
	virtual void NpRegisterBeginCall(const char *name, SNetProfileStackEntry** entry, float budge, bool rmi) = 0;
	virtual void NpBeginFunction(SNetProfileStackEntry* entry, bool read) = 0;
	virtual void NpEndFunction() = 0;
	virtual bool NpIsInitialised() = 0;
	virtual SNetProfileStackEntry* NpGetNullProfile() = 0;

	/////////////////////////////////////////////////////////////////////////////
	// Rebroadcaster

	// IsRebroadcasterEnabled
	// Informs the caller if the rebroadcaster is enabled or not
	virtual bool IsRebroadcasterEnabled(void) const = 0;

	// AddRebroadcasterConnection
	// Adds a connection to the rebroadcaster mesh
	// pChannel		- pointer to the channel being added
	// channelID	- game side channel ID associated with pChannel (if known)
	virtual void AddRebroadcasterConnection(INetChannel* pChannel, TNetChannelID channelID) = 0;
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Host Migration
	virtual void EnableHostMigration(bool bEnabled) = 0;
	virtual bool IsHostMigrationEnabled(void) = 0;

	virtual void TerminateHostMigration(CrySessionHandle gh) = 0;
	virtual void AddHostMigrationEventListener(IHostMigrationEventListener* pListener, const char* pWho, EListenerPriorityType priority) = 0;
	virtual void RemoveHostMigrationEventListener(IHostMigrationEventListener* pListener) = 0;
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Expose Encryption to game
	// Block encryption
	virtual void EncryptBuffer(uint8* pOutput, const uint8* pInput, uint32 bufferLength, const uint8* pKey, uint32 keyLength) = 0;
	virtual void DecryptBuffer(uint8* pOutput, const uint8* pInput, uint32 bufferLength, const uint8* pKey, uint32 keyLength) = 0;

	// When doing block encryption on many buffers with the same key it is much more efficient to create a cipher with BeginCipher
	// then call the following functions as many times as needed before calling EndCipher to free the cipher.
	virtual void EncryptBuffer(TCipher cipher, uint8* pOutput, const uint8* pInput, uint32 bufferLength) = 0;
	virtual void DecryptBuffer(TCipher cipher, uint8* pOutput, const uint8* pInput, uint32 bufferLength) = 0;

	// Rijndael encrypts/decrypts in 16 byte blocks so the return value the number of bytes encrypted/decrypted will be less than bufferLength if bufferLength isn't a multiple of 16.
	virtual uint32 RijndaelEncryptBuffer(uint8* pOutput, const uint8* pInput, uint32 bufferLength, const uint8* pKey, uint32 keyLength) = 0;
	virtual uint32 RijndaelDecryptBuffer(uint8* pOutput, const uint8* pInput, uint32 bufferLength, const uint8* pKey, uint32 keyLength) = 0;

	// Streamed encryption
	virtual TCipher BeginCipher(const uint8* pKey, uint32 keyLength) = 0;
	virtual void Encrypt(TCipher cipher, uint8* pOutput, const uint8* pInput, uint32 bufferLength) = 0;
	virtual void Decrypt(TCipher cipher, uint8* pOutput, const uint8* pInput, uint32 bufferLength) = 0;
	virtual void EndCipher(TCipher cipher) = 0;

};

// Description:
//	 This interface is implemented by CryNetwork, and is used by 
//	 INetMessageSink::DefineProtocol to find all of the message sinks that should
//	 be attached to a channel.
// See also:
//	 INetMessageSink::DefineProtocol
struct IProtocolBuilder
{

	virtual ~IProtocolBuilder(){}
	virtual void AddMessageSink( INetMessageSink * pSink, const SNetProtocolDef& protocolSending, const SNetProtocolDef& protocolReceiving ) = 0;

};

// Description:
//	 This interface must be implemented by anyone who wants to receive CTP messages.
struct INetMessageSink
{

	virtual ~INetMessageSink(){}
	// Description:
	//	 Called on setting up an endpoint to figure out what kind of messages can be sent
	//	 and received.
	virtual void DefineProtocol( IProtocolBuilder * pBuilder ) = 0;
	virtual bool HasDef( const SNetMessageDef * pDef ) = 0;

};

#ifndef OLD_VOICE_SYSTEM_DEPRECATED

struct IVoiceGroup
{

	virtual void AddEntity( const EntityId id ) = 0;
	virtual void RemoveEntity( const EntityId id ) = 0;
	virtual void AddRef() = 0;
	virtual void Release() = 0;

};

// Description:
//    An IVoiceReader provides voice samples to be sent over network.
struct IVoiceDataReader
{

	virtual bool Update() = 0;
	virtual uint32 GetSampleCount() = 0;
	virtual int16* GetSamples() = 0;

};

// Description:
//	 An IVoiceContext provides interface for network voice data manipulation.
/*UNIQUE_IFACE*/					// Commented out until de-virtualiser can cope with #ifdef
struct IVoiceContext
{

	//Description:
	//    Sets voice data reader for entity, voice context requests it when it needs new data.
	virtual void SetVoiceDataReader(EntityId id,IVoiceDataReader*) = 0;

	// Description:
	//	  Retrieves voice packet from network system.
	// Note:
	//	  Called directly from the network thread to reduce voice transmission latency.
	virtual bool GetDataFor( EntityId id, uint32 numSamples, int16 * samples ) = 0;

	// Description:
	//	 Creates new voice group.
	virtual IVoiceGroup* CreateVoiceGroup() = 0;

	virtual void Mute(EntityId requestor, EntityId id, bool mute) = 0;
	virtual bool IsMuted(EntityId requestor, EntityId id) = 0;
	virtual void PauseDecodingFor(EntityId id, bool pause) = 0;

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual bool IsEnabled() = 0;

	virtual void GetMemoryStatistics(ICrySizer* pSizer) = 0;

	// Description:
	//	 Forces recreation of routing table (when players change voice group, for instance).
	virtual void InvalidateRoutingTable() = 0;

};
#endif

// Description:
//    An INetContext manages the list of objects synchronized over the network
//    ONLY to be implemented in CryNetwork.
UNIQUE_IFACE struct INetContext
{

	virtual ~INetContext(){}
	// Description:
	//    Releases this context.
	virtual void DeleteContext() = 0;
	
	// Description:
	//    Records this context as a demo file.
	virtual void ActivateDemoRecorder( const char * filename ) = 0;
	// Description:
	//    Returs demo recorder channel
  virtual INetChannel* GetDemoRecorderChannel() const = 0;
	//    Records this context as a demo file.
	virtual void ActivateDemoPlayback( const char * filename, INetChannel * pClient, INetChannel * pServer ) = 0;
	// Description:
	//		Are we playing back a demo session?
	virtual bool IsDemoPlayback() const = 0;
  // Description:
  //		Are we recording a demo session?
  virtual bool IsDemoRecording() const = 0;
	// Description:
	//    If we're recording, log an RMI call to a file.
	virtual void LogRMI( const char * function, ISerializable * pParams ) = 0;
	// Description:
	//    Logs a custom (C++ based) RMI call to the demo file.
	virtual void LogCppRMI( EntityId id, IRMICppLogger * pLogger ) = 0;

	// Description:
	//    Enables on a server to lower pass-through message latency.
	virtual void EnableBackgroundPassthrough( bool enable ) = 0;

	// Description:
	//    Calls just after construction to declare which aspect bits have which characteristics.
	// Arguments:
	//    aspectBit		- The bit we are changing (1,2,4,8,16,32,64 or 128).
	//    aspectFlags	- Some combination of eAF_* that describes how this parameter will change.
	//    basePriority	- Base priority for this aspect (TODO: may be removed).
	virtual void DeclareAspect( const char * name, NetworkAspectType aspectBit, uint8 aspectFlags ) = 0;
	// Description:
	//		Sets the channel mask for this aspect (aspectchannelmask & channelmask must be non 0 for send to occur)
	// Arguments:
	//		aspectBit	- The bit we are setting the mask for
	//		mask - The channel mask
	virtual void SetAspectChannelMask( NetworkAspectType aspectBit, ChannelMaskType mask) = 0;
	// Description:
	//		Returns the channel mask for this aspect
	// Arguments:
	//		aspectBit	- The bit we are setting the mask for
	virtual ChannelMaskType GetAspectChannelMask(NetworkAspectID aspectID) = 0;
	// Description:
	//    Modifies the profile of an aspect.
	// Arguments:
	//    aspectBit - The aspect we are changing.
	//    profile	- The new profile of the aspect.
	virtual void SetAspectProfile( EntityId id, NetworkAspectType aspectBit, uint8 profile ) = 0;
	// Description:
	//    Fetches the profile on an aspect; this is a very heavyweight method as it must completely flush the state of network queues before operating.
	virtual uint8 GetAspectProfile( EntityId id, NetworkAspectType aspectBit ) = 0;
	// Description:
	//    Binds an object to the network so it starts synchronizing its state.
	// Arguments:
	//    id			- A user supplied id for this object (probably the entity id :)).
	//    parentId	- A user supplied id for this object's parent (0 = no parent)
	//    aspectBits	- A bit mask specifying which aspects are enabled now.
	virtual void BindObject( EntityId id, EntityId parentId, NetworkAspectType aspectBits, bool bStatic ) = 0;
	// Description:
	//	The below can be used per object to prevent delegatable aspects from being delegated
	//
	virtual void SetDelegatableMask( EntityId id, NetworkAspectType delegateMask ) = 0;	
	virtual void SafelyUnbind( EntityId id ) = 0;
	// Description:
	//    Removes the binding of an object to the network.
	virtual bool IsBound( EntityId id ) = 0;
	// Description:
	//    Must be called ONCE in response to a message sent from a SendSpawnObject call on the server
	//    (from the client).
	virtual void SpawnedObject( EntityId id ) = 0;
	// Description:
	//    Determines if an object is bound or not.
	virtual bool UnbindObject( EntityId id ) = 0;
#if RESERVE_UNBOUND_ENTITIES
	// Description:
	//		Retrieve a reserved EntityId for previous known netID that had no entity bound to it
	// Arguments:
	//    partialNetID		- id portion of a known SNetObjectID, without the salt.
	virtual EntityId RemoveReservedUnboundEntityMapEntry(uint16 partialNetID) = 0;
#endif
	// Description:
	//    Enables/disables the synchronization of some aspects over the network.
	// Arguments:
	//    id			- The id of a *bound* object.
	//    aspectBits	- The aspects to enable/disable.
	//    enabled		- Are we enabling new aspects, or disabling old ones.
	virtual void EnableAspects( EntityId id, NetworkAspectType aspectBits, bool enabled ) = 0;
	// Description:
	//    Some aspects of an object have been changed - and those aspects should be
	//    updated shortly.
	// Note:
	//	 The only way to get eAF_UpdateOccasionally aspects updated.
	// Arguments:
	//    id - The id of the object changed.
	//    aspectBits - A bit field describing which aspects have been changed.
	virtual void ChangedAspects( EntityId id, NetworkAspectType aspectBits ) = 0;
	// Description:
	//    Passes authority for updating an object to some remote channel;
	//    this channel must have had SetServer() called on it at construction time.
	// Note:
	//    Only those aspects marked as eAF_Delegatable are passed on.
	// Arguments:
	//    id			- The id of a *bound* object to change authority for.
	//    pControlling	- The channel who will now control the object (or NULL if we wish to
	//                    take control).
	virtual void DelegateAuthority( EntityId id, INetChannel * pControlling ) = 0;

	// Description:
	//    Changes the game context. 
	// Note:
	//	  Destroy all objects, and cause all channels to load
	//    a new level, and reinitialize state.
	virtual bool ChangeContext() = 0;

	// Description:
	//    The level has finished loading 
	// Example:
	//	  The slow part of context establishment is complete.
	// Note:
	//    Call this after a call to IGameContext::EstablishContext.
	// See also:
	//	 IGameContext::EstablishContext
	virtual void EstablishedContext( int establishToken ) = 0;

	// Description:
	//    Removing an RMI listener -- make sure there's no pointers left to it.
	virtual void RemoveRMIListener( IRMIListener * pListener ) = 0;

	// Description:
	//    Determines if the context on the remote end of a channel has authority over an object.
	virtual bool RemoteContextHasAuthority( INetChannel * pChannel, EntityId id ) = 0;

	// Description:
	//    Specifies an objects 'network parent'.
	//    - Child objects are unspawned after the parent object is.
	//    - Child objects are spawned after the parent object is.
	virtual void SetParentObject( EntityId objId, EntityId parentId ) = 0;

	virtual void RequestRemoteUpdate( EntityId id, NetworkAspectType aspects ) = 0;

	// Description:
	//    add a break event to the log of breaks for this context
	virtual void LogBreak( const SNetBreakDescription& des ) = 0;

	// Description:
	//    set scheduling parameters for an object
	//    normal and owned are 4cc's from game/scripts/network/scheduler.xml
	virtual bool SetSchedulingParams( EntityId objId, uint32 normal, uint32 owned ) = 0;
	// Description:
	//    add a priority pulse to an object (the shape of the pulse is described in the scheduling group in game/scripts/network/scheduler.
	virtual void PulseObject( EntityId objId, uint32 pulseType ) = 0;

	virtual int RegisterPredictedSpawn( INetChannel * pChannel, EntityId id ) = 0;
	virtual void RegisterValidatedPredictedSpawn( INetChannel * pChannel, int predictionHandle, EntityId id ) = 0;

	virtual void GetMemoryStatistics(ICrySizer* pSizer) = 0;

	// Description:
	//    register a file that should be server controlled
	virtual void RegisterServerControlledFile( const char * filename ) = 0;
	// Description:
	//    get an ICryPak interface for server controlled files
	virtual ICryPak * GetServerControlledICryPak() = 0;
	// Description:
	//    Read XML out of a synced file
	virtual XmlNodeRef GetServerControlledXml( const char * filename ) = 0;

	// Description:
	//    convert EntityId to netId and vice versa
	virtual SNetObjectID GetNetID( EntityId userID, bool ensureNotUnbinding = true ) = 0;
	virtual EntityId GetEntityID(SNetObjectID netID) = 0;
	virtual void Resaltify( SNetObjectID& id ) = 0;

	// Description:
	//    Get a mask of all aspects declared with the eAF_ServerControllerOnly flag
	virtual NetworkAspectType ServerControllerOnlyAspects() const = 0;

	// Description:
	//    Get a mask of all aspects declared with the eAF_Delegatable flag
	virtual NetworkAspectType DelegatableAspects() const = 0;


	// Description:
	//    Updates the location of an object.
	// Note:
	//	  Allows correct priorities to be assigned to it.
#if FULL_ON_SCHEDULING
	virtual void ChangedTransform( EntityId id, const Vec3& pos, const Quat& rot, float drawDist ) = 0;
	virtual void ChangedFov( EntityId id, float fov ) = 0;
#endif

#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	virtual IVoiceContext* GetVoiceContext() = 0;
#endif
};

struct INetSender
{
	INetSender( TSerialize sr, uint32 nCurrentSeq, uint32 nBasisSeq, bool isServer ) : ser(sr)
	{
		this->nCurrentSeq = nCurrentSeq;
		this->nBasisSeq = nBasisSeq;
		this->isServer = isServer;
	}

	virtual ~INetSender(){}
	virtual void BeginMessage( const SNetMessageDef * pDef ) = 0;
	virtual void BeginUpdateMessage( SNetObjectID ) = 0;
	virtual void EndUpdateMessage() = 0;
	virtual uint32 GetStreamSize()=0;	

	TSerialize ser;
	bool isServer;
	uint32 nCurrentSeq;
	uint32 nBasisSeq;
};

struct INetBaseSendable
{
	INetBaseSendable() : m_cnt(0) {}

	virtual ~INetBaseSendable() {}

	virtual size_t GetSize() = 0;
	virtual EMessageSendResult Send( INetSender * pSender ) = 0;
	// Summary:
	//	 Callback for when we know what happened to a packet
	virtual void UpdateState( uint32 nFromSeq, ENetSendableStateUpdate update ) = 0;


	void AddRef()
	{
		CryInterlockedIncrement(&m_cnt);
	}
	void Release()
	{
		if (CryInterlockedDecrement(&m_cnt) <= 0)
			DeleteThis();
	}

private:
	volatile int m_cnt;

	virtual void DeleteThis()
	{
		delete this;
	}
};

static const int MAXIMUM_PULSES_PER_STATE = 6;
class CPriorityPulseState : public CMultiThreadRefCount
{
public:
	CPriorityPulseState() : m_count(0) {}

	struct SPulse
	{
		SPulse(uint32 k=0, CTimeValue t=0.0f) : key(k), tm(t) {}
		uint32 key;
		CTimeValue tm;
		bool operator<( const SPulse& rhs ) const
		{
			return key < rhs.key;
		}
		bool operator<( const uint32 _key ) const
		{
			return key < _key;
		}
	};

	void Pulse( uint32 key )
	{
		CTimeValue tm = gEnv->pTimer->GetAsyncTime();
		SPulse pulseKey(key);
		SPulse * pPulse = std::lower_bound(m_pulses, m_pulses+m_count, pulseKey);
		if (pPulse == m_pulses+m_count || key != pPulse->key)
		{
			if (m_count >= MAXIMUM_PULSES_PER_STATE)
			{
#ifdef _DEBUG
				CryLog("Pulse ignored due to pulse overpopulation; increase MAXIMUM_PULSES_PER_STATE in INetwork.h");
#endif
				return;
			}
			else
			{
				m_pulses[m_count++] = SPulse(key, tm);
				std::sort(m_pulses, m_pulses+m_count);
			}
		}
		else
		{
			pPulse->tm = tm;
		}
	}

	const SPulse * GetPulses() const { return m_pulses; }
	uint32 GetNumPulses() const { return m_count; }

private:
	uint32 m_count;
	SPulse m_pulses[MAXIMUM_PULSES_PER_STATE];
};
typedef _smart_ptr<CPriorityPulseState> CPriorityPulseStatePtr;

struct SMessagePositionInfo
{
	SMessagePositionInfo() : havePosition(false), haveDrawDistance(false) {}
	bool haveDrawDistance;
	bool havePosition;
	float drawDistance;
	Vec3 position;
	SNetObjectID obj;
};

#if ENABLE_PACKET_PREDICTION
struct SMessageTag
{
	uint64 varying1;		// Helps to uniquely identify a message which may have different serialisation format (multiple aspects.. different entity id's)
	uint32 messageId;
	uint32 varying2;
};
const bool operator < (const SMessageTag& left,const SMessageTag& right);

class IMessageMapper
{
public:

	virtual ~IMessageMapper() {};
	virtual uint32 GetMsgId(const SNetMessageDef*) const =0;

};
#endif

struct INetSendable : public INetBaseSendable
{
public:
	INetSendable( uint32 flags, ENetReliabilityType reliability ) : m_flags(flags), m_group(0), m_priorityDelta(0.0f), m_reliability(reliability) {}

	virtual const char * GetDescription() = 0;
	virtual void GetPositionInfo( SMessagePositionInfo& pos ) = 0;

#if ENABLE_RMI_BENCHMARK || ENABLE_URGENT_RMIS
	virtual IRMIMessageBody* GetRMIMessageBody() { return NULL; }
#endif

#if ENABLE_PACKET_PREDICTION
	virtual SMessageTag GetMessageTag(INetSender* pSender,IMessageMapper* mapper) = 0;
#endif

	ENetReliabilityType GetReliability() const { return m_reliability; }
	uint32 GetGroup() const { return m_group; }
	float GetPriorityDelta() const { return m_priorityDelta; }

	bool CheckParallelFlag( EMessageParallelFlags f )
	{
		return (m_flags & f) != 0;
	}

	void SetGroup( uint32 group ) { m_group = group; }
	void SetPriorityDelta( float prioDelta ) { m_priorityDelta = prioDelta; }

	// Note:
	//	 Should only be called by the network engine.
	void SetPulses( CPriorityPulseStatePtr pulses ) { m_pulses = pulses; }
	const CPriorityPulseState * GetPulses() { return m_pulses; }

	uint32 GetFlags() const { return m_flags; }

private:
	ENetReliabilityType m_reliability;
	uint32 m_group;
	uint32 m_flags;
	float m_priorityDelta;
	CPriorityPulseStatePtr m_pulses;
};

typedef _smart_ptr<INetSendable> INetSendablePtr;

class INetSendableHook : public INetBaseSendable 
{
#if RESERVE_UNBOUND_ENTITIES
public:
	INetSendableHook() : m_partialNetID(0) {}
	void SetPartialNetID(uint16 nid) { m_partialNetID = nid; }
protected:
	uint16 m_partialNetID;
#endif
};
typedef _smart_ptr<INetSendableHook> INetSendableHookPtr;

enum ESynchObjectResult
{
	eSOR_Ok,
	eSOR_Failed,
	eSOR_Skip,
};

// Description:
//    Interface for a channel to call in order to create/destroy objects, and when changing
//    context, to properly configure that context.
UNIQUE_IFACE struct IGameContext
{

	virtual ~IGameContext(){}
	// Description:
	//    Initializes global tasks that we need to perform to establish the game context.
	virtual bool InitGlobalEstablishmentTasks( IContextEstablisher * pEst, int establishedToken ) = 0;
	// Description:
	//    Initializes tasks that we need to perform on a channel to establish the game context.
	virtual bool InitChannelEstablishmentTasks( IContextEstablisher * pEst, INetChannel * pChannel, int establishedToken ) = 0;

	virtual INetSendableHookPtr CreateObjectSpawner( EntityId id, INetChannel * pChannel ) = 0;
	virtual void ObjectInitClient( EntityId id, INetChannel * pChannel ) = 0;
	virtual bool SendPostSpawnObject( EntityId id, INetChannel * pChannel ) = 0;

	// Description:
	//    We have control of an objects delegatable aspects (or not).
	virtual void ControlObject( EntityId id, bool bHaveControl ) = 0;

	// Description:
	//    Synchronizes a single aspect of an entity 
	// Note:
	//	  nAspect will have exactly one bit set
	//    describing which aspect to synch.
	virtual ESynchObjectResult SynchObject( EntityId id, NetworkAspectType nAspect, uint8 nCurrentProfile, TSerialize ser, bool verboseLogging ) = 0;
	// Description:
	//    Changes the current profile of an object .
	// Note:
	//	  Game code should ensure that things work out correctly.
	// Example:
	//	 Change physicalization.
	virtual bool SetAspectProfile( EntityId id, NetworkAspectType nAspect, uint8 nProfile ) = 0;
	// Description:
	//    An entity has been unbound (we may wish to destroy it).
	virtual void UnboundObject( EntityId id ) = 0;
	// Description:
	//    An entity has been bound (we may wish to do something with that info).
	virtual void BoundObject( EntityId id, NetworkAspectType nAspects ) = 0;
	// Description:
	//    Handles a remote method invocation.
	virtual INetAtSyncItem * HandleRMI( bool bClient, EntityId objID, uint8 funcID, TSerialize ser, INetChannel * pChannel ) = 0;
	// Description:
	//		Passes current demo playback mapped entity ID of the original demo recording server (local) player.
	virtual void PassDemoPlaybackMappedOriginalServerPlayer(EntityId id) = 0;
	// Description:
	//    Fetches the default (spawned) profile for an aspect.
	virtual uint8 GetDefaultProfileForAspect( EntityId id, NetworkAspectType aspectID ) = 0;

	virtual CTimeValue GetPhysicsTime() = 0;
	virtual void BeginUpdateObjects( CTimeValue physTime, INetChannel * pChannel ) = 0;
	virtual void EndUpdateObjects() = 0;

	virtual void OnEndNetworkFrame() = 0;
	virtual void OnStartNetworkFrame() = 0;

	virtual uint32 HashAspect( EntityId id, NetworkAspectType nAspect ) = 0;
	virtual void PlaybackBreakage( int breakId, INetBreakagePlaybackPtr pBreakage ) = 0;
	virtual void* ReceiveSimpleBreakage( TSerialize ser ) { return NULL; }
	virtual void PlaybackSimpleBreakage( void* userData, INetBreakageSimplePlaybackPtr pBreakage ) {}

	virtual string GetConnectionString(CryFixedStringT<HOST_MIGRATION_MAX_PLAYER_NAME_SIZE>* pNameOverride, bool fake) const = 0;

	virtual void CompleteUnbind( EntityId id ) = 0;

	virtual void GetMemoryStatistics(ICrySizer* pSizer) {};

};


static const int CREATE_CHANNEL_ERROR_SIZE = 256;
struct SCreateChannelResult
{
	explicit SCreateChannelResult(IGameChannel* ch):pChannel(ch){errorMsg[0] = 0;}
	explicit SCreateChannelResult(EDisconnectionCause dc):pChannel(0),cause(dc){errorMsg[0] = 0;}
	IGameChannel*				pChannel;
	EDisconnectionCause cause;
	char								errorMsg[CREATE_CHANNEL_ERROR_SIZE];
};

struct IGameNub
{

	virtual ~IGameNub(){}
	virtual void Release() = 0;
	// Description:
	//    Creates a new game channel to handle communication with a client or server.
	// Arguments:
	//    pChannel			- The INetChannel implementing the communications part of this channel; you
	//						  should call INetChannel::SetChannelType if you want client/server state
	//						  machine based context setup semantics.
	//    connectionString	- NULL if we're creating a channel in response to a call to ConnectTo
	//						  locally, otherwise it's the connectionString passed to ConnectTo remotely.
	// Return:
	//    Pointer to the new game channel; Release() will be called on it by the network
	//    engine when it's no longer required.
	virtual SCreateChannelResult CreateChannel( INetChannel * pChannel, const char * connectionString ) = 0;

	// Description:
	//		Notifies the GameNub that an active connection attempt failed (before a NetChannel is created).
	//		By implementing this interface function, the game can effectively capture pre-channel connection failures.
	// Note:
	//		The GameNub should be prepared to be destroyed shortly after this call is finished
	virtual void FailedActiveConnect( EDisconnectionCause cause, const char * description ) = 0;

};

UNIQUE_IFACE struct IGameChannel : public INetMessageSink
{

	virtual ~IGameChannel(){}
	// Description:
	//    Network engine will no longer use this object; it should be deleted.
	virtual void Release() = 0;
	// Description:
	//    The other side has disconnected from us; cleanup is occuring, and we'll soon be released.
	// Arguments:
	//    cause - Why the disconnection occurred.
	virtual void OnDisconnect( EDisconnectionCause cause, const char * description ) = 0;

};

UNIQUE_IFACE struct INetNub
{
	struct SStatistics
	{
		SStatistics() : bandwidthUp(0), bandwidthDown(0) {}
		float bandwidthUp;
		float bandwidthDown;
	};

	virtual ~INetNub(){}
	// Description:
	//    Game will no longer use this object; it should be deleted.
	virtual void DeleteNub() = 0;
	// Description:
	//    Connects to a remote host.
	// Arguments:
	//    address			- The address to connect to ("127.0.0.1:20319" for example).
	//    connectionString	- An arbitrary string to pass to the other end of this connection saying what we'd like to do.
	// Returns:
	//    True for success, false otherwise.
	virtual bool ConnectTo( const char * address, const char * connectionString ) = 0;
	// Description:
	//    Fetches current nub-wide statistics.
	virtual const INetNub::SStatistics& GetStatistics() = 0;
  // Description:
  //   Returns true if we nub is connecting now.
  virtual bool IsConnecting() = 0;
  // Description:
  //  An authorization result comes from master server.
  virtual void DisconnectPlayer(EDisconnectionCause cause, EntityId plr_id, const char* reason) = 0;
  // Description:
  //  Collects memory usage info.
	virtual void GetMemoryStatistics(ICrySizer* pSizer) = 0;

	virtual int GetNumChannels() = 0;

	virtual bool HasPendingConnections() = 0;

};

#if ENABLE_RMI_BENCHMARK

#define RMI_BENCHMARK_MAX_RECORDS    256 // Integer of serialization width of SRMIBenchmarkParams::seq must be able to represent index into array of this size.
#define RMI_BENCHMARK_INVALID_ENTITY 0xFFFFFFFF
#define RMI_BENCHMARK_INVALID_SEQ    0xFF

enum ERMIBenchmarkMessage
{
	eRMIBM_Ping,
	eRMIBM_Pong,
	eRMIBM_Pang,
	eRMIBM_Peng,
	eRMIBM_InvalidMessage
};

enum ERMIBenchmarkEnd
{
	eRMIBE_Client,
	eRMIBE_Server
};

enum ERMIBenchmarkThread
{
	eRMIBT_Game,
	eRMIBT_Network
};

enum ERMIBenchmarkAction
{
	eRMIBA_Queue,
	eRMIBA_Send,
	eRMIBA_Receive,
	eRMIBA_Handle
};

enum ERMIBenchmarkLogPoint
{
	eRMIBLP_ClientGameThreadQueuePing,
	eRMIBLP_ClientNetworkThreadSendPing,
	eRMIBLP_ServerNetworkThreadReceivePing,
	eRMIBLP_ServerGameThreadHandlePing,
	eRMIBLP_ServerGameThreadQueuePong,
	eRMIBLP_ServerNetworkThreadSendPong,
	eRMIBLP_ClientNetworkThreadReceivePong,
	eRMIBLP_ClientGameThreadHandlePong,
	eRMIBLP_ClientGameThreadQueuePang,
	eRMIBLP_ClientNetworkThreadSendPang,
	eRMIBLP_ServerNetworkThreadReceivePang,
	eRMIBLP_ServerGameThreadHandlePang,
	eRMIBLP_ServerGameThreadQueuePeng,
	eRMIBLP_ServerNetworkThreadSendPeng,
	eRMIBLP_ClientNetworkThreadReceivePeng,
	eRMIBLP_ClientGameThreadHandlePeng,
	eRMIBLP_Num
};

#define RMIBenchmarkGetEnd( point )     ( ( ERMIBenchmarkEnd )( ( ( ( point ) + 2 ) & 4 ) >> 2 ) )
#define RMIBenchmarkGetThread( point )  ( ( ERMIBenchmarkThread )( ( ( ( point ) + 1 ) & 2 ) >> 1 ) )
#define RMIBenchmarkGetAction( point )  ( ( ERMIBenchmarkAction )( ( point ) & 3 ) )
#define RMIBenchmarkGetMessage( point ) ( ( ERMIBenchmarkMessage )( ( point ) >> 2 ) )

struct SRMIBenchmarkParams
{
	SRMIBenchmarkParams()
	: message( eRMIBM_InvalidMessage ),
		entity( RMI_BENCHMARK_INVALID_ENTITY ),
		seq( RMI_BENCHMARK_INVALID_SEQ )
	{
	}

	SRMIBenchmarkParams( ERMIBenchmarkMessage msg, EntityId ent, uint8 seq, bool two )
	: message( msg ),
		entity( ent ),
		seq( seq ),
		twoRoundTrips( two )
	{
	}

	void SerializeWith( TSerialize ser )
	{
		ser.Value( "entity", entity, 'eid' );
		ser.Value( "message", message, 'ui2' );
		ser.Value( "seq", seq, 'ui8' );
		ser.Value( "twoRoundTrips", twoRoundTrips, 'bool' );
	}

	EntityId                entity;
	uint8                   message;
	uint8                   seq;
	bool                    twoRoundTrips;
};

#endif

struct INetChannel : public INetMessageSink
{
	// Note:
	//	 See CNetCVars - net_defaultChannel<xxx> for defaults
	struct SPerformanceMetrics
	{
#if NEW_BANDWIDTH_MANAGEMENT
#define INVALID_PERFORMANCE_METRIC (0)	
		SPerformanceMetrics()
			: m_bandwidthShares(INVALID_PERFORMANCE_METRIC)
			, m_packetRate(INVALID_PERFORMANCE_METRIC)
			, m_packetRateIdle(INVALID_PERFORMANCE_METRIC)
		{}

		uint32 m_bandwidthShares;
		uint32 m_packetRate;
		uint32 m_packetRateIdle;
#else
		SPerformanceMetrics() :
			pBitRateDesired(NULL),
			pBitRateToleranceHigh(NULL),
			pBitRateToleranceLow(NULL),
			pPacketRateDesired(NULL),
			pIdlePacketRateDesired(NULL),
			pPacketRateToleranceHigh(NULL),
			pPacketRateToleranceLow(NULL)
		{
		}
		// Desired bit rate (in bits-per-second).
		ICVar * pBitRateDesired;
		// Bit rate targets can climb to bitRateDesired * (1.0f + bitRateToleranceHigh).
		ICVar * pBitRateToleranceHigh;
		// Bit rate targets can fall to bitRateDesired * (1.0f - bitRateToleranceLow).
		ICVar * pBitRateToleranceLow;
		// Desired packet rate (in packets-per-second)
		ICVar * pPacketRateDesired;
		// Desired packet rate when nothing urgent needs to be sent (in packets-per-second).
		ICVar * pIdlePacketRateDesired;
		// Packet rate targets can climb to packetRateDesired * (1.0f + packetRateToleranceHigh).
		ICVar * pPacketRateToleranceHigh;
		// Packet rate targets can fall to packetRateDesired * (1.0f - packetRateToleranceLow).
		ICVar * pPacketRateToleranceLow;
#endif // NEW_BANDWIDTH_MANAGEMENT
	};

	struct SStatistics
	{
		SStatistics() : bandwidthUp(0), bandwidthDown(0) {}
		float bandwidthUp;
		float bandwidthDown;
	};

	virtual ChannelMaskType GetChannelMask() = 0;
	virtual void SetChannelMask(ChannelMaskType newMask) = 0;
	virtual void SetClient(INetContext* pNetContext, bool cheatProtection) = 0;
	virtual void SetServer(INetContext* pNetContext, bool cheatProtection) = 0;
	virtual void SetPeer(INetContext* pNetContext, bool cheatProtection) = 0;

	// Description:
	//    Sets/resets the server password.
	// Arguments:
	//    password - The new password string; will be checked at every context change if the length>0.
	virtual void SetPassword( const char * password ) = 0;

	// Description:
	//    Sets tolerances on packet delivery rate, bandwidth consumption, packet size, etc...
	// Arguments:
	//    pMetrics - An SPerformanceMetrics structure describing these tolerances.
	virtual void SetPerformanceMetrics( SPerformanceMetrics * pMetrics ) = 0;
	// Description:
	//    Disconnects this channel.
	virtual void Disconnect( EDisconnectionCause cause, const char * fmt, ... ) = 0;
	// Description:
	//    Sends a message to the other end of this channel.
	virtual void SendMsg( INetMessage * ) = 0;
	// Description:
	//    Like AddSendable, but removes an old message if it still exists.
	// See also:
	//	 AddSendable
	virtual bool SubstituteSendable( INetSendablePtr pMsg, int numAfterHandle, const SSendableHandle * afterHandle, SSendableHandle * handle ) = 0;
	// Description:
	//    Lower level, more advanced sending interface; enforces sending after afterHandle, and returns a handle to this message in handle.
	virtual bool AddSendable( INetSendablePtr pMsg, int numAfterHandle, const SSendableHandle * afterHandle, SSendableHandle * handle ) = 0;
	// Description:
	//    Undoes a sent message if possible.
	virtual bool RemoveSendable( SSendableHandle handle ) = 0;
	// Description:
	//    Gets current channel based statistics for this channel.
	virtual const SStatistics& GetStatistics() = 0;
	// Description:
	//    Gets the remote time.
	virtual CTimeValue GetRemoteTime() const = 0;
	// Description:
	//    Gets the current ping.
	virtual float GetPing( bool smoothed ) const = 0;
	// Description:
	//    Checks if the system is suffering high latency.
	virtual bool IsSufferingHighLatency(CTimeValue nTime) const = 0;
	// Description:
	//    Gets the time since data was last received on this channel.
	virtual CTimeValue GetTimeSinceRecv() const = 0;
	// Description:
	//    Dispatches a remote method invocation.
	virtual void DispatchRMI( IRMIMessageBodyPtr pBody ) = 0;
	// Description:
	//    Declares an entity that "witnesses" the world... allows prioritization.
	virtual void DeclareWitness( EntityId id ) = 0;
	// Description:
	//    Checks if this channel is connected locally.
	virtual bool IsLocal() const = 0;
	// Description:
	//    Checks if this connection has been successfully established.
	virtual bool IsConnectionEstablished() const = 0;
	// Description:
	//    Checks if this channel is a fake one. 
	// Example:
	//	 Demorecording, debug channel etc.
	// Note:
	//    ContextView extensions will not be created for fake channels.
	virtual bool IsFakeChannel() const = 0;
	// Description:
	//    Gets a descriptive string describing the channel.
	virtual const char*  GetNickname() = 0;
  // Description:
  //    Gets a descriptive string describing the channel.
  virtual const char * GetName() = 0;
	// Description:
	//    Sets a persistent nickname for this channel (MP playername).
	virtual void SetNickname(const char* name) = 0;
	// Description:
	//    Gets the local channel ID.
	virtual TNetChannelID GetLocalChannelID() = 0;
	// Description:
	//    Gets the remote channel ID.
	virtual TNetChannelID GetRemoteChannelID() = 0;

	virtual CrySessionHandle GetSession() const = 0;

	virtual IGameChannel * GetGameChannel() = 0;

	// is this channel currently transitioning between levels?
	virtual bool IsInTransition() = 0;

	virtual EContextViewState GetContextViewState() const = 0;
	virtual EChannelConnectionState GetChannelConnectionState() const = 0;
	virtual int GetContextViewStateDebugCode() const = 0;

	// has timing synchronization reached stabilization
	virtual bool IsTimeReady() const = 0;

	// Description:
	//    Gets the unique and persistent profile id for this client (profile id is associated with the user account).
	virtual int GetProfileId() const = 0;

	// Description:
	//    Checks if remote channel have pre-ordered copy.
	virtual bool IsPreordered() const = 0;

	virtual void GetMemoryStatistics(ICrySizer* pSizer, bool countingThis = false) = 0;

	// Description:
	//    add a wait for file sync complete marker to a context establisher
	virtual void AddWaitForFileSyncComplete( IContextEstablisher * pEst, EContextViewState when ) = 0;

	virtual void CallUpdate(CTimeValue time) {};
	virtual void CallUpdateIfNecessary(CTimeValue time, bool force) {};

	virtual void RequestUpdate(CTimeValue time) = 0;
	virtual bool HasGameRequestedUpdate() = 0;

	virtual void SetMigratingChannel(bool bIsMigrating) = 0;
	virtual bool IsMigratingChannel() const = 0;


#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	virtual CTimeValue TimeSinceVoiceTransmission() = 0;
	virtual CTimeValue TimeSinceVoiceReceipt( EntityId id ) = 0;
	virtual void AllowVoiceTransmission( bool allow ) = 0;
#endif

#if defined(DEDICATED_SERVER)
  virtual IDefenceContext * GetDefenceContext() = 0;
#endif

#if ENABLE_RMI_BENCHMARK
	virtual void LogRMIBenchmark( ERMIBenchmarkAction action, const SRMIBenchmarkParams& params, void ( *pCallback )( ERMIBenchmarkLogPoint point0, ERMIBenchmarkLogPoint point1, int64 delay, void* pUserData ), void* pUserData ) = 0;
#endif
};

#if ENABLE_RMI_BENCHMARK

template< typename T > inline void NetLogRMIReceived( const T& params, INetChannel* pChannel )
{
	// Do nothing.
}

template< typename T > inline const SRMIBenchmarkParams* NetGetRMIBenchmarkParams( const T& params )
{
	return NULL;
}

template<> inline void NetLogRMIReceived< SRMIBenchmarkParams >( const SRMIBenchmarkParams& params, INetChannel* pChannel )
{
	pChannel->LogRMIBenchmark( eRMIBA_Receive, params, NULL, NULL );
}

template<> inline const SRMIBenchmarkParams* NetGetRMIBenchmarkParams< SRMIBenchmarkParams >( const SRMIBenchmarkParams& params )
{
	return &params;
}

#else

#define NetLogRMIReceived( params, pChannel )

#endif

UNIQUE_IFACE struct IGameSecurity
{

	virtual ~IGameSecurity(){}
	// Description:
	//    Callback for making sure we're not communicating with a banned IP address.
	virtual bool IsIPBanned( uint32 ip ) = 0;
	// Description:
	//    Called when a cheater is detected.
	virtual void OnPunkDetected( const char * addr, EPunkType punkType ) = 0;

};

// Summary:
//	 This interface defines what goes into a CTP message.
class INetMessage : public INetSendable
{
public:
	INetMessage( const SNetMessageDef * pDef ) : INetSendable(pDef->parallelFlags, pDef->reliability), m_pDef(pDef) {}
	// Summary:
	//	 Gets the message definition - a static structure describing this message.
	inline const SNetMessageDef * GetDef() const { return m_pDef; }

	// Summary:
	//	 Writes the packets payload to a stream (possibly using the pSerialize helper).
	virtual EMessageSendResult WritePayload( TSerialize ser, uint32 nCurrentSeq, uint32 nBasisSeq ) = 0;

	virtual EMessageSendResult Send( INetSender * pSender ) { pSender->BeginMessage( m_pDef ); return WritePayload( pSender->ser, pSender->nCurrentSeq, pSender->nBasisSeq ); }
	virtual const char * GetDescription() { return m_pDef->description; }
	virtual ENetReliabilityType GetReliability() { return m_pDef->reliability; }

	virtual void GetPositionInfo( SMessagePositionInfo& pos ) {}


	// Summary:
	//	 Auto-implementation of INetSendable.
#if ENABLE_PACKET_PREDICTION
	virtual SMessageTag GetMessageTag(INetSender* pSender,IMessageMapper* mapper)
	{
		SMessageTag mTag;
		mTag.messageId=mapper->GetMsgId(m_pDef);
		mTag.varying1=0;// Assuming a 1-1 correspondence with data rates for now
		mTag.varying2=0;// Assuming a 1-1 correspondence with data rates for now

		return mTag;
	}
#endif

protected:
	void ResetMessageDef( const SNetMessageDef * pDef )
	{
		m_pDef = pDef;
	}

private:
	const SNetMessageDef * m_pDef;
};

struct INetAtSyncItem
{

	virtual ~INetAtSyncItem(){}
	virtual bool Sync() = 0;
	virtual bool SyncWithError(EDisconnectionCause &disconnectCause, string &disconnectMessage) = 0;
	virtual void DeleteThis() = 0;

};

struct IRMIListener
{

	virtual ~IRMIListener(){}
	virtual void OnSend( INetChannel * pChannel, int userId, uint32 nSeq ) = 0;
	virtual void OnAck( INetChannel * pChannel, int userId, uint32 nSeq, bool bAck ) = 0;

};

// Summary:
//	 Kludgy, record C++ RMI's for the demo recorder.
struct IRMICppLogger
{

	virtual ~IRMICppLogger(){}
	virtual const char * GetName() = 0;
	virtual void SerializeParams( TSerialize ser ) = 0;

};

// Summary:
//	 This class defines a remote procedure call message.
struct IRMIMessageBody
{
	IRMIMessageBody( 
		ENetReliabilityType reliability_, 
		ERMIAttachmentType attachment_, 
		EntityId objId_, 
		uint8 funcId_, 
		IRMIListener * pListener_, 
		int userId_,
		EntityId dependentId_ ) :
		m_cnt(0),
		reliability(reliability_), 
		attachment(attachment_), 
		objId(objId_),
		dependentId(dependentId_),
		funcId(funcId_),
		pMessageDef(0), 
		userId(userId_),
		pListener(pListener_)
	{
	}
	IRMIMessageBody( 
		ENetReliabilityType reliability_, 
		ERMIAttachmentType attachment_, 
		EntityId objId_, 
		const SNetMessageDef * pMessageDef_, 
		IRMIListener * pListener_, 
		int userId_,
		EntityId dependentId_ ) :
		m_cnt(0),
		reliability(reliability_), 
		attachment(attachment_), 
		objId(objId_),
		dependentId(dependentId_),
		funcId(0),
		pMessageDef(pMessageDef_), 
		userId(userId_),
		pListener(pListener_)
	{
	}

	virtual ~IRMIMessageBody() {}
	virtual void SerializeWith( TSerialize ser ) = 0;
	virtual size_t GetSize() = 0;

#if ENABLE_RMI_BENCHMARK
	virtual const SRMIBenchmarkParams* GetRMIBenchmarkParams() = 0;
#endif

	const ENetReliabilityType reliability;
	const ERMIAttachmentType attachment;
	const EntityId objId;
	const EntityId dependentId;
	const uint8 funcId;
	// Can optionally set this to send a defined message instead of a script style function.
	const SNetMessageDef * pMessageDef;
	// These two define a listening interface for really advance user stuff.
	const int userId;
	IRMIListener * pListener;

	void AddRef()
	{
		CryInterlockedIncrement(&m_cnt);
	}
	void Release()
	{
		if (CryInterlockedDecrement(&m_cnt) <= 0)
			DeleteThis();
	}

private:
	volatile int m_cnt;

	virtual void DeleteThis()
	{
		delete this;
	}
};

// Summary:
//	 This class provides a mechanism for the network library to obtain information
//	 about the game being played.
UNIQUE_IFACE struct IGameQuery
{

	virtual ~IGameQuery(){}
	virtual XmlNodeRef GetGameState() = 0;

};

// Summary:
//	 This interface should is implemented by CryNetwork and provides the information
//	 for an IGameQueryListener... releasing this will release the game query listener.
struct INetQueryListener
{

	virtual ~INetQueryListener(){}
	virtual void DeleteNetQueryListener() = 0;

};

struct SServerData
{
	int iNumPlayers;
	int iMaxPlayers;
	string strMap;
	string strMode;
	string strVersion;
};

// Summary:
//	 This interface should be implemented by the game to receive asynchronous game
//	 query results.
UNIQUE_IFACE struct IGameQueryListener
{

	virtual ~IGameQueryListener(){}
	// Summary:
	//	 Adds a server to intern list if not already existing, else just update data.
	virtual void AddServer(const char* description, const char* target, const char* additionalText, uint32 ping) = 0;
	// Summary:
	//	 Removes a server independently from last answer etc.
	virtual void RemoveServer(string address) = 0;
	// Summary:
	//	 Adds a received server pong.
	virtual void AddPong(string address, uint32 ping) = 0;\
	// Summary:
	//	 Returns a vector of running servers (as const char*).
	// Note:
	//	 Please delete the list (not the servers!) afterwards!
	virtual void GetCurrentServers(char*** pastrServers, int& o_amount) = 0;
	// Summary:
	//	 Returns a specific server by number (NULL if not available).
	virtual void GetServer(int number, char** server, char** data, int& ping) = 0;
	// Summary:
	//	 Returns the game server's data as a string and it's ping as an integer by reference.
	virtual const char* GetServerData(const char* server, int& o_ping) = 0;
	// Summary:
	//	 Refresh pings of all servers in the list.
	virtual void RefreshPings() = 0;
	virtual void OnReceiveGameState( const char *, XmlNodeRef ) = 0;
	virtual void Update() = 0;
	virtual void Release() = 0;
	// Summary:
	//	 Connects a network game to the specified server.
	virtual void ConnectToServer(const char* server) = 0;
	// Summary:
	//	 Retrieves infos from the data string.
	virtual void GetValuesFromData(char *strData,SServerData *pServerData) = 0;
	virtual void GetMemoryStatistics(ICrySizer* pSizer) = 0;

};

UNIQUE_IFACE struct ILanQueryListener : public INetQueryListener
{

	virtual ~ILanQueryListener(){}
	// Summary:
	//	 Sends a ping to the specified server.
	virtual void SendPingTo(const char * addr) = 0;
	// Summary:
	//	 Returns a pointer to the game query listener (game-side code of the listener).
	virtual IGameQueryListener* GetGameQueryListener() = 0;
	virtual void GetMemoryStatistics(ICrySizer * pSizer) = 0;

};

struct SContextEstablishState
{
	SContextEstablishState() 
		: contextState(eCVS_Initial)
		, pSender(NULL)
	{
	}
	EContextViewState contextState;
	INetChannel * pSender;
};

enum EContextEstablishTaskResult
{
	eCETR_Ok = 1,
	eCETR_Failed = 0,
	eCETR_Wait = 2
};

struct IContextEstablishTask
{

	virtual ~IContextEstablishTask(){}
	virtual void Release() = 0;
	virtual EContextEstablishTaskResult OnStep( SContextEstablishState& ) = 0;
	virtual void OnFailLoading( bool hasEntered ) = 0;
	virtual const char * GetName() = 0;

};

UNIQUE_IFACE struct IContextEstablisher
{

	virtual ~IContextEstablisher(){}
	virtual void GetMemoryStatistics(ICrySizer* pSizer) = 0;
	virtual void AddTask( EContextViewState state, IContextEstablishTask * pTask ) = 0;

};

////////////////////////////////////////////////////////////////////////////////////////
// other stuff


// exports;
extern "C"{
	CRYNETWORK_API INetwork *CreateNetwork(ISystem *pSystem, int ncpu);
	typedef INetwork *(*PFNCREATENETWORK)(ISystem *pSystem, int ncpu);
}

////////////////////////////////////////////////////////////////////////////////////////
// profiling

#if NEW_BANDWIDTH_MANAGEMENT
template <class T, int N>
class CCyclicStatsBuffer
{
public:
	CCyclicStatsBuffer() : m_count(0), m_total(T())
	{
		for (size_t index = 0; index < N; ++index)
		{
			m_values[index] = T();
		}
	}

	void Clear()
	{
		m_count = 0;
		m_index = N - 1;
	}

	void AddSample( T x )
	{
		m_index = (m_index + 1) % N;
		m_total += x - m_values[m_index];
		m_values[m_index] = x;
		if (++m_count > N)
		{
			m_count = N;
		}
	}

	bool Empty() const
	{
		return m_count == 0;
	}

	size_t Size() const
	{
		return m_count;
	}

	size_t Capacity() const
	{
		return N;
	}

	T GetTotal() const
	{
		return m_total;
	}

	float GetAverage() const
	{
		return static_cast<float>(m_total)/static_cast<float>(m_count);
	}

	T GetFirst() const
	{
		NET_ASSERT( !Empty() );
		return m_values[m_index];
	}

	T GetLast() const
	{
		NET_ASSERT( !Empty() );
		return m_values[(m_index-1)%N];
	}

	T operator[](size_t index) const
	{
		NET_ASSERT( !Empty() );
		return m_values[(m_index+index)%N];
	}

private:
	size_t m_count;
	size_t m_index;
	T m_values[N];
	T m_total;
};
#else
template <class T, int N>
class CCyclicStatsBuffer
{
public:
	CCyclicStatsBuffer() : m_count(0), m_haveMedian(false), m_haveTotal(false) {}

	void Clear()
	{
		m_count = 0;
		m_haveMedian = m_haveTotal = false;
	}

	void AddSample( T x )
	{
		m_values[m_count++ % N] = x;
		m_haveMedian = m_haveTotal = false;
	}

	bool Empty() const
	{
		return m_count == 0;
	}

	size_t Size() const
	{
		return MIN(m_count, N);
	}

	size_t Capacity() const
	{
		return N;
	}

	T GetTotal() const
	{
		if (!m_haveTotal)
		{
			T sum = 0;
			size_t last = Size();
			for (size_t i=0; i<last; i++)
			{
				sum += m_values[i];
			}
			m_total = sum;
			m_haveTotal = true;
		}
		return m_total;
	}

	T GetMedian() const
	{
		if (!m_haveMedian)
		{
			T temp[N];
			size_t last = Size();
			memcpy( temp, m_values, last * sizeof(T) );
			std::sort( temp, temp + last );
			m_median = temp[last/2];
			m_haveMedian = true;
		}
		return m_median;
	}

	float GetAverage() const
	{
		return float(GetTotal())/Size();
	}

	T GetFirst() const
	{
		NET_ASSERT( !Empty() );
		if (m_count > N)
		{
			return m_values[m_count%N];
		}
		else
		{
			return m_values[0];
		}
	}

	T GetLast() const
	{
		NET_ASSERT( !Empty() );
		if (m_count >= N)
		{
			return m_values[(m_count+N-1)%N];
		}
		else
		{
			return m_values[m_count-1];
		}
	}

private:
	size_t m_count;
	T m_values[N];

	mutable bool m_haveMedian;
	mutable bool m_haveTotal;
	mutable T m_median;
	mutable T m_total;
};
#endif // NEW_BANDWIDTH_MANAGEMENT

#if NET_PROFILE_ENABLE
	#define NET_PROFILE_NAME_LENGTH 128
	#define NO_BUDGET 0.f

	#define BITCOUNT_BOOL 1
	#define BITCOUNT_STRINGID 16
	#define BITCOUNT_TIME 80
	#define BITCOUNT_NETID 16

	#define NET_PROFILE_AVERAGE_SECS (5)

	struct SNetProfileCount
	{
		CCyclicStatsBuffer<uint32, NET_PROFILE_AVERAGE_SECS> m_callsAv;
		CCyclicStatsBuffer<uint32, NET_PROFILE_AVERAGE_SECS> m_seqbitsAv;
		CCyclicStatsBuffer<uint32, NET_PROFILE_AVERAGE_SECS> m_rmibitsAv;
		CCyclicStatsBuffer<float, NET_PROFILE_AVERAGE_SECS> m_payloadAv;
		CCyclicStatsBuffer<float, NET_PROFILE_AVERAGE_SECS> m_onWireAv;

		float	m_worst;
		float m_worstAv;
		float	m_heir;		
		float	m_self;

		void Clear()
		{
			m_callsAv.Clear();
			m_callsAv.AddSample(0);
			m_seqbitsAv.Clear();
			m_seqbitsAv.AddSample(0);
			m_rmibitsAv.Clear();
			m_rmibitsAv.AddSample(0);
			m_payloadAv.Clear();
			m_payloadAv.AddSample(.0f);
			m_onWireAv.Clear();
			m_onWireAv.AddSample(.0f);

			m_worst = 0.f;
			m_worstAv = 0.f;
			m_heir = 0.f;
			m_self = 0.f;
		}
	};

	struct SNetProfileStackEntry
	{
		CryFixedStringT<NET_PROFILE_NAME_LENGTH> m_name;

		uint32 m_tickBits;		// Number of estimated bits sent this frame (all clients included)
		uint32 m_tickCalls;		// Number of calls this frame (for seq bits, 1 call is counted per client)
		uint32 m_bits;				// Number of estimated bits sent over the last second (for seq bits it only counts them once, i.e. num clients doesn't affect the count. For rmis all clients are included)
		uint32 m_rmibits;			// Number of bits sent over the last second (all clients included)
		uint32 m_calls;				// Number of calls over the last second

		float m_budget;
		SNetProfileStackEntry *m_parent;
		SNetProfileStackEntry *m_next;		// next sibling
		SNetProfileStackEntry *m_child;	// all children
		SNetProfileCount counts;
		uint32 m_untouchedCounter;
		bool m_rmi;

		SNetProfileStackEntry()
		{
			ClearAll();
		}
		
		void ClearTickBits()
		{
			m_tickBits = 0;
			m_tickCalls = 0;
		}
	
		void ClearStatistics()
		{
			m_calls = 0;
			m_bits = 0;
			m_rmibits = 0;
			m_budget = 0.f;
			ClearTickBits();
		}

		void ClearAll()
		{
			m_name = "";
			m_parent = NULL;
			m_next = NULL;
			m_child = NULL;
			m_rmi = false;
			m_untouchedCounter = 0;
			counts.Clear();
			ClearStatistics();
		}
	};
	
	#define NET_PROFILE_JOIN_STRINGS(a,b)	a##_##b
	#define NET_PROFILE_NAME_CORE(a,b)		NET_PROFILE_JOIN_STRINGS(a,b)
	#define NET_PROFILE_NAME(a)				NET_PROFILE_NAME_CORE(a, __LINE__)

	#define NET_PROFILE_BEGIN_CORE(string, read, budget, rmi) \
	{ \
		INetwork *pNetwork = gEnv->pNetwork; \
		if(pNetwork && pNetwork->NpIsInitialised()) \
		{ \
			static SNetProfileStackEntry* NET_PROFILE_NAME(s_netProfileStackEntry) = pNetwork->NpGetNullProfile(); \
			if(!pNetwork->NpGetChildFromCurrent(string, &NET_PROFILE_NAME(s_netProfileStackEntry), rmi)) \
			{ \
				pNetwork->NpRegisterBeginCall(string, &NET_PROFILE_NAME(s_netProfileStackEntry), budget, rmi); \
			} \
			pNetwork->NpBeginFunction(NET_PROFILE_NAME(s_netProfileStackEntry), read); \
		} \
	}
	#define NET_PROFILE_COUNT_READ_BITS(count) \
	{ \
		INetwork *pNetwork = gEnv->pNetwork; \
		if(pNetwork && pNetwork->NpIsInitialised()) \
		{ \
			pNetwork->NpCountReadBits(count); \
		} \
	}

	#define NET_PROFILE_BEGIN(string, read) NET_PROFILE_BEGIN_CORE(string, read, NO_BUDGET, false)
	#define NET_PROFILE_BEGIN_BUDGET(string, read, budget) NET_PROFILE_BEGIN_CORE(string, read, budget, false)
	#define NET_PROFILE_BEGIN_RMI(string, read) NET_PROFILE_BEGIN_CORE(string, read, NO_BUDGET, true)
	#define NET_PROFILE_END() INetwork *_pNetwork = gEnv->pNetwork; if(_pNetwork && _pNetwork->NpIsInitialised()) { _pNetwork->NpEndFunction(); }

	class CNetProfileScope
	{
		public: inline ~CNetProfileScope() { NET_PROFILE_END(); }
	};

	#define NET_PROFILE_SCOPE_CORE(string, read, budget) NET_PROFILE_BEGIN_BUDGET(string, read, budget); CNetProfileScope NET_PROFILE_NAME(profileScope)
	#define NET_PROFILE_SCOPE(string, read) NET_PROFILE_SCOPE_CORE(string, read, NO_BUDGET)
	#define NET_PROFILE_SCOPE_RMI(string, read) NET_PROFILE_BEGIN_RMI(string, read); CNetProfileScope NET_PROFILE_NAME(profileScope)
	#define NET_PROFILE_SCOPE_BUDGET(string, read, budget) NET_PROFILE_SCOPE_CORE(string, read, budget)
#else
	#define NET_PROFILE_COUNT_READ_BITS(count)
	#define NET_PROFILE_BEGIN(string, read)
	#define NET_PROFILE_BEGIN_BUDGET(string, read, budget)
	#define NET_PROFILE_BEGIN_RMI(string, read)
	#define NET_PROFILE_END()
	#define NET_PROFILE_SCOPE(string, read)
	#define NET_PROFILE_SCOPE_RMI(string, read)
	#define NET_PROFILE_SCOPE_BUDGET(string, read, budget)
#endif

#endif //_INETWORK_H_
