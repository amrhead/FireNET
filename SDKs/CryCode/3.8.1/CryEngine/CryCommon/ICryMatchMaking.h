#ifndef __ICRYMATCHMAKING_H__
#define __ICRYMATCHMAKING_H__
#pragma once

// Overview of ICryMatchMaking
// MatchMaking provides a way for gamers to find each other to play multi player game sessions.
// A gamer can host a session or search for a session to join.
// Custom game data can be added to a session to provide other gamers information about the game being played.
// When searching for games to join the results can be filtered so only games the gamer is interested in are returned.
//
// MatchMaking Setup
// Before MatchMaking can be used the games custom session data must be registered with a call to SessionRegisterUserData.
// The custom data is defined using an array of SCryLobbyUserData and can have a maximum of 64 elements.
// m_id should be a unique id used to identify the data item.
// In Live the session user data has to be defined with the xlast program and uploaded to Microsoft before matchmaking can be used.
// The xlast program will produce an h file with ID's for the user data and these ID's should be used.
// In NP due to limitations of the service, you should just use any UID (the matchmaking service will then place the values
// appropriately to fit) Key note : Only the first 8 int32 fields registered will be searchable/filterable.
// In CryLAN any UID can be used. It will happily use the same ID's defined for the underlying online service for each platform.
// The data types that can be used for custom session user data are
// eCLUDT_Int64, eCLUDT_Int32, eCLUDT_Int16, eCLUDT_Int8, eCLUDT_Float64, eCLUDT_Float32, eCLUDT_Int64NoEndianSwap
//
// MatchMaking on the host
// The host should start by calling SessionCreate specifying the data that describes the session.
// If the functions completes successfully this data will be returned to peers when they search for sessions.
// As well as the custom data, other session data that should be supplied is the number of public and private slots that are available for users to join.
// Private slots can only be joined by a users who are invited to the game while public slots can be joined by anybody.
// The session name should also be given.
// Some SDK's don't allow the session name to be set and will replace the name given, normally with the user name of the user who created the session.
// The final thing to set is if the match is ranked. In Live, ranked sessions do arbitration in the background.
// 
// During a game it is common for the game data to change and hence the data returned by searches needs to change to reflect this. e.g The game moves on to a different map.
// The host has 2 functions it can call to update the changeable data. These functions can be called anytime during the lifetime of the session.
// SessionUpdate can be called to update the sessions custom user data and takes an array of SCryLobbyUserData containing the data items to update.
// SessionUpdateSlots can be called to update the number of public and private slots available for users to join. e.g a private game may want to be made public.
// The new total number of slots should always be greater of equal to the current number of users in the session.
//
// When ready to start gameplay the host should call SessionStart and send a message to the other peers telling them to do the same.
// In ranked sessions after SessionStart has been called no more users are allowed to join the session. In unranked session users will still be allowed to join.
// When gameplay is finished SessionEnd should be called and a message should be sent to the other peers to tell them to do the same.
// If ICryStats is being used for leaderboards then writing to the leaderboards should be done between SessionStart and SessionEnd.
// In Live the leaderboard data written is sent to the Live servers when SessionEnd is called.
// If the game wishes to start gameplay again with the same session then SessionStart can be called again. e.g the same group of players moving on to another round.
//
// When the session is complete SessionDelete should be called this will free up all resources used by the session and stop the session being adverised
// so it will no longer be returned in searches.
//
// MatchMaking on the peers
// Before a peer can join a session it must find a CrySessionID of a session to join.
// There are 2 ways a CrySessionID can be obtained.
// A call to SessionSearch can be made that will return sessions that match the SCrySessionSearchParam structure passed in.
//
// The SCrySessionSearchParam structure passed in contains all the information needed to filter the results returned so only sessions
// the user is interested in are returned.
// In Live all the search types must also be defined in the xlast program and uploaded to Microsoft before they can be used.
// The h file the xlast program creates will contain ID's for each search type. m_type should be set to the ID of the search being done.
// Other systems can use any ID.
// An array of SCrySessionSearchData is also given that is used to filter sessions.
// An SCrySessionSearchData contains an SCryLobbyUserData that is compared against the equivalent item in the session using the ECrySessionSearchOperator given.
// Only sessions that return true for all the SCrySessionSearchData's given will be returned.
// Due to the limitations of PSN if PSN support is needed searches should only be defined using up to 8 int32 fields.
// m_numFreeSlots should be set to only return sessions with at least this many free public slots.
// m_ranked should be set to return ranked or unranked matches.
// m_maxNumReturn should be set to return at most this many sessions.
//
// The callback passed to SessionSearch will be called with the session information of each session found.
// If the error code is eCLE_SuccessContinue then a session is also provided and the callback will be called again.
// If the error code is eCLE_SuccessUnreachable then a session is also provided and the callback will be called again.
// This code indicates that a connection attempt to this session will fail most likely due to incompatible NATs.
// If the error code is eCLE_Success then the session will be NULL and the search is complete.
//
// The second way a CrySessionID can be obtained is by the user accepting an invite or joining a session in progress via the underlying SDKs UI.
// To get the CrySessionID when the user does this RegisterEventInterest should be called with eCLSE_InviteAccepted and a callback in ICryLobby during startup.
// When the callback is called the game should join the given CrySessionID.
//
// When a CrySessionID has been obtained call SessionJoin to try and join this session.
// The join could fail if the session has become unjoinable since the CrySessionID was obtained e.g Session has become full or the host has called SessionDelete.
// It can also fail due to incompatable NAT types.
//
// SessionStart should be called when gameplay starts. A message should be sent from the host to indicate when this is.
// SessionEnd should be called when gameplay ends. A message should be sent from the host to indicate when this is or if the user decides to leave.
//
// When the session is complete SessionDelete should be called this will free up all resources used by the session.
//

