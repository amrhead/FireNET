// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef INTERFACES_COMMON_DATA_H_INCLUDED
#define INTERFACES_COMMON_DATA_H_INCLUDED

#include "Cry_Math.h"
#include "BaseTypes.h"
#include "smartptr.h"

// To be removed once we provide full C++11 support!
#if !defined(NPTR)
#if __cplusplus > 199711L
#define NPTR nullptr
#else
#define NPTR NULL
#endif
#endif // !NPTR

// To be removed once we provide full C++11 support!
#if !defined(ATL_ENUM_TYPE)
#if __cplusplus > 199711L
#define ATL_ENUM_TYPE : TATLEnumFlagsType
#else
#define ATL_ENUM_TYPE
#endif
#endif // !NPTR

typedef uint32 TATLIDType;

typedef TATLIDType TAudioObjectID;
#define INVALID_AUDIO_OBJECT_ID ((TAudioObjectID)(0))
#define GLOBAL_AUDIO_OBJECT_ID ((TAudioObjectID)(1))
typedef TATLIDType TAudioControlID;
#define INVALID_AUDIO_CONTROL_ID ((TAudioControlID)(0))
typedef TATLIDType TAudioSwitchStateID;
#define INVALID_AUDIO_SWITCH_STATE_ID ((TAudioSwitchStateID)(0))
typedef TATLIDType TAudioEnvironmentID;
#define INVALID_AUDIO_ENVIRONMENT_ID ((TAudioEnvironmentID)(0))
typedef TATLIDType TAudioPreloadRequestID;
#define INVALID_AUDIO_PRELOAD_REQUEST_ID ((TAudioPreloadRequestID)(0))
typedef TATLIDType TAudioEventID;
#define INVALID_AUDIO_EVENT_ID ((TAudioEventID)(0))
typedef TATLIDType TAudioFileEntryID;
#define INVALID_AUDIO_FILE_ENTRY_ID ((TAudioFileEntryID)(0))
typedef TATLIDType TAudioTriggerImplID;
#define INVALID_AUDIO_TRIGGER_IMPL_ID ((TAudioTriggerImplID)(0))
typedef TATLIDType TAudioTriggerInstanceID;
#define INVALID_AUDIO_TRIGGER_INSTANCE_ID ((TAudioTriggerInstanceID)(0))
typedef TATLIDType TATLEnumFlagsType;
#define INVALID_AUDIO_ENUM_FLAG_TYPE ((TATLEnumFlagsType)(0))
#define ALL_AUDIO_REQUEST_SPECIFIC_TYPE_FLAGS ((TATLEnumFlagsType)(0xFFFFFFFF))
typedef TATLIDType TAudioProxyID;
#define INVALID_AUDIO_PROXY_ID ((TAudioProxyID)(0))
#define DEFAULT_AUDIO_PROXY_ID ((TAudioProxyID)(1))

struct SATLWorldPosition
{
	SATLWorldPosition()
		: mPosition(IDENTITY)
	{}

	SATLWorldPosition(Vec3 const& rPos)
		: mPosition(Matrix34(IDENTITY, rPos))
	{}

	SATLWorldPosition(Matrix34 const& rPos)
		: mPosition(rPos)
	{}

	ILINE Vec3	GetPositionVec() const {return mPosition.GetColumn3();}
	ILINE Vec3	GetUpVec() const {return mPosition.GetColumn2();}
	ILINE Vec3	GetForwardVec() const {return mPosition.GetColumn1();}

	//will normalize the forward vector (if it was not already unit length)
	ILINE void NormalizeForwardVec()
	{
		float lengthForwardVecSqr = mPosition.m01 * mPosition.m01 + mPosition.m11 * mPosition.m11 + mPosition.m21 * mPosition.m21;
		if (fabs_tpl(1.0f - lengthForwardVecSqr) > VEC_EPSILON)
		{
			float lengthInverted = isqrt_fast_tpl(lengthForwardVecSqr);
			mPosition.m01 *= lengthInverted;
			mPosition.m11 *= lengthInverted;
			mPosition.m21 *= lengthInverted;
		}
	}

	//will normalize the up vector (if it was not already unit length)
	ILINE void NormalizeUpVec()
	{
		float lengthForwardVecSqr = mPosition.m02 * mPosition.m02 + mPosition.m12 * mPosition.m12 + mPosition.m22 * mPosition.m22;
		if (fabs_tpl(1.0f - lengthForwardVecSqr) > VEC_EPSILON)
		{
			float lengthInverted = isqrt_fast_tpl(lengthForwardVecSqr);
			mPosition.m02 *= lengthInverted;
			mPosition.m12 *= lengthInverted;
			mPosition.m22 *= lengthInverted;
		}
	}

	Matrix34 mPosition;
};

#define AUDIO_TRIGGER_IMPL_ID_NUM_RESERVED 100// IDs below that value are used for the CATLTriggerImpl_Internal