enum EDisconnectionCause
{
	// This cause must be first! - timeout occurred.
	eDC_Timeout = 0,
	// Incompatible protocols.
	eDC_ProtocolError,
	// Failed to resolve an address.
	eDC_ResolveFailed,
	// Versions mismatch.
	eDC_VersionMismatch,
	// Server is full.
	eDC_ServerFull,
	// User initiated kick.
	eDC_Kicked,
	// Teamkill ban/ admin ban.
	eDC_Banned,
	// Context database mismatch.
	eDC_ContextCorruption,
	// Password mismatch, cdkey bad, etc.
	eDC_AuthenticationFailed,
	// Misc. game error.
	eDC_GameError,
	// DX11 not found.
	eDC_NotDX11Capable,
	// The nub has been destroyed.
	eDC_NubDestroyed,
	// Icmp reported error.
	eDC_ICMPError,
	// NAT negotiation error.
	eDC_NatNegError,
	// Punk buster detected something bad.
	eDC_PunkDetected,
	// Demo playback finished.
	eDC_DemoPlaybackFinished,
	// Demo playback file not found.
	eDC_DemoPlaybackFileNotFound,
	// User decided to stop playing.
	eDC_UserRequested,
	// User should have controller connected.
	eDC_NoController,
	// Unable to connect to server.
	eDC_CantConnect,
	// Arbitration failed in a live arbitrated session.
	eDC_ArbitrationFailed,
	// Failed to successfully join migrated game
	eDC_FailedToMigrateToNewHost,
	// The session has just been deleted
	eDC_SessionDeleted,
	// Kicked due to having a high ping
	eDC_KickedHighPing,
	// Kicked due to reserved user joining
	eDC_KickedReservedUser,
	// Class registry mismatch
	eDC_ClassRegistryMismatch,
	// Global ban
	eDC_GloballyBanned,
	// Global ban stage 1 messaging
	eDC_Global_Ban1,
	// Global ban stage 2 messaging
	eDC_Global_Ban2,
	// This cause must be last! - unknown cause.
	eDC_Unknown
};

typedef uint32 CrySessionHandle;
const CrySessionHandle CrySessionInvalidHandle = 0xffffffff;
const uint64 CryMatchMakingInvalidSessionSID = 0;

struct SCryMatchMakingConnectionUID
{
	uint64	m_sid;
	uint32	m_uid;

	SCryMatchMakingConnectionUID()
	{
		m_sid = CryMatchMakingInvalidSessionSID;
		m_uid = 0;
	}

	bool operator == (const SCryMatchMakingConnectionUID &other) const
	{
		return m_sid == other.m_sid && m_uid == other.m_uid;
	}

	bool operator != (const SCryMatchMakingConnectionUID &other) const
	{
		return !(m_sid == other.m_sid && m_uid == other.m_uid);
	}

	// The below are purely for sorting purposes (they don't actually make sense from a uid point of view!)

	bool operator < (const SCryMatchMakingConnectionUID &other) const
	{
		if (m_sid < other.m_sid)
		{
			return true;
		}
		else
		{
			if ((m_sid == other.m_sid) && (m_uid < other.m_uid))
			{
				return true;
			}
		}

		return false;
	}

	int operator - (const SCryMatchMakingConnectionUID &other) const
	{
		if (*this < other)
			return -1;
		if (*this == other)
			return 0;
		return 1;
	}
};

const SCryMatchMakingConnectionUID CryMatchMakingInvalidConnectionUID;	// Default construction creates an invalid ID, this is just a convenient name.

#include <ICryLobby.h> // <> required for Interfuscator

class CCryLobbyPacket;

// Session create flags (system)
#define CRYSESSION_CREATE_FLAG_SEARCHABLE		0x00000001			// If set the session will be searchable and returned in session searches. If set uses XSESSION_CREATE_USES_MATCHMAKING on live if not set uses SCE_NP_MATCHING2_ROOM_FLAG_ATTR_HIDDEN on NP
#define CRYSESSION_CREATE_FLAG_INVITABLE		0x00000002			// If set friends can be invited to this session. In Live only 1 session can be invitable.
#define CRYSESSION_CREATE_FLAG_NUB					0x00000004			// Set if the session will be connected to a CNetNub

// Session create flags (game)
#define CRYSESSION_CREATE_FLAG_MIGRATABLE						0x00010000	// Set if the session is migratable to another host
#define CRYSESSION_CREATE_FLAG_CAN_SEND_SERVER_PING	0x00020000	// Session capable of sending server ping information
#define CRYSESSION_CREATE_FLAG_JOIN_BY_INVITE				0x00040000	// Set if we were invited to this session

#define CRYSESSION_CREATE_FLAG_SYSTEM_MASK	0x0000ffff			// Mask for the system flags (set when creating a session, not modifiable)
#define CRYSESSION_CREATE_FLAG_GAME_MASK		0xffff0000			// Mask for the game flags (can be modified after creating a session)
#define CRYSESSION_CREATE_FLAG_GAME_FLAGS_SHIFT (16)

// Local flags (system)
#define CRYSESSION_LOCAL_FLAG_USED													0x00000001	// Session is in use
#define CRYSESSION_LOCAL_FLAG_HOST													0x00000002	// This machine is the host of this session
#define CRYSESSION_LOCAL_FLAG_USER_DATA_EVENTS_STARTED			0x00000004
#define CRYSESSION_LOCAL_FLAG_CAN_SEND_HOST_HINTS						0x00000008	// Session capable of sending host hint information
#define CRYSESSION_LOCAL_FLAG_HOST_HINT_INFO_DIRTY					0x00000010	// Indicates the session needs to send host hint information
#define CRYSESSION_LOCAL_FLAG_HOST_HINT_EXTERNAL_INFO_DIRTY	0x00000020	// Indicates the session needs to send host hint information to the lobby service provider (if supported)
#define CRYSESSION_LOCAL_FLAG_STARTED												0x00000040	// Indicates this session has been started

// Local flags (game)
#define CRYSESSION_LOCAL_FLAG_HOST_MIGRATION_CAN_BE_HOST	0x00010000	// Initially cleared until the game decides this machine has enough information to be able to become host during a host migration event

#define CRYSESSION_LOCAL_FLAG_SYSTEM_MASK									0x0000ffff	// Mask for system controlled local session flags
#define CRYSESSION_LOCAL_FLAG_GAME_MASK										0xffff0000	// Mask for game controlled local session flags

#define MAX_MATCHMAKING_SESSIONS					4
#define MAX_SESSION_NAME_LENGTH						32

struct SCrySessionData
{
	SCrySessionData()
	{
		m_data = NULL;
		m_numData = 0;
		m_numPublicSlots = 0;
		m_numPrivateSlots = 0;
		memset(m_name, 0, MAX_SESSION_NAME_LENGTH);
		m_ranked = false;
	}

	SCryLobbyUserData*		m_data;
	uint32								m_numData;
	uint32								m_numPublicSlots;
	uint32								m_numPrivateSlots;
	char									m_name[MAX_SESSION_NAME_LENGTH];
	bool									m_ranked;
};

enum eHostMigrationState
{
	eHMS_Idle,
	eHMS_Initiate,
	eHMS_DisconnectClient,
	eHMS_WaitForNewServer,
	eHMS_DemoteToClient,
	eHMS_PromoteToServer,
	eHMS_ReconnectClient,
	eHMS_Finalise,
	eHMS_StateCheck,
	eHMS_Terminate,
	eHMS_Resetting,
	eHMS_Unknown,

	eHMS_NUM_STATES
};

enum eStatusCmdMode
{
	eSCM_AllSessions = 1,
	eSCM_LegacyRConCompatabilityConnectionsOnly,
	eSCM_LegacyRConCompatabilityNoNub
};

enum ECrySessionSearchOperator
{
	eCSSO_Equal,
	eCSSO_NotEqual,
	eCSSO_LessThan,
	eCSSO_LessThanEqual,
	eCSSO_GreaterThan,
	eCSSO_GreaterThanEqual,
	eCSSO_BitwiseAndNotEqualZero
};

// The value returned for a user datum whose ID maps to eCGSK_RegionKey will have exactly two of these bits
// set: a narrow region and its containing wide region. The m_region of a
// SCrySessionSearchParam can have any combination of these bits set: 0 will
// perform no region filtering, and any other value will cause a result to be
// omitted if the bitwise AND of its m_region with the search's m_region is 0.

enum EServerRegion
{
	eSR_Americas = BIT( 0 ),
	eSR_NorthAmerica = BIT( 1 ),
	eSR_Carribean = BIT( 2 ),
	eSR_CentralAmerica = BIT( 3 ),
	eSR_SouthAmerica = BIT( 4 ),

	eSR_Africa = BIT( 5 ),
	eSR_CentralAfrica = BIT( 6 ),
	eSR_EastAfrica = BIT( 7 ),
	eSR_NorthernAfrica = BIT( 8 ),
	eSR_SouthernAfrica = BIT( 9 ),
	eSR_WestAfrica = BIT( 10 ),

	eSR_Asia = BIT( 11 ),
	eSR_EastAsia = BIT( 12 ),
	eSR_Pacific = BIT( 13 ),
	eSR_SouthAsia = BIT( 14 ),
	eSR_SouthEastAsia = BIT( 15 ),

	eSR_Europe = BIT( 16 ),
	eSR_BalticStates = BIT( 17 ),
	eSR_CommonwealthOfIndependentStates = BIT( 18 ),
	eSR_EasternEurope = BIT( 19 ),
	eSR_MiddleEast = BIT( 20 ),
	eSR_SouthEastEurope = BIT( 21 ),
	eSR_WesternEurope = BIT( 22 ),

	eSR_All = BIT( 23 ) - 1
};

struct SCrySessionSearchData
{
	SCryLobbyUserData					m_data;
	ECrySessionSearchOperator	m_operator;
};

struct SCrySessionSearchParam
{
	uint32									m_type;
	SCrySessionSearchData*	m_data;
	uint32									m_numData;
	uint32									m_numFreeSlots;
	uint32									m_maxNumReturn;
	bool										m_ranked;
};