#define	MAX_AUDIO_FILE_PATH_LENGTH 256
#define	MAX_AUDIO_FILE_NAME_LENGTH 128
#define	MAX_AUDIO_OBJECT_NAME_LENGTH 256

enum EAudioRequestFlags ATL_ENUM_TYPE
{
	eARF_NONE										= 0,
	eARF_PRIORITY_NORMAL				= BIT(0),
	eARF_PRIORITY_HIGH					= BIT(1),
	eARF_EXECUTE_BLOCKING				= BIT(2),
	eARF_SYNC_CALLBACK					= BIT(3),
	eARF_SYNC_FINISHED_CALLBACK	= BIT(4),
	eARF_STAY_IN_MEMORY					= BIT(5),
	eARF_THREAD_SAFE_PUSH				= BIT(6),
};

enum EAudioRequestType ATL_ENUM_TYPE
{
	eART_NONE														= 0,
	eART_AUDIO_MANAGER_REQUEST					= 1,
	eART_AUDIO_CALLBACK_MANAGER_REQUEST	= 2,
	eART_AUDIO_OBJECT_REQUEST						= 3,
	eART_AUDIO_LISTENER_REQUEST					= 4,
	eART_AUDIO_ALL_REQUESTS							= 0xFFFFFFFF,
};

enum EAudioRequestResult ATL_ENUM_TYPE
{
	eARR_NONE			= 0,
	eARR_SUCCESS	= 1,
	eARR_FAILURE	= 2,
};

struct SAudioRequestDataBase
{
	explicit SAudioRequestDataBase(EAudioRequestType const eType = eART_NONE)
		: eRequestType(eType)
	{}

	virtual ~SAudioRequestDataBase() {}

	EAudioRequestType const	eRequestType;
};

struct SAudioCallBackInfos
{
	SAudioCallBackInfos(SAudioCallBackInfos const& rOther)
		: pObjectToNotify(rOther.pObjectToNotify)
		, pUserData(rOther.pUserData)
		, pUserDataOwner(rOther.pUserDataOwner)
		, nRequestFlags(rOther.nRequestFlags)
	{}

	explicit SAudioCallBackInfos(
		void* const pPassedObjectToNotify = NPTR,
		void* const pPassedUserData = NPTR,
		void* const pPassedUserDataOwner = NPTR,
		TATLEnumFlagsType const nPassedRequestFlags = eARF_PRIORITY_NORMAL)
		: pObjectToNotify(pPassedObjectToNotify)
		, pUserData(pPassedUserData)
		, pUserDataOwner(pPassedUserDataOwner)
		, nRequestFlags(nPassedRequestFlags)
	{}

	static const SAudioCallBackInfos& GetEmptyObject() { static SAudioCallBackInfos emptyInstance; return emptyInstance; }

	void* const							pObjectToNotify;
	void* const							pUserData;
	void* const							pUserDataOwner;
	TATLEnumFlagsType const	nRequestFlags;
};

struct SAudioRequest
{
	SAudioRequest()
		: nFlags(eARF_NONE)
		, nAudioObjectID(INVALID_AUDIO_OBJECT_ID)
		, pOwner(NPTR)
		, pUserData(NPTR)
		, pUserDataOwner(NPTR)
		, pData(NPTR)
	{}

	~SAudioRequest() {}

	TATLEnumFlagsType				nFlags;
	TAudioObjectID					nAudioObjectID;
	void*										pOwner;
	void*										pUserData;
	void*										pUserDataOwner;
	SAudioRequestDataBase*	pData;

private:

	SAudioRequest(SAudioRequest const& other);
	SAudioRequest& operator=(SAudioRequest const& other);
};

struct SAudioRequestInfo
{
	explicit SAudioRequestInfo(EAudioRequestResult const ePassedResult, void* const	pPassedOwner, void* const pPassedUserData, void* const pPassedUserDataOwner, EAudioRequestType const ePassedAudioRequestType, TATLEnumFlagsType const nPassedSpecificAudioRequest, TAudioControlID const nPassedAudioControlID, TAudioObjectID const nPassedAudioObjectID)
		: eResult(ePassedResult)
		, pOwner(pPassedOwner)
		, pUserData(pPassedUserData)
		, pUserDataOwner(pPassedUserDataOwner)
		, eAudioRequestType(ePassedAudioRequestType)
		, nSpecificAudioRequest(nPassedSpecificAudioRequest)
		, nAudioControlID(nPassedAudioControlID)
		, nAudioObjectID(nPassedAudioObjectID)
	{}

	EAudioRequestResult const	eResult;
	void* const								pOwner;
	void* const								pUserData;
	void* const								pUserDataOwner;
	EAudioRequestType const		eAudioRequestType;
	TATLEnumFlagsType	const		nSpecificAudioRequest;
	TAudioControlID const			nAudioControlID;
	TAudioObjectID const			nAudioObjectID;
};

#endif // INTERFACES_COMMON_DATA_H_INCLUDED