enum ECrySessionSearchResultFlag
{
	eCSSRF_RequirePassword = BIT( 0 )
};

struct SCrySessionSearchResult
{
	SCrySessionData				m_data;
	CrySessionID					m_id;
	uint32								m_numFilledSlots;
	uint32								m_numFriends;
	uint32								m_ping;							// In milliseconds
	uint32								m_flags;
};

#define CRYLOBBY_USER_DATA_SIZE_IN_BYTES	16

struct SCryUserInfoResult
{
	SCryMatchMakingConnectionUID	m_conID;
	CryUserID	m_userID;
	char			m_userName[CRYLOBBY_USER_NAME_LENGTH];
	uint8			m_userData[CRYLOBBY_USER_DATA_SIZE_IN_BYTES];
	bool			m_isDedicated;
};

struct SCryLobbySessionUserData
{
	CrySessionHandle						session;
	SCryUserInfoResult					data;
};

struct SCryLobbySessionEventData
{
	CrySessionHandle						session;
};

typedef void* CrySessionRequesterID;
const CrySessionRequesterID CrySessionInvalidRequesterID = NULL;

struct SCryLobbySessionRequestInfo
{
	CCryLobbyPacket*							pPacket;
	CrySessionHandle							session;
	CrySessionRequesterID					requester;
};

// CryMatchmakingCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingCallback)(CryLobbyTaskID taskID, ECryLobbyError error, void* arg);

// CryMatchmakingFlagsCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// h				- If successful a handle to the session that owns the flags
// flags		- The flags for the session
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingFlagsCallback)(CryLobbyTaskID taskID, ECryLobbyError error, CrySessionHandle h, uint32 flags, void* arg);

// CryMatchmakingSessionCreateCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// h				- If successful a handle to the session created used to identify the session in other matchmaking functions
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionCreateCallback)(CryLobbyTaskID taskID, ECryLobbyError error, CrySessionHandle h, void* arg);

// CryMatchmakingSessionSearchCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code
//						eCLE_SuccessContinue if session contains a valid result and search is continuing.
//						eCLE_SuccessUnreachable if the session contains a valid result but it has been found that a connection attempt will fail and the search is continuing.
//						eCLE_Success and session NULL if the search has finished and function succeeded or an error that occurred while processing the function
// session	- Pointer to a session search result if error was eCLE_SuccessContinue
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionSearchCallback)(CryLobbyTaskID taskID, ECryLobbyError error, SCrySessionSearchResult* session, void* arg);

// CryMatchmakingSessionQueryCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code
//						eCLE_SuccessContinue if session contains a valid result and search is continuing.
//						eCLE_Success and session NULL if the search has finished and function succeeded or an error that occurred while processing the function
// session	- Pointer to a session search result if error was eCLE_SuccessContinue
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionQueryCallback)(CryLobbyTaskID taskID, ECryLobbyError error, SCrySessionSearchResult* session, void* arg);

// CryMatchmakingSessionGetUsersCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code
//						eCLE_Success if session contains a valid result.
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionGetUsersCallback)(CryLobbyTaskID taskID, ECryLobbyError error, SCryUserInfoResult* session, void* arg);

// CryMatchmakingSessionJoinCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// h				- If successful a handle to the session created used to identify the session in other matchmaking functions
// ip, port	- If successful the address that can be used to communicate with the host.
//						NAT traversal will have been done by the underlying SDK so communication will always be possible with this address
//						but game connection could still fail if host quits before game connection is done.
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionJoinCallback)(CryLobbyTaskID taskID, ECryLobbyError error, CrySessionHandle h, uint32 ip, uint16 port, void* arg);

// CryMatchmakingSessionSetupDedicatedServerCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// ip, port	- If successful the address that can be used to connect to the dedicated server.
// pArg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionSetupDedicatedServerCallback)(CryLobbyTaskID taskID, ECryLobbyError error, uint32 ip, uint16 port, void* pArg);

// CryMatchmakingSessionSendRequestInfoCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// pPacket	-	If error is eCLE_Success the packet sent back from the host of the session. NULL otherwise.
// pArg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionSendRequestInfoCallback)(CryLobbyTaskID taskID, ECryLobbyError error, CCryLobbyPacket* pPacket, void* pArg);

// CryMatchmakingSessionGetAdvertisementDataCallback
// taskID		-	Task ID allocated when the function was executed
// error		- Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function
// pData		- Resulting data block
// dataSize - Size of data block
// arg			- Pointer to application-specified data
typedef void (*CryMatchmakingSessionGetAdvertisementDataCallback)(CryLobbyTaskID taskID, ECryLobbyError error, uint8* pData, uint32 dataSize, void* arg);

struct IHostMigrationEventListener;
struct CryUserID;
namespace Live
{
	namespace State 
	{
		struct INetworkingUser;
	} // } namespace State
} // } namespace Live
struct ICryMatchMaking
{
public:
	virtual ~ICryMatchMaking(){}
	// SessionRegisterUserData
	// This function must be called by all peers before any other matchmaking calls.
	// It defines the applications custom data used for it's sessions.
	// This data is advertised when a host creates a session and gets returned when a session search is performed
	// data				- Pointer to an array of SCryLobbyUserData that defines the applications custom data for a session
	// numData		- Number of items in the data array
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionRegisterUserData(SCryLobbyUserData* data, uint32 numData, CryLobbyTaskID* taskID, CryMatchmakingCallback cb, void* cbArg) = 0;

#ifdef DURANGO
	// SetIUser
	// Durango specific.  This is to feed the multiplayer system the user state interface which contains session information.
	// The matchmaking system only really needs the secure device address in order to function, and the async match flow is different enough
	// from the way the existing matchmaking works that the surface area required is about 80% game side.
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError		SetINetworkingUser( Live::State::INetworkingUser * user ) = 0;

#endif

	// SessionCreate
	// Called by the host to create a session and make it available for others to join.
	// users			- Pointer to an array of local users who will be joining the session after creation. The first user will be the session owner
	//							The numbers given should be the pad numbers the users are using.
	//							It is important that these are correct on live as multiple accounts can be signed in one for each pad.
	//							If the wrong pad numbers are passed in then the wrong accounts will be used for the session.
	//							Remember even on a single account machine and single player game the user doesn't have to be using and signed in on pad 0
	// numUsers		- Number of users in the users array
	// flags			- Session create flags that control how the session behaves
	// data				- Pointer to a SCrySessionData that describes the session
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionCreate(uint32* users, int numUsers, uint32 flags, SCrySessionData* data, CryLobbyTaskID* taskID, CryMatchmakingSessionCreateCallback cb, void* cbArg) = 0;
	
	// SessionMigrate
	// Called by the migrating host to ensure the lobby information is correct for the migrated session
	// h					- Handle of the current session we are migrating
	// users			- Pointer to an array of local users who will be joining the session after creation. The first user will be the session owner
	//							The numbers given should be the pad numbers the users are using.
	//							It is important that these are correct on live as multiple accounts can be signed in one for each pad.
	//							If the wrong pad numbers are passed in then the wrong accounts will be used for the session.
	//							Remember even on a single account machine and single player game the user doesn't have to be using and signed in on pad 0
	// numUsers		- Number of users in the users array
	// flags			- Session create flags that control how the session behaves
	// data				- Pointer to a SCrySessionData that describes the session
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionMigrate(CrySessionHandle h,uint32* pUsers, int numUsers, uint32 flags, SCrySessionData* pData, CryLobbyTaskID* pTaskID, CryMatchmakingSessionCreateCallback pCB, void* pCBArg) = 0;

	// SessionUpdate
	// Called by the host to update the applications custom data. Only the data items that need updating need to be given.
	// h					- Handle to the session to be updated
	// data				- Pointer to an array of SCryLobbyUserData that contains the applications custom data that will be updated for the session
	// numData		- Number of items in the data array
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionUpdate(CrySessionHandle h, SCryLobbyUserData* data, uint32 numData, CryLobbyTaskID* taskID, CryMatchmakingCallback cb, void* cbArg) = 0;

	// SessionUpdateSlots
	// Called by the host to update the number of public and private slots.
	// Any users currently in the session will be kept in the session if there are suitable slots for them.
	// h					- Handle to the session to be updated.
	// numPublic	- The number of public slots to change to.
	// numPrivate	- The number of private slots to change to.
	// pTaskID		- Pointer to buffer to store the task ID to identify this call in the callback.
	// pCB				- Callback function that is called when function completes.
	// pCBArg			- Pointer to application-specified data that is passed to the callback.
	// return			- eCLE_Success if function successfully started or an error code if function failed to start.
	virtual ECryLobbyError SessionUpdateSlots(CrySessionHandle h, uint32 numPublic, uint32 numPrivate, CryLobbyTaskID* pTaskID, CryMatchmakingCallback pCB, void* pCBArg) = 0;

	// SessionQuery
	// Called by any one to retrieve the current state of the session.
	// h					- Handle to the session to query
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionQuery(CrySessionHandle h, CryLobbyTaskID* taskID, CryMatchmakingSessionQueryCallback cb, void* cbArg) = 0;

	// SessionGetUsers
	// Called by any one to retrieve the list of users in the session.
	// h					- Handle to the session to get user information from
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionGetUsers(CrySessionHandle h, CryLobbyTaskID* taskID, CryMatchmakingSessionGetUsersCallback cb, void* cbArg) = 0;

	// SessionStart
	// Called by all peers when gameplay starts.
	// In Live ranked sessions the session becomes unjoinable between SessionStart and SessionEnd
	// h					- Handle to the session to start
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionStart(CrySessionHandle h, CryLobbyTaskID* taskID, CryMatchmakingCallback cb, void* cbArg) = 0;

	// SessionEnd
	// Called by all peers when gameplay ends.
	// h					- Handle to the session to end
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionEnd(CrySessionHandle h, CryLobbyTaskID* taskID, CryMatchmakingCallback cb, void* cbArg) = 0;

	// SetLocalUserData
	// Set the local user data for the given user. This data gets sent around the system and is returned for each user in a session with a call to SessionGetUsers.
	// h					- The handle of the session to set the user data for.
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// user				- The pad number of the user whose data to set.
	// pData			- The data you wish to set for the user.
	// dataSize		- The size of the data max CRYLOBBY_USER_DATA_SIZE_IN_BYTES
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	virtual ECryLobbyError SessionSetLocalUserData(CrySessionHandle h, CryLobbyTaskID* pTaskID, uint32 user, uint8* pData, uint32 dataSize, CryMatchmakingCallback pCB, void* pCBArg) = 0;

	// SessionDelete
	// Called by all members of the session to delete it when they have finished with it.
	// h					- Handle to the session to delete
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionDelete(CrySessionHandle h, CryLobbyTaskID* taskID, CryMatchmakingCallback cb, void* cbArg) = 0;

	// SessionSearch
	// Performs a search for sessions that match the specified search criteria.
	// user				- The pad number of the user doing the search.
	//							In Live the TrueSkill system is used. Each session has a skill value worked out from the members skill values.
	//							The skill value of the user doing the search is used to return sessions that have skill values closest to the users value.
	// param			- Pointer to a SCrySessionSearchParam that defines how sessions should be filtered
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionSearch(uint32 user, SCrySessionSearchParam* param, CryLobbyTaskID* taskID, CryMatchmakingSessionSearchCallback cb, void* cbArg) = 0;

	// SessionJoin
	// Called to join a session previously created by a session host. Session id's can be obtained from a session search or from a session invite notification.
	// users			-	Pointer to an array of pad numbers of the local users who will be joining the session
	// numUsers		- Number of users in the users array
	// flags			- Session create flags that control how the session behaves
	// id					-	A CrySessionID of the session that will be joined
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionJoin(uint32* users, int numUsers, uint32 flags, CrySessionID id, CryLobbyTaskID* taskID, CryMatchmakingSessionJoinCallback cb, void* cbArg) = 0;

	// SessionSetupDedicatedServer
	// When CryDedicatedServerArbitrator is being used the host calls this function to setup and get connection info for a dedicated server for their session.
	// h					- Handle to the session to setup a dedicated server for.
	// pPacket		- A packet that gets forwarded to the dedicated server. Contains game specific data that is used to setup the dedicated server so it is ready to accept connections.
	// pTaskID		- Pointer to buffer to store the task ID to identify this call in the callback
	// pCB				- Callback function that is called when function completes
	// pCBArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionSetupDedicatedServer(CrySessionHandle h, CCryLobbyPacket* pPacket, CryLobbyTaskID* pTaskID, CryMatchmakingSessionSetupDedicatedServerCallback pCB, void* pCBArg) = 0;

	// SessionReleaseDedicatedServer
	// Called by the host to release the dedicated server allocated to the given session by SessionSetupDedicatedServer
	// h					- Handle to the session to release the dedicated server for.
	// pTaskID		- Pointer to buffer to store the task ID to identify this call in the callback
	// pCB				- Callback function that is called when function completes
	// pCBArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionReleaseDedicatedServer(CrySessionHandle h, CryLobbyTaskID* pTaskID, CryMatchmakingCallback pCB, void* pCBArg) = 0;

	// SessionSendRequestInfo
	// When not connected to the session send a packet to the session host and wait for a response.
	// The session host needs to have registered an event interest for eCLSE_SessionRequestInfo
	// pPacket		- The packet to send. Contains game specific data to specify the data being requested
	// id					-	A CrySessionID of the session that the packet will be sent to
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual	ECryLobbyError SessionSendRequestInfo(CCryLobbyPacket* pPacket, CrySessionID id, CryLobbyTaskID* pTaskID, CryMatchmakingSessionSendRequestInfoCallback pCB, void* pCBArg) = 0;

	// SessionSendRequestInfoResponse
	// Send a packet back to the requester when an eCLSE_SessionRequestInfo event is received.
	// pPacket		- The packet to send. Contains the game specific data requested.
	// requester	- A CrySessionRequesterID of the requester to send the packet to.
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual	ECryLobbyError SessionSendRequestInfoResponse(CCryLobbyPacket* pPacket, CrySessionRequesterID requester) = 0;

	// SessionJoinFromConsole
	// Attempts to join the first searchable hosted session at the ip:port specified by cl_serveraddr
	virtual void SessionJoinFromConsole(void) = 0;

	// SessionEnsureBestHost
	// Uses the internal host hinting to dictate whether to migrate the session
	// Should really only be used in the pre-game lobby as the player list should be relatively stable during the game
	// gh					- game session handle
	// taskID			- Pointer to buffer to store the task ID to identify this call in the callback
	// cb					- Callback function that is called when function completes
	// cbArg			- Pointer to application-specified data that is passed to the callback
	// return			- eCLS_SUCCESS if best host determined
	virtual ECryLobbyError SessionEnsureBestHost(CrySessionHandle gh, CryLobbyTaskID* pTaskID, CryMatchmakingCallback pCB, void* pCBArg) = 0;

	// SessionTerminateHostHintingForGroup
	// This function should be called when a group of connections (including the host connection) are about to leave the
	// session as an atomic operation.  It will change the hints for the specified connections so that they're deemed
	// unsuitable as candidates for becoming the new host, synchronise the hints across the connections and then stop
	// sending hint updates for that session.  The specified connections can then leave the session and a new host will
	// be chosen from the remaining connections, and hinting will then resume.
	// gh							- game session handle
	// pConnections		- array of connections that are about to be disconnected
	// numConnections	- number of connections that are about to be disconnected
	// pTaskID				- Pointer to buffer to store the task ID to identify this call in the callback
	// pCB						- Callback function that is called when function completes
	// pCBArg					- Pointer to application-specified data that is passed to the callback
	// return					- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionTerminateHostHintingForGroup(CrySessionHandle gh, SCryMatchMakingConnectionUID* pConnections, uint32 numConnections, CryLobbyTaskID* pTaskID, CryMatchmakingCallback pCB, void* pCBArg) = 0;

	// SessionSetLocalFlags
	// Sets the specified local game flags for the specified session
	// gh							- game session handle
	// flags					- local game flags
	// pTaskID				- Pointer to buffer to store the task ID to identify this call in the callback
	// pCB						- Callback function that is called when function completes
	// pCBArg					- Pointer to application-specified data that is passed to the callback
	// return					- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionSetLocalFlags(CrySessionHandle gh, uint32 flags, CryLobbyTaskID* pTaskID, CryMatchmakingFlagsCallback pCB, void* pCBArg) = 0;

	// SessionClearLocalFlags
	// Clears the specified local game flags for the specified session
	// gh							- game session handle
	// flags					- local game flags
	// pTaskID				- Pointer to buffer to store the task ID to identify this call in the callback
	// pCB						- Callback function that is called when function completes
	// pCBArg					- Pointer to application-specified data that is passed to the callback
	// return					- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionClearLocalFlags(CrySessionHandle gh, uint32 flags, CryLobbyTaskID* pTaskID, CryMatchmakingFlagsCallback pCB, void* pCBArg) = 0;

	// SessionGetLocalFlags
	// Gets the local game flags for the specified session
	// gh							- game session handle
	// pTaskID				- Pointer to buffer to store the task ID to identify this call in the callback
	// pCB						- Callback function that is called when function completes
	// pCBArg					- Pointer to application-specified data that is passed to the callback
	// return					- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SessionGetLocalFlags(CrySessionHandle gh, CryLobbyTaskID* pTaskID, CryMatchmakingFlagsCallback pCB, void* pCBArg) = 0;

	// SessionSetAdvertisementData
	// Sets a data block that can be remotely retrieved without joining a session
	// gh							- Game session handle
	// pData					- Pointer to data block to set
	// dataSize				- Size of the data block
	// pTaskID				- Pointer to buffer to store the task ID to identify this call in the callback
	// pCb						- Callback function that is called when the function completes
	// pCbArg					-	Pointer to application-specified data that is passed to the callback
	// return					- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError	SessionSetAdvertisementData(CrySessionHandle gh, uint8* pData, uint32 dataSize, CryLobbyTaskID* pTaskID, CryMatchmakingCallback pCb, void* pCbArg) = 0;

	// SessionGetAdvertisementData
	// Gets a remote data block for session, without needing to join that session
	// sessionId			- SessionID for the remote session
	// pTaskID				- Pointer to buffer to store the task ID to identify this call in the callback
	// pCb						- Callback function that is called when the function completes
	// pCbArg					-	Pointer to application-specified data that is passed to the callback
	// return					- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError	SessionGetAdvertisementData(CrySessionID sessionId, CryLobbyTaskID* pTaskID, CryMatchmakingSessionGetAdvertisementDataCallback pCb, void* pCbArg) = 0;

	// GetSessionIDFromConsole
	// Returns the sessionID from the first searchable hosted session at the ip:port specified by cl_serveraddr
	// return - returns valid sessionId or CrySessionInvalidID if it fails to find a session
	virtual CrySessionID GetSessionIDFromConsole(void) = 0;

	// GetSessionAddressFromSessionID
	// Get the session address from a session ID.
	// sessionID			- session ID
	// hostIP					- output session IP address in host byte order
	// hostPort				- output session port in host byte order
	// return					- eCLE_Success if successful, otherwise an error
	virtual ECryLobbyError GetSessionAddressFromSessionID( CrySessionID sessionID, uint32& hostIP, uint16& hostPort ) = 0;

	// CancelTask
	// Cancel the given task. The task will still be running in the background but the callback will not be called when it finishes.
	// taskID			- The task to cancel
	virtual void CancelTask(CryLobbyTaskID taskID) = 0;

	// SendToServer
	// Send a packet from a client to the server of the given session.
	// pPacket		- The packet to send
	// h					- The handle of the session to send to the server of
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual	ECryLobbyError SendToServer(CCryLobbyPacket* pPacket, CrySessionHandle h) = 0;

	// SendToAllClients
	// Send a packet from the server to all the clients of the given session.
	// pPacket		- The packet to send
	// h					- The handle of the session to send to all the clients of
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual ECryLobbyError SendToAllClients(CCryLobbyPacket* pPacket, CrySessionHandle h) = 0;

	// SendToClient
	// Send a packet from the server to the given client of the given session
	// pPacket		- The packet to send
	// h					- The handle of the session to send to a client of
	// uid				- The connection uid of the client to send to
	// return			- eCLE_Success if function successfully started or an error code if function failed to start
	virtual	ECryLobbyError SendToClient(CCryLobbyPacket* pPacket, CrySessionHandle h, SCryMatchMakingConnectionUID uid) = 0;

	// SessionGetCrySessionIDFromCrySessionHandle
	// Get the CrySessionID from a CrySessionHandle.
	// h					- The handle of the session to get the CrySessionID of.
	// return			- The CrySessionID of the session.
	virtual CrySessionID SessionGetCrySessionIDFromCrySessionHandle(CrySessionHandle h) = 0;
	
	//GetSessionIDSizeInPacket
	//Returns the size of CrySessionID in a packet for specific platform
	//return			- The size of the platform specific CrySessionID
	virtual uint32 GetSessionIDSizeInPacket() const = 0;

	//WriteSessionIDToPacket
	//Adds the sessionId to a packet so it can be send across the network
	//sessionId		- SessionId you want to add to the packet
	//pPacket			- Packet you want to add the sessionId to
	//return			- eCLS_Success if function it writes it to the packet
	virtual ECryLobbyError WriteSessionIDToPacket(CrySessionID sessionId, CCryLobbyPacket* pPacket) const = 0;

	//ReadSessionIDFromPacket
	//Gets a platform specific sessionsId from a packet
	//pPacket			- packet you want to read the sessionID from
	//return			- CrySessionID that was inside the packet
	virtual CrySessionID ReadSessionIDFromPacket(CCryLobbyPacket* pPacket) const = 0;
	
	//GetConnectionUIDFromGameSessionHandleAndChannelID
	//Gets a matchmaking connection UID from the game session handle and the channel ID
	//gh					- game session handle
	//channelID		- channel ID
	virtual SCryMatchMakingConnectionUID	GetConnectionUIDFromGameSessionHandleAndChannelID(CrySessionHandle gh, uint16 channelID) = 0;

	// GetHostConnectionUID
	// Gets the matchmaking connection UID of the host of the given session.
	// gh					- game session handle
	// return			- SCryMatchMakingConnectionUID of the host
	virtual SCryMatchMakingConnectionUID	GetHostConnectionUID(CrySessionHandle gh) = 0;

	// TerminateHostMigration
	// Terminates host migration for the specified session
	virtual void TerminateHostMigration(CrySessionHandle gh) = 0;

	// GetSessionHostMigrationState
	// Returns the host migration state of the specified session
	virtual eHostMigrationState GetSessionHostMigrationState(CrySessionHandle gh) = 0;

	// GetSessionPlayerPing
	// Get the ping value for a player in a session
	// uid				 - session and player id
	// pPing			 - ping
	// return			 - eCLE_Success of successful, otherwise an error
	virtual ECryLobbyError GetSessionPlayerPing( SCryMatchMakingConnectionUID uid, CryPing* const pPing ) = 0;

	// GetSessionIDFromSessionURL
	// Get a CrySessionID from a session URL
	// pSessionURL - session URL
	// pSessionID	 - session ID
	// return			 - eCLE_Success if successful, otherwise an error
	virtual ECryLobbyError GetSessionIDFromSessionURL( const char* const pSessionURL, CrySessionID* const pSessionID ) = 0;

	// GetSessionIDFromIP
	// Get a CrySessionID from an ASCII IP address
	// pAddr			 - IP address
	// pSessionID	 - session ID
	// return			 - eCLE_Success if successful, otherwise an error
	virtual ECryLobbyError GetSessionIDFromIP( const char* const pAddr, CrySessionID* const pSessionID ) = 0;

	// GetTimeSincePacketFromServerMS
	// Returns the time since we last received a packet in milliseconds from the server for the given session handle
	// gh					- game session handle
	// return			- time since packet received in milliseconds
	virtual uint32 GetTimeSincePacketFromServerMS(CrySessionHandle gh) = 0;

	// DisconnectFromServer
	// Forces the server connection to disconnect
	// gh					- game session handle
	virtual void DisconnectFromServer(CrySessionHandle gh) = 0;

	// StatusCmd
	// Logs all connections for all sessions to the console (used by 'status' command)
	// mode				- see eStatusCmdMode for modes
	virtual void StatusCmd(eStatusCmdMode mode) = 0;

	// KickCmd
	// Kick console command
	// cxID - connection ID to kick (or CryLobbyInvalidConnectionID to ignore this param)
	// userID - e.g. GPProfile id to kick (or 0 to ignore this param)
	// pName - user name to ban (or NULL to ignore this param)
	// cause - reason for the kick
	virtual void KickCmd(uint32 cxID, uint64 userID, const char* pName, EDisconnectionCause cause) = 0;

	// Kick
	// pUserID - CryUserID* of the user to kick
	// cause - reason for the kick
	virtual void Kick(const CryUserID* pUserID, EDisconnectionCause cause) = 0;

	// BanCmd
	// ban console command
	// userID - e.g. GPProfile id to ban
	// timeout - ban timeout in minutes
	virtual void BanCmd(uint64 userID, float timeout) = 0;

	// BanCmd
	// ban console command
	// pUserName - user name to ban
	// timeout - ban timeout in minutes
	virtual void BanCmd(const char* pUserName, float timeout) = 0;

	// UnbanCmd
	// unban console command
	// userID - e.g. GPProfile id to unban
	virtual void UnbanCmd(uint64 userID) = 0;

	// UnbanCmd
	// unban console command
	// pUserName - user name to unban
	virtual void UnbanCmd(const char* pUserName) = 0;

	// BanStatusCmd
	// Logs all banned users to the console
	virtual void BanStatus(void) = 0;

	// Ban
	// pUserID - CryUserID* of the user to ban
	// timeout - ban timeout in minutes
	virtual void Ban(const CryUserID* pUserID, float timeout) = 0;

	// GetNameFromGameSession
	// Retreive the user name from a matchmaking connection
	// gh - Game session handle
	virtual const char* GetNameFromGameSessionHandle(CrySessionHandle gh) = 0;
	// </interfuscator:shuffle>
};

#endif // __ICRYMATCHMAKING_H__
